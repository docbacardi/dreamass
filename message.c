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

#include <stdio.h>
#include <stdarg.h>

#include "globals.h"

/*-----------------------------------*/

const char *warnTexts[] =
{
	"Unknown or non-final label forced to %d bit to fit a legal adressmode",
	"Adressmode forces expansion of byte parameter to word size",
	"Binary include exceeds 64k, cutting down",
	"segdef option '%s' specified twice, using last value",
	"outfile option '%s' specified twice, using last value",
	"indirect jmp to last pagebyte ($%04x). 6502/6510 bug!",
	".PSEUDOPC block is still open at end of file",
	"local block is still open at end of file"
};


/*
 * arguments for the '-W' commandline switch
 */
const warnFlag_t warnFlags[12] =
{
	{ "none",	0x00000000,			0x00000000 },
	{ "normal",	W_Normal,			0xffffffff },

	{ "user",	W_User,				0xffffffff },
	{ "preuse",	W_UnknownLabelForcedSize,	0xffffffff },
	{ "adrmode",	W_AddressmodeForcedSize,	0xffffffff },
	{ "bincludes",	W_BinaryIncludeTooLong,		0xffffffff },
	{ "segdef",	W_SegdefOptionTwice,		0xffffffff },
	{ "outfile",	W_OutfileOptionTwice,		0xffffffff },
	{ "jmpind",	W_IndirectJmp,			0xffffffff },
	{ "pcblock",	W_PhaseIdxShift,		0xffffffff },
	{ "localblock",	W_LocalBlockOpen,		0xffffffff },

	{ "all",	0xffffffff,			0xffffffff }
};


const char *errorTexts[] =
{
	"out of memory error",
	"too many files",
	"file not found: %s",
	"read error in file: %s",
	"missing name for macro",
	"expecting macro name, which must start with a letter or '_'. found: '%c'",
	"opening bracket '(' without closing one",
	"expecting parameter name, which must start with a letter or '_'. found: '%c'",
	"unknown parameter: %s",
	"expecting '{'",
	"expecting '}'",
	"'$' without number",
	"missing '\"' at end of the string",
	"'%' without number",
	"unknown pseudoopcode or undefined macro",
	"too many lines",
	"garbage at end of line",
	"number without operand",
	"string without operand",
	"expecting assignment to variable",
	"assignment without right side",
	"redefinition of variable '%s'",
	"missing argument for '.dsb'",
	"comma without following argument for '.db'",
	"comma without following argument for '.dw'",
	"syntax error after '*'",
	"operand at start of command",
	"unknown adressmode",
	"illegal adressmode or cpu for this opcode",
	"branch out of range error",
	"number too large: $%x",
	"string too long to convert to number",
	"pseudoopcode found in term. forgot a newline?",
	"opcode found in term. forgot a newline?",
	"term too long",
	"division by zero",
	"'#if' still open at end of file",
	"'#else' without '#if'",
	"maximum depth reached",
	"missing preprocessor argument",
	"unable to evaluate preprocessor argument in first pass",
	"'#elsif' without '#if'",
	"'#endif' without '#if'",
	"first argument must be format string",
	"too many parameters",
	"too many macros",
	"unable to create file: %s",
	"write error in file: %s",
	"number of passed macro parameters (%d) does not match %d",
	"pseudoopcode in #print statement",
	"preprocessor command in #print statement",
	"opcode in #print statement",
	"error parsing term",
	"too many nested macros",
	"missing argument in align statement",
	"string must not be empty",
	"string too long",
	"assignment must have a variable on the left side",
	"redefinition of variable with assignment operator",
	"too many variables",
	"too many local blocks",
	"closing local block without matching opening one",
	"missing filename",
	"missing argument after comma",
	"binary include offset exceeds file length",
	"binary include length exceeds file length",
	"'@' without labelname",
	"specified level exceeds the current depth of local blocks",
	"too many subsections in segment",
	"missing end address in range definition",
	"range start must be smaller than the end",
	"unknown keyword in segment definition",
	"no name in #segdef",
	"no range in #segdef",
	"segment of type ZP must not exceed $ff",
	"wrong type",
	"missing segment name",
	"unknown keyword in outfile definition",
	"too many segments",
	"no segments in outfile definition",
	"too many outfiles",
	"unknown segment",
	"segment elements overlapping: $%04x-$%04x and $%04x-$%04x",
	"variable undefined: %s",
	"segelement out of segment memory area: %s",
	"undef variable in '.segment' statement. not possible yet",
	"unknown segment: '%s'. no preuse yet",
	"'.realpc' without matching '.pseudopc'",
	"too many nested '.pseudopc's",
	".PSEUDOPC block is still open at end of source",
	"local block is still open at end of source",
	"missing argument in pseudopc statement",
	"missing argument in registersize change",
	"registersize is non defined or non final",
	"registersize must be 8 or 16 and not: %d",
	"registersize 16 is only allowed for 65816 cpu. current cpu: %s",
	"missing argument in cpu statement",
	"unknown cpu type: %s",
	"redefinition of macro %s",
	"macro name is reserved",
	"unknown preprocessor command",
	"system error"
};

/*-----------------------------------*/


void systemError(ERRORMSG msg, ...)
{
	va_list ap;


	++errorcnt;
	fprintf(stdout, "error:");
	va_start(ap, msg);
	vfprintf(stdout, *(errorTexts+msg), ap);
	va_end(ap);
	fprintf(stdout, "\n");
}


void scanError(ERRORMSG msg, filescnt_t fileidx, linescnt_t linenr, ...)
{
	va_list ap;


	++errorcnt;
	printString(stdout, filelist_getName(fileidx));
	fprintf(stdout, ":%u: error:", linenr);
	va_start(ap, linenr);
	vfprintf(stdout, *(errorTexts+msg), ap);
	va_end(ap);
	fprintf(stdout, "\n");
}


void error(ERRORMSG msg, ...)
{
	va_list ap;


	++errorcnt;
	printString(stdout, filelist_getName(pp_getFileIdx()));
	fprintf(stdout, ":%u: error:", pp_getLineNr());
	va_start(ap, msg);
	vfprintf(stdout, *(errorTexts+msg), ap);
	va_end(ap);
	fprintf(stdout, "\n");
}


void warning(WARNMSG msg, ...)
{
	va_list ap;


	if(
		msg<arraysize(warnFlags) &&
		(cfg_warnflags & warnFlags[msg].value)!=0 &&
		++warningcnt<cfg_maxwarnings
	  )
	{
		printString(stdout, filelist_getName(pp_getFileIdx()));
		fprintf(stdout, ":%u: warning:", pp_getLineNr());
		va_start(ap, msg);
		vfprintf(stdout, *(warnTexts+msg), ap);
		va_end(ap);
		fprintf(stdout, "\n");
	}
}


