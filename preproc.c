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

extern bool PetMode;

sourcefile_t *pp_src = NULL;

/*-----------------------------------*/
/* private routines                  */

bool ppf_debug(lineelement_t *selem);
bool ppf_else(lineelement_t *selem);
bool ppf_elsif(lineelement_t *selem);
bool ppf_elsifdef(lineelement_t *selem);
bool ppf_elsiffile(lineelement_t *selem);
bool ppf_elsifndef(lineelement_t *selem);
bool ppf_elsifnfile(lineelement_t *selem);
bool ppf_endif(lineelement_t *selem);
bool ppf_error(lineelement_t *selem);
bool ppf_if(lineelement_t *selem);
bool ppf_ifdef(lineelement_t *selem);
bool ppf_iffile(lineelement_t *selem);
bool ppf_ifndef(lineelement_t *selem);
bool ppf_ifnfile(lineelement_t *selem);
bool ppf_include(lineelement_t *selem);
bool ppf_outfile(lineelement_t *selem);
bool ppf_print(lineelement_t *selem);
bool ppf_segdef(lineelement_t *selem);
bool ppf_warn(lineelement_t *selem);

bool output(bool quiet);

bool nextvalid(void);

bool existFile(const stringsize_t *filename);

/*-----------------------------------*/

const pp_keytri preproc[19] =
{
	{ "else",	4,	PP_ELSE },
	{ "elsif",	5,	PP_ELSIF },
	{ "elsifdef",	8,	PP_ELSIFDEF },
	{ "elsiffile",	9,	PP_ELSIFFILE },
	{ "elsifndef",	9,	PP_ELSIFNDEF },
	{ "elsifnfile",	10,	PP_ELSIFNFILE },
	{ "endif",	5,	PP_ENDIF },
	{ "error",	5,	PP_ERROR },
	{ "if",		2,	PP_IF },
	{ "ifdef",	5,	PP_IFDEF },
	{ "iffile",	6,	PP_IFFILE },
	{ "ifndef",	6,	PP_IFNDEF },
	{ "ifnfile",	7,	PP_IFNFILE },
	{ "include",	7,	PP_INCLUDE },
	{ "macro",	5,	PP_MACRODEF },
	{ "outfile",	7,	PP_OUTFILE },
	{ "print",	5,	PP_PRINT },
	{ "segdef",	6,	PP_SEGDEF },
	{ "warning",	7,	PP_WARNING }
};

#define max_pp_stackpos 1023
uint16_t pp_stackpos = 0;
PPZUST pp_stack[max_pp_stackpos+1] = { ZU_START };

/*
 * size of this array is sizeof(preproc)*count(PPZUST)
 */
const pp_func pp_functions[19] =
{
	ppf_else,
	ppf_elsif,
	ppf_elsifdef,
	ppf_elsiffile,
	ppf_elsifndef,
	ppf_elsifnfile,
	ppf_endif,
	ppf_error,
	ppf_if,
	ppf_ifdef,
	ppf_iffile,
	ppf_ifndef,
	ppf_ifnfile,
	ppf_include,
	NULL,			/* PP_MACRODEF */
	ppf_outfile,
	ppf_print,
	ppf_segdef,
	ppf_warn
};

struct segattribs_t
{
	char *name;
	stringsize_t len;
	SEGMENT_T typ;
} segattribs[5] =
{
	{ "abs",	3,	SEGTYP_ABS },
	{ "bss",	3,	SEGTYP_BSS },
	{ "zp",		2,	SEGTYP_ZP },
	{ "fillup",	6,	SEGTYP_ZP },	/* dummy value for typ */
	{ "force",	5,	SEGTYP_ZP }	/* dummy value for typ */
};
/* the special case 'fillup' is numbshitty prerelease to feed to cvser 3 */
#define SEGATTR_FILLUP 3
/* There are 4 attributes */
#define SEGATTR_CNT 5

struct fileattribs_t
{
	char *name;
	stringsize_t len;
} fileattribs[2] =
{
	{ "sort",	4 },
	{ "startadr",	8 }
};
#define FILEATTR_CNT 2

/*-----------------------------------*/

