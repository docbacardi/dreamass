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

#ifndef __PREPROC_H__
#define __PREPROC_H__


typedef enum
{
	ZU_START		= 0x00,
	ZU_IF_TRUE,
	ZU_IF_NEVERBEENTRUE
/*	ZU_IF_HASBEENTRUE */
} PPZUST;

typedef struct
{
	char *key;
	uint8_t keylen;
	PREPROC idx;
} pp_keytri;

extern const pp_keytri preproc[19];

bool pp_init(sourcefile_t *src);
const lineelement_t *pp_peek(void);
const lineelement_t *pp_get(void);
bool pp_skip(void);

bool pp_eat(void);

void pp_skipGarbage(void);
linesize_t pp_getPos(void);
linescnt_t pp_getLineNr(void);
uint16_t pp_getFileIdx(void);
void pp_delItems(linesize_t from, linesize_t to);
void pp_replaceItem(linesize_t pos, lineelement_t *lelem);
void pp_replaceVarDef(linesize_t pos, lineelement_t *lelem);

typedef bool (*pp_func)(lineelement_t *);


#endif		/* __PREPROC_H__ */

