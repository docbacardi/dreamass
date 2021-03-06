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


#ifndef __PASS_H__
#define __PASS_H__


/*-----------------------------------*/

extern uint32_t errorcnt;
extern uint32_t warningcnt;

extern bool dumpPass;

void newPass(void);

bool pass_parse(sourcefile_t *src);
bool pass_dump(sourcefile_t *src);
bool pass_showUndefs(sourcefile_t *src);

bool pass_line(void);

uint8_t Asc2Pet(uint8_t c);
uint8_t Asc2Scr(uint8_t c);

/*-----------------------------------*/

extern bool allBytesResolved;

/*-----------------------------------*/

#endif		/* __PASS_H__ */

