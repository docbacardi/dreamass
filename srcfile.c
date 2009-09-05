/***************************************************************************
 *   Copyright (C) 2002, 2003, 2004, 2005, 2006 by Christoph Thelen        *
 *   DocBacardi@the-dreams.de                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*-----------------------------------*/

#include "globals.h"

/*-----------------------------------*/
/* local                             */

int readFileLine(sourcefile_t *that);

typedef bool (readFunc)(sourcefile_t *that);

readFunc readMacroHead;
readFunc readMacroStart;
readFunc readMacroDef;
readFunc readRemMulti;
readFunc readText;

char *readString(sourcefile_t *that);
bool readMacroPar(sourcefile_t *that);

bool getline(sourcefile_t *that);
char *getMacroLine(sourcefile_t *that, linesize_t *lsize);
bool readMacroParameters(sourcefile_t *that, char ***buf);
bool readMacroValues(sourcefile_t *that, char ***buf);

bool addLine(sourcefile_t *that);

bool addNumber(linebuffer_t *lbuf, uint32_t num);
bool addOperand(linebuffer_t *lbuf, OPERAND op);
bool addPreproc(linebuffer_t *lbuf, PREPROC pp);
bool addPsyopc(linebuffer_t *lbuf, PSYOPC psyopc);
bool addOpcode(linebuffer_t *lbuf, OPCODE mne);
bool addString(linebuffer_t *lbuf, char *str, stringsize_t len);
bool addText(linebuffer_t *lbuf, char *txt, stringsize_t len);
bool addTextNum(linebuffer_t *lbuf, uint32_t txtnum, uint8_t size);

bool addLineelement(linebuffer_t *lbuf, lineelement_t *lelem);

char unescape(char ec);

void src_debug_line(sourcefile_t *that, linebuffer_t *lbuf, FILE *dfh, bool recurse);

char **macPar=NULL;
uint8_t macPar_count=0;
char **macVal=NULL;
uint8_t macVal_count=0;

extern uint16_t pass_cnt;

/*-----------------------------------*/

const lineelement_t le_eol = { LE_EOL, {0} };
const lineelement_t le_eof = { LE_EOF, {0} };

/*
 * This is a jumptable containing the function according to
 * the state of the readFile routine: readin_state
 * MUST BE ALIGNED TO THE ORDER OF 'enum READIN_STATE' !
 */
readFunc *readJmp[5] =
{
	readText,
	readRemMulti,
	readMacroHead,
	readMacroStart,
	readMacroDef,
};

/*-----------------------------------*/

sourcefile_t *newSourcefile(void)
{
	sourcefile_t *that;


	if( (that=(sourcefile_t*)(malloc(sizeof(sourcefile_t))))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}
	memset(that,0,sizeof(sourcefile_t));

	/*
	 * Macro-Fifo init
	 */
	that->macfifo_count = 0;
	that->macfifo_last = NULL;
	if( (that->macfifo=(mfifo_t*)(malloc((that->macfifo_buflen=16)*sizeof(mfifo_t))))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}

	that->linebuf_size = 0;
	if( (that->linebuf=(linebuffer_t*)(malloc((that->linebuf_alloc=16)*sizeof(linebuffer_t))))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}

	return that;
}


void delSourcefile(sourcefile_t *that)
{
	linebuffer_t *lbuf;
	linescnt_t lnum;
	linesize_t lpos;


	for(lnum=0; lnum<that->linebuf_size; lnum++)
	{
		lbuf = that->linebuf + lnum;
		for(lpos=0; lpos<lbuf->line_size; lpos++)
			freeLineElement( lbuf->line + lpos );
		free( lbuf->line );
	}
	free( that->linebuf );

	if( that->li!=NULL )
		free( that->li );

	if( that->plaintext_start!=NULL )
		free(that->plaintext_start);

	free( that->macfifo );

	free( that );
}


bool cmdlineSourcefile(sourcefile_t *that, char *buffer, size_t buffer_size)
{
	stringsize_t *filename;


	that->linenr = 0;

	/* add source structure to the list */
	filename = cstr2string("*** command line ***", 20);
	if( (that->fileidx=filelist_addFile(that, filename))==(filescnt_t)-1 ) {
		goto __error_exit;
	}
	free(filename);

	that->plaintext_start = that->plaintext_pos = buffer;
	that->plaintext_end = that->plaintext_start + buffer_size;

	that->readin_state = READIN_TEXT;
	that->lbuf.linenr = 0;
	that->lbuf.line_size = 0;
	that->lbuf.line_alloc = 0;
	that->lbuf.line = NULL;
	that->li = that->li_pos = that->li_end = NULL;

	/* read the first line of the sourcefile */
	if( readFileLine(that)==-1 ) {
		goto __error_exit;
	}

	return true;

__error_exit:
	if( that->plaintext_start!=NULL )
	{
		free(that->plaintext_start);
		that->plaintext_start = NULL;
	}

	return false;
}


bool readSourcefile(sourcefile_t *that, stringsize_t *filename, sourcefile_t *parent)
{
	struct stat finfo;
	int infile;
	char *cfilename;


	infile = -1;
	cfilename = NULL;
	that->linenr = 0;

	/* get char version of filename */
	if( (cfilename=string2cstr(filename))==NULL )
		goto __error_exit;

	/* add source structure to the list */
	if( (that->fileidx=filelist_addFile(that, filename))==(filescnt_t)-1 )
		goto __error_exit;

	/* open and stat file */
	if(
		(infile=filelist_ropen(filename))!=-1 &&
		fstat(infile, &finfo)==0
	  )
	{
		/*   alloc buffer for the plaintext file  */
		if( (that->plaintext_start=(char*)malloc(finfo.st_size))==NULL )
		{
			systemError(EM_OutOfMemory);
			goto __error_exit;
		}
		/*   set read and end counter  */
		that->plaintext_end = (that->plaintext_pos=that->plaintext_start)+finfo.st_size;
		/*   read complete file into mem  */
		if( readFile(infile, that->plaintext_start, finfo.st_size)==false ) {
			if( parent==NULL )
				systemError(EM_FileNotFound_s, cfilename);
			else
				scanError(EM_FileNotFound_s, parent->fileidx, parent->linenr, cfilename);
			goto __error_exit;
		}
		/*   close file  */
		close(infile);
		infile = -1;
	}
	else
	{
		if( parent==NULL )
			systemError(EM_FileNotFound_s, cfilename);
		else
			scanError(EM_FileNotFound_s, parent->fileidx, parent->linenr, cfilename);
		goto __error_exit;
	}

	that->readin_state = READIN_TEXT;
	that->lbuf.linenr = 0;
	that->lbuf.line_size = 0;
	that->lbuf.line_alloc = 0;
	that->lbuf.line = NULL;
/*  	that->lbuf.text_line = NULL;  */
	that->li = that->li_pos = that->li_end = NULL;

	/*   read the first line of the sourcefile  */
	if( readFileLine(that)==-1 ) {
		goto __error_exit;
	}

	if( cfilename!=NULL )
		free(cfilename);

	return true;

__error_exit:
	if( cfilename!=NULL )
		free(cfilename);
	if( infile!=-1 )
		close(infile);
	if( that->plaintext_start!=NULL )
	{
		free(that->plaintext_start);
		that->plaintext_start = NULL;
	}

	return false;
}


