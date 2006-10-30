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

#ifndef __OPERAND_H__
#define __OPERAND_H__


#include "da_stdint.h"

/*-------------------------------------
 Here are all the opcodes. The enum OPERAND
 and the tables operator_keys and op_pri must
 be aligned,
 that operator_keys[OP_BRACKETOPEN] equals '('
-------------------------------------*/


typedef enum
{
	OP_Bo		= 0x00,		/*  (          */
	OP_Bc		= 0x01,		/*  )          */
	OP_Sbo		= 0x02,		/*  [          */
	OP_Sbc		= 0x03,		/*  ]          */
	OP_LtLt		= 0x04,		/*  <<         */
	OP_GtGt		= 0x05,		/*  >>         */
	OP_Gt		= 0x06,		/*  >          */
	OP_Lt		= 0x07,		/*  <          */
	OP_Comma	= 0x08,		/*  ,          */
	OP_Carro	= 0x09,		/*  #          */
	OP_Dot		= 0x0a,		/*  .          */
	OP_Colon	= 0x0b,		/*  :          */
	OP_AndAnd	= 0x0c,		/*  &&         */
	OP_And		= 0x0d,		/*  &          */
	OP_OrOr		= 0x0e,		/*  ||         */
	OP_Or		= 0x0f,		/*  |          */
	OP_Not		= 0x10,		/*  ^          */
	OP_Tilde	= 0x11,		/*  ~          */
	OP_Plus		= 0x12,		/*  +          */
	OP_Minus	= 0x13,		/*  -          */
	OP_Star		= 0x14,		/*  *          */
	OP_Slash	= 0x15,		/*  /          */
	OP_EqEq		= 0x16,		/*  ==         */
	OP_ExclEq	= 0x17,		/*  !=         */
	OP_LtEq		= 0x18,		/*  <=         */
	OP_GtEq		= 0x19,		/*  >=         */
	OP_IsDef	= 0x1a,		/*  isdef      */
	OP_SegmentOf	= 0x1b,		/*  segmentof  */
	OP_SizeOf	= 0x1c,		/*  sizeof     */
	OP_Mod		= 0x1d,		/*  mod        */
	OP_Eq		= 0x1e,		/*  =          */
	OP_PlusPlus	= 0x1f,		/*  ++         */
	OP_MinusMinus	= 0x20,		/*  --         */
	OP_DotEq	= 0x21,		/*  :=         */
	OP_PlusEq	= 0x22,		/*  +=         */
	OP_MinusEq	= 0x23,		/*  -=         */
	OP_StarEq	= 0x24,		/*  *=         */
	OP_SlashEq	= 0x25,		/*  /=         */
	OP_modEq	= 0x26,		/*  mod=       */
	OP_AndEq	= 0x27,		/*  &=         */
	OP_NotEq	= 0x28,		/*  ^=         */
	OP_OrEq		= 0x29,		/*  |=         */
	OP_LtLtEq	= 0x2a,		/*  <<=        */
	OP_GtGtEq	= 0x2b		/*  >>=        */
} OPERAND;      

#define FN_IdxMask   0x3f
#define FN_TypMask   0xc0
#define FN_PreUnary  0x40
#define FN_PostUnary 0x80
#define FN_Binary    0xc0

typedef enum
{
	FN_no			= 0x00,

	FN_ForceSize		= 0x00 | FN_Binary,
	FN_IsDef		= 0x01 | FN_PreUnary,
	FN_SegmentOf		= 0x02 | FN_PreUnary,
	FN_SizeOf		= 0x03 | FN_PreUnary,

	FN_BitNot		= 0x04 | FN_PreUnary,
	FN_PostInc		= 0x05 | FN_PostUnary,
	FN_PreInc		= 0x06 | FN_PreUnary,
	FN_PostDec		= 0x07 | FN_PostUnary,
	FN_PreDec		= 0x08 | FN_PreUnary,
	FN_PosSign		= 0x09 | FN_PreUnary,
	FN_NegSign		= 0x0a | FN_PreUnary,

	FN_Mult			= 0x0b | FN_Binary,
	FN_Div			= 0x0c | FN_Binary,
	FN_Modulo		= 0x0d | FN_Binary,

	FN_Add			= 0x0e | FN_Binary,
	FN_Sub			= 0x0f | FN_Binary,

	FN_ShiftLeft		= 0x10 | FN_Binary,
	FN_ShiftRight		= 0x11 | FN_Binary,

	FN_Smaller		= 0x12 | FN_Binary,
	FN_LessEqual		= 0x13 | FN_Binary,
	FN_GreaterEqual		= 0x14 | FN_Binary,
	FN_Greater		= 0x15 | FN_Binary,

	FN_Equal		= 0x16 | FN_Binary,
	FN_Unequal		= 0x17 | FN_Binary,

	FN_BitAnd		= 0x18 | FN_Binary,
	FN_HiByte		= 0x19 | FN_PreUnary,
	FN_LoByte		= 0x1a | FN_PreUnary,

	FN_BitEor		= 0x1b | FN_Binary,

	FN_BitOr		= 0x1c | FN_Binary,

	FN_BoolAnd		= 0x1d | FN_Binary,

	FN_BoolOr		= 0x1e | FN_Binary,


    FN_AssigningStart	= 0x1f,
	FN_Assign		= 0x1f | FN_Binary,
	FN_ReAssign		= 0x20 | FN_Binary,
	FN_MultAssign		= 0x21 | FN_Binary,
	FN_DivAssign		= 0x22 | FN_Binary,
	FN_ModAssign		= 0x23 | FN_Binary,
	FN_AddAssign		= 0x24 | FN_Binary,
	FN_SubAssign		= 0x25 | FN_Binary,
	FN_ShiftLeftAssign	= 0x26 | FN_Binary,
	FN_ShiftRightAssign	= 0x27 | FN_Binary,
	FN_AndAssign		= 0x28 | FN_Binary,
	FN_EorAssign		= 0x29 | FN_Binary,
	FN_OrAssign		= 0x2a | FN_Binary,
    FN_AssigningEnd	= 0x2a,


	FN_BracketOpen		= 0x3e | FN_PreUnary,
	FN_BracketClosed	= 0x3f /* this is not handled as a function but a number */
} FUNCTION;


typedef struct
{
	FUNCTION preUnary;
	FUNCTION postUnary;
	FUNCTION binary;
} op_func_s;

extern const char *operator_key[44];
extern const uint8_t operator_keylen[44];

extern const op_func_s op2fn[44];
extern uint8_t fn_pri[64];

#endif		/* __OPERAND_H__ */

