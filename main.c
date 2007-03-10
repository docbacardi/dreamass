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

#include <stdio.h>
#include <time.h>

/*-----------------------------------*/

#include "globals.h"

/*-----------------------------------*/

#define PATHSEPARATOR '/'

/*-----------------------------------*/

bool cfg_verbose = false;
bool cfg_showVersion = false;
bool cfg_debug = false;
bool cfg_showHelp = false;
bool cfg_allowSimpleRedefine = false;
long cfg_warnflags = W_Normal;

uint32_t cfg_maxwarnings = 20;
uint32_t cfg_maxerrors = 20;

stringsize_t **srcFileNames = NULL;
char *outFileName = "a.out";
char *debugLogName = NULL;
char *errorLogName = NULL;
char *labelLogName = NULL;
FILE *debugLog = NULL;

stringsize_t **includePaths = NULL;
stringsize_t includeMaxLen;
includePathscnt_t includePaths_count;

char *cmdLineSrc = NULL;
size_t cmdLineSrc_count;

srcFileNamescnt_t srcFileNames_count;

uint16_t pass_cnt=0;

sourcefile_t *topLevelSrc = NULL;

/*-----------------------------------*/
/* locals                            */

bool baekwha(const char *pcArgv0);
bool predefineVars(void);
bool assembleAllFiles(void);
sourcefile_t *assemble(void);

void showVersion(void);
void showHelp(const char *pcArgv0);
bool parseArgs(int argc, char **argv);
void dumpObject(sourcefile_t *src, char *ofname);
void dumpLabels(char *dfname);

/*-----------------------------------*/

int main(int argc, char **argv)
{
	stringsize_t **sc, **se;
	bool ok;


	/*   clear all local variables  */
	filelist_zero();
	macro_zero();
	srcstack_zero();
	termlist_zero();
	variable_zero();
	segment_zero();
	mne_init();

	ok =
	(
		parseArgs(argc,argv) &&
		baekwha(*argv)
	);

	/*   cleanup  */
	segment_del();
	del_variable();
	termlist_delete();
	srcstack_cleanUp();
	macro_cleanUp();
	filelist_cleanUp();

	if( includePaths!=NULL )
	{
		for( se=(sc=includePaths)+includePaths_count; sc<se; ++sc )
			free(*sc);
		free( includePaths );
	}

	/* If debug log is open (!=NULL) then close the file */
	if( debugLog!=NULL )
	{
		fprintf(debugLog, "</body></html>\n");
		fclose(debugLog);
	}

	if( cfg_verbose )
		printf("\n\nBye.\n\n");

	return ((ok&&(errorcnt==0)) ? EXIT_SUCCESS:EXIT_FAILURE);
}


bool baekwha(const char *pcArgv0)
{
	time_t t_start, t_end;
	double t_elapsed;
	bool ok;


	if( cfg_showHelp )
	{
		showHelp(pcArgv0);
		return true;
	} else if( cfg_showVersion ) {
		showVersion();
		return true;
	}

	/*
	 * Open Debug log if '-d' was set
	 */
	if( cfg_debug )
	{
		if( (debugLog=fopen(debugLogName,"w"))==NULL )
		{
			fprintf(stderr, "error opening debug log '%s' : %s\n", debugLogName, strerror(errno));
			return false;
		}
		fprintf(debugLog, "<html><head><title>DreamAss debug log</title></head><body>\n");
	}

	/*
	 * No Sourecfiles means nothing to do
	 */
	if( srcFileNames_count==0 )
	{
		printf("No sourcefiles - nothing to do.\n");
		return true;
	}

	if(
		filelist_init(16) &&
		macro_init(16) &&
		srcstack_init(16) &&
		termlist_init(1024) &&
		new_variable(1024) &&
		segment_init(16)
	  )
	{
		/* Stop assembly time from now on */
		t_start = time(NULL);

		ok = assembleAllFiles();
		if( ok )
		{
			if( cfg_verbose )
			{
				t_end = time(NULL);
				t_elapsed = difftime(t_end, t_start);
				printf("%f seconds used\n", t_elapsed);
			}

			if( warningcnt>=cfg_maxwarnings )
			{
				printf("further warnings suppressed.\n");
			}

			if( cfg_verbose )
			{
				if( warningcnt )
					printf("%u",warningcnt);
				else
					printf("no");
				printf(" warning%s, ",(warningcnt==1)?"":"s");
				if( errorcnt )
					printf("%u",errorcnt);
				else
					printf("no");
				printf(" error%s.\n",(errorcnt==1)?"":"s");
			}

			if( errorcnt==0 )
			{
				if( labelLogName!=NULL )
					dumpLabels( labelLogName );

				if( !allBytesResolved )
				{
					fprintf(stderr, "Some vars still undefined. Dump not yet supported, sorry!\n");
					pass_showUndefs(topLevelSrc);
				}
				else
					dumpObject(topLevelSrc, outFileName);
			}
		}
		if( topLevelSrc!=NULL )
			delSourcefile(topLevelSrc);

		return ok;
	}

	return false;
}