/* Delete sections in #if, delall==true -> delete all sections, false -> delete just one */
bool ifDelete(bool delall)
{
	lineelement_t *lelem;
	PREPROC tPreProcTyp;
	int ifcnt=1;


	while( ifcnt!=0 )
	{
		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ==LE_EOF ) {
			error(EM_IfStillOpen);
			return false;
		} 
		if( lelem->typ==LE_PREPROC )
		{
			/* get preproc typ */
			tPreProcTyp = lelem->data.pp;

			switch( tPreProcTyp )
			{
			case PP_IF:
			case PP_IFDEF:
			case PP_IFFILE:
			case PP_IFNDEF:
			case PP_IFNFILE:
				/* detect nested conditionals */
				++ifcnt;
				freeLineElement(lelem);
				if( !src_next(pp_src) )
				{
					return false;
				}
				break;
			case PP_ENDIF:
				--ifcnt;
				if( ifcnt!=0 || delall )
				{
					if( !src_next(pp_src) )
					{
						return false;
					}
					freeLineElement(lelem);
				}
				break;
			case PP_ELSE:
			case PP_ELSIF:
			case PP_ELSIFDEF:
			case PP_ELSIFFILE:
			case PP_ELSIFNDEF:
			case PP_ELSIFNFILE:
				/* closed a nested conditional? */
				if( !delall && ifcnt==1 )
				{
					--ifcnt;
					break;
				}
				/* else fallthrough to delete part */
			case PP_ERROR:
			case PP_INCLUDE:
			case PP_MACRODEF:
			case PP_OUTFILE:
			case PP_PRINT:
			case PP_SEGDEF:
			case PP_WARNING:
				/* delete lineelement */
				freeLineElement(lelem);
				if( !src_next(pp_src) )
				{
					return false;
				}
			}
		}
		else
		{
			if( lelem->typ!=LE_EOL )
			{
				freeLineElement(lelem);
			}
			if( !src_next(pp_src) )
			{
				return false;
			}
		}
	};

	return true;
}

/* skip over the rest of #if */
bool ifSkip(void)
{
	lineelement_t *lelem;
	int ifcnt=1;


	while( ifcnt!=0 )
	{
		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ==LE_EOF ) {
			error(EM_IfStillOpen);
			return false;
		}
		if( lelem->typ==LE_PREPROC )
		{
			switch( lelem->data.pp )
			{
			case PP_IF:
			case PP_IFDEF:
			case PP_IFFILE:
			case PP_IFNDEF:
			case PP_IFNFILE:
				++ifcnt;
				break;
			case PP_ENDIF:
				--ifcnt;
				break;
			case PP_ELSE:
			case PP_ELSIF:
			case PP_ELSIFFILE:
			case PP_ELSIFDEF:
			case PP_ELSIFNDEF:
			case PP_ELSIFNFILE:
			case PP_ERROR:
			case PP_INCLUDE:
			case PP_MACRODEF:
			case PP_OUTFILE:
			case PP_PRINT:
			case PP_SEGDEF:
			case PP_WARNING:
				/* nothing to do */
				break;
			};
		}
		if( !src_next(pp_src) ) {
			return false;
		}
	}

	return true;
}


bool ppf_else(lineelement_t *selem)
{
	freeLineElement(selem);

	switch( pp_stack[pp_stackpos] )
	{
	case ZU_START:
		error(EM_ElsifWithoutIf);
		return false;
	case ZU_IF_NEVERBEENTRUE:
		pp_stack[pp_stackpos] = ZU_IF_TRUE;
		break;
	case ZU_IF_TRUE:
		--pp_stackpos;
		if( !ifDelete(true) )
			return false;
		break;
	};
	return true;
}


bool ppf_elsif(lineelement_t *selem)
{
	linesize_t lpos_start;
	VARIABLE var;
	length_t undef = { false, 0 };


	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	if( !read_term(TS_4BYTE,&var, false) )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}

	if( var.defined )
	{
		/* free the #if lineelement */
		freeLineElement(selem);
		/* free the lineelements for the term */
		pp_delItems(lpos_start, pp_src->slpos);

		switch( pp_stack[pp_stackpos] )
		{
		case ZU_START:
			error(EM_ElsifWithoutIf);
			return false;
		case ZU_IF_NEVERBEENTRUE:
			if( var.valt.value.num!=0 )
				pp_stack[pp_stackpos] = ZU_IF_TRUE;
			else
			{
				if( !ifDelete(false) )
					return false;
			}
			break;
		case ZU_IF_TRUE:
			--pp_stackpos;
			if( !ifDelete(true) )
				return false;
			break;
		};
	}
	else
	{
		/*
		 * The argument for #elsif is undefined. This means we don't
		 * know which branch to take and as I'm not doing some crazy
		 * size guessing, the PC will turn undefined too.
		 */
		/* undef PC */
		segment_addLength(undef);
		/* a part of the source was not processed */
		allBytesResolved = false;
		switch( pp_stack[pp_stackpos] )
		{
		case ZU_START:
			error(EM_ElsifWithoutIf);
			return false;
		case ZU_IF_NEVERBEENTRUE:
			/* turn the #elsif into an #if */
			selem->data.pp=PP_IF;
			/* skip over the rest of #if */
			if( !ifSkip() )
				return false;
			break;
		case ZU_IF_TRUE:
			--pp_stackpos;
			if( !ifDelete(true) )
				return false;
			break;
		};
	}

	return true;
}


