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

#ifndef __PSYOPCODE_H__
#define __PSYOPCODE_H__

#include "da_stdint.h"

typedef enum
{
	PSY_DB			= 0,
	PSY_DW			= 1,
	PSY_TEXT		= 2,
	PSY_PET			= 3,
	PSY_SCR			= 4,
	PSY_SETPET		= 5,
	PSY_SETSCR		= 6,
	PSY_DSB			= 7,
	PSY_ALIGN		= 8,
	PSY_BLOCKSTART		= 9,
	PSY_BLOCKEND		= 10,
	PSY_BINCLUDE		= 11,
	PSY_SEGMENT		= 12,
	PSY_PSEUDOPC		= 13,
	PSY_REALPC		= 14,
	PSY_ASIZE		= 15,
	PSY_XYSIZE		= 16,
	PSY_CPU			= 17,
	PSY_DEBUGCMD		= 18
} PSYOPC;


typedef struct
{
	char *key;
	uint8_t keylen;
	PSYOPC idx;
} psyopc_s;


extern const psyopc_s psyopc[24];

void reset_TextMode(void);
elength_t pass_psyopc(void);

#endif		/* __PSYOPCODE_H__ */