/*   return value: 1 = read line, ok  */
/*                 0 = no line left, ok  */
/*                -1 = error!  */
int readFileLine(sourcefile_t *that)
{
	bool lineReady;
	linebuffer_t* nlbuf;
	char c;
	macro_t *mac;


	lineReady = false;

	/*   loop until a complete block was parsed, e.g. multiline comment, so  */
	/*   that addLine can be called  */
	while( (that->macfifo_last!=NULL || that->plaintext_pos<that->plaintext_end) && !lineReady )
	{
		/*   is plaintext left in the linebuffer? (happens for lines with ':')  */
		if( that->li_pos>=that->li_end )
		{
			/*   nothing left in buffer  */

			/*   TODO: keep the line in the linebuffer. this way we'll see how macros expand :)  */
			/*   free old linebuffer  */
			if( that->li!=NULL )
			{
				free( that->li );
				that->li = NULL;
			}
			/*   get next line  */
			if( !getline(that) )
			{
				/*   error!  */
				return -1;
			}

			/*   remember new line number  */
			that->lbuf.linenr = that->linenr;
/*  			that->linebuf[that->linebuf_size].text_line = that->li;  */
		}

		if( cfg_debug )
		{
			if( that->macfifo_count==0 )
			{
				fprintf(debugLog, "<tt>L%04X</tt>: \"", that->linenr);
			}
			else
			{
				fprintf(debugLog, "<tt>M%04X</tt>(", that->macfifo_last->mline);
				mac = macro_get(that->macfifo_last->macroIdx);
				if( mac==NULL )
				{
					return -1;
				}
				printString(debugLog, mac->sname);
				fprintf(debugLog, "): \"");
			}
			fwrite(that->li_pos, that->li_end-that->li_pos, 1, debugLog);
			fprintf(debugLog, "\"<br>\n");
		}


		/*   parse the whole line until lineend, start of macro or ':'  */
		while( that->li_pos<that->li_end )
		{
			c=*that->li_pos;
			if( c=='\n' || c=='\r' )
			{
				/*   skip linefeed character  */
				++that->li_pos;
				/*   is the parser inside a block (that->readin_state!=READIN_TEXT) ?  */
				if( that->readin_state==READIN_TEXT && that->lbuf.line!=NULL )
				{
					/*   no -> the newline finishes the current block  */
					lineReady = true;
					break;
				}
			}
			else if( !readJmp[that->readin_state](that) )
			{
				/*   error!  */
				return -1;
			}
		}
	}

	/*   reached the end of the file?  */
	if( that->plaintext_pos>=that->plaintext_end )
	{
		/*   If the last line of the file did not end with a EOL there's still something in that->lbuf  */
		if( that->readin_state==READIN_TEXT && that->lbuf.line!=NULL )
		{
			lineReady = true;
		}

		/*   LBuf is resized by just doubling the old size. So it might be way too big. Resize to optimal (==used) length.  */
		if(
			that->linebuf_size!=0 &&
			that->linebuf_size<that->linebuf_alloc &&
			(nlbuf=(linebuffer_t*)realloc(that->linebuf,that->linebuf_size*sizeof(linebuffer_t)))!=NULL
		  )
		{
			that->linebuf=nlbuf;
			that->linebuf_alloc = that->linebuf_size;
		}
	}

	/*   Append lbuf to sourcebuffer  */
	if( lineReady )
	{
		if( cfg_debug )
		{
			/*   debug output  */
			src_debug_line(that, &that->lbuf, debugLog, false);
		}
		if( !addLine(that) )
		{
			/*   error!  */
			return -1;
		}
	}

	return lineReady ? 1 : 0;
}


