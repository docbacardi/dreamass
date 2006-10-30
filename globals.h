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


#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "da_stdint.h"
#include "sizes.h"


#ifndef __cplusplus
typedef int bool;
#define true ((int)1)
#define false ((int)0)
/* typedef enum { false=0, true=1 } bool; */
#endif


#ifndef isblank
#define isblank(a) (a==' ' || a=='\t')
#endif

#define arraysize(a) (sizeof(a)/sizeof(a[0]))

typedef struct
{
	bool defined;
	uint32_t len;
} length_t;

typedef struct
{
	bool err;
	length_t len;
} elength_t;

#include "main.h"
#include "opcodes.h"
#include "operand.h"
#include "preproc2.h"
#include "variable.h"
#include "psyopc.h"
#include "segment.h"
#include "macro.h"

#include "linebuf.h"
#include "term.h"
#include "termfunc.h"
#include "srcfile.h"
#include "preproc.h"
#include "srcstack.h"
#include "mne.h"
#include "filelist.h"
#include "pass.h"

#include "message.h"
#include "navi.h"

/*-----------------------------------*/