bool ppf_elsifdef(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool vexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL )
	{
		return false;
	}
	if( lelem->typ!=LE_TEXT )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) )
	{
		return false;
	}

	vexists = existVariable(lelem->data.txt);

	pp_delItems(lpos_start, pp_src->slpos);

	switch( pp_stack[pp_stackpos] )
	{
	case ZU_START:
		error(EM_ElsifWithoutIf);
		return false;
	case ZU_IF_NEVERBEENTRUE:
		if( vexists )
			pp_stack[pp_stackpos] = ZU_IF_TRUE;
		else
		{
			if( !ifDelete(false) )
				return false;
		}
		break;
	case ZU_IF_TRUE:
		--pp_stackpos;
		if( !ifDelete(true) )
			return false;
		break;
	};
	return true;
}


bool ppf_elsiffile(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool fexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL )
	{
		return false;
	}
	if( lelem->typ!=LE_STRING )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) )
	{
		return false;
	}

	fexists = existFile(lelem->data.str);

	pp_delItems(lpos_start, pp_src->slpos);

	switch( pp_stack[pp_stackpos] )
	{
	case ZU_START:
		error(EM_ElsifWithoutIf);
		return false;
	case ZU_IF_NEVERBEENTRUE:
		if( fexists )
		{
			pp_stack[pp_stackpos] = ZU_IF_TRUE;
		}
		else
		{
			if( !ifDelete(false) )
			{
				return false;
			}
		}
		break;
	case ZU_IF_TRUE:
		--pp_stackpos;
		if( !ifDelete(true) )
		{
			return false;
		}
		break;
	};
	return true;
}


bool ppf_elsifndef(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool vexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_TEXT )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) ) {
		return false;
	}

	vexists = existVariable(lelem->data.txt);

	pp_delItems(lpos_start, pp_src->slpos);

	switch( pp_stack[pp_stackpos] )
	{
	case ZU_START:
		error(EM_ElsifWithoutIf);
		return false;
	case ZU_IF_NEVERBEENTRUE:
		if( vexists )
		{
			if( !ifDelete(false) )
				return false;
		}
		else
			pp_stack[pp_stackpos] = ZU_IF_TRUE;
		break;
	case ZU_IF_TRUE:
		--pp_stackpos;
		if( !ifDelete(true) )
			return false;
		break;
	};
	return true;
}


bool ppf_elsifnfile(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool fexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL )
	{
		return false;
	}
	if( lelem->typ!=LE_STRING )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) )
	{
		return false;
	}

	fexists = existFile(lelem->data.str);

	pp_delItems(lpos_start, pp_src->slpos);

	switch( pp_stack[pp_stackpos] )
	{
	case ZU_START:
		error(EM_ElsifWithoutIf);
		return false;
	case ZU_IF_NEVERBEENTRUE:
		if( fexists )
		{
			if( !ifDelete(false) )
			{
				return false;
			}
		}
		else
		{
			pp_stack[pp_stackpos] = ZU_IF_TRUE;
		}
		break;
	case ZU_IF_TRUE:
		--pp_stackpos;
		if( !ifDelete(true) )
		{
			return false;
		}
		break;
	};
	return true;
}


bool ppf_endif(lineelement_t *selem)
{
	freeLineElement(selem);

	switch( pp_stack[pp_stackpos] )
	{
	case ZU_START:
		error(EM_EndifWithoutIf);
		return false;
	case ZU_IF_NEVERBEENTRUE:
	case ZU_IF_TRUE:
		pp_stackpos--;
		break;
	};
	return true;
}

bool ppf_error(lineelement_t *selem)
{
	++errorcnt;
	printString(stdout, filelist_getName(pp_getFileIdx()) );
	fprintf(stdout, ":%u: error:", pp_getLineNr());
	return output(false);
}

bool ppf_if(lineelement_t *selem)
{
	linesize_t lpos_start;
	VARIABLE var;
	length_t undef = { false, 0 };


	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	if( !read_term(TS_4BYTE,&var, false) )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}

	if( var.defined )
	{
		/* free the #if lineelement */
		freeLineElement(selem);
		/* free the lineelements for the term */
		pp_delItems(lpos_start, pp_src->slpos);

		if( var.valt.value.num!=0 )
			pp_stack[++pp_stackpos] = ZU_IF_TRUE;
		else
		{
			pp_stack[++pp_stackpos] = ZU_IF_NEVERBEENTRUE;
			if( !ifDelete(false) )
				return false;
		}
	}
	else
	{
		/*
		 * The argument for #if is undefined. This means we don't
		 * know which branch to take and as I'm not doing some crazy
		 * size guessing, the PC will turn undefined too.
		 */
		/* undef PC */
		segment_addLength(undef);
		/* a part of the source was not processed */
		allBytesResolved = false;
		/* skip over the rest of #if */
		if( !ifSkip() )
			return false;
	}
	return true;
}


