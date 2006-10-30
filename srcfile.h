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
#include <sys/stat.h>

#ifndef __SOURCEFILE_H__
#define __SOURCEFILE_H__


enum READIN_STATE {
	READIN_TEXT		= 0,
	READIN_REMMULTI		= 1,
	READIN_MACROHEAD	= 2,
	READIN_MACROSTART	= 3,
	READIN_MACRODEF		= 4
};


typedef uint8_t macfifocnt_t;

typedef struct
{
	linescnt_t mline;
	macro_cnt macroIdx;
	char **par;
	char *old_li, *old_li_pos, *old_li_end;
} mfifo_t;


typedef struct
{
	/*
	 * unparsed text
	 */
	char *plaintext_start;
	char *plaintext_end;
	char *plaintext_pos;

	/*
	 * lineposition
	 */
	char *li, *li_pos, *li_end;
	linescnt_t linenr;

	linebuffer_t *slbuf;
	linescnt_t slnum;
	linesize_t slpos;

	filescnt_t fileidx;

	linescnt_t linebuf_size;
	linescnt_t linebuf_alloc;
	linebuffer_t *linebuf;

	/*
	 * Macro-Fifo
	 */
	macfifocnt_t macfifo_count;
	macfifocnt_t macfifo_buflen;
	mfifo_t *macfifo;
	mfifo_t *macfifo_last;
	linescnt_t macline;

	enum READIN_STATE readin_state;
	linebuffer_t lbuf;

	/*
	 * Temp Vars to define/use a macro
	 */
	macro_cnt macroIdx;
	linescnt_t mlines_buflen;

} sourcefile_t;


sourcefile_t *newSourcefile(void);
void delSourcefile(sourcefile_t *that);

bool cmdlineSourcefile(sourcefile_t *that, char *buffer, size_t buffer_size);
bool readSourcefile(sourcefile_t *that, stringsize_t *filename, sourcefile_t *parent);

void src_debug(sourcefile_t *that, FILE *dfh);

void src_reset(sourcefile_t *that);
bool src_next(sourcefile_t *that);
const lineelement_t *src_peek(sourcefile_t *that);

void freeLineElement(lineelement_t *lelem);


#endif		/* __SOURCEFILE_H__ */

