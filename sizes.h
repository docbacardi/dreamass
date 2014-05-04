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
/*
 * sizes.h
 *
 * Here you can adjust the maximum sizes for all arrays.
 *
 */

#ifndef __SIZES_H__
#define __SIZES_H__

/*-----------------------------------*/
/* Start of config                   */
/*-----------------------------------*/

/*
 * ARRAYSIZE_IncludePaths:
 * Maximum number of include paths which can be passed via commandline
 * default is $100 which means 1 byte
 *
 * ARRAYSIZE_SourceFiles:
 * Maximum number of source files which can be passed via commandline
 * default is $100 which means 1 byte
 *
 * ARRAYSIZE_Files:
 * Maximum number of files
 * default is $10000 which means 2 byte
 *
 * ARRAYSIZE_Lines:
 * Maximum number of lines for one file
 * default is $100000000 which means 4 byte
 *
 * ARRAYSIZE_LineChars:
 * Maximum number of chars per line
 * default is $100000000 which means 4 byte
 *
 * ARRAYSIZE_Macros:
 * Maximum number of macros
 * default is $10000 which means 2 byte
 *
 * ARRAYSIZE_MacroParameters:
 * Maximum number of parameters for one macro
 * default is $100 which means 1 byte
 *
 * ARRAYSIZE_String:
 * Maximum number of characters in one string or binary include
 * default is $10000 which means 2 byte
 *
 * ARRAYSIZE_Term:
 * Maximum number of elements in one term. An element is a number or an operator.
 * default is $10000 which means 2 byte
 */


typedef size_t includePathscnt_t;

typedef size_t srcFileNamescnt_t;

typedef size_t filescnt_t;

typedef size_t linescnt_t;

typedef size_t linesize_t;

typedef size_t macro_cnt;

typedef size_t macroparam_cnt;

typedef size_t stringsize_t;

typedef size_t termsize_t;



typedef size_t localdepth_t;

typedef size_t varcnt_t;

typedef size_t termcnt_t;

typedef size_t seglistsize_t;

#endif		/* __SIZES_H__ */