bool ppf_ifdef(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool vexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_TEXT )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) ) {
		return false;
	}

	vexists = existVariable(lelem->data.txt);

	pp_delItems(lpos_start, pp_src->slpos);

	if( vexists )
		pp_stack[++pp_stackpos] = ZU_IF_TRUE;
	else
	{
		pp_stack[++pp_stackpos] = ZU_IF_NEVERBEENTRUE;
		if( !ifDelete(false) )
			return false;
	}
	return true;
}


bool ppf_iffile(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool fexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_STRING )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) ) {
		return false;
	}

	fexists = existFile(lelem->data.str);

	pp_delItems(lpos_start, pp_src->slpos);

	if( fexists )
		pp_stack[++pp_stackpos] = ZU_IF_TRUE;
	else
	{
		pp_stack[++pp_stackpos] = ZU_IF_NEVERBEENTRUE;
		if( !ifDelete(false) )
			return false;
	}
	return true;
}


bool ppf_ifndef(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool vexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_TEXT )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) ) {
		return false;
	}

	vexists = existVariable(lelem->data.txt);

	pp_delItems(lpos_start, pp_src->slpos);

	if( vexists )
	{
		pp_stack[++pp_stackpos] = ZU_IF_NEVERBEENTRUE;
		if( !ifDelete(false) )
			return false;
	}
	else {
		pp_stack[++pp_stackpos] = ZU_IF_TRUE;
	}
	return true;
}


bool ppf_ifnfile(lineelement_t *selem)
{
	linesize_t lpos_start;
	lineelement_t *lelem;
	bool fexists;


	freeLineElement(selem);

	if( pp_stackpos>=max_pp_stackpos )
	{
		error(EM_MaxDepth);
		return false;
	}

	lpos_start = pp_src->slpos;
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_STRING )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}
	if( !src_next(pp_src) ) {
		return false;
	}

	fexists = existFile(lelem->data.str);

	pp_delItems(lpos_start, pp_src->slpos);

	if( fexists )
	{
		pp_stack[++pp_stackpos] = ZU_IF_NEVERBEENTRUE;
		if( !ifDelete(false) )
			return false;
	}
	else {
		pp_stack[++pp_stackpos] = ZU_IF_TRUE;
	}
	return true;
}


bool ppf_include(lineelement_t *selem)
{
	lineelement_t *lelem;
	sourcefile_t *nsrc;


	/* get the filename for the include command */
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_STRING )
	{
		error(EM_MissingArgForPreProc);
		return false;
	}

	if( !srcstack_push(pp_src) )
		return false;

	nsrc = newSourcefile();
	if( nsrc==NULL ) {
		return false;
	}
	if( !readSourcefile(nsrc, lelem->data.str, pp_src) ) {
		return false;
	}

	freeLineElement( selem );
	freeLineElement( lelem );

	/* don't include empty buffers (like files which only contain macro defs) */
	if( nsrc->linebuf_size!=0 ) {
		selem->typ = LE_SRC;
		selem->data.srcidx = nsrc->fileidx;

		src_next(pp_src);
		pp_src=nsrc;
		src_reset(pp_src);
	}

	return true;
}


bool ppf_print(lineelement_t *selem)
{
	if( !dumpPass )
	{
		printString(stdout, filelist_getName(pp_getFileIdx()) );
		fprintf(stdout, ":%u: print:", pp_getLineNr());
	}
	return output(dumpPass);
}


