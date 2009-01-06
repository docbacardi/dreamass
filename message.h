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

#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "da_stdint.h"

/*-----------------------------------*/

typedef enum
{
	EM_OutOfMemory,
	EM_TooManyFiles,
	EM_FileNotFound_s,
	EM_ReadError_s,
	EM_NoMacroName,
	EM_ExpectingMacroName_c,
	EM_NoClosingBracket,
	EM_ExpectingParameterName_c,
	EM_UnknownParameter_s,
	EM_ExpectingCOBracket,
	EM_ExpectingCCBracket,
	EM_HexWithoutNumber,
	EM_NoEndingQuotes,
	EM_BinWithoutNumber,
	EM_UnknownPsyOpcodeOrMacro,
	EM_TooManyLines,
	EM_GarbageAtEndOfLine,
	EM_NumberWithoutOperand,
	EM_StringWithoutOperand,
	EM_ExpectingVarAssignment,
	EM_AssignmentHasNoRightSide,
	EM_VarRedefinition_s,
	EM_DSBWithoutArg,
	EM_DBNothingAfterComma,
	EM_DWNothingAfterComma,
	EM_SyntaxErrorAfterPC,
	EM_OperandAtStartOfCommand,
	EM_UnknownAdressmode,
	EM_IllAdressmodeForOpcode,
	EM_BranchOutOfRange,
	EM_Overflow_d,
	EM_StringTooLongForNumber,
	EM_PsyInTerm,
	EM_OpcInTerm,
	EM_TermTooLong,
	EM_DivByZero,
	EM_IfStillOpen,
	EM_ElseWithoutIf,
	EM_MaxDepth,
	EM_MissingArgForPreProc,
	EM_PPArgUndefd,
	EM_ElsifWithoutIf,
	EM_EndifWithoutIf,
	EM_MissingFormatString,
	EM_TooManyParameters,
	EM_TooManyMacros,
	EM_CreateFile_s,
	EM_WriteError_s,
	EM_MacroParameterNumber_d_d,
	EM_PsyInPrint,
	EM_PreInPrint,
	EM_MneInPrint,
	EM_TermParse,
	EM_MacroTooDeep,
	EM_AlignWithoutArg,
	EM_EmptyString,
	EM_StringTooLong,
	EM_VarAtRightAssignment,
	EM_Redefinition,
	EM_TooManyVariables,
	EM_TooManyLocalBlocks,
	EM_NoOpeningLocalBlock,
	EM_MissingFilename,
	EM_NoArgAfterComma,
	EM_OffsExcFilelen,
	EM_LenExcFilelen,
	EM_AtWithoutLabelname,
	EM_LevelExceedsDepth,
	EM_TooManySegElems,
	EM_RangeHasNoEnd,
	EM_RangeStartGrEqEnd,
	EM_UnknownSegAttrib,
	EM_SegDefNoName,
	EM_SegDefNoRange,
	EM_SegDefZpOverZeropage,
	EM_TypeMismatch,
	EM_NoSegmentName,
	EM_UnknownFileAttrib,
	EM_TooManySegments,
	EM_FileDefNoSegments,
	EM_TooManyOutfiles,
	EM_UnknownSegment,
	EM_SegmentsOverlapping_d_d_d_d,
	EM_VarUndef_s,
	EM_SegElemMem_s,
	EM_UndefVarInSegSwitch,
	EM_UndefSegment_s,
	EM_UnboundDephase,
	EM_TooManyNestedPhases,
	EM_PseudoPCStillOpenAtEnd,
	EM_LocalBlockStillOpenAtEnd,
	EM_PseudopcWithoutArg,
	EM_RegSizeWithoutArg,
	EM_RegSizeNotDefinedOrNotFinal,
	EM_IllegalRegSizeArg_d,
	EM_RegSize16NotAllowed_s,
	EM_CpuWithoutArg,
	EM_UnknownCpuType_s,
	EM_MacroRedefinition_s,
	EM_ReservedMacroName,
	EM_UnknownPreproc,
	EM_SystemError
} ERRORMSG;


#define W_User				0x00000001
#define W_UnknownLabelForcedSize	0x00000002
#define W_AddressmodeForcedSize		0x00000004
#define W_BinaryIncludeTooLong		0x00000008
#define W_SegdefOptionTwice		0x00000010
#define W_OutfileOptionTwice		0x00000020
#define W_IndirectJmp			0x00000040
#define W_PhaseIdxShift			0x00000080
#define W_LocalBlockOpen		0x00000100

#define W_Normal			W_User | W_BinaryIncludeTooLong | W_SegdefOptionTwice | W_OutfileOptionTwice | W_IndirectJmp | W_PhaseIdxShift | W_LocalBlockOpen


typedef enum
{
	WM_ForcedLabelSize	= 0,
	WM_ByteToWordExpansion,
	WM_BIncOver64k,
	WM_SegDefOptTwice_s,
	WM_FileDefOptTwice_s,
	WM_JmpIndAtFF_d,
	WM_PhaseIdxShift,
	WM_LocalBlockOpen
} WARNMSG;

typedef struct
{
	const char *pcFlag;
	const unsigned long ulValue;
	const unsigned long ulMask;
} warnFlag_t;

extern const warnFlag_t warnFlags[12];

/*-----------------------------------*/

void systemError(ERRORMSG msg, ...);
void scanError(ERRORMSG msg, filescnt_t fileidx, linescnt_t linenr, ...);
void error(ERRORMSG msg, ...);
void warning(WARNMSG msg, ...);

/*-----------------------------------*/

#endif		/* __MESSAGES_H__ */
