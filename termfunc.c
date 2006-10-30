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


/*-----------------------------------*/

#include "globals.h"

/*-----------------------------------*/

typedef value_t (basic_fn)(value_t, value_t, bool*);
typedef VARERR (fn_hl)(term*, termsize_t, termsize_t, basic_fn* );

typedef struct
{
	fn_hl *hl;
	basic_fn *bs;
} fmap_t;

extern fmap_t functions[43];

/*-----------------------------------*/

VARERR term_eval(term *that)
{
	VARERR ve;


	/*   get the result of the term  */
	ve = term_rEval(that, that->root);

	/*   copy datastructs  */
	if( !ve.err )
	{
		copyVariable( &ve.var );
	}

	/*   return the unlinked result  */
	return ve;
}


VARERR term_rEval(term *that, termsize_t n)
{
	VARERR ve = { true, { false, false, false, false, (seglistsize_t)-1, { VALTYP_NUM, 0, { 0 }}}};
	termsize_t le, ri;
	FUNCTION fn;
	fmap_t func;


	if( n==no_term )
		return ve;

	le = that->t[n].left;
	ri = that->t[n].right;

	switch( that->t[n].typ )
	{
	case TT_CONST_NUM:
		ve.err = false;
		ve.var.defined = true;
		ve.var.used = false;
		ve.var.final = true;
		ve.var.readwrite = false;
		ve.var.segidx = segment_getActSegIdx();
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.byteSize = that->t[n].byteSize;
		ve.var.valt.value.num = that->t[n].data.constant;

		/*   Return here to skip 'replacement' of constant with constant after the switch  */
		return ve;

	case TT_CONST_STR:
		ve.err = false;
		ve.var.defined = true;
		ve.var.used = false;
		ve.var.final = true;
		ve.var.readwrite = false;
		ve.var.segidx = segment_getActSegIdx();
		ve.var.valt.typ = VALTYP_STR;
		ve.var.valt.byteSize = that->t[n].byteSize;
		ve.var.valt.value.str = stringClone(that->t[n].data.string);
		break;

	case TT_VARIABLE:
		ve = readVariable(that->t[n].data.varname);
		/*   copy datastructs  */
		if( !ve.err )
		{
			copyVariable( &ve.var );
		}

		break;
	case TT_PC:
		ve.err = false;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.used = true;
		ve.var.final = segment_isPCFinal();
		ve.var.readwrite = false;
		ve.var.segidx = segment_getActSegIdx();
		ve.var.defined = segment_isPCDefined(); 
		if( ve.var.defined ) {
			ve.var.valt.value.num = segment_getPC();
			ve.var.valt.byteSize = value_getRealSize(ve.var.valt.value.num);
		} else {
			ve.var.valt.byteSize = 0;
			ve.var.valt.value.num = 0;
		}
		break;
	case TT_FUNCTION:
		fn = that->t[n].data.function;
		if( fn==FN_BracketClosed )
		{
			assert( le==no_term && ri!=no_term );
			ve = term_rEval(that, ri);
		}
		else
		{
			func = *(functions+(fn&FN_IdxMask));
			ve = func.hl(that, le, ri, func.bs);
		};
		break;
	};

	/*   if var is defined replace node with a constant  */
	if( !ve.err && ve.var.defined && ve.var.final )
	{
		clear_termElement(that->t+n);

		switch( ve.var.valt.typ )
		{
		case VALTYP_NUM:
			that->t[n].typ = TT_CONST_NUM;
			that->t[n].byteSize = ve.var.valt.byteSize;
			that->t[n].data.constant = ve.var.valt.value.num;
			break;

		case VALTYP_STR:
			that->t[n].typ = TT_CONST_STR;
			that->t[n].byteSize = ve.var.valt.byteSize;
			that->t[n].data.string = ve.var.valt.value.str;
			break;
		}
	}

	return ve;
}


/*-----------------------------------*/