bool ppf_outfile(lineelement_t *selem)
{
	lineelement_t *lelem;
	linesize_t lpos_start;
	stringsize_t *filename;
	bool sort, dumpadr;
	uint8_t fillbyte;
	bool defsort, defdumpadr, deffillbyte;
	stringsize_t tlen;
	char *text;
	uint8_t cnt0;
	uint32_t num;
	NALLOC segnames;


	if( nalloc_init(&segnames, sizeof(stringsize_t*), 16)==false ) {
		return false;
	}
	/* nothing is defined yet */
	defsort = defdumpadr = deffillbyte = false;
	lpos_start = pp_src->slpos;
	/* default values */
	sort = dumpadr = false;
	fillbyte = 0;

	/* get "filename" or @ for the commandline -o value */
	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ==LE_STRING ) {
		filename = stringClone(lelem->data.str);
	}
	else if( lelem->typ==LE_TEXT && *lelem->data.txt==1 && *((char*)(lelem->data.txt+1))=='@' ) {
		filename = NULL;
	}
	else {
		error(EM_MissingFilename);
		return false;
	}
	if( !src_next(pp_src) ) {
		return false;
	}

	while( 1 )
	{
		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ!=LE_OPERAND || lelem->data.op!=OP_Comma ) {
			break;
		}
		if( !src_next(pp_src) ) {
			return false;
		}

		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		switch( lelem->typ )
		{
		case LE_STRING:
			/* add name to list */
			if( nalloc_size(&segnames, segnames.elemCount+1)==false ) {
				return false;
			}
			*(((stringsize_t**)segnames.buf)+(segnames.elemCount++)) = stringClone(lelem->data.str);
			if( !src_next(pp_src) ) {
				return false;
			}
			break;
		case LE_TEXT:
			tlen = *lelem->data.txt;
			text = (char*)(lelem->data.txt+1);
			for( cnt0=0; cnt0<FILEATTR_CNT; ++cnt0)
			{
				if( tlen==(fileattribs+cnt0)->len && !strncasecmp( (fileattribs+cnt0)->name, text, tlen ) )
					break;
			}
			if( cnt0>=FILEATTR_CNT )
			{
				error(EM_UnknownFileAttrib);
				return false;
			}
			else if( cnt0==0 )
			{
				if( defsort )
					warning(WM_FileDefOptTwice_s, "sort");
				defsort = true;
				sort = true;
			}
			else
			{
				if( defdumpadr )
					warning(WM_FileDefOptTwice_s, "dumpadr");
				defdumpadr = true;
				dumpadr = true;
			}
			if( !src_next(pp_src) ) {
				return false;
			}
			break;
		case LE_NUMBER:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
			/* this must be the fillbyte */
			num=lelem->data.num;
			if( !src_next(pp_src) ) {
				return false;
			}
			if( deffillbyte )
				warning(WM_FileDefOptTwice_s, "fillbyte");
			deffillbyte = true;
			if( num>0xff )
			{
				error(EM_Overflow_d, num);
				return false;
			}
			fillbyte=(uint8_t)num;
			break;
		case LE_EOL:
		case LE_EOF:
			error(EM_NoArgAfterComma);
			return false;
		case LE_OPERAND:
		case LE_PSYOPC:
		case LE_MNE:
			error(EM_UnknownFileAttrib);
			return false;
		case LE_EMPTY:
		case LE_SRC:
		case LE_VARDEF:
		case LE_VARTERM:
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
		case LE_PREPROC:
		case BE_1BYTE:
		case BE_2BYTE:
		case BE_3BYTE:
		case BE_4BYTE:
		case BE_DSB:
		case BE_nBYTE:
		case TE_1BYTE:
		case TE_2BYTE:
		case TE_3BYTE:
		case TE_4BYTE:
		case TE_RBYTE:
		case TE_RLBYTE:
		case TE_FLEX:
		case TE_nBYTE:
		case TE_JIADR:
			/* Should never happen */
			assert( false );
		};
	};

	if( segnames.elemCount==0 )
	{
		error(EM_FileDefNoSegments);
		return false;
	}
	freeLineElement(selem);
	pp_delItems(lpos_start,pp_src->slpos);
/*
	printf("Outfile definition:\n\tFilename:");
	if( filename!=NULL )
	{
		putchar('"');
		printString(stdout, filename);
		putchar('"');
	}
	else
		printf("outfile");
	printf("\n\tSort: %s\n\tWrite Startadress: %s\n\tFillbyte: $%02x\n\tSegments: ", sort?"yes":"no", dumpadr?"yes":"no", fillbyte);
	for(newlen=0; newlen<segnames_count; ++newlen)
	{
		putchar('"');
		printString(stdout, *(segnames+newlen) );
		putchar('"');
		if( newlen+1<segnames_count )
			printf(", ");
	}
	printf("\n--------\n");
*/
	return segment_newOutfile(filename, sort, dumpadr, fillbyte, (stringsize_t**)segnames.buf, segnames.elemCount);
}


