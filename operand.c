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


#include "operand.h"


const char *operator_key[44] =
{
	"(",
	")",
	"[",
	"]",
	"<<",
	">>",
	">",
	"<",
	",",
	"#",
	".",
	":",
	"&&",
	"&",
	"||",
	"|",
	"^",
	"~",
	"+",
	"-",
	"*",
	"/",
	"==",
	"!=",
	"<=",
	">=",
	"isdef",
	"segmentof",
	"sizeof",
	"mod",
	"=",
	"++",
	"--",
	":=",
	"+=",
	"-=",
	"*=",
	"/=",
	"mod=",
	"&=",
	"^=",
	"|=",
	"<<=",
	">>=",
};

const uint8_t operator_keylen[44] =
{
	1,		/*  "("          */
	1,		/*  ")"          */
	1,		/*  "["          */
	1,		/*  "]"          */
	2,		/*  "<<"         */
	2,		/*  ">>"         */
	1,		/*  ">"          */
	1,		/*  "<"          */
	1,		/*  ","          */
	1,		/*  "#"          */
	1,		/*  "."          */
	1,		/*  ":"          */
	2,		/*  "&&"         */
	1,		/*  "&"          */
	2,		/*  "||"         */
	1,		/*  "|"          */
	1,		/*  "^"          */
	1,		/*  "~"          */
	1,		/*  "+"          */
	1,		/*  "-"          */
	1,		/*  "*"          */
	1,		/*  "/"          */
	2,		/*  "=="         */
	2,		/*  "!="         */
	2,		/*  "<="         */
	2,		/*  ">="         */
	5,		/*  "isdef"      */
	9,		/*  "segmentof"  */
	6,		/*  "sizeof"     */
	3,		/*  "mod"        */
	1,		/*  "="          */
	2,		/*  "++"         */
	2,		/*  "--"         */
	2,		/*  ":="         */
	2,		/*  "+="         */
	2,		/*  "-="         */
	2,		/*  "*="         */
	2,		/*  "/="         */
	3,		/*  "mod="       */
	2,		/*  "&="         */
	2,		/*  "^="         */
	2,		/*  "|="         */
	3,		/*  "<<="        */
	3,		/*  ">>="        */
};


const op_func_s op2fn[44] =
{
	/* pre unary,		post unary,	binary			*/
	{ FN_BracketOpen,	FN_no,		FN_no			},	/*  OP_Bo          */
	{ FN_no,		FN_no,		FN_no			},	/*  OP_Bc          */
	{ FN_no,		FN_no,		FN_no			},	/*  OP_Sbo         */
	{ FN_no,		FN_no,		FN_no			},	/*  OP_Sbc         */
	{ FN_no,		FN_no,		FN_ShiftLeft		},	/*  OP_LtLt        */
	{ FN_no,		FN_no,		FN_ShiftRight		},	/*  OP_GtGt        */
	{ FN_HiByte,		FN_no,		FN_Greater		},	/*  OP_Gt          */
	{ FN_LoByte,		FN_no,		FN_Smaller		},	/*  OP_Lt          */
	{ FN_no,		FN_no,		FN_no			},	/*  OP_Comma       */
	{ FN_no,		FN_no,		FN_no			},	/*  OP_Carro       */
	{ FN_no,		FN_no,		FN_ForceSize		},	/*  OP_Dot         */
	{ FN_no,		FN_no,		FN_no			},	/*  OP_Colon       */
	{ FN_no,		FN_no,		FN_BoolAnd		},	/*  OP_AndAnd      */
	{ FN_no,		FN_no,		FN_BitAnd		},	/*  OP_And         */
	{ FN_no,		FN_no,		FN_BoolOr		},	/*  OP_OrOr        */
	{ FN_no,		FN_no,		FN_BitOr		},	/*  OP_Or          */
	{ FN_no,		FN_no,		FN_BitEor		},	/*  OP_Not         */
	{ FN_BitNot,		FN_no,		FN_no			},	/*  OP_Tilde       */
	{ FN_PosSign,		FN_no,		FN_Add			},	/*  OP_Plus        */
	{ FN_NegSign,		FN_no,		FN_Sub			},	/*  OP_Minus       */
	{ FN_no,		FN_no,		FN_Mult			},	/*  OP_Star        */
	{ FN_no,		FN_no,		FN_Div			},	/*  OP_Slash       */
	{ FN_no,		FN_no,		FN_Equal		},	/*  OP_EqEq        */
	{ FN_no,		FN_no,		FN_Unequal		},	/*  OP_ExclEq      */
	{ FN_no,		FN_no,		FN_LessEqual		},	/*  OP_LtEq        */
	{ FN_no,		FN_no,		FN_GreaterEqual		},	/*  OP_GtEq        */
	{ FN_IsDef,		FN_no,		FN_no			},	/*  OP_IsDef       */
	{ FN_SegmentOf,		FN_no,		FN_no			},	/*  OP_SegmentOf   */
	{ FN_SizeOf,		FN_no,		FN_no			},	/*  Op_SizeOf      */
	{ FN_no,		FN_no,		FN_Modulo		},	/*  OP_mod         */
	{ FN_no,		FN_no,		FN_Assign		},	/*  OP_Eq          */
	{ FN_PreInc,		FN_PostInc,	FN_no			},	/*  OP_PlusPlus    */
	{ FN_PreDec,		FN_PostDec,	FN_no			},	/*  OP_MinusMinus  */
	{ FN_no,		FN_no,		FN_ReAssign		},	/*  OP_DotEq       */
	{ FN_no,		FN_no,		FN_AddAssign		},	/*  OP_PlusEq      */
	{ FN_no,		FN_no,		FN_SubAssign		},	/*  OP_MinusEq     */
	{ FN_no,		FN_no,		FN_MultAssign		},	/*  OP_StarEq      */
	{ FN_no,		FN_no,		FN_DivAssign		},	/*  OP_SlashEq     */
	{ FN_no,		FN_no,		FN_ModAssign		},	/*  OP_modEq       */
	{ FN_no,		FN_no,		FN_AndAssign		},	/*  OP_AndEq       */
	{ FN_no,		FN_no,		FN_EorAssign		},	/*  OP_NotEq       */
	{ FN_no,		FN_no,		FN_OrAssign		},	/*  OP_OrEq        */
	{ FN_no,		FN_no,		FN_ShiftLeftAssign	},	/*  OP_LtLtEq      */
	{ FN_no,		FN_no,		FN_ShiftLeftAssign	},	/*  OP_GtGtEq      */
};


