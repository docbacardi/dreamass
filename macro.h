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


#ifndef __MACRO_H__
#define __MACRO_H__

#include "da_stdint.h"

typedef union
{
	char *line;
	macroparam_cnt pidx;
} mline_data_t;



/*
 * mline_t is one element in a macro. this can be:
 * 1) plain text which ends with '\n' or before a prarmeter
 * 2) a parameter, which is identified by 0 length and it's index
 */
typedef struct
{
	linesize_t textLength;		/*  0 means Parameter  */
	mline_data_t data;
} mline_t;

/*
 * macro_t is a structure for the name and an array of all
 * mline_t elements in this macro.
 */
typedef struct
{
	bool isActive;
	stringsize_t *sname;
	macroparam_cnt parameter_count;
	linescnt_t mlines_count;
	mline_t *mlines;
} macro_t;

void macro_zero(void);
bool macro_init(macro_cnt nmemb);
void macro_cleanUp(void);

macro_cnt macro_add(char *mname, linesize_t mname_len);
linescnt_t macro_addLine(macro_cnt macroIdx, mline_data_t data, linesize_t line_len, linescnt_t mlines_buflen);
bool macro_finalize(macro_cnt macroIdx, linescnt_t mlines_buflen);
bool macro_activate(macro_cnt macroIdx);

macro_cnt macro_find(const char *name, linesize_t mname_len);
macro_t *macro_get(macro_cnt macroIdx);

#endif		/* __MACRO_H__ */