bool ppf_segdef(lineelement_t *selem)
{
	lineelement_t *lelem;
	bool deffillup, defforce, deftyp, deffillbyte, defrange;
	SEGMENT_T segtyp;
	uint8_t fillbyte, cnt0;
	bool fillup, force;
	stringsize_t *name;
	uint32_t num, start, end;
	stringsize_t tlen;
	char *text;
	bool neot;
	linesize_t lpos_start;
	seglistsize_t segidx;


	/* nothing is defined yet */
	deffillup = defforce = deftyp = deffillbyte = defrange = false;
	name = NULL;

	/* prevent some 'might be used uninitialized' warnings */
	start=end=0;

	/* the default values */
	segtyp = SEGTYP_ABS;
	fillbyte = 0;
	fillup = force = false;

	lpos_start = pp_src->slpos;
	neot = true;
	do {
		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		switch( lelem->typ )
		{
		case LE_STRING:
			if( name!=NULL )
				warning(WM_SegDefOptTwice_s, "name");
			if( (name=stringClone(lelem->data.str))==NULL )
				return false;
			if( !src_next(pp_src) ) {
				return false;
			}
			break;
		case LE_TEXT:
			tlen = *lelem->data.txt;
			text = (char*)(lelem->data.txt+1);
			for( cnt0=0; cnt0<SEGATTR_CNT; ++cnt0)
			{
				if( tlen==(segattribs+cnt0)->len && !strncasecmp( (segattribs+cnt0)->name, text, tlen ) )
					break;
			}
			if( cnt0>=SEGATTR_CNT )
			{
				error(EM_UnknownSegAttrib);
				return false;
			}
			else if( cnt0==SEGATTR_FILLUP )
			{
				if( deffillup )
					warning(WM_SegDefOptTwice_s, "fillup");
				deffillup = true;
				fillup=true;
			}
			else if( cnt0==SEGATTR_FILLUP+1 )
			{
				if( defforce )
					warning(WM_SegDefOptTwice_s, "force");
				defforce = true;
				force=true;
			}
			else
			{
				if( deftyp )
					warning(WM_SegDefOptTwice_s, "typ");
				deftyp = true;
				segtyp = (segattribs+cnt0)->typ;
			}
			if( !src_next(pp_src) ) {
				return false;
			}
			break;
		case LE_NUMBER:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
			/* this can be the range or the fillbyte */
			num=lelem->data.num;
			if( !src_next(pp_src) ) {
				return false;
			}
			lelem = (lineelement_t*)src_peek(pp_src);
			if( lelem==NULL ) {
				return false;
			}
			if( lelem->typ==LE_OPERAND && lelem->data.op==OP_Minus )
			{
				if( defrange )
					warning(WM_SegDefOptTwice_s, "range");
				defrange = true;
				if( !src_next(pp_src) ) {
					return false;
				}
				lelem = (lineelement_t*)src_peek(pp_src);
				if( lelem==NULL ) {
					return false;
				}
				if( lelem->typ!=LE_NUMBER )
				{
					error(EM_RangeHasNoEnd);
					return false;
				}
				start = num;
				end = lelem->data.num;
				if( start>=end )
				{
					error(EM_RangeStartGrEqEnd);
					return false;
				}
				if( !src_next(pp_src) ) {
					return false;
				}
			}
			else
			{
				if( deffillbyte )
					warning(WM_SegDefOptTwice_s, "fillbyte");
				deffillbyte = true;
				if( num>0xff )
				{
					error(EM_Overflow_d, num);
					return false;
				}
				fillbyte=(uint8_t)num;
			}
			break;
		case LE_EOL:
		case LE_EOF:
			error(EM_NoArgAfterComma);
			return false;
		case LE_OPERAND:
		case LE_PSYOPC:
		case LE_MNE:
			error(EM_UnknownSegAttrib);
			return false;
		case LE_EMPTY:
		case LE_SRC:
		case LE_VARDEF:
		case LE_VARTERM:
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
		case LE_PREPROC:
		case BE_1BYTE:
		case BE_2BYTE:
		case BE_3BYTE:
		case BE_4BYTE:
		case BE_DSB:
		case BE_nBYTE:
		case TE_1BYTE:
		case TE_2BYTE:
		case TE_3BYTE:
		case TE_4BYTE:
		case TE_RBYTE:
		case TE_RLBYTE:
		case TE_FLEX:
		case TE_nBYTE:
		case TE_JIADR:
			/* Should never happen */
			assert( false );
		};
		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ==LE_OPERAND && lelem->data.op==OP_Comma ) {
			if( !src_next(pp_src) ) {
				return false;
			}
		}
		else {
			neot=false;
		}
	} while( neot );

	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ!=LE_EOL )
	{
		error(EM_GarbageAtEndOfLine);
		return false;
	}
	if( name==NULL )
	{
		error(EM_SegDefNoName);
		return false;
	}
	if( !defrange )
	{
		error(EM_SegDefNoRange);
		return false;
	}

	if( segtyp==SEGTYP_ZP && end>0xff )
	{
		error(EM_SegDefZpOverZeropage);
		return false;
	}

	freeLineElement(selem);
	pp_delItems(lpos_start,pp_src->slpos);
/*
	printf("SegDef:\n\tName: ");
	printString(stdout, name);
	printf("\n\tRange: $%04X - $%04X\n\tTyp :", start, end);
	switch( segtyp )
	{
	case SEGTYP_ABS:
		printf("ABS");
		break;
	case SEGTYP_BSS:
		printf("BSS");
		break;
	case SEGTYP_ZP:
		printf("ZP");
		break;
	default :
		printf("???");
		break;
	};
	printf("\n\tFillByte: $%02X\n\tFillup: %s\n\tForce: %s\n", fillbyte, fillup?"yes":"no", force?"yes":"no" );
	printf("--------\n");
*/
	segidx = segment_new(name, start, end, fillup, fillbyte, force, segtyp);

	return ( segidx!=(seglistsize_t)-1 );
}


