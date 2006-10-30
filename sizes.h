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

/*  default sizes  */
#define ARRAYSIZE_IncludePaths		1
#define ARRAYSIZE_CmdLineDefines	1
#define ARRAYSIZE_SourceFiles		1
#define ARRAYSIZE_Files			2
#define ARRAYSIZE_Lines			4
#define ARRAYSIZE_LineChars		4
#define ARRAYSIZE_Macros		2
#define ARRAYSIZE_MacroParameters	1
#define ARRAYSIZE_String		2
#define ARRAYSIZE_Term			2

/*  amiga(unexpanded) sizes
#define ARRAYSIZE_IncludePaths		1
#define ARRAYSIZE_CmdLineDefines	1
#define ARRAYSIZE_SourceFiles		1
#define ARRAYSIZE_Files			2
#define ARRAYSIZE_Lines			4
#define ARRAYSIZE_LineChars		2
#define ARRAYSIZE_Macros		1
#define ARRAYSIZE_MacroParameters	1
#define ARRAYSIZE_String		2
#define ARRAYSIZE_Term			1
*/

/*  c64 sizes
#define ARRAYSIZE_IncludePaths		1
#define ARRAYSIZE_CmdLineDefines	1
#define ARRAYSIZE_SourceFiles		1
#define ARRAYSIZE_Files			1
#define ARRAYSIZE_Lines			2
#define ARRAYSIZE_LineChars		1
#define ARRAYSIZE_Macros		1
#define ARRAYSIZE_MacroParameters	1
#define ARRAYSIZE_String		2
#define ARRAYSIZE_Term			1
*/

/*-----------------------------------*/
/* End of config                     */
/*-----------------------------------*/

#if ARRAYSIZE_IncludePaths==1
typedef uint8_t		includePathscnt_t;
#else
#if ARRAYSIZE_IncludePaths==2
typedef uint16_t	includePathscnt_t;
#else
#if ARRAYSIZE_IncludePaths==4
typedef uint32_t	includePathscnt_t;
#else
#error "ARRAYSIZE_IncludePaths has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_SourceFiles==1
typedef uint8_t		srcFileNamescnt_t;
#else
#if ARRAYSIZE_SourceFiles==2
typedef uint16_t	srcFileNamescnt_t;
#else
#if ARRAYSIZE_SourceFiles==4
typedef uint32_t	srcFileNamescnt_t;
#else
#error "ARRAYSIZE_SourceFiles has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_Files==1
typedef uint8_t		filescnt_t;
#else
#if ARRAYSIZE_Files==2
typedef uint16_t	filescnt_t;
#else
#if ARRAYSIZE_Files==4
typedef uint32_t	filescnt_t;
#else
#error "ARRAYSIZE_Files has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_Lines==1
typedef uint8_t		linescnt_t;
#else
#if ARRAYSIZE_Lines==2
typedef uint16_t	linescnt_t;
#else
#if ARRAYSIZE_Lines==4
typedef uint32_t	linescnt_t;
#else
#error "ARRAYSIZE_Lines has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_LineChars==1
typedef uint8_t		linesize_t;
#else
#if ARRAYSIZE_LineChars==2
typedef uint16_t	linesize_t;
#else
#if ARRAYSIZE_LineChars==4
typedef uint32_t	linesize_t;
#else
#error "ARRAYSIZE_LineChars has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_Macros==1
typedef uint8_t		macro_cnt;
#else
#if ARRAYSIZE_Macros==2
typedef uint16_t	macro_cnt;
#else
#if ARRAYSIZE_Macros==4
typedef uint32_t	macro_cnt;
#else
#error "ARRAYSIZE_Macros has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_MacroParameters==1
typedef uint8_t		macroparam_cnt;
#else
#if ARRAYSIZE_MacroParameters==2
typedef uint16_t	macroparam_cnt;
#else
#if ARRAYSIZE_MacroParameters==4
typedef uint32_t	macroparam_cnt;
#else
#error "ARRAYSIZE_MacroParameters has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_String==1
typedef uint8_t		stringsize_t;
#else
#if ARRAYSIZE_String==2
typedef uint16_t	stringsize_t;
#else
#if ARRAYSIZE_String==4
typedef uint32_t	stringsize_t;
#else
#error "ARRAYSIZE_String has illegal value!"
#endif
#endif
#endif

#if ARRAYSIZE_Term==1
typedef uint8_t		termsize_t;
#else
#if ARRAYSIZE_Term==2
typedef uint16_t	termsize_t;
#else
#if ARRAYSIZE_Term==4
typedef uint32_t	termsize_t;
#else
#error "ARRAYSIZE_Term has illegal value!"
#endif
#endif
#endif

typedef uint16_t localdepth_t;

typedef uint32_t varcnt_t;

typedef uint32_t termcnt_t;

typedef uint32_t seglistsize_t;

#endif		/* __SIZES_H__ */