bool assembleAllFiles(void)
{
/*
	char *cc, *ce;


	printf("*** command line start ***\n");
	cc = cmdLineSrc;
	ce = cc + cmdLineSrc_count;
	while( cc<ce ) {
		printf("%c", *cc);
		++cc;
	}
	printf("*** command line end ***\n");
*/
	topLevelSrc = assemble();

	return (topLevelSrc!=NULL);

}


sourcefile_t *assemble(void)
{
	sourcefile_t *src;


	newPass();

	if( (src=newSourcefile())!=NULL && cmdlineSourcefile(src, cmdLineSrc, cmdLineSrc_count) )
	{
		do
		{
			pass_cnt++;
			if( cfg_verbose )
			{
				printf("[pass %d]\n", pass_cnt);
			}

			if( !pass_parse(src) )
			{
				delSourcefile(src);
				src = NULL;
				break;
			}

			if( cfg_debug )
			{
				fprintf(debugLog, "<hr width=\"100%%\">\n<h1>[pass %d]</h1><p>\n", pass_cnt);
				fprintf(debugLog, "<hr width=\"100%%\">\n<h2>LABELS</h2><p>\n");
				dump(debugLog, (localdepth_t)-1,true);

				fprintf(debugLog, "<hr width=\"100%%\">\n<h2>SOURCE</h2><p>\n");
				src_debug(src, debugLog);
				segment_debug(debugLog);
			}
			if( cfg_verbose )
			{
				printf("\n");
			}

			if( cfg_verbose && errorcnt==0 )
			{
				if( allBytesResolved ) {
					printf("All bytes resolved\n");
				}
				else {
					printf("Unresolved bytes left\n");
				}
/*
				if( resolvedSomething() ) {
					printf("Undefined vars resolved in this pass\n");
				}
				else {
					printf("No undefined vars resolved in this pass\n");
				}
*/
			}

		} while( errorcnt==0 && resolvedSomething() && !allBytesResolved );
	}

	return src;
}


void showVersion(void)
{
	printf("DreamAss ");
#ifdef VERSION_ALL
	printf(VERSION_ALL);
#endif
	printf("\n\n");
	printf("Copyright (C) 2002-2006 Christoph Thelen.\n");
	printf("There is NO warranty.  You may redistribute this software\n");
	printf("under the terms of the GNU General Public License.\n");
	printf("For more information about these matters, see the files named COPYING.\n");
}


void showHelp(const char *pcArgv0)
{
	printf(
		"Usage: %s [OPTION | FILE]...\n"
		"A crossassembler for 6510, 65816 and compatible CPUs.\n"
		"\n"
		"Options:\n"
		"  -d,  --debug <FILENAME>       write debug output to <FILENAME>.\n"
		"                                default is no debug output.\n"
		"  -D<VAR>=<VAL>                 define variable <VAR> with value <VAL>.\n"
		"  -e,  --error-log <FILENAME>   log all errors to <FILENAME>.\n"
		"                                default is no errorlogging.\n"
		"  -h,  --help                   display this help and exit.\n"
		"  -I<PATH>                      add <PATH> to include paths.\n"
		"  -l,  --label-log <FILENAME>   write all labels with their value to <FILENAME>.\n"
		"                                default is no label log.\n"
		"  -me, --max-errors <NUMBER>    stop after <NUMBER> errors, 0 means endless.\n"
		"                                default is 20.\n"
		"  -mw, --max-warnings <NUMBER>  stop after <NUMBER> warnings, 0 means endless.\n"
		"                                default is 20.\n"
		"  -o,  --output <FILENAME>      write output to <FILENAME>.\n"
		"                                default is a.out .\n"
		"  -r,  --simple-redefine        allow simple redefines.\n"
		"                                default is off.\n"
		"  -v,  --verbose                be verbose.\n"
		"                                default is verbose off.\n"
		"       --version                display version information and exit.\n"
		"  -W<TAG>                       modify warning level.\n"
		"\n"
		"Non-option arguments are filenames of assembler sources. They are searched\n"
		"in the directory where dreamass was invoked from. If this fails, all include\n"
		"paths are searched in the order of their appearance on the comandline.\n"
		"Files included from within the assembler source are searched the same way.\n"
		"\n"
		"Examples:\n"
		"  dreamass main.src                             assemble main.src to a.out\n"
		"  dreamass -o main.prg main.src                 assemble main.src to main.prg\n"
		"  dreamass -Dcars=20 race.src                   define cars=20 and assemble race.src\n"
		"  dreamass -DData=\"names.db\" addressbook.src    define Data=\"names.db\" and assemble addressbook.src\n"
		"\n"
		"Email bug reports to DocBacardi@the-dreams.de.\n"
		"DreamAss home page: http://rrtools.berlios.de\n", pcArgv0 );
}


