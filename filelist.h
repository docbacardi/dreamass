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


#ifndef __FILELIST_H__
#define __FILELIST_H__

typedef struct
{
	stringsize_t *name;
	sourcefile_t *src;
} SRCNAME;

void filelist_zero(void);
bool filelist_init(filescnt_t nmemb);
void filelist_cleanUp(void);
filescnt_t filelist_addFile(sourcefile_t *src, stringsize_t *filename);
stringsize_t *filelist_getName(filescnt_t fileidx);
sourcefile_t *filelist_getSrc(filescnt_t fileidx);

int filelist_ropen(const stringsize_t *name);

#endif		/* __FILELIST_H__ */