uint8_t fn_pri[64] =
{
	0x01,	/*  FN_ForceSize         */
	0x01,	/*  FN_IsDef             */
	0x01,	/*  FN_SegmentOf         */
	0x01,	/*  FN_SizeOf            */

	0x02,	/*  FN_BitNot            */
	0x02,	/*  FN_PostInc           */
	0x02,	/*  FN_PreInc            */
	0x02,	/*  FN_PostDec           */
	0x02,	/*  FN_PreDec            */
	0x02,	/*  FN_PosSign           */
	0x02,	/*  FN_NegSign           */

	0x03,	/*  FN_Mult              */
	0x03,	/*  FN_Div               */
	0x03,	/*  FN_Modulo            */

	0x04,	/*  FN_Add               */
	0x04,	/*  FN_Sub               */

	0x05,	/*  FN_ShiftLeft         */
	0x05,	/*  FN_ShiftRight        */

	0x06,	/*  FN_Smaller           */
	0x06,	/*  FN_LessEqual         */
	0x06,	/*  FN_GreaterEqual      */
	0x06,	/*  FN_Greater           */

	0x07,	/*  FN_Equal             */
	0x07,	/*  FN_Unequal           */

	0x08,	/*  FN_BitAnd            */
	0x08,	/*  FN_HiByte            */
	0x08,	/*  FN_LoByte            */

	0x09,	/*  FN_BitEor            */

	0x0a,	/*  FN_BitOr             */

	0x0b,	/*  FN_BoolAnd           */

	0x0c,	/*  FN_BoolOr            */

	0x0d,	/*  FN_Assign            */
	0x0d,	/*  FN_ReAssign          */
	0x0d,	/*  FN_MultAssign        */
	0x0d,	/*  FN_DivAssign         */
	0x0d,	/*  FN_ModAssign         */
	0x0d,	/*  FN_AddAssign         */
	0x0d,	/*  FN_SubAssign         */
	0x0d,	/*  FN_ShiftLeftAssign   */
	0x0d,	/*  FN_ShiftRightAssign  */
	0x0d,	/*  FN_AndAssign         */
	0x0d,	/*  FN_EorAssign         */
	0x0d,	/*  FN_OrAssign          */

	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */
	0xff,	/*  empty                */

	/*
	 * FN_Bracket is a special case. As everything should be inserted _below_ the bracket, it has
	 * the lowest possible priority for inserting.
	 */
	0xfe,	/*  FN_BracketOpen       */
	0xfe	/*  FN_BracketClose      */
};