bool addLine(sourcefile_t *that)
{
	linescnt_t nlen;
	lineelement_t *lelems;
	linebuffer_t* nlbuf;


	if(
		that->lbuf.line_size<that->lbuf.line_alloc &&
		(lelems=(lineelement_t*)realloc(that->lbuf.line,that->lbuf.line_size*sizeof(lineelement_t)))!=NULL
	  )
	{
		that->lbuf.line = lelems;
		that->lbuf.line_alloc = that->lbuf.line_size;
	}

	if( that->linebuf_size>=that->linebuf_alloc )
	{
		if( that->linebuf_alloc==(linescnt_t)-1 )
		{
			scanError(EM_TooManyLines, that->fileidx, that->linenr);
			return false;
		}

		if( (nlen=that->linebuf_alloc<<1)<that->linebuf_alloc )
			nlen=(linescnt_t)-1;
		if( (nlbuf=(linebuffer_t*)(realloc(that->linebuf,nlen*sizeof(linebuffer_t))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
		that->linebuf = nlbuf;
		that->linebuf_alloc = nlen;
	}

	*(that->linebuf + (that->linebuf_size++)) = that->lbuf;
	that->lbuf.line = NULL;

	return true;
}


/*
 * Read in Macro Name for definition
 * Call readMacroParameters to get the parameters
 */
bool readMacroHead(sourcefile_t *that)
{
	char *wo_start, *wo_pos;
	macro_t *mac;


	/*  Skip whitespace  */
	while( ++that->li_pos<that->li_end && isblank(*that->li_pos) );

	if( that->li_pos>=that->li_end )
	{
		scanError(EM_NoMacroName, that->fileidx, that->linenr);
		return false;
	}

	/*
	 * Read in the Macroname, which must begin with a letter or '_'
	 * Rest of the name can be letter, digit or '_'
	 */
	if( !isalpha(*that->li_pos) && *that->li_pos!='_' )
	{
		scanError(EM_ExpectingMacroName_c, that->fileidx, that->linenr, *that->li_pos);
		return false;
	}
	wo_start = that->li_pos;
	while( ++that->li_pos<that->li_end && (isalnum(*that->li_pos) || *that->li_pos=='_') );
	wo_pos = that->li_pos;

	if( (that->macroIdx=macro_add(wo_start, that->li_pos-wo_start))==(macro_cnt)-1 ) {
		return false;
	}
	that->mlines_buflen = 16;

	if( !readMacroParameters(that, &macPar) ) {
		return false;
	}
	mac = macro_get(that->macroIdx);
	mac->parameter_count = macPar_count;

	that->readin_state = READIN_MACROSTART;

	return true;
}


/*
 * Wait for Macro Start Char '{'. If it's followed by
 */
bool readMacroStart(sourcefile_t *that)
{
/*  	mline_data_t mld;  */


	/*  Skip whitespace  */
	while( that->li_pos<that->li_end && isblank(*that->li_pos) ) {
		++that->li_pos;
	}

	/*  Wait for opening '{'  */
	if( that->li_pos<that->li_end && *(that->li_pos++)=='{' ) {
		that->readin_state = READIN_MACRODEF;
	}
	else {
		scanError(EM_ExpectingCOBracket, that->fileidx, that->linenr);
		return false;
	}

	return readMacroDef(that);
}


bool readMacroDef(sourcefile_t *that)
{
	mline_data_t mld;
	char *cmp0,*cmp1,*cmp2;
	char *wo_start;
	size_t keylen;
	uint8_t cnt0;
	/*   rest is only for debug output  */
	mline_t *ml;
	linescnt_t debug_cnt0;
	linesize_t debug_cnt1;
	linesize_t len;
	lineelement_t lelem;
	macro_t *mac;


	wo_start = that->li_pos;
	while( that->li_pos<that->li_end )
	{
		/*   check for escape character  */
		if( *that->li_pos=='\\' && that->li_pos+1<that->li_end ) {
			that->li_pos+=2;
		}
		/*   check for a parameter (must be enclosed with '{}'  */
		else if( *that->li_pos=='{' )
		{
			/*   add text before parameter to the macro linebuffer  */
			if( (keylen=that->li_pos-wo_start)>0 )
			{
				mld.line = wo_start;
				if( (that->mlines_buflen=macro_addLine(that->macroIdx, mld, keylen, that->mlines_buflen))==(linescnt_t)-1 )
					return false;
			}
			/*
			 *Skip whitespace after the bracket
			 */
			while( ++that->li_pos<that->li_end && isblank(*that->li_pos) );

			/*
			 *Read in the Parametername
			 */
			if( !isalpha(*that->li_pos) && *that->li_pos!='_' )
			{
				scanError(EM_ExpectingParameterName_c, that->fileidx, that->linenr, *that->li_pos);
				return false;
			}

			wo_start = that->li_pos;
			while( ++that->li_pos<that->li_end && (isalnum(*that->li_pos) || *that->li_pos=='_') );
			keylen = that->li_pos-wo_start;

			cnt0=0;
			while( cnt0<macPar_count )
			{
				/*
				 * compare string with entries in parametertable.
				 * string is not 0-terminated therefore all the fuss
				 */
				cmp0=*(macPar+cnt0);
				cmp1=wo_start;
				cmp2=cmp0+keylen;
				while( cmp0<cmp2 && *cmp0==*cmp1 )
				{
					++cmp0;
					++cmp1;
				}
				if( *cmp0==0 )
					break;
				else
					++cnt0;
			};
			if( cnt0<macPar_count )
			{
				mld.pidx = cnt0;
				if( (that->mlines_buflen=macro_addLine(that->macroIdx, mld, 0, that->mlines_buflen))==(linescnt_t)-1 )
					return false;
			}
			else
			{
				*that->li_pos=0;
				scanError(EM_UnknownParameter_s, that->fileidx, that->linenr, wo_start);
				return false;
			}

			/*
			 *Skip whitespace
			 */
			while( that->li_pos<that->li_end && isblank(*that->li_pos) )
				++that->li_pos;

			if( that->li_pos>=that->li_end )
			{
				scanError(EM_ExpectingCCBracket, that->fileidx, that->linenr);
				return false;
			}
			if( *(that->li_pos++)!='}' )
			{
				scanError(EM_ExpectingCCBracket, that->fileidx, that->linenr);
				return false;
			}

			wo_start = that->li_pos;
		}
		else if( *that->li_pos=='}' ) {
			/*  Text before '}'  */
			if( (keylen=that->li_pos-wo_start)>0 )
			{
				mld.line = wo_start;
				if( (that->mlines_buflen=macro_addLine(that->macroIdx, mld, keylen, that->mlines_buflen))==(linescnt_t)-1 )
					return false;
			}

			wo_start = that->li_pos++;
			that->readin_state = READIN_TEXT;

			/*   finalize macro  */
			if( !macro_finalize(that->macroIdx, that->mlines_buflen) ) {
				return false;
			}

			/*   add macro lineelement  */
			lelem.typ = LE_MACRODEF;
			lelem.data.macroIdx = that->macroIdx;
			if( !addLineelement(&that->lbuf, &lelem) ) {
				return false;
			}

			if( macPar!=NULL ) {
				/*   free the parameter list  */
				while( macPar_count>0 ) {
					free( *(macPar+(--macPar_count)) );
				}
				free( macPar );
			}

			if( cfg_debug ) {
				fprintf(debugLog, "<p>Macro ");
				mac = macro_get(that->macroIdx);
				if( mac==NULL )
				{
					fprintf(debugLog, "NULL");
				}
				else
				{
					printString(debugLog, mac->sname);
					fprintf(debugLog, ":<br><tt>\n");
					for(debug_cnt0=0; debug_cnt0<mac->mlines_count; ++debug_cnt0)
					{
						ml = mac->mlines+debug_cnt0;
						fprintf(debugLog, "[%04d]", debug_cnt0);
						if( (len=ml->textLength)!=0 )
						{
							fprintf(debugLog, "'");
							for(debug_cnt1=0; debug_cnt1<len; ++debug_cnt1)
							{
								fprintf(debugLog, "%c", ml->data.line[debug_cnt1] );
							}
						}
						else
						{
							fprintf(debugLog, "Parameter %d", ml->data.pidx );
						}
						fprintf(debugLog, "<br>\n");
					}
					fprintf(debugLog, "</tt>\n");
				}
			}

			break;
		}
		else {
			that->li_pos++;
		}
	};
	/*  Ouput rest of Line  */
	if( that->readin_state==READIN_MACRODEF )
	{
/*  		*li_pos='\n';  */
		mld.line = wo_start;
		if( (that->mlines_buflen=macro_addLine(that->macroIdx, mld, that->li_pos-wo_start, that->mlines_buflen))==(linescnt_t)-1 )
			return false;
	}

	return true;
}


bool readMacroPar(sourcefile_t *that)
{
	macro_t *mac;
	size_t keylen;
	macfifocnt_t newlen;
	mfifo_t *newbuf;


	mac = macro_get(that->macroIdx);
	if( !readMacroValues(that, &macVal) )
		return false;

	if( macVal_count!=mac->parameter_count )
	{
		scanError(EM_MacroParameterNumber_d_d, that->fileidx, that->linenr, macVal_count, mac->mlines_count);
		return false;
	}

	if( cfg_debug )
	{
		fprintf(debugLog, "Found %d parameters: ", macVal_count);
		for(keylen=0; keylen<macVal_count; keylen++)
			fprintf(debugLog, "%s%s", macVal[keylen], (keylen+1<macVal_count)?", ":"" );
		fprintf(debugLog, "<br>\n");
	}

	/*
	 * Add Macro to fifo
	 */
	if( that->macfifo_count >= that->macfifo_buflen )
	{
		if( that->macfifo_buflen==((macfifocnt_t)-1) )
		{
			scanError(EM_MacroTooDeep, that->fileidx, that->linenr);
			return false;
		}

		if( (newlen=that->macfifo_buflen<<1)<that->macfifo_buflen )
			newlen=((macfifocnt_t)-1);
		if( (newbuf=(mfifo_t*)(realloc(that->macfifo, newlen*sizeof(mfifo_t))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
		that->macfifo = newbuf;
		that->macfifo_buflen = newlen;
	}
	(that->macfifo_last=that->macfifo+that->macfifo_count)->macroIdx = that->macroIdx;
	++that->macfifo_count;
	that->macfifo_last->mline=0;
	that->macfifo_last->par=macVal;

	/*   save old values to continue after the macro  */
	that->macfifo_last->old_li = that->li;
	that->macfifo_last->old_li_pos = that->li_pos;
	that->macfifo_last->old_li_end = that->li_end;

	/*   Empty the linebuffer to force getline  */
	that->li=that->li_pos=that->li_end=NULL;

	/*   and continue with the parsing  */
	/*  that->readin_state = READIN_TEXT;  */

	return true;
}


bool readRemMulti(sourcefile_t *that)
{
	while( *that->li_pos!='*' && *that->li_pos!='\n' && *that->li_pos!='\r' && that->li_pos<that->li_end )
		++that->li_pos;
	if( *that->li_pos=='*' && ++that->li_pos<that->li_end && *that->li_pos=='/' )
	{
		++that->li_pos;
		that->readin_state = READIN_TEXT;
	}

	return true;
}


bool readText(sourcefile_t *that)
{
	char *wo_pos, *wo_start;
	uint32_t num;
	uint32_t txtnum;
	uint8_t operand_idx, operand_len;
	size_t keylen;
	uint16_t opcode_key;
	char c;
	bool find;
	uint8_t cnt0;
	macro_t *mac;

	union
	{
		const pp_keytri *ppcnt;
		const psyopc_s *pscnt;
		const uint16_t *mnecnt;
	} ptrs;



	c = *that->li_pos;

	if( isdigit(c) )
	{
		num = 0;
		do
		{
			num *= 10;
			num += (c&0x0f);
		} while( ++that->li_pos<that->li_end && isdigit(c=*that->li_pos) );
		if( !addNumber(&that->lbuf, num) )
			return false;
	}
	else if( c=='$' )
	{
		num = 0;
		find=false;
		while( ++that->li_pos<that->li_end && isxdigit(c=*that->li_pos) )
		{
			num <<= 4;
			num |= (c+(isdigit(c)?0:9))&0x0f;
			find=true;
		}
		if( find )
		{
			if( !addNumber(&that->lbuf, num) )
				return false;
		}
		else
		{
			scanError(EM_HexWithoutNumber, that->fileidx, that->linenr);
			return false;
		}
	}
	else if( c=='"' )
	{
		wo_start = ++that->li_pos;
		if( (wo_pos=readString(that))==NULL )
			return false;
		keylen = wo_pos-wo_start;

		if( !addString(&that->lbuf, wo_start, (stringsize_t)keylen ) )
			return false;
	}
	else if( c=='%' )
	{
		num = 0;
		find=false;
		while( that->li_pos<that->li_end && (c=*(++that->li_pos))>='0' && c<='1' )
		{
			num <<= 1;
			num |= (c&1);
			find=true;
		}
		if( find )
		{
			if( !addNumber(&that->lbuf, num) )
				return false;
		}
		else
		{
			scanError(EM_BinWithoutNumber, that->fileidx, that->linenr);
			return false;
		}
	}
	else if( c=='\'' )
	{
		txtnum=0;
		cnt0=0;

		while( that->li_pos<that->li_end && (c=*(++that->li_pos))!='\'' )
		{
			if( c=='\\' )
			{
				if( that->li_pos>=that->li_end )
				{
					scanError(EM_NoEndingQuotes, that->fileidx, that->linenr);
					return false;
				}
				c = unescape(*(++that->li_pos) );
			}
			txtnum <<= 8;
			txtnum |= c;
			if( ++cnt0>4 )
			{
				scanError(EM_StringTooLong, that->fileidx, that->linenr);
				return false;
			}
		}
		if( c!='\'' )
		{
			scanError(EM_NoEndingQuotes, that->fileidx, that->linenr);
			return false;
		}
		++that->li_pos;
		if( cnt0==0 )
		{
			scanError(EM_EmptyString, that->fileidx, that->linenr);
			return false;
		}
		if( !addTextNum(&that->lbuf, txtnum, cnt0-1 ) )
			return false;
	}
	else if( c==';' )
	{
		while( *that->li_pos!='\n' && *that->li_pos!='\r' && that->li_pos<that->li_end )
			++that->li_pos;
	}
	else if( c=='#' )
	{
		wo_start = ++that->li_pos;
		while( that->li_pos<that->li_end && (isalpha(*that->li_pos) || *that->li_pos=='_') )
			that->li_pos++;
		keylen = that->li_pos-wo_start;

		ptrs.ppcnt=preproc;
		while( ptrs.ppcnt<preproc+arraysize(preproc) )
		{
			if( keylen==ptrs.ppcnt->keylen && !strncasecmp(ptrs.ppcnt->key, wo_start, keylen) )
				break;
			else
				ptrs.ppcnt++;
		}
		if( ptrs.ppcnt<preproc+arraysize(preproc) )
		{
			if( ptrs.ppcnt->idx==PP_MACRODEF ) {
				that->readin_state = READIN_MACROHEAD;
			}
			else
			{
				if( !addPreproc(&that->lbuf, ptrs.ppcnt->idx ) )
					return false;
			}
		}
		else
		{
			that->li_pos=wo_start;
			if( !addOperand(&that->lbuf, OP_Carro) )
				return false;
		}
	}
	else if( c=='.' )
	{
		wo_start = ++that->li_pos;
		if( *wo_start=='(' || *wo_start==')' )
		{
			++that->li_pos;
		}
		else if( that->li_pos<that->li_end && (isalpha(*that->li_pos) || *that->li_pos=='_') )
		{
			do
			{
				++that->li_pos;
			}
			while( that->li_pos<that->li_end && (isalnum(*that->li_pos) || *that->li_pos=='_') );
		}
		keylen = that->li_pos-wo_start;

		/*   look for a pseudo opcode  */
		ptrs.pscnt=psyopc;
		while( ptrs.pscnt<psyopc+arraysize(psyopc) )
		{
			if( keylen==ptrs.pscnt->keylen && !strncasecmp(ptrs.pscnt->key, wo_start, keylen) )
				break;
			else
				ptrs.pscnt++;
		}
		/*   found psy opcode?  */
		if( ptrs.pscnt<psyopc+arraysize(psyopc) )
		{
			if( !addPsyopc(&that->lbuf, ptrs.pscnt->idx ) )
				return false;
		}
		/*   no -> look for macro  */
		else if( (that->macroIdx=macro_find(wo_start, keylen))!=(macro_cnt)-1 )
		{
			if( cfg_debug )
			{
				fprintf(debugLog, "<p>Macro found: ");
				mac = macro_get(that->macroIdx);
				if( mac!=NULL )
				{
					printString(debugLog, mac->sname);
				}
				else
				{
					fprintf(debugLog, "NULL");
				}
				fprintf(debugLog, "<br>\n");
			}
			if( !readMacroPar(that) )
				return false;
		}
		else
		{
			if( !addOperand(&that->lbuf, OP_Dot) )
				return false;
		}
	}
	else if( isblank(c) )
	{
		while( ++that->li_pos<that->li_end && isblank(*that->li_pos) );
	}
	else if( that->li_pos+1<that->li_end && c=='/' && *(that->li_pos+1)=='*' )
	{
		++that->li_pos;
		that->readin_state = READIN_REMMULTI;
	}
	else
	{
		operand_idx=operand_len=0;
		for( cnt0=0; cnt0<arraysize(operator_key); cnt0++)
			if( operand_len<(keylen=operator_keylen[cnt0]) && !strncasecmp(operator_key[cnt0], that->li_pos, keylen) )
			{
				operand_idx=cnt0;
				operand_len=keylen;
			}
		if( operand_len )
		{
			if( !addOperand(&that->lbuf, (OPERAND)(operand_idx) ) )
				return false;
			that->li_pos+=operand_len;
		}
		else
		{
			wo_start = that->li_pos;
			while( ++that->li_pos<that->li_end && (isalnum(*that->li_pos) || *that->li_pos=='_') );
			wo_pos = that->li_pos;
			find=false;
			if( (keylen=wo_pos-wo_start)==3 && isalpha(*wo_start) && isalpha(*(wo_start+1)) && isalnum(*(wo_start+2)) )
			{
				opcode_key = (toupper(*wo_start)&0x1f)|(toupper(*(wo_start+1))&0x1f)<<5|(toupper(*(wo_start+2))&0x3f)<<10;

				ptrs.mnecnt=mne_keys;
				while( ptrs.mnecnt<mne_keys+arraysize(mne_keys) && opcode_key!=*ptrs.mnecnt )
					ptrs.mnecnt++;
				if( ptrs.mnecnt<mne_keys+arraysize(mne_keys) )
				{
					if( !addOpcode(&that->lbuf, (OPCODE)(ptrs.mnecnt-mne_keys) ) )
						return false;
					find=true;
				}
			}
			if( !find )
			{
#if ARRAYSIZE_String<ARRAYSIZE_LineChars
				if( (keylen=wo_pos-wo_start)>((stringsize_t)-1) )
				{
					scanError(EM_StringTooLong, that->fileidx, that->linenr);
					return false;
				}
#endif
				if( !addText(&that->lbuf, wo_start, (stringsize_t)keylen) )
					return false;
			}
		}
	}

	return true;
}


char *readString(sourcefile_t *that)
{
	char *cs, *cp, c;
	size_t keylen;


	cs = cp = that->li_pos;
	c = 0;

	while( that->li_pos<that->li_end && (c=*(that->li_pos++))!='"' )
	{
		if( c=='\\' )
		{
			if( that->li_pos>=that->li_end )
			{
				scanError(EM_NoEndingQuotes, that->fileidx, that->linenr);
				return NULL;
			}
			c = unescape(*(that->li_pos++) );
		}
		*(cp++) = c;
	}
	if( c!='"' )
	{
		scanError(EM_NoEndingQuotes, that->fileidx, that->linenr);
		return NULL;
	}
	else if( (keylen=cp-cs)==0 )
	{
		scanError(EM_EmptyString, that->fileidx, that->linenr);
		return NULL;
	}
#if ARRAYSIZE_String<ARRAYSIZE_LineChars
	else if( keylen>((stringsize_t)-1) )
	{
		scanError(EM_StringTooLong, that->fileidx, that->linenr);
		return false;
	}
#endif
	else
		return cp;
}


bool getline(sourcefile_t *that)
{
	char *sbuf=NULL;
	char *ibuf=NULL;
	char *ebuf=NULL;
	char *nbuf=NULL;
	linesize_t lsize;
	linesize_t nsize;
	bool neol = 1;
	char **parc, **pare;
	macro_t *mac;
	int ic;


	that->li=NULL;
	if( that->macfifo_last!=NULL )
	{
		mac = macro_get(that->macfifo_last->macroIdx);
		if( mac==NULL )
		{
			return false;
		}
		if( that->macfifo_last->mline < mac->mlines_count )
		{
			if( (that->li=getMacroLine(that, &lsize))==NULL )
				return false;

			that->li_end = (that->li_pos=that->li)+lsize;
		}
		else
		{
			that->li = that->macfifo_last->old_li;
			that->li_pos = that->macfifo_last->old_li_pos;
			that->li_end = that->macfifo_last->old_li_end;
			pare=(parc=that->macfifo_last->par)+mac->parameter_count;
			while( parc<pare )
			{
				free( *parc );
				++parc;
			};
			free( that->macfifo_last->par );
			that->macfifo_last->par = NULL;
			that->macfifo_last = ( --that->macfifo_count==0 ) ? NULL : that->macfifo+that->macfifo_count-1;
		}
	}
	else
	{
		/*
		 * Normal Source, No Macro here
		 */

		/*   linenumber up  */
		++that->linenr;

		/*   alloc buffer for a line, first assume max 16 chars  */
		if( (sbuf=ibuf=((char*)(malloc(lsize=16))))==NULL )
			return false;

		do
		{
			/*   pointer to end of buffer  */
			ebuf=sbuf+lsize;
			/*   Read in line until EOF or linebreak, which is one of this:  */
			/*   "\r", "\r\n", "\n"  */
			neol = true;
			do
			{
				switch( ic=*that->plaintext_pos )
				{
				/*   cr -> is there one more char in the filebuffer and is it lf? -> next char  */
				case '\r':
					if( that->plaintext_pos+1<that->plaintext_end && *(that->plaintext_pos+1)=='\n' )
						++that->plaintext_pos;
					/*   fall through  */
				/*   lf -> end of line  */
				case '\n':
					neol = false;
					/*   fall through  */
				/*   copy char to input buffer  */
				default:
					*(ibuf++)=(char)ic;
					break;
				};
			/*   loop while (1) there are still chars to process (2) no eol found (3) room for one more char in the output buffer  */
			} while( ++that->plaintext_pos<that->plaintext_end && neol && ibuf<ebuf );

			/*   size of read block. can be sbuf-ebuf for 'buffer full' or less if eol was found  */
			lsize = ibuf-sbuf;

			/*   buffer must be expanded if no eol was found and output pointer is at end of buffer  */
			if( neol && ibuf==ebuf )
			{
				/*   is buffer already at max size?  */
				if( lsize==(linesize_t)-1 )
				{
					free( sbuf );
					return false;
				}
				/*   try to double size, but at least max it  */
				if( (nsize=lsize<<1)<lsize )
					nsize=(linesize_t)-1;
				/*   realloc buffer  */
				if( (nbuf=(char*)realloc(sbuf, nsize))==NULL )
				{
					free( sbuf );
					return false;
				}
				/*   accept new values  */
				sbuf=nbuf;
				ibuf=sbuf+lsize;
				lsize=nsize;
			}
		} while( that->plaintext_pos<that->plaintext_end && neol );
		that->li_end = (that->li_pos=that->li=sbuf)+lsize;
	}

	return true;
}


char *getMacroLine(sourcefile_t *that, linesize_t *lsize)
{
	mline_t *ls, *lc, *le;
	char *cp, *cs, *ce, ec=0;
	char *line;
	linesize_t len, lsum=0;
	macro_t *mac;
	mfifo_t *mf;


	mf = that->macfifo_last;
	/*
	 * Get Length of line
	 */
	mac = macro_get(mf->macroIdx);
	if( mac==NULL )
	{
		systemError(EM_SystemError);
		return NULL;
	}
	ls = lc = mac->mlines + mf->mline;
	le = mac->mlines + mac->mlines_count;

	while( lc<le && ec!='\n' )
	{
		if( lc->textLength==0 )
			lsum += strlen( *(mf->par + lc->data.pidx) );
		else
		{
			lsum += (len=lc->textLength);
			ec = *(lc->data.line + len -1);
		}
		++lc;
		++mf->mline;
	};

	/*
	 * Allocate Length Buffer
	 */
	if( (cp=line=(char*)malloc(lsum))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}

	/*
	 * Copy Line Elements to buffer
	 */
	while( ls<lc )
	{
		if( ls->textLength==0 )
		{
			cs = *(mf->par + ls->data.pidx);
			while( *cs )
			{
				if( *cs=='\\' && *(cs+1)!=0 )
				{
					++cs;
					*(cp++)=unescape(*(cs++));
				}
				else
					*(cp++)=*(cs++);
			};
		}
		else
		{
			ce = (cs=ls->data.line)+ls->textLength;
			while( cs<ce )
			{
				if( *cs=='\\' && cs+1<ce )
				{
					++cs;
					*(cp++)=unescape(*(cs++));
				}
				else
					*(cp++)=*(cs++);
			};
		}
		++ls;
	};

	/*
	 * If the line contained escaped chars, the buffer was too large.
	 * resize it to the correct length
	 */
	if( (len=cp-line)<lsum && (cs=(char*)realloc(line,len))!=NULL )
	{
		line = cs;
		lsum = len;
	}

	if( cfg_debug )
	{
		cs = line;
		cp = cs+lsum;
		fprintf(debugLog, "Line (lsum %d):", lsum);
		while( cs<cp )
			fprintf(debugLog, "%c", *cs++ );
		fprintf(debugLog, "<br>\n");
	}

	*lsize = lsum;
	return line;
}


bool readMacroParameters(sourcefile_t *that, char ***buf)
{
	uint8_t macPar_newlen, macPar_buflen=0;
	char **macPar_buf = NULL;
	char **macPar_newbuf;
	bool find;
	char *wo_start;
	size_t slen;


	macPar_count = macPar_buflen = 0;

	/*  Skip whitespace  */
	while( that->li_pos<that->li_end && isblank(*that->li_pos) )
		++that->li_pos;

	if( that->li_pos<that->li_end && *that->li_pos=='(' )
	{
		/*  Skip whitespace  */
		while( ++that->li_pos<that->li_end && isblank(*that->li_pos) );
		if( that->li_pos>=that->li_end )
		{
			scanError(EM_NoClosingBracket, that->fileidx, that->linenr);
			return false;
		}
		if( *that->li_pos!=')' )
		{
			/*
			 * allocate array for parameters
			 */
			if( (macPar_buf=(char**)(malloc((macPar_buflen=16)*sizeof(char*))))==NULL )
			{
				systemError(EM_OutOfMemory);
				return false;
			}

			do
			{
				if( !isalpha(*that->li_pos) && *that->li_pos!='_' )
				{
					scanError(EM_ExpectingParameterName_c, that->fileidx, that->linenr, *that->li_pos);
					return false;
				}
				wo_start = that->li_pos;
				while( ++that->li_pos<that->li_end && (isalnum(*that->li_pos) || *that->li_pos=='_') );
				slen = that->li_pos-wo_start;

				if( macPar_count >= macPar_buflen )
				{
					if( macPar_buflen==((macroparam_cnt)-1) )
					{
						scanError(EM_TooManyParameters, that->fileidx, that->linenr);
						return false;
					}

					if( (macPar_newlen=macPar_buflen<<1)<macPar_buflen )
						macPar_newlen=((macroparam_cnt)-1);
					if( (macPar_newbuf=(char**)(realloc(macPar_buf, macPar_newlen*sizeof(char**))))==NULL )
					{
						systemError(EM_OutOfMemory);
						return false;
					}
					macPar_buf = macPar_newbuf;
					macPar_buflen = macPar_newlen;
				}

				if( (macPar_buf[macPar_count]=(char*)(malloc(slen+1)))==NULL )
				{
					systemError(EM_OutOfMemory);
					return false;
				}
				memcpy(macPar_buf[macPar_count], wo_start, slen);
				*(macPar_buf[macPar_count]+slen) = 0;
				++macPar_count;

				/*  Skip whitespace  */
				while( that->li_pos<that->li_end && isblank(*that->li_pos) )
					++that->li_pos;

				/*  Look for seperating ','  */
				if( (find=(that->li_pos<that->li_end && *that->li_pos==',')) )
				{
					/*  Skip whitespace after comma  */
					while( ++that->li_pos<that->li_end && isblank(*that->li_pos) );
				}

				if( that->li_pos>=that->li_end )
				{
					scanError(EM_NoClosingBracket, that->fileidx, that->linenr);
					return false;
				}
			} while( find );
			if( *that->li_pos!=')' )
			{
				scanError(EM_NoClosingBracket, that->fileidx, that->linenr);
				return false;
			}
		}
		/*   skip ')'  */
		++that->li_pos;
	}

	*buf=macPar_buf;
	return true;
}


bool readMacroValues(sourcefile_t *that, char ***buf)
{
	uint8_t macVal_newlen, macVal_buflen=0;
	char **macVal_buf = NULL;
	char **macVal_newbuf;
	char *wo_start;
	size_t slen;
	char c=0;


	macVal_count = macVal_buflen = 0;

	/*  Skip whitespace  */
	while( that->li_pos<that->li_end && isblank(*that->li_pos) )
		++that->li_pos;

	if( that->li_pos<that->li_end && *that->li_pos=='(' )
	{
		/*
		 * allocate array for parameters
		 */
		if( (macVal_buf=(char**)(malloc((macVal_buflen=16)*sizeof(char*))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}

		do
		{
			/*  Skip whitespace  */
			while( ++that->li_pos<that->li_end && isblank(*that->li_pos) );
			if( that->li_pos>=that->li_end )
			{
				scanError(EM_NoClosingBracket, that->fileidx, that->linenr);
				return false;
			}

			/*   Read in the Parameters  */
			if( (c=*(wo_start=that->li_pos))==')' )
			{
				break;
			}
			else if( c=='"' )
			{
				while( ++that->li_pos<that->li_end && (c=*that->li_pos)!='"' )
				{
					if( c=='\\' && ++that->li_pos>=that->li_end )
						break;	/*  force a EM_NoEndingQuotes  */
				}
				if( c!='"' )
				{
					scanError(EM_NoEndingQuotes, that->fileidx, that->linenr);
					return false;
				}
				slen = (++that->li_pos)-wo_start;
			}
			else
			{
				while( that->li_pos<that->li_end && (c=*that->li_pos)!=',' && c!=')' && !isblank(c) )
				{
					if( c=='\\' && ++that->li_pos>=that->li_end )
					{
						scanError(EM_NoClosingBracket, that->fileidx, that->linenr);
						return false;
					}
					++that->li_pos;
				}
				slen = that->li_pos-wo_start;
			}

			if( macVal_count >= macVal_buflen )
			{
				if( macVal_buflen==((macroparam_cnt)-1) )
				{
					scanError(EM_TooManyParameters, that->fileidx, that->linenr);
					return false;
				}

				if( (macVal_newlen=macVal_buflen<<1)<macVal_buflen )
					macVal_newlen=((macroparam_cnt)-1);
				if( (macVal_newbuf=(char**)(realloc(macVal_buf, macVal_newlen*sizeof(char**))))==NULL )
				{
					systemError(EM_OutOfMemory);
					return false;
				}
				macVal_buf = macVal_newbuf;
				macVal_buflen = macVal_newlen;
			}

			if( (macVal_buf[macVal_count]=(char*)(malloc(slen+1)))==NULL )
			{
				systemError(EM_OutOfMemory);
				return false;
			}
			memcpy(macVal_buf[macVal_count], wo_start, slen);
			*(macVal_buf[macVal_count]+slen) = 0;

			if( cfg_debug )
				fprintf(debugLog, "Parameter %d: %s<br>\n", macVal_count, *(macVal_buf+macVal_count));

			++macVal_count;

			/*  Skip whitespace  */
			while( that->li_pos<that->li_end && isblank(*that->li_pos) )
				++that->li_pos;

			if( that->li_pos>=that->li_end )
			{
				scanError(EM_NoClosingBracket, that->fileidx,that-> linenr);
				return false;
			}
		} while( that->li_pos<that->li_end && *that->li_pos==',' );
		if( *(that->li_pos++)!=')' )
		{
			scanError(EM_NoClosingBracket, that->fileidx, that->linenr);
			return false;
		}
	}
	*buf=macVal_buf;
	return true;
}


bool addNumber(linebuffer_t *lbuf, uint32_t num)
{
	lineelement_t lelem;


	lelem.typ = LE_NUMBER;
	lelem.data.num = num;
	return addLineelement(lbuf,&lelem);
}


bool addOperand(linebuffer_t *lbuf, OPERAND op)
{
	lineelement_t lelem;


	lelem.typ = LE_OPERAND;
	lelem.data.op = op;
	return addLineelement(lbuf,&lelem);
}


bool addPreproc(linebuffer_t *lbuf, PREPROC pp)
{
	lineelement_t lelem;


	lelem.typ = LE_PREPROC;
	lelem.data.pp = pp;
	return addLineelement(lbuf,&lelem);
}


bool addPsyopc(linebuffer_t *lbuf, PSYOPC tPsyopc)
{
	lineelement_t lelem;


	lelem.typ = LE_PSYOPC;
	lelem.data.psyopc = tPsyopc;
	return addLineelement(lbuf,&lelem);
}


bool addOpcode(linebuffer_t *lbuf, OPCODE mne)
{
	lineelement_t lelem;


	lelem.typ = LE_MNE;
	lelem.data.mne = mne;
	return addLineelement(lbuf,&lelem);
}


bool addString(linebuffer_t *lbuf, char *str, stringsize_t len)
{
	lineelement_t lelem;


	lelem.typ = LE_STRING;
	if( (lelem.data.str=cstr2string(str,len))==NULL )
		return false;
	return addLineelement(lbuf,&lelem);
}


bool addText(linebuffer_t *lbuf, char *txt, stringsize_t len)
{
	lineelement_t lelem;


	lelem.typ = LE_TEXT;
	if( (lelem.data.txt=cstr2string(txt,len))==NULL )
		return false;
	return addLineelement(lbuf,&lelem);
}


bool addTextNum(linebuffer_t *lbuf, uint32_t txtnum, uint8_t size)
{
	lineelement_t lelem;
	const LINEELEMENT_TYP stab[4] =
	{
		LE_TEXTNUM1,
		LE_TEXTNUM2,
		LE_TEXTNUM3,
		LE_TEXTNUM4
	};


	assert( size<4 );
	lelem.typ = *(stab+size);
	lelem.data.txtnum = txtnum;
	return addLineelement(lbuf,&lelem);
}


bool addLineelement(linebuffer_t *lbuf, lineelement_t *lelem)
{
	lineelement_t* nline;
	linesize_t nlen;


	if( lbuf->line==NULL )
	{
		lbuf->line_size = 0;
		lbuf->line_alloc = 16;
		if( (lbuf->line=(lineelement_t*)(malloc(16*sizeof(lineelement_t))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
	}
	else if( lbuf->line_size>=lbuf->line_alloc )
	{
		if( lbuf->line_alloc==(linesize_t)-1 )
		{
			systemError(EM_TooManyLines);
			return false;
		}

		if( (nlen=lbuf->line_alloc<<1)<lbuf->line_alloc )
			nlen=(linesize_t)-1;
		if( (nline=(lineelement_t*)(realloc(lbuf->line,nlen*sizeof(lineelement_t))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
		lbuf->line = nline;
		lbuf->line_alloc = nlen;
	}

	*(lbuf->line + (lbuf->line_size++)) = *lelem;
	return true;
}


char unescape(char ec)
{
	switch( ec )
	{
	case 'n':
		return '\n';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	default:
		return ec;
	};
}


void src_reset(sourcefile_t *that)
{
	assert( that->linebuf_size>0 );

	that->slnum=that->slpos=0;
	that->slbuf = that->linebuf;
}


bool src_next(sourcefile_t *that)
{
	assert( that->slbuf!=NULL );

	if( that->slnum>=that->linebuf_size ) {
		/*   no elements left, just skip  */
		return true;
	}

	do
	{
		if( ++that->slpos>that->slbuf->line_size )
		{
			that->slpos = 0;
			++that->slnum;
			if( that->slnum>=that->linebuf_size ) {
				/*   get next line  */
				switch( readFileLine(that) ) {
				case 1:
					/*   ok, line read  */
					break;
				case 0:
					/*   ok, but eof reached  */
					return true;
				default:
					return false;
				}
			}
			that->slbuf = that->linebuf + that->slnum;
		}
	} while( that->slpos<that->slbuf->line_size && (that->slbuf->line+that->slpos)->typ==LE_EMPTY );

	return true;
}


const lineelement_t *src_peek(sourcefile_t *that)
{
	lineelement_t *lelem = NULL;


	assert( that->slbuf!=NULL );

	while( that->slnum<that->linebuf_size && that->slpos<that->slbuf->line_size && (lelem=that->slbuf->line+that->slpos)->typ==LE_EMPTY ) {
		if( !src_next(that) ) {
			return NULL;
		}
	}

	if( that->slnum>=that->linebuf_size )
		return &le_eof;

	if( that->slpos>=that->slbuf->line_size )
		return &le_eol;

	return lelem;
}


void freeLineElement(lineelement_t *lelem)
{
	switch( lelem->typ )
	{
	case LE_NUMBER:
	case LE_TEXTNUM1:
	case LE_TEXTNUM2:
	case LE_TEXTNUM3:
	case LE_TEXTNUM4:
	case LE_OPERAND:
	case LE_MNE:
	case LE_PSYOPC:
	case LE_PREPROC:
	case BE_1BYTE:
	case BE_2BYTE:
	case BE_3BYTE:
	case BE_4BYTE:
	case BE_DSB:
	case TE_1BYTE:
	case TE_2BYTE:
	case TE_3BYTE:
	case TE_4BYTE:
	case TE_RBYTE:
	case TE_RLBYTE:
	case TE_FLEX:
	case TE_nBYTE:
	case TE_JIADR:
	case LE_EMPTY:
	case LE_EOL:
	case LE_EOF:
	case LE_SEGMENTENTER:
	case LE_SEGELEMENTER:
	case LE_LOCALBLOCK:
	case LE_PHASE:
	case LE_DEPHASE:
	case LE_SETCODE:
	case LE_ASIZE:
	case LE_XYSIZE:
	case LE_CPUTYPE:
	case LE_MACRODEF:
	case LE_VARTERM:
		break;

	case LE_VARDEF:
		if( lelem->data.vardef.var.valt.typ==VALTYP_STR ) {
			free(lelem->data.vardef.var.valt.value.str);
		}
		break;

	case LE_STRING:
		free(lelem->data.str);
		break;
	case LE_TEXT:
		free(lelem->data.txt);
		break;
	case BE_nBYTE:
		free(lelem->data.b_nbyte);
		break;
	case LE_SRC:
		delSourcefile( filelist_getSrc(lelem->data.srcidx) );
		break;
	};
	lelem->typ = LE_EMPTY;
}


void src_debug(sourcefile_t *that, FILE *dfh)
{
	linescnt_t cnt0;
	stringsize_t *filename;
	linebuffer_t *lbuf;


	fprintf(dfh, "FileIdx: %d ", that->fileidx);
	filename = filelist_getName( that->fileidx);
	printString(dfh, filename);
	fprintf(dfh, "<br>\n");

	fprintf(dfh, "Linebuffer Size:    %d<br>\n", that->linebuf_size );

	fprintf(dfh, "<tt>\n");
	for(cnt0=0; cnt0<that->linebuf_size; cnt0++)
	{
		fprintf(dfh, "%04X:", cnt0);
		lbuf = that->linebuf + cnt0;
		src_debug_line(that, lbuf, dfh, true);
	}
	fprintf(dfh, "</tt>\n");
}


void src_debug_line(sourcefile_t *that, linebuffer_t *lbuf, FILE *dfh, bool recurse)
{
	linesize_t cnt1;
	lineelement_t *lelem;
	stringsize_t nc;
	uint8_t *bp;
	uint16_t mnePackedName;
	char mneUnpackedName[4];
	stringsize_t *filename;
	macro_t *mac;
	uint32_t pc;
	int pc_width;
	char pc_formatstring[9] = { 0, ' ', '%', '0', '0', 'X', ':', ' ', 0 };


	/*   init some vars  */
	mneUnpackedName[3] = 0;

	/* show the linenumber */
	fprintf(dfh, "<tt>L%04X: ", lbuf->linenr);

	/* if PC is final, show 'F' */
	pc_formatstring[0] = ( segment_isPCFinal() ) ? 'F' : '.';
	pc_width = 0;
	if( segment_isPCDefined() )
	{
		/* PC is defined, show the address */
		pc = segment_getPC();
		/* the PC width depends on the current CPU */
		switch( getCurrentCpu() )
		{
		case CPUTYPE_6510:
		case CPUTYPE_6510Ill:
			/* all 6510 models have a 16 bit PC */
			pc_width = 4;
			break;
		case CPUTYPE_65816:
			/* 65816 has 16 bit PC and 8 bit bankbyte */
			pc_width = 6;
			break;
		case CPUTYPE_UNKNOWN:
			/* unknown CPU defaults to 32 bit PC */
			pc_width = 8;
			break;
		}
		pc_formatstring[4] = '0'|pc_width;
		fprintf(dfh, pc_formatstring, pc);
	}
	else
	{
		/* PC is undefined, show 'undef' */
		fprintf(dfh, "undef:");
	}

	for(cnt1=0; cnt1<lbuf->line_size; cnt1++)
	{
		lelem = lbuf->line + cnt1;
		switch( lelem->typ )
		{
		case LE_NUMBER:
			fprintf(dfh, "$%x ",lelem->data.num);
			break;
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
			fprintf(dfh, "<b>TXTNUM</b>(%d) ",lelem->data.txtnum);
			break;
		case LE_OPERAND:
			if( lelem->data.op<arraysize(operator_key) )
			{
				fprintf(dfh, "%s ",operator_key[lelem->data.op]);
			}
			else
			{
				fprintf(dfh, "*** ILLEGAL OPERATOR INDEX: %d ***", lelem->data.op);
			}
			break;
		case LE_PREPROC:
			if( lelem->data.pp<arraysize(preproc) )
			{
				fprintf(dfh, "<b>#%s</b> ",preproc[lelem->data.pp].key);
			}
			else
			{
				fprintf(dfh, "*** ILLEGAL PREPROC INDEX: %d ***", lelem->data.pp);
			}
			break;
		case LE_PSYOPC:
			if( lelem->data.psyopc<arraysize(psyopc) )
			{
				fprintf(dfh, "<b>.%s</b> ",psyopc[lelem->data.psyopc].key);
			}
			else {
				fprintf(dfh, "*** ILLEGAL PSYOPC INDEX: %d ***", lelem->data.psyopc);
			}
			break;
		case LE_MNE:
			if( lelem->data.mne<arraysize(mne_keys) )
			{
				/*   construct mne name from compressed mne_keys entry  */
				mnePackedName = mne_keys[lelem->data.mne];
				mneUnpackedName[0] = (mnePackedName&0x1f)|('A'-1);
				mneUnpackedName[1] = ((mnePackedName>>5)&0x1f)|('A'-1);
				mneUnpackedName[2] = (mnePackedName>>10)&0x3f;
				if( mneUnpackedName[2]<'0' )
				{
					mneUnpackedName[2] |= 'A'-1;
				}
				fprintf(dfh, "<b>%s</b> ",mneUnpackedName);
			}
			else
			{
				fprintf(dfh, "*** ILLEGAL OPCODE INDEX: %d ***", lelem->data.mne);
			}
			break;
		case LE_STRING:
			fputc('"', dfh);
			printString(dfh, lelem->data.str);
			fputc('"', dfh);
			break;
		case LE_TEXT:
			fprintf(dfh, "<i>");
			printString(dfh, lelem->data.txt);
			fprintf(dfh, "</i> ");
			break;
		case BE_1BYTE:
			fprintf(dfh, "$%02x ",lelem->data.b_1byte);
			break;
		case BE_2BYTE:
			fprintf(dfh, "$%04x ",lelem->data.b_2byte);
			break;
		case BE_3BYTE:
			fprintf(dfh, "$%06x ",lelem->data.b_3byte);
			break;
		case BE_4BYTE:
			fprintf(dfh, "$%08x ",lelem->data.b_4byte);
			break;
		case BE_DSB:
			fprintf(dfh, "<b>DSB</b>($%04x,$%02X) ",lelem->data.dsb.length,lelem->data.dsb.fillbyte);
			break;
		case BE_nBYTE:
			fprintf(dfh, "<b>nBYTE</b>(");
			nc=*lelem->data.b_nbyte;
			bp= (uint8_t*)(lelem->data.b_nbyte+1);
			while( nc-- )
				fprintf(dfh, "$%02x%c", *(bp++), (nc!=0)?',':' ' );
			fprintf(dfh, ") ");
			break;
		case TE_1BYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(1BYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(1BYTE) ");
#endif
			break;
		case TE_2BYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(2BYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(2BYTE) ");
#endif
			break;
		case TE_3BYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(3BYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(3BYTE) ");
#endif
			break;
		case TE_4BYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(4BYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(4BYTE) ");
#endif
			break;
		case TE_RBYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(RBYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(RBYTE) ");
#endif
			break;
		case TE_RLBYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(RLBYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(RLBYTE) ");
#endif
			break;
		case TE_FLEX:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(FLEX)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(FLEX) ");
#endif
			break;
		case TE_nBYTE:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(nBYTE)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(nBYTE) ");
#endif
			break;
		case TE_JIADR:
#ifdef __DUMP_TERMS__
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>TERM</b>(JsrInd Address)</a> ", lelem->data.termidx, pass_cnt);
#else
			fprintf(dfh, "<b>TERM</b>(JsrInd Address) ");
#endif
			break;
		case LE_EMPTY:
			fprintf(dfh, ". ");
			break;
		case LE_EOL:
			fprintf(dfh, "SOFTCR ");
			break;
		case LE_EOF:
			/*   should never happen  */
			assert( 0 );
		case LE_SRC:
			if( recurse==true ) {
				fprintf(dfh, "</tt><p>\n");
				src_debug( filelist_getSrc(lelem->data.srcidx), dfh);
	
				fprintf(dfh, "<p>back to FileIdx: %d ", that->fileidx);
				filename = filelist_getName( that->fileidx);
				printString(dfh, filename);
				fprintf(dfh, "<br>\n(continuing...) ");
			} else {
				fprintf(dfh, "<b>SOURCE</b>(");
				printString(dfh, filelist_getName(lelem->data.srcidx) );
				fprintf(dfh, ") ");
			}
			break;
		case LE_VARDEF:
			fprintf(dfh, "<b>VARDEF</b>(");
			if( lelem->data.vardef.varadr.varidx!=no_var )
			{
				printString( dfh, getVarName(lelem->data.vardef.varadr.varidx) );
			}
			else
			{
				fprintf(dfh, "undefined");
			}
			switch( lelem->data.vardef.var.valt.typ )
			{
			case VALTYP_NUM:
				fprintf(dfh, "=$%08x) ", lelem->data.vardef.var.valt.value.num);
				break;
			case VALTYP_STR:
				if( lelem->data.vardef.var.valt.value.str!=NULL )
				{
					fprintf(dfh, "=\"");
					printString(dfh, lelem->data.vardef.var.valt.value.str);
					fprintf(dfh, "\")");
				}
				else
				{
					fprintf(dfh, "= NULL )");
				}
				break;
			}
			break;
		case LE_VARTERM:
			fprintf(dfh, "<a href=\"#term%x_p%d\"><b>VARTERM</b></a> ", lelem->data.termidx, pass_cnt);
			break;
		case LE_SEGMENTENTER:
			fprintf(dfh, "<b>SEGMENTENTER</b>($%08x) ", lelem->data.segmentidx);
			break;
		case LE_SEGELEMENTER:
			fprintf(dfh, "<b>SEGELEMENTER</b>($%08x) ", lelem->data.segelemidx);
			break;
		case LE_LOCALBLOCK:
			fprintf(dfh, "<b>BLOCKENTER</b>($%08x) ", lelem->data.blockidx);
			break;
		case LE_PHASE:
			fprintf(dfh, "<b>PHASE</b>($%04x) ", lelem->data.phasepc);
			break;
		case LE_DEPHASE:
			fprintf(dfh, "<b>DEPHASE</b> ");
			break;
		case LE_SETCODE:
			fprintf(dfh, "<b>CODE</b>(%s) ", lelem->data.code?"scr":"pet");
			break;
		case LE_ASIZE:
			fprintf(dfh, "<b>ASIZE</b>(%d) ", lelem->data.regsize);
			break;
		case LE_XYSIZE:
			fprintf(dfh, "<b>XYSIZE</b>(%d) ", lelem->data.regsize);
			break;
		case LE_CPUTYPE:
			if( lelem->data.cputype<arraysize(cputype_name) ) {
				fprintf(dfh, "<b>.%s</b> ",cputype_name[lelem->data.cputype].name);
			}
			else {
				fprintf(dfh, "*** ILLEGAL CPUTYPE INDEX: %d ***", lelem->data.cputype);
			}
			break;
		case LE_MACRODEF:
			fprintf(dfh, "<b>MACRODEF</b>(");
			mac = macro_get(lelem->data.macroIdx);
			if( mac!=NULL )
			{
				printString(dfh, mac->sname);
			}
			else
			{
				fprintf(dfh, "NULL");
			}
			fprintf(dfh, ") ");
			break;
		};
	}
	fprintf(dfh, "</tt><br>\n");
}

