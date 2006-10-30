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


#ifndef __SRCSTACK_H__
#define __SRCSTACK_H__

#include "da_stdint.h"

typedef uint16_t srcstackcnt_t;

void srcstack_zero(void);
bool srcstack_init(srcstackcnt_t nmemb);
void srcstack_cleanUp(void);
bool srcstack_push( sourcefile_t *src );
sourcefile_t *srcstack_pull(void);
sourcefile_t *srcstack_peek(void);

#endif		/* __SRCSTACK_H__ */

