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


#ifndef __TERM_H__
#define __TERM_H__

/*-----------------------------------*/

#define no_term 0xffff

/*-----------------------------------*/

typedef enum
{
	TT_CONST_NUM,		/*   constant number              */
	TT_CONST_STR,		/*   constant string              */
	TT_VARIABLE,		/*   a variable or term           */
	TT_PC,			/*   program counter              */
	TT_FUNCTION		/*   operand like '+' or 'IsDef'  */
} TERMELEM_TYP;

typedef enum
{
	TS_1BYTE  = 0,		/*   1 byte numeric term                     */
	TS_2BYTE  = 1,		/*   2 byte numeric term                     */
	TS_3BYTE  = 2,		/*   3 byte numeric term                     */
	TS_4BYTE  = 3,		/*   4 byte numeric term                     */
	TS_RBYTE  = 4,		/*   1 byte relative numeric term            */
	TS_RLBYTE = 5,		/*   2 byte relative numeric term            */
	TS_FLEX   = 6,		/*   1 - 4 byte numeric term                 */
	TS_nBYTE  = 7,		/*   1 - 4 byte numeric term or string term  */
	TS_STRING = 8		/*   string term                             */
} TERMSIZE;

/*-----------------------------------*/

typedef struct
{
	TERMELEM_TYP typ;
	termsize_t up;
	termsize_t left;
	termsize_t right;
	uint32_t byteSize;
	union
	{
		uint32_t constant;
		stringsize_t *string;
		stringsize_t *varname;
		FUNCTION function;
	} data;
} termelem_s;

typedef struct
{
	termelem_s *t;

	termsize_t t_size;
	termsize_t t_alloc;

	termsize_t root;
	termsize_t inspos;

/*  	const lineelement_t *vle;  */
	linesize_t vls;

	uint16_t bracketcnt;
} term;

/*-----------------------------------*/

void termlist_zero(void);
bool termlist_init(termcnt_t nmemb);
void termlist_delete(void);
void del_term(term *that);
void clear_termElement(termelem_s *telem);

bool read_term(TERMSIZE size, VARIABLE *pvar, bool toBinary);
bool term_readVardef(const lineelement_t *preelem, linesize_t linepos_start);
bool term_makePCVar(stringsize_t *varname, linesize_t varpos);
term *getTerm(termcnt_t termidx);
VARERR term_eval(term *that);

bool term_makeGraph(FILE *dfh, termcnt_t termidx);

void term_findUndefs(termcnt_t termidx);

/*-----------------------------------*/

#endif		/* __TERM_H__ */


