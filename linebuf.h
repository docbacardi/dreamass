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


#ifndef __LINEBUFFER_H__
#define __LINEBUFFER_H__


#include "da_stdint.h"


#define LINEELEMENT_CLASSMASK	0xe0
#define LINEELEMENT_TYPMASK	0x1f
#define LINEELEMENT_UNPARSED	0x20
#define LINEELEMENT_BIN		0x40
#define LINEELEMENT_TERM	0x60
#define LINEELEMENT_PREPROC	0x80

typedef enum
{
	LE_EMPTY	= 0,
	LE_EOL,
	LE_EOF,

	LE_SRC		= LINEELEMENT_PREPROC	| 0x00,
	LE_VARDEF	= LINEELEMENT_PREPROC	| 0x01,
	LE_VARTERM	= LINEELEMENT_PREPROC	| 0x02,
	LE_SEGMENTENTER	= LINEELEMENT_PREPROC	| 0x03,
	LE_SEGELEMENTER	= LINEELEMENT_PREPROC	| 0x04,
	LE_PHASE	= LINEELEMENT_PREPROC	| 0x05,
	LE_DEPHASE	= LINEELEMENT_PREPROC	| 0x06,
	LE_LOCALBLOCK	= LINEELEMENT_PREPROC	| 0x07,
	LE_SETCODE	= LINEELEMENT_PREPROC	| 0x08,
	LE_ASIZE	= LINEELEMENT_PREPROC	| 0x09,
	LE_XYSIZE	= LINEELEMENT_PREPROC	| 0x0a,
	LE_CPUTYPE	= LINEELEMENT_PREPROC	| 0x0b,
	LE_MACRODEF	= LINEELEMENT_PREPROC	| 0x0c,

	LE_NUMBER	= LINEELEMENT_UNPARSED	| 0x00,
	LE_OPERAND	= LINEELEMENT_UNPARSED	| 0x01,
	LE_PSYOPC	= LINEELEMENT_UNPARSED	| 0x02,
	LE_PREPROC	= LINEELEMENT_UNPARSED	| 0x03,
	LE_MNE		= LINEELEMENT_UNPARSED	| 0x04,
	LE_STRING	= LINEELEMENT_UNPARSED	| 0x05,
	LE_TEXT		= LINEELEMENT_UNPARSED	| 0x06,
	LE_TEXTNUM1	= LINEELEMENT_UNPARSED	| 0x07,
	LE_TEXTNUM2	= LINEELEMENT_UNPARSED	| 0x08,
	LE_TEXTNUM3	= LINEELEMENT_UNPARSED	| 0x09,
	LE_TEXTNUM4	= LINEELEMENT_UNPARSED	| 0x0a,

	BE_1BYTE	= LINEELEMENT_BIN	| 0x00,
	BE_2BYTE	= LINEELEMENT_BIN	| 0x01,
	BE_3BYTE	= LINEELEMENT_BIN	| 0x02,
	BE_4BYTE	= LINEELEMENT_BIN	| 0x03,
	BE_DSB		= LINEELEMENT_BIN	| 0x04,
	BE_nBYTE	= LINEELEMENT_BIN	| 0x05,

	TE_1BYTE	= LINEELEMENT_TERM	| 0x00,
	TE_2BYTE	= LINEELEMENT_TERM	| 0x01,
	TE_3BYTE	= LINEELEMENT_TERM	| 0x02,
	TE_4BYTE	= LINEELEMENT_TERM	| 0x03,
	TE_RBYTE	= LINEELEMENT_TERM	| 0x04,
	TE_RLBYTE	= LINEELEMENT_TERM	| 0x05,
	TE_FLEX		= LINEELEMENT_TERM	| 0x06,
	TE_nBYTE	= LINEELEMENT_TERM	| 0x07,
	TE_JIADR	= LINEELEMENT_TERM	| 0x08
} LINEELEMENT_TYP;


typedef struct
{
	LINEELEMENT_TYP typ;
	union
	{
		uint32_t num;
		uint32_t txtnum;
		OPERAND op;
		PREPROC pp;
		PSYOPC psyopc;
		OPCODE mne;
		stringsize_t *str;
		stringsize_t *txt;

		uint8_t		b_1byte;
		uint16_t	b_2byte;
		uint32_t	b_3byte;
		uint32_t	b_4byte;
		stringsize_t	*b_nbyte;

		struct
		{
			uint16_t	length;
			uint8_t		fillbyte;
		} dsb;

		termcnt_t termidx;
		filescnt_t srcidx;
		localdepth_t blockidx;
		struct
		{
			VARADR varadr;		/* That's 6 Bytes */
			VARIABLE var;		/* ... and here 5 so alltogether 11 bytes. All others are just 4 bytes. :( */
		} vardef;

		seglistsize_t	segmentidx;
		seglistsize_t	segelemidx;
		uint16_t	phasepc;
		bool code;	/* pet or screencode */
		uint8_t regsize;	/* registersize for 65816 cpu (can be 8 or 16) */
		CPUTYPE cputype;
		macro_cnt macroIdx;
	} data;
} lineelement_t;


typedef struct
{
/*	filescnt_t fileidx; */
	linescnt_t linenr;

	linesize_t line_size;
	linesize_t line_alloc;
	lineelement_t* line;
/*	stringsize_t *text_line; */
} linebuffer_t;


#endif		/* __LINEBUFFER_H__ */