bool parseArgs(int argc, char **argv)
{
	char **argp, **arge;
	stringsize_t **newbuf, slen;
	char *newbuf_c;
	includePathscnt_t includePaths_buflen;
	size_t cmdLineSrc_buflen;
	uint8_t incslash;
	char *wflagpos;
	const warnFlag_t *wc, *we;
	long wflags;
	bool wflags_set;
	long int num;
	char *epos;
	char *srcpos;
	size_t arglen, cmdlen;
	union
	{
		includePathscnt_t i;
		linescnt_t c;
	} newlen;
	char *cc, *cd, *ce;


	srcFileNames_count = includePaths_count = 0;
	includeMaxLen = 0;
	wflags = 0;
	wflags_set = false;

	includePaths_buflen = 16;
	includePaths = (stringsize_t**)malloc(includePaths_buflen*sizeof(stringsize_t*));
	if( includePaths==NULL ) {
		systemError(EM_OutOfMemory);
		goto __EXC_CleanUp__;
	}

	cmdLineSrc_buflen = 256;
	cmdLineSrc = (char*)malloc(cmdLineSrc_buflen);
	if( cmdLineSrc==NULL ) {
		systemError(EM_OutOfMemory);
		goto __EXC_CleanUp__;
	}

	/*
	 * Read in Args
	 */
	argp=argv;
	arge=argv+argc;

	while( ++argp<arge )
	{
		if( !strcmp(*argp, "-v") || !strcmp(*argp, "--verbose") ) {
			cfg_verbose = true;
		}
		else if( !strcmp(*argp, "-h") || !strcmp(*argp, "--help") ) {
			cfg_showHelp = true;
		}
		else if( !strcmp(*argp, "--version") ) {
			cfg_showVersion = true;
		}
		else if( !strcmp(*argp, "-d") || !strcmp(*argp, "--debug") ) {
			if( ++argp>=arge )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *(argp-1) );
				break;
			}
			cfg_debug = true;
			debugLogName = *argp;
		}
		else if( !strcmp(*argp, "-e") || !strcmp(*argp, "--error-log") )
		{
			if( ++argp>=arge )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *(argp-1) );
				break;
			}
			errorLogName = *argp;
		}
		else if( !strcmp(*argp, "-l") || !strcmp(*argp, "--label-log") )
		{
			if( ++argp>=arge )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *(argp-1) );
				break;
			}
			labelLogName = *argp;
		}
		else if( !strcmp(*argp, "-o") || !strcmp(*argp, "--output") )
		{
			if( ++argp>=arge )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *(argp-1) );
				break;
			}
			outFileName = *argp;
		}
		else if( !strcmp(*argp, "-me") || !strcmp(*argp, "--max-errors") )
		{
			if( ++argp>=arge )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *(argp-1) );
				break;
			}
			else
			{
				num = strtol(*argp,&epos,10);
				if( *epos!=0 )
				{
					fprintf(stderr, "Non-numeric Argument for parameter '%s'\n", *(argp-1) );
					break;
				}
				else if( num<1 )
				{
					cfg_maxerrors = (uint32_t)-1;
				}
				else
				{
					cfg_maxerrors = num;
				}
			}
		}
		else if( !strcmp(*argp, "-mw") || !strcmp(*argp, "--max-warnings") )
		{
			if( ++argp>=arge )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *(argp-1) );
				break;
			}
			else
			{
				num = strtol(*argp,&epos,10);
				if( *epos!=0 )
				{
					fprintf(stderr, "Non-numeric Argument for parameter '%s'\n", *(argp-1) );
					break;
				}
				else if( num<1 )
				{
					cfg_maxwarnings = (uint32_t)-1;
				}
				else
				{
					cfg_maxwarnings = num;
				}
			}
		}
		else if( !strcmp(*argp, "-r") || !strcmp(*argp, "--simple-redefines") ) {
			cfg_allowSimpleRedefine = true;
		}
		else if( !strncmp(*argp, "-I", 2) )
		{
			if( *(*argp+2)==0 )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *argp );
				break;
			}
			if( includePaths_count >= includePaths_buflen )
			{
				if( includePaths_buflen==((includePathscnt_t)-1) )
				{
					error(EM_TooManyFiles);
					goto __EXC_CleanUp__;
				}

				if( (newlen.i=includePaths_buflen<<1)<includePaths_buflen )
					newlen.i=((includePathscnt_t)-1);
				if( (newbuf=(stringsize_t**)(realloc(includePaths, newlen.i*sizeof(stringsize_t*))))==NULL )
				{
					systemError(EM_OutOfMemory);
					goto __EXC_CleanUp__;
				}
				includePaths = newbuf;
				includePaths_buflen = newlen.i;
			}
			slen = strlen(*argp+2);
			incslash = ( *(*argp+1+slen)==PATHSEPARATOR ) ? 0 : 1;

			/* count backslashes */
			cc = *argp+2;
			ce = cc + slen;
			while(cc<ce) {
				if( *(cc++)=='\\' ) {
					++incslash;
				}
			}

			if( (*(includePaths+includePaths_count)=(stringsize_t*)(malloc(slen+sizeof(stringsize_t)+incslash)))==NULL ) {
				systemError(EM_OutOfMemory);
				goto __EXC_CleanUp__;
			}
			*(*(includePaths+includePaths_count)) = slen+incslash;
			if( includeMaxLen<slen+incslash ) {
				includeMaxLen=slen+incslash;
			}
			/* copy to destination and expand backslashes */
			cc = *argp+2;
			cd = (char*)(*(includePaths+includePaths_count)+1);
			while(cc<ce) {
				if( (*(cd++)=*(cc++))=='\\' ) {
					*(cd++) = '\\';
				}
			}
			if( *(cc-1)!=PATHSEPARATOR ) {
				*cd = PATHSEPARATOR;
			}
			++includePaths_count;
		}
		else if( !strncmp(*argp, "-D", 2) )
		{
			if( *(*argp+2)==0 )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *argp );
				break;
			}

			arglen = strlen(*argp+2);
			cmdlen = arglen + 1;

			if( cmdLineSrc_count+cmdlen>cmdLineSrc_buflen ) {
				if( cmdLineSrc_buflen<arglen ) {
					newlen.c = cmdLineSrc_buflen + cmdlen;
				} else {
					newlen.c = cmdLineSrc_buflen<<1;
				}
				if( newlen.c<cmdLineSrc_buflen ) {
					error(EM_TooManyLines);
					goto __EXC_CleanUp__;
				}
				newbuf_c = (char*)realloc(cmdLineSrc, newlen.c);
				if( newbuf_c==NULL ) {
					systemError(EM_OutOfMemory);
					goto __EXC_CleanUp__;
				}
				cmdLineSrc = newbuf_c;
				cmdLineSrc_buflen = newlen.c;
			}

			srcpos = cmdLineSrc+cmdLineSrc_count;
			memcpy( srcpos, *argp+2, arglen );
			srcpos += arglen;
			*srcpos = '\n';
			cmdLineSrc_count += cmdlen;
		}
		else if( !strncmp(*argp, "-W", 2) )
		{
			/* '-W' needs a parameter like '-Wall' */
			if( *(wflagpos=*argp+2)==0 )
			{
				fprintf(stderr, "Missing Argument for parameter '%s'\n", *argp );
				break;
			}
			else
			{
				wc = warnFlags;
				we = wc + arraysize(warnFlags);
				while( wc<we )
				{
					if( strcmp(wflagpos,wc->flag)==0 )
					{
						wflags &= wc->mask;
						wflags |= wc->value;
						wflags_set = true;
						break;
					}
					++wc;
				}
				if( wc>=we )
					fprintf(stderr, "Unknown warning flag '%s'\n", wflagpos );
			}
		}
		else
		{
			arglen = strlen(*argp);

			incslash = 0;
			/* count backslashes */
			cc = *argp;
			ce = cc + arglen;
			while(cc<ce) {
				if( *(cc++)=='\\' ) {
					++incslash;
				}
			}

			cmdlen = arglen + incslash + 12;

			if( cmdLineSrc_count+cmdlen>cmdLineSrc_buflen ) {
				if( cmdLineSrc_buflen<cmdlen ) {
					newlen.c = cmdLineSrc_buflen + cmdlen;
				} else {
					newlen.c = cmdLineSrc_buflen<<1;
				}
				if( newlen.c<cmdLineSrc_buflen ) {
					error(EM_TooManyLines);
					goto __EXC_CleanUp__;
				}
				newbuf_c = (char*)realloc(cmdLineSrc, newlen.c);
				if( newbuf_c==NULL ) {
					systemError(EM_OutOfMemory);
					goto __EXC_CleanUp__;
				}
				cmdLineSrc = newbuf_c;
				cmdLineSrc_buflen = newlen.c;
			}

			srcpos = cmdLineSrc+cmdLineSrc_count;
			memcpy(srcpos, "#include \"", 10);
			srcpos += 10;
			/* copy to destination and expand backslashes */
			cc = *argp;
			while(cc<ce) {
				if( (*(srcpos++)=*(cc++))=='\\' ) {
					*(srcpos++) = '\\';
				}
			}
			memcpy(srcpos, "\"\n", 2);

			cmdLineSrc_count += cmdlen;
			++srcFileNames_count;
		}
	};


	if( includePaths_count==0 )
	{
		free( includePaths );
		includePaths = NULL;
	}
	else if( includePaths_count<includePaths_buflen && (newbuf=(stringsize_t**)realloc(includePaths,includePaths_count*sizeof(stringsize_t*)))!=NULL )
	{
		includePaths=newbuf;
		includePaths_buflen = includePaths_count;
	}

	if( cmdLineSrc_count==0 )
	{
		free( cmdLineSrc );
		cmdLineSrc = NULL;
	}
	else if( cmdLineSrc_count<cmdLineSrc_buflen && (newbuf_c=(char*)realloc(cmdLineSrc,cmdLineSrc_count))!=NULL )
	{
		cmdLineSrc=newbuf_c;
		cmdLineSrc_buflen = cmdLineSrc_count;
	}

	if( wflags_set )
		cfg_warnflags = wflags;
	else
		cfg_warnflags = W_Normal;