bool ppf_warn(lineelement_t *selem)
{
	++warningcnt;
	printString(stdout, filelist_getName(pp_getFileIdx()) );
	fprintf(stdout, ":%u: warning:", pp_getLineNr());
	return output(false);
}


bool output(bool quiet)
{
	const lineelement_t *lelem;
	VARIABLE var;


	while( 1 )
	{
		lelem=src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ==LE_EOL || lelem->typ==LE_EOF ) {
			break;
		}
		switch( lelem->typ )
		{
		case BE_1BYTE:
		case BE_2BYTE:
		case BE_3BYTE:
		case BE_4BYTE:
		case BE_nBYTE:
		case BE_DSB:
		case LE_EMPTY:
		case LE_EOL:
		case LE_EOF:
		case LE_SRC:
		case LE_VARDEF:
		case LE_VARTERM:
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
		case TE_JIADR:
		case TE_RBYTE:
		case TE_RLBYTE:
			/* should never happen */
			assert( false );
		case LE_PSYOPC:
			error(EM_PsyInPrint);
			return false;
		case LE_PREPROC:
			error(EM_PreInPrint);
			return false;
		case LE_MNE:
			error(EM_MneInPrint);
			return false;
		case LE_STRING:
		case LE_TEXT:
		case LE_NUMBER:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
		case LE_OPERAND:
		case TE_1BYTE:
		case TE_2BYTE:
		case TE_3BYTE:
		case TE_4BYTE:
		case TE_FLEX:
		case TE_nBYTE:
			if( !read_term(TS_nBYTE, &var, false) )
			{
				error(EM_TermParse);
				return false;
			}
			if( !quiet )
			{
				if( var.defined )
				{
					switch( var.valt.typ )
					{
					case VALTYP_NUM:
						printf("$%x", var.valt.value.num );
						break;
					case VALTYP_STR:
						putchar('"');
						printString(stdout, var.valt.value.str);
						putchar('"');
						break;
					};
				}
				else
					printf("undefined");
			}
			deleteVariable( &var );
		};
		lelem = src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ==LE_OPERAND && lelem->data.op==OP_Comma )
		{
			if( !src_next(pp_src) ) {
				return false;
			}
			if( !quiet )
				printf(", ");
		}
		else
			break;
	}
	if( !quiet )
		printf("\n");

	return true;
}


bool existFile(const stringsize_t *filename)
{
	int infile;
	bool fFileExists;


	infile = -1;

	fprintf(stderr, "looking for '");
	printString(stderr, filename);
	fprintf(stderr, "'\n");

	/* open and stat file */
	infile = filelist_ropen(filename);
	fFileExists = (infile!=-1) ? true : false;
	fprintf(stderr, "result: %d\n", fFileExists);

	if( infile!=-1 )
	{
		close(infile);
	}

	return fFileExists;
}

/*-----------------------------------*/

bool nextvalid(void)
{
	lineelement_t *lelem;
	sourcefile_t *newsrc;
	bool nv = true;
	VARERR ve;


	while( nv )
	{
		lelem = (lineelement_t*)src_peek(pp_src);
		if( lelem==NULL ) {
			return false;
		}
		switch( lelem->typ )
		{
		case LE_PREPROC:
			if( !src_next(pp_src) ) {
				return false;
			}
			if( !pp_functions[lelem->data.pp](lelem) )
				return false;
			break;
		case LE_EOF:
			if( (newsrc=srcstack_pull())!=NULL )
				pp_src = newsrc;
			else
				nv = false;
			break;
		case LE_SRC:
			if( !srcstack_push(pp_src) )
				return false;
			if( !src_next(pp_src) ) {
				return false;
			}
			pp_src = filelist_getSrc( lelem->data.srcidx );
			src_reset(pp_src);
			break;
		case LE_VARDEF:
			if( !src_next(pp_src) ) {
				return false;
			}
			writeVarAdr(lelem->data.vardef.varadr, lelem->data.vardef.var);
			break;
		case LE_VARTERM:
			if( !src_next(pp_src) ) {
				return false;
			}

			/*
			 * Evaluating the term is already enough. If it's defined and
			 * final, it will replace the LE_VARTERM element on its own
			 */
			if( (ve=term_eval(getTerm(lelem->data.termidx))).err )
				assert( false );
			deleteVariable( &ve.var );
			break;
		case LE_SEGMENTENTER:
			if( !src_next(pp_src) ) {
				return false;
			}
			segment_enterSegment(lelem->data.segmentidx);
			break;
		case LE_SEGELEMENTER:
			if( !src_next(pp_src) ) {
				return false;
			}
			segment_enterSegElem(lelem->data.segelemidx);
			break;
		case LE_LOCALBLOCK:
			if( !src_next(pp_src) ) {
				return false;
			}
			setLocalBlock(lelem->data.blockidx);
			break;
		case LE_PHASE:
			if( !src_next(pp_src) ) {
				return false;
			}
			/* this is a lineelement. the number of nested phases should already be checked */
			if( !segment_phase(lelem->data.phasepc) )
				assert( false );
			break;
		case LE_DEPHASE:
			if( !src_next(pp_src) ) {
				return false;
			}
			/* this is a lineelement. the matching phase should already be checked */
			if( !segment_dephase() )
				assert( false );
			break;
		case LE_SETCODE:
			if( !src_next(pp_src) ) {
				return false;
			}
			PetMode = lelem->data.code;
			break;
		case LE_ASIZE:
			if( !src_next(pp_src) ) {
				return false;
			}
			setRegisterSize_A(lelem->data.regsize);
			break;
		case LE_XYSIZE:
			if( !src_next(pp_src) ) {
				return false;
			}
			setRegisterSize_XY(lelem->data.regsize);
			break;
		case LE_CPUTYPE:
			if( !src_next(pp_src) ) {
				return false;
			}
			setCpuType(lelem->data.cputype);
			break;
		case LE_MACRODEF:
			if( !src_next(pp_src) ) {
				return false;
			}
			if( !macro_activate(lelem->data.macroIdx) ) {
				return false;
			}
			freeLineElement(lelem);
			break;

		case LE_NUMBER:
		case LE_OPERAND:
		case LE_PSYOPC:
		case LE_MNE:
		case LE_STRING:
		case LE_TEXT:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
		case BE_1BYTE:
		case BE_2BYTE:
		case BE_3BYTE:
		case BE_4BYTE:
		case BE_DSB:
		case BE_nBYTE:
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
			nv = false;
			break;
		};
	};
	return true;
}