value_t bfn_shiftLeft(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num<<r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_shiftRight(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num>>r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t ufn_hiByte(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( r.typ==VALTYP_NUM )
	{
		out.value.num = (r.value.num>>8)&0xff;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_greater(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num>r.value.num)?1:0;
		out.byteSize = 1;
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.value.num = (stringCmp(l.value.str,r.value.str)==1)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t ufn_loByte(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( r.typ==VALTYP_NUM )
	{
		out.value.num = r.value.num&0xff;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_smaller(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num<r.value.num)?1:0;
		out.byteSize = 1;
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.value.num = (stringCmp(l.value.str,r.value.str)==-1)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_boolAnd(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num&&r.value.num)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_bitAnd(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num&r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_boolOr(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num||r.value.num)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_bitOr(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num|r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_bitEor(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num^r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t ufn_not(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};
	const uint32_t eorTab[4] =
	{
		0x000000ff,
		0x0000ffff,
		0x00ffffff,
		0xffffffff
	};


	if( r.typ==VALTYP_NUM )
	{
		/*   limit out byte size to 0-3  */
		out.byteSize = (r.byteSize<=3) ? r.byteSize : 3;

		/*   invert 'out.byteSize' part of value  */
		out.value.num = r.value.num^eorTab[out.byteSize];
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t ufn_posSign(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( r.typ==VALTYP_NUM )
	{
		return r;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
		return out;
	}
}


value_t bfn_add(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.typ = VALTYP_NUM;
		out.value.num = l.value.num+r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.typ = VALTYP_STR;
		if( (out.value.str=(stringsize_t*)malloc(sizeof(stringsize_t)+(*l.value.str)+(*r.value.str)))==NULL )
		{
			*err=true;
			systemError(EM_OutOfMemory);
			return out;
		}
		*out.value.str = (*l.value.str)+(*r.value.str);
		memcpy(out.value.str+1,l.value.str+1,*l.value.str);
		memcpy(((char*)(out.value.str+1))+(*l.value.str),r.value.str+1,*r.value.str);
		out.byteSize = *out.value.str;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t ufn_negSign(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( r.typ==VALTYP_NUM )
	{
		out.value.num = -r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_sub(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num-r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_mult(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = l.value.num*r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_div(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		if( r.value.num==0 )
		{
			error(EM_DivByZero);
			*err=true;
			return out;
		}
		out.value.num = l.value.num/r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_equal(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num==r.value.num)?1:0;
		out.byteSize = 1;
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.value.num = (stringCmp(l.value.str,r.value.str)==0)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_unEqual(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num!=r.value.num)?1:0;
		out.byteSize = 1;
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.value.num = (stringCmp(l.value.str,r.value.str)!=0)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_lessEqual(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num<=r.value.num)?1:0;
		out.byteSize = 1;
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.value.num = (stringCmp(l.value.str,r.value.str)<=0)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_greaterEqual(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		out.value.num = (l.value.num>=r.value.num)?1:0;
		out.byteSize = 1;
	}
	else if( l.typ==VALTYP_STR && r.typ==VALTYP_STR )
	{
		out.value.num = (stringCmp(l.value.str,r.value.str)>=0)?1:0;
		out.byteSize = 1;
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_modulo(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		if( r.value.num==0 )
		{
			error(EM_DivByZero);
			*err=true;
			return out;
		}
		out.value.num = l.value.num%r.value.num;
		out.byteSize = value_getRealSize(out.value.num);
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t bfn_forceSize(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	if( l.typ==VALTYP_NUM && r.typ==VALTYP_NUM )
	{
		/*   forced size must not be negativ  */
		if( r.value.num<0 ) {
			error(EM_Overflow_d, r.value.num);
			*err=true;
		}
		/*   is the forced size smaller than the values size?  */
		else if( ((uint32_t)r.value.num)<l.byteSize ) {
			/*   yes -> error  */
			error(EM_Overflow_d, r.value.num);
			*err=true;
		}
		else {
			out.byteSize = r.value.num;
			out.value.num = l.value.num;
		}
	}
	else
	{
		error(EM_TypeMismatch);
		*err=true;
	}
	return out;
}


value_t ufn_sizeOf(value_t l, value_t r, bool *err)
{
	value_t out = { VALTYP_NUM, 0, { 0 }};


	out.byteSize = 1;
	out.value.num = r.byteSize;
	return out;
}


/*-----------------------------------*/


/*
 * Preunary Functions
 * Parameter:
 *   term *that		-- the term this operator is part of
 *   termsize_t le	-- index to the left operand
 *   termsize_t ri	-- index to the right operand
 *   basic_bfn *basicfn	-- pointer to the basic function
 */
VARERR fn_preunary(term *that, termsize_t le, termsize_t ri, basic_fn *basicufn)
{
	VARERR vr;
	VARERR ve;


	/*   Get the value for the right side  */
	vr = term_rEval(that, ri);

	/*   Evaluate the basic function only if the right side of the operand is ok  */
	/*   and the right side is defined  */
	ve.err = vr.err;
	ve.var.defined = vr.var.defined;
	if( ve.err==false && ve.var.defined==true ) {
		ve.var.valt = basicufn( vr.var.valt, vr.var.valt, &ve.err );
		ve.var.used = false;
		ve.var.final = vr.var.final;
		ve.var.readwrite = false;
		ve.var.segidx = segment_getActSegIdx();
	} else {
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.byteSize = 0;
		ve.var.valt.value.num =  0;
		ve.var.used = false;
		ve.var.final = false;
		ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
	}

	return ve;
}


/*
 * Simple Binary Functions (opposed to assigning ones)
 * Parameter:
 *   term *that		-- the term this operator is part of
 *   termsize_t le	-- index to the left operand
 *   termsize_t ri	-- index to the right operand
 *   basic_bfn *basicfn	-- pointer to the basic function
 */
VARERR fn_sbinary(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR vl, vr;
	VARERR ve;


	/*   Get the values for the left and right side  */
	vl = term_rEval(that, le);
	vr = term_rEval(that, ri);

	/*   Evaluate the basic function only if both sides of the operand are ok  */
	/*   and both sides are defined  */
	ve.err = vl.err||vr.err;
	ve.var.defined = vl.var.defined&&vr.var.defined;
	if( ve.err==false && ve.var.defined==true )
	{
		ve.var.used = false;
		ve.var.final = vl.var.final && vr.var.final;
		ve.var.readwrite = false;
		ve.var.segidx = segment_getActSegIdx();
		ve.var.valt = basicbfn( vl.var.valt, vr.var.valt, &ve.err );
	}
	else
	{
		ve.var.used = false;
		ve.var.final = false;
		ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
		ve.var.valt.value.num =  0;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.byteSize = 0;
	}

	return ve;
}


/*
 * Assigning Binary Functions (opposed to simple ones)
 * Parameter:
 *   term *that		-- the term this operator is part of
 *   termsize_t le	-- index to the left operand
 *   termsize_t ri	-- index to the right operand
 *   basic_afn *basicfn	-- pointer to the basic function
 */
VARERR fn_abinary(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR vl, ve;
	termelem_s *tle;
	lineelement_t vle;
	VARADR vadr;
	length_t undef = { false, 0 };


	memset(&vl, 0, sizeof(VARERR));

	switch( (tle=that->t+le)->typ )
	{
	case TT_VARIABLE:
		vl = readVariable(tle->data.varname);
		break;

	case TT_PC:
		vl.err = false;
		vl.var.valt.typ = VALTYP_NUM;
		vl.var.used = true;
		vl.var.final = segment_isPCFinal();
		vl.var.readwrite = false;
		vl.var.segidx = segment_getActSegIdx();
		vl.var.defined = segment_isPCDefined();
		if( vl.var.defined )
		{
			vl.var.valt.value.num = segment_getPC();
			vl.var.valt.byteSize = value_getRealSize(vl.var.valt.value.num);
		}
		else
		{
			vl.var.valt.byteSize = 0;
			vl.var.valt.value.num = 0;
		}
		break;

	case TT_CONST_NUM:
	case TT_CONST_STR:
	case TT_FUNCTION:
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		vl.err = true;
		break;
	};

	if( vl.err )
	{
		return vl;
	}

	/*   Get the value of the right side to ve  */
	ve = term_rEval(that, ri);
	if( ve.err )
	{
		return ve;
	}

	/*   test if it is ok and defined and if the left side is defined  */
	if( vl.var.defined && ve.var.defined )
	{
		/*   Get the value of the operation  */
		ve.var.valt = basicbfn(vl.var.valt, ve.var.valt, &ve.err);
		if( ve.err )
			return ve;

		/*   insert LE_VARDEF/LE_SEGENTER after the lineelement where the term started (there's the TE_* now)  */

		switch( tle->typ )
		{
		case TT_VARIABLE:
			/*  Check if the values type (ve.var.valt.typ) matches the variable's type (var->valt.typ)  */
			if( vl.var.valt.typ!=ve.var.valt.typ )
			{
				error(EM_TypeMismatch);
				ve.err = true;
				return ve;
			}
			/*   set the actual segment as the point of last modifiaction (segidx)  */
			ve.var.segidx = segment_getActSegIdx();
			/*   set the value of the variable (var)  */
			vadr = writeVariable(tle->data.varname, ve.var, true);
			if( ve.var.final )
			{
				/*   set type of free lineelement to LE_VARDEF  */
				vle.typ = LE_VARDEF;
				/*   set the value of the vardef (vle)  */
				vle.data.vardef.var = ve.var;
				copyVariable(&vle.data.vardef.var);
				/*   Copy the adress for the vardef  */
				vle.data.vardef.varadr = vadr;
				/*   result is a constant, this means read only  */
				vle.data.vardef.var.readwrite = false;
				/*   replace lineelement  */
				pp_replaceVarDef(that->vls++, &vle);
			}
			break;

		case TT_PC:
			/*   PC defs have to be final. I have no idea yet how to handle non-final PC defs :(  */
			assert( ve.var.final );

			/*  It's a PC definition, it has to be a number and not a string  */
			if( ve.var.valt.typ!=VALTYP_NUM )
			{
				error(EM_TypeMismatch);
				ve.err = true;
				return ve;
			}
			/*   get next free lineelement  */
			/*   vle=((lineelement_t*)that->vle)++;  */
			/*   set type of free lineelement to LE_SEGENTER  */
			vle.typ = LE_SEGELEMENTER;
			/*   create new SegElem. Always fixed as '*=' without argument is not allowed inside terms  */
			vle.data.segelemidx = segment_newSegElem(true, ve.var.valt.value.num );
			/*   result is a constant, this means read only  */
			vle.data.vardef.var.readwrite = false;
			/*   replace lineelement  */
			pp_replaceVarDef(that->vls++, &vle);
			break;

		case TT_CONST_NUM:
		case TT_CONST_STR:
		case TT_FUNCTION:
			/*   this should never happen, they should trigger an  */
			/*   error at the beginning of this function  */
			assert( false );
		}
	}
	else
	{
		/*   result of this operation is undefined  */
		ve.var.valt.value.num = 0;
		ve.var.defined = ve.var.used = ve.var.final = ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;

		switch( tle->typ )
		{
		case TT_VARIABLE:
			/*   undefine the variable  */
			vadr = writeVariable(tle->data.varname, ve.var, true);
			break;

		case TT_PC:
			/*   undefine the PC  */
			segment_addLength(undef);
			break;

		case TT_CONST_NUM:
		case TT_CONST_STR:
		case TT_FUNCTION:
			/*   this should never happen, they should trigger an  */
			/*   error at the beginning of this function  */
			assert( false );
		}
	}

	return ve;
}


VARERR fn_assign_r(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn, bool allowReassign)
{
	VARERR ve;
	termelem_s *tle;
	lineelement_t vle;
	VARADR vadr;
	length_t undef = { false, 0 };


	if( (tle=that->t+le)->typ!=TT_VARIABLE && tle->typ!=TT_PC )
	{
		/*   No Variable or PC on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
		return ve;
	}

	/*   Get the value of the right side to ve,  */
	/*   test if it is ok and defined  */
	if( !(ve=term_rEval(that, ri)).err && ve.var.defined )
	{
		/*   insert LE_VARDEF/LE_SEGENTER after the lineelement where the term started (there's the TE_* now)  */

		if( tle->typ==TT_VARIABLE )
		{
			/*   set the actual segment as the point of last modifiaction (segidx)  */
			ve.var.segidx = segment_getActSegIdx();
			/*   set the variable  */
			vadr = writeVariable(tle->data.varname, ve.var, allowReassign);
			/*   make variable unique  */
			copyVariable(&ve.var);
			if( ve.var.final )
			{
				/*   set type of free lineelement to LE_VARDEF  */
				vle.typ = LE_VARDEF;
				/*   set the value of the vardef (vle)  */
				vle.data.vardef.var = ve.var;
				copyVariable(&vle.data.vardef.var);
				/*   Copy the adress for the vardef  */
				vle.data.vardef.varadr = vadr;
				/*   replace lineelement  */
				pp_replaceVarDef(that->vls++, &vle);
			}
		}
		else
		{
			/*   PC defs have to be final. I have no idea yet how to handle non-final PC defs :(  */
			assert( ve.var.final );

			/*   get next free lineelement  */
			/*   vle=((lineelement_t*)that->vle)++;  */
			/*   set type of free lineelement to LE_SEGENTER  */
			vle.typ = LE_SEGELEMENTER;
			/*   create new SegElem. Always fixed as '*=' without argument is not allowed inside terms  */
			vle.data.segelemidx = segment_newSegElem(true, ve.var.valt.value.num );
			/*   replace lineelement  */
			pp_replaceVarDef(that->vls++, &vle);
		}
	}
	else
	{
		/*   result of this operation is undefined  */
		ve.var.defined = ve.var.used = ve.var.final = ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.byteSize = 0;
		ve.var.valt.value.num = 0;

		switch( tle->typ )
		{
		case TT_VARIABLE:
			/*   undefine the variable  */
			vadr = writeVariable(tle->data.varname, ve.var, true);
			break;

		case TT_PC:
			/*   undefine the PC  */
			segment_addLength(undef);
			break;

		case TT_CONST_NUM:
		case TT_CONST_STR:
		case TT_FUNCTION:
			/*   this should never happen, they should trigger an  */
			/*   error at the beginning of this function  */
			assert( false );
		}
	}

	return ve;
}


VARERR fn_assign(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	return fn_assign_r(that, le, ri, basicbfn, cfg_allowSimpleRedefine);
}


VARERR fn_reassign(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	return fn_assign_r(that, le, ri, basicbfn, true);
}


VARERR fn_postinc(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR ve;
	VARIABLE *var, pc;
	termelem_s *tle;
	lineelement_t vle;
	VARADR vadr;
	bool isVar;


	memset(&ve, 0, sizeof(VARERR));

	switch( (tle=that->t+le)->typ )
	{
	case TT_VARIABLE:
		isVar = true;
		if( !getRMWPointer(tle->data.varname, &vadr, &var) )
		{
			ve.err = true;
			return ve;
		}
		break;
	case TT_PC:
		isVar = false;
		pc.valt.typ = VALTYP_NUM;
		pc.defined = segment_isPCDefined();
		pc.valt.value.num = segment_getPC();
		var = &pc;
		break;
	default:
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
		return ve;
	};

	/*  Is left side defined? -> modify left side  */
	if( var!=NULL && (ve.var.defined=var->defined) )
	{
		/*   increment can only work on numbers  */
		if( var->valt.typ!=VALTYP_NUM )
		{
			error(EM_TypeMismatch);
			ve.err = true;
			return ve;
		}
		/*   insert LE_VARDEF after the lineelement where the term started (there's the TE_* now)  */

		if( isVar )
		{
			/*   set the actual segment as the point of last modifiaction (segidx)  */
			var->segidx = segment_getActSegIdx();
			/*   set the returnvalue  */
			ve.var = *var;
			if( ve.var.final )
			{
				/*   get next lineelement  */
				/*  vle=((lineelement_t*)that->vle)++;  */
				/*   it should be empty, set type to LE_VARDEF  */
				vle.typ = LE_VARDEF;
				/*   set the value for the vardef  */
				vle.data.vardef.var = *var;
				/*   Copy the adress  */
				vle.data.vardef.varadr = vadr;
				/*   replace lineelement  */
				pp_replaceVarDef(that->vls++, &vle);
			}
			/*  inc after the assignment  */
			++var->valt.value.num;
		}
		else
		{
			/*   get next lineelement  */
			/*  vle=((lineelement_t*)that->vle)++;  */
			/*   set type of free lineelement to LE_SEGENTER  */
			vle.typ = LE_SEGELEMENTER;
			ve.var.valt=var->valt;
			/*   create new SegElem. Always fixed as '*=' without argument is not allowed inside terms  */
			vle.data.segelemidx = segment_newSegElem(true, var->valt.value.num+1 );
			/*   replace lineelement  */
			pp_replaceVarDef(that->vls++, &vle);
		}
		ve.err = false;
	}
	else
	{
		ve.var.defined = false;
		ve.var.final = false;
		ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
		ve.var.used = false;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.value.num = 0;
		ve.err = false;
	}

	return ve;
}


VARERR fn_preinc(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR ve;
	VARIABLE *var, pc;
	termelem_s *tri;
	lineelement_t vle;
	VARADR vadr;
	bool isVar;


	memset(&ve, 0, sizeof(VARERR));

	switch( (tri=that->t+ri)->typ )
	{
	case TT_VARIABLE:
		isVar = true;
		if( !getRMWPointer(tri->data.varname, &vadr, &var) )
		{
			ve.err = true;
			return ve;
		}
		break;
	case TT_PC:
		isVar = false;
		pc.valt.typ = VALTYP_NUM;
		pc.defined = segment_isPCDefined();
		pc.valt.value.num = segment_getPC();
		var = &pc;
		break;
	default:
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
		return ve;
	};

	/*  Is left and right side defined? -> modify left side  */
	if( var!=NULL && (ve.var.defined=var->defined) )
	{
		/*   increment can only work on numbers  */
		if( var->valt.typ!=VALTYP_NUM )
		{
			error(EM_TypeMismatch);
			ve.err = true;
			return ve;
		}
		/*   insert LE_VARDEF after the lineelement where the term started (there's the TE_* now)  */

		if( isVar )
		{
			/*  inc before assignment  */
			++var->valt.value.num;
			/*   set the actual segment as the point of last modifiaction (segidx)  */
			var->segidx = segment_getActSegIdx();
			/*   set the return value  */
			ve.var = *var;
			if( ve.var.final )
			{
				/*   get next lineelement  */
				/*   vle=((lineelement_t*)that->vle)++;  */
				/*   it should be empty, set type to LE_VARDEF  */
				vle.typ = LE_VARDEF;
				/*   set the value for the vardef  */
				vle.data.vardef.var = *var;
				/*   Copy the adress  */
				vle.data.vardef.varadr = vadr;
				/*   replace lineelement  */
				pp_replaceVarDef(that->vls++, &vle);
			}
		}
		else
		{
			/*   get next lineelement  */
			/*   vle=((lineelement_t*)that->vle)++;  */
			/*   set type of free lineelement to LE_SEGENTER  */
			vle.typ = LE_SEGELEMENTER;
			++var->valt.value.num;
			ve.var.valt = var->valt;
			/*   create new SegElem. Always fixed as '*=' without argument is not allowed inside terms  */
			vle.data.segelemidx = segment_newSegElem(true, var->valt.value.num );
			/*   replace lineelement  */
			pp_replaceVarDef(that->vls++, &vle);
		}
		ve.err = false;
	}
	else
	{
		ve.var.defined = false;
		ve.var.final = false;
		ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
		ve.var.used = false;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.value.num = 0;
		ve.err = false;
	}

	return ve;
}


VARERR fn_postdec(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR ve;
	VARIABLE *var, pc;
	termelem_s *tle;
	lineelement_t vle;
	VARADR vadr;
	bool isVar;


	memset(&ve, 0, sizeof(VARERR));

	switch( (tle=that->t+le)->typ )
	{
	case TT_VARIABLE:
		isVar = true;
		if( !getRMWPointer(tle->data.varname, &vadr, &var) )
		{
			ve.err = true;
			return ve;
		}
		break;
	case TT_PC:
		isVar = false;
		pc.valt.typ = VALTYP_NUM;
		pc.defined = segment_isPCDefined();
		pc.valt.value.num = segment_getPC();
		var = &pc;
		break;
	default:
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
		return ve;
	};

	/*  Is left and right side defined? -> modify left side  */
	if( var!=NULL && (ve.var.defined=var->defined) )
	{
		/*   increment can only work on numbers  */
		if( var->valt.typ!=VALTYP_NUM )
		{
			error(EM_TypeMismatch);
			ve.err = true;
			return ve;
		}
		/*   insert LE_VARDEF after the lineelement where the term started (there's the TE_* now)  */

		if( isVar )
		{
			/*   set the actual segment as the point of last modifiaction (segidx)  */
			var->segidx = segment_getActSegIdx();
			/*   set the return value  */
			ve.var = *var;
			if( ve.var.final )
			{
				/*   get next lineelement  */
				/*   vle=((lineelement_t*)that->vle)++;  */
				/*   it should be empty, set type to LE_VARDEF  */
				vle.typ = LE_VARDEF;
				/*   set the value for the vardef  */
				vle.data.vardef.var = *var;
				/*   Copy the adress  */
				vle.data.vardef.varadr = vadr;
				/*   replace lineelement  */
				pp_replaceVarDef(that->vls++, &vle);
			}
			/*   dec after assignment  */
			--var->valt.value.num;
		}
		else
		{
			/*   get next lineelement  */
			/*   vle=((lineelement_t*)that->vle)++;  */
			/*   set type of free lineelement to LE_SEGENTER  */
			vle.typ = LE_SEGELEMENTER;
			ve.var.valt=var->valt;
			/*   create new SegElem. Always fixed as '*=' without argument is not allowed inside terms  */
			vle.data.segelemidx = segment_newSegElem(true, var->valt.value.num-1 );
			/*   replace lineelement  */
			pp_replaceVarDef(that->vls++, &vle);
		}
		ve.err = false;
	}
	else
	{
		ve.var.defined = false;
		ve.var.final = false;
		ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
		ve.var.used = false;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.value.num = 0;
		ve.err = false;
	}

	return ve;
}


VARERR fn_predec(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR ve;
	VARIABLE *var, pc;
	termelem_s *tri;
	lineelement_t vle;
	VARADR vadr;
	bool isVar;


	memset(&ve, 0, sizeof(VARERR));

	switch( (tri=that->t+ri)->typ )
	{
	case TT_VARIABLE:
		isVar = true;
		if( !getRMWPointer(tri->data.varname, &vadr, &var) )
		{
			ve.err = true;
			return ve;
		}
		break;
	case TT_PC:
		isVar = false;
		pc.valt.typ = VALTYP_NUM;
		pc.defined = segment_isPCDefined();
		pc.valt.value.num = segment_getPC();
		var = &pc;
		break;
	default:
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
		return ve;
	};

	/*   does the left side exist yet and is the right side defined? -> modify left side  */
	if( var!=NULL && var->defined )
	{
		/*   increment can only work on numbers  */
		if( var->valt.typ!=VALTYP_NUM )
		{
			error(EM_TypeMismatch);
			ve.err = true;
			return ve;
		}
		/*   insert LE_VARDEF after the lineelement where the term started (there's the TE_* now)  */
		ve.var.defined = true;

		if( isVar )
		{
			/*   dec before assignment  */
			--var->valt.value.num;
			/*   set the actual segment as the point of last modifiaction (segidx)  */
			var->segidx = segment_getActSegIdx();
			/*   set the return value  */
			ve.var = *var;
			if( ve.var.final )
			{
				/*   get next lineelement  */
				/*   vle=((lineelement_t*)that->vle)++;  */
				/*   it should be empty, set type to LE_VARDEF  */
				vle.typ = LE_VARDEF;
				/*   set the value for the vardef  */
				vle.data.vardef.var = *var;
				/*   Copy the adress  */
				vle.data.vardef.varadr = vadr;
				/*   replace lineelement  */
				pp_replaceVarDef(that->vls++, &vle);
			}
		}
		else
		{
			/*   get next lineelement  */
			/*   vle=((lineelement_t*)that->vle)++;  */
			/*   set type of free lineelement to LE_SEGENTER  */
			vle.typ = LE_SEGELEMENTER;
			--var->valt.value.num;
			ve.var.valt = var->valt;
			/*   create new SegElem. Always fixed as '*=' without argument is not allowed inside terms  */
			vle.data.segelemidx = segment_newSegElem(true, var->valt.value.num );
			/*   replace lineelement  */
			pp_replaceVarDef(that->vls++, &vle);
		}
		ve.err = false;
	}
	else
	{
		ve.var.defined = false;
		ve.var.final = false;
		ve.var.readwrite = false;
		ve.var.segidx = (seglistsize_t)-1;
		ve.var.used = false;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.value.num = 0;
		ve.err = false;
	}

	return ve;
}

VARERR fn_isDef(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR ve;
	bool isDef;
	termelem_s *tri;


	memset(&ve, 0, sizeof(VARERR));

	if( (tri=that->t+ri)->typ!=TT_VARIABLE && tri->typ!=TT_PC )
	{
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
	}
	else
	{
		/*   Get the variable  */
		if( tri->typ==TT_VARIABLE )
		{
			if( (ve=readVariable(tri->data.varname)).err )
				return ve;
			isDef = ve.var.defined;
		}
		else
			isDef = segment_isPCDefined();
		
		ve.err = false;
		ve.var.defined = true;
		ve.var.used = false;
		ve.var.valt.typ = VALTYP_NUM;
		ve.var.valt.value.num = isDef?1:0;
	}

	return ve;
}


VARERR fn_segmentOf(term *that, termsize_t le, termsize_t ri, basic_fn *basicbfn)
{
	VARERR ve;
	termelem_s *tri;
	stringsize_t *name;


	memset(&ve, 0, sizeof(VARERR));

	if( (tri=that->t+ri)->typ!=TT_VARIABLE && tri->typ!=TT_PC )
	{
		/*   No Variable on the left side of the assignment  */
		error(EM_VarAtRightAssignment);
		ve.err = true;
	}
	else
	{
		/*   Get the variable  */
		if( tri->typ==TT_VARIABLE )
		{
			if( (ve=readVariable(tri->data.varname)).err )
				return ve;
			if( ve.var.segidx!=(seglistsize_t)-1 )
				name = segment_getNameOf(ve.var.segidx);
			else
				name = NULL;
		}
		else
			name = segment_getName();

		ve.err = false;
		ve.var.used = false;
		ve.var.valt.typ = VALTYP_STR;
		if( (ve.var.defined=(name!=NULL)) ) {
			ve.var.valt.value.str = stringClone(name);
		}
	}

	return ve;
}


fmap_t functions[43]=
{
	{ fn_sbinary,		bfn_forceSize },	/*   FN_ForceSize           */
	{ fn_isDef,		NULL },			/*   FN_IsDef               */
	{ fn_segmentOf,		NULL },			/*   FN_SegmentOf           */
	{ fn_preunary,		ufn_sizeOf },		/*   FN_SizeOf              */
	{ fn_preunary,		ufn_not },		/*   FN_BitNot              */
	{ fn_postinc,		NULL },			/*   FN_PostInc             */
	{ fn_preinc,		NULL },			/*   FN_PreInc              */
	{ fn_postdec,		NULL },			/*   FN_PostDec             */
	{ fn_predec,		NULL },			/*   FN_PreDec              */
	{ fn_preunary,		ufn_posSign },		/*   FN_PosSign             */
	{ fn_preunary,		ufn_negSign },		/*   FN_NegSign             */
	{ fn_sbinary,		bfn_mult },		/*   FN_Mult                */
	{ fn_sbinary,		bfn_div },		/*   FN_Div                 */
	{ fn_sbinary,		bfn_modulo },		/*   FN_Modulo              */
	{ fn_sbinary,		bfn_add },		/*   FN_Add                 */
	{ fn_sbinary,		bfn_sub },		/*   FN_Sub                 */
	{ fn_sbinary,		bfn_shiftLeft },	/*   FN_ShiftLeft           */
	{ fn_sbinary,		bfn_shiftRight },	/*   FN_ShiftRight          */
	{ fn_sbinary,		bfn_smaller },		/*   FN_Smaller             */
	{ fn_sbinary,		bfn_lessEqual },	/*   FN_LessEqual           */
	{ fn_sbinary,		bfn_greaterEqual },	/*   FN_GreaterEqual        */
	{ fn_sbinary,		bfn_greater },		/*   FN_Greater             */
	{ fn_sbinary,		bfn_equal },		/*   FN_Equal               */
	{ fn_sbinary,		bfn_unEqual },		/*   FN_Unequal             */
	{ fn_sbinary,		bfn_bitAnd },		/*   FN_BitAnd              */
	{ fn_preunary,		ufn_hiByte },		/*   FN_HiByte              */
	{ fn_preunary,		ufn_loByte },		/*   FN_LoByte              */
	{ fn_sbinary,		bfn_bitEor },		/*   FN_BitEor              */
	{ fn_sbinary,		bfn_bitOr },		/*   FN_BitOr               */
	{ fn_sbinary,		bfn_boolAnd },		/*   FN_BoolAnd             */
	{ fn_sbinary,		bfn_boolOr },		/*   FN_BoolOr              */
	{ fn_assign,		NULL },			/*   FN_Assign              */
	{ fn_reassign,		NULL },			/*   FN_ReAssign            */
	{ fn_abinary,		bfn_mult },		/*   FN_MultReAssign        */
	{ fn_abinary,		bfn_div },		/*   FN_DivReAssign         */
	{ fn_abinary,		bfn_modulo },		/*   FN_ModReAssign         */
	{ fn_abinary,		bfn_add },		/*   FN_AddReAssign         */
	{ fn_abinary,		bfn_sub },		/*   FN_SubReAssign         */
	{ fn_abinary,		bfn_shiftLeft },	/*   FN_ShiftLeftReAssign   */
	{ fn_abinary,		bfn_shiftRight },	/*   FN_ShiftRightReAssign  */
	{ fn_abinary,		bfn_bitAnd },		/*   FN_AndReAssign         */
	{ fn_abinary,		bfn_bitEor },		/*   FN_EorReAssign         */
	{ fn_abinary,		bfn_bitOr }		/*   FN_OrReAssign          */
};

/*-----------------------------------*/