/*
	printf("\n*** Arguments Start ***\n");
	printf("Verbose: %s\n", (cfg_verbose?"yes":"no"));
	printf("Output file: %s\n", (outFileName!=NULL)?outFileName:"none");
	printf("Error log: %s\n", (errorLogName!=NULL)?errorLogName:"none");
	printf("Label log: %s\n", (labelLogName!=NULL)?labelLogName:"none");
	printf("Warnflags were %s set\n", wflags_set?"":"not");
	printf("Warnflags: %lx\n", cfg_warnflags);
	printf("Include paths: ");
	for(newlen.i=0; newlen.i<includePaths_count; newlen.i++)
	{
		printString(stdout, includePaths[newlen.i]);
		if( newlen.i+1<includePaths_count )
			printf(", ");
	}
	printf("\n");
	printf("Source Files: ");
	for(newlen.s=0; newlen.s<srcFileNames_count; newlen.s++)
	{
		printString(stdout, srcFileNames[newlen.s]);
		if( newlen.s+1<srcFileNames_count )
			printf(", ");
	}
	printf("\n*** Arguments End ***\n");
*/
	return true;

__EXC_CleanUp__:
	if( includePaths!=NULL )
		free(includePaths);
	if( srcFileNames!=NULL )
		free(srcFileNames);
	includePaths = NULL;
	srcFileNames = NULL;
	return false;
}


void dumpObject(sourcefile_t *src, char *ofname)
{
	if( cfg_verbose )
	{
		printf("Writing object to '%s'\n", ofname);
	}

	if( pass_dump(src) )
	{
		segment_writeFiles(ofname);
	}
}


void dumpLabels(char *dfname)
{
	FILE *dfh = NULL;


	if( cfg_verbose )
		printf("Writing labels to '%s'\n", dfname);

	if( (dfh=fopen(dfname,"w"))==NULL )
	{
		error(EM_CreateFile_s, dfname);
	}
	else
	{
		if( !dump(dfh,0,false) )
			error(EM_WriteError_s, dfname);
		fclose(dfh);
	}
}