/*-----------------------------------*/

bool pp_init(sourcefile_t *src)
{
	pp_src = src;
	pp_stackpos = 0;
	*pp_stack = ZU_START;
	return nextvalid();
}

const lineelement_t *pp_peek(void)
{
	return src_peek(pp_src);
}


const lineelement_t *pp_get(void)
{
	const lineelement_t *lelem;


	lelem = src_peek(pp_src);
	if( lelem==NULL ) {
		return NULL;
	}
	if( !src_next(pp_src) ) {
		return NULL;
	}
	if( !nextvalid() ) {
		return NULL;
	}
	return lelem;
}


bool pp_eat(void)
{
	lineelement_t *lelem;


	lelem = (lineelement_t*)src_peek(pp_src);
	if( lelem==NULL ) {
		return false;
	}
	freeLineElement( lelem );
	if( !src_next(pp_src) ) {
		return false;
	}
	return nextvalid();
}


bool pp_skip(void)
{
	if( !src_next(pp_src) ) {
		return false;
	}
	return nextvalid();
}


void pp_skipGarbage(void)
{
	const lineelement_t *lelem;


	while( (lelem=src_peek(pp_src))!=NULL && lelem->typ!=LE_EOF && lelem->typ!=LE_EOL ) {
		/* ignore more errors in this line */
		src_next(pp_src);
	}
}


linescnt_t pp_getLineNr(void)
{
	if( pp_src!=NULL && pp_src->slnum<pp_src->linebuf_size )
		return (pp_src->linebuf+pp_src->slnum)->linenr;
	else
		return 0;
}


uint16_t pp_getFileIdx(void)
{
	return (pp_src!=NULL)?pp_src->fileidx:(uint16_t)-1;
}


linesize_t pp_getPos(void)
{
	return pp_src->slpos;
}


void pp_delItems(linesize_t from, linesize_t to)
{
	linesize_t pos;

	assert( from<=to );
	assert( pp_src->slbuf!=NULL );
	assert( from<pp_src->slbuf->line_size );
	assert( to<=pp_src->slbuf->line_size );

	for( pos=from; pos<to; pos++)
	{
		if( (pp_src->slbuf->line[pos].typ&LINEELEMENT_CLASSMASK)!=LINEELEMENT_PREPROC )
			freeLineElement( pp_src->slbuf->line + pos );
	}
}


void pp_replaceItem(linesize_t pos, lineelement_t *lelem)
{
	assert( pp_src->slbuf!=NULL );
	assert( pos<=pp_src->slbuf->line_size );
	assert( pp_src->slbuf->line[pos].typ==LE_EMPTY );

	pp_src->slbuf->line[pos] = *lelem;
}


void pp_replaceVarDef(linesize_t pos, lineelement_t *lelem)
{
	assert( pp_src->slbuf!=NULL );
	assert( pos<=pp_src->slbuf->line_size );
	assert( pp_src->slbuf->line[pos].typ==LE_EMPTY || pp_src->slbuf->line[pos].typ==LE_VARTERM );

	pp_src->slbuf->line[pos] = *lelem;
}


