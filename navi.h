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

#ifndef __NAVI_H__
#define __NAVI_H__

/*-----------------------------------*/

typedef struct {
	size_t elemSize;
	size_t maxLen;
	void *buf;
	size_t elemCount;
	size_t bufLen;
} NALLOC;

/*-----------------------------------*/

int stringCmp(const stringsize_t *s1, const stringsize_t *s2);
stringsize_t *stringClone(const stringsize_t *str);
stringsize_t *cstr2string(const char *cstr, const stringsize_t clen);
char *string2cstr(const stringsize_t *str);
void printString(FILE *f, const stringsize_t *str);
bool readFile(int filedsc, char *pc, size_t length);

/*-----------------------------------*/

bool nalloc_init(NALLOC *ptNAlloc, size_t elemSize, size_t initialLen);
bool nalloc_size(NALLOC *ptNAlloc, size_t reqLen);
void nalloc_free(NALLOC *ptNAlloc);

/*-----------------------------------*/

#endif		/* __NAVI_H__ */

