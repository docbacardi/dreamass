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

#ifndef __VARIABLE_H__
#define __VARIABLE_H__


#include "da_stdint.h"
#include <stdio.h>

#define no_var (varcnt_t)-1

#ifndef sgn
#define sgn(a) (a==0?0:(a>0?1:-1))
#endif /*  sgn  */

/*-----------------------------------*/

typedef enum
{
	VALTYP_NUM,
	VALTYP_STR
} VALTYP;

/*
 * this struct contains the complete information to
 * access a variable in the vartree.
 * varidx is the index in the table representing the
 * tree and localidx is the local block's index in the
 * according locals table
 */
typedef struct
{
	varcnt_t varidx;
	localdepth_t localidx;
} VARADR;


typedef struct
{
	VALTYP typ;			/*   type of this variable (numeric or string)  */
        uint32_t byteSize;		/*   size of value in bytes (important for forced sizes)  */
	union
	{
		int32_t num;
		stringsize_t *str;
	} value;
} value_t;


typedef struct
{
	bool defined;
	bool used;
	bool final;
	bool readwrite;
	seglistsize_t segidx;
	value_t valt;
} VARIABLE;


typedef struct
{
	bool err;
	VARIABLE var;
} VARERR;

/*-----------------------------------*/

void variable_zero(void);
bool new_variable(varcnt_t nmemb );
void del_variable(void);

/*   write a value to a variable which is specified by the name  */
VARADR writeVariable(const stringsize_t *name, const VARIABLE valt, const bool allowRedefine);
/*   write a value to a variable which is specified by the address  */
void writeVarAdr(const VARADR adr, const VARIABLE invar);

VARERR readVariable(const stringsize_t *name);

bool getRMWPointer(const stringsize_t *name, VARADR *adr, VARIABLE **var);

/*   get name  */
const stringsize_t *getVarName(varcnt_t varidx);

/*   test if variable exists  */
bool existVariable(const stringsize_t *name);
bool existAndDefinedVariable(const stringsize_t *name);

bool resolvedSomething(void);
void resetResolveCounter(void);

bool dump(FILE *dfh, localdepth_t level, bool useHtml);
/*  void tdump(FILE *dfh);  */

localdepth_t localDive(void);
localdepth_t localUp(void);
localdepth_t getLocalBlock(void);
void setLocalBlock(localdepth_t bidx);

void setIndex( localdepth_t index );

uint32_t value_getRealSize(int32_t val);

/*   delete the datastructures of a variable, (e.g. for strings the string array)  */
void deleteVariable(VARIABLE *var);

/*   copy a variable with respects to the datastructures (e.g. for strings the string array)  */
void copyVariable(VARIABLE *var);

/*-----------------------------------*/

#endif		/* __VARIABLE_H__ */

