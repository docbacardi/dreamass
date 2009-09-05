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

#ifndef __OPCODES_H__
#define __OPCODES_H__

/*-------------------------------------
 Here are all the opcodes. The enum OPCODE
 and the table mne_keys must be aligned,
 that mne_keys[MNE_ADC] equals MNE_CODE('a','d','c')
-------------------------------------*/

#define MNE_CODE(a,b,c) (a&0x1f)|(b&0x1f)<<5|(c&0x3f)<<10

typedef enum
{
	MNE_ACN = 0x00,
	MNE_ADC = 0x01,
	MNE_AHX = 0x02,
	MNE_ALR = 0x03,
	MNE_ANC = 0x04,
	MNE_AND = 0x05,
	MNE_ARR = 0x06,
	MNE_ASL = 0x07,
	MNE_AXS = 0x08,

	MNE_BCC = 0x09,
	MNE_BCS = 0x0a,
	MNE_BEQ = 0x0b,
	MNE_BIT = 0x0c,
	MNE_BMI = 0x0d,
	MNE_BNE = 0x0e,
	MNE_BPL = 0x0f,
	MNE_BRA = 0x10,
	MNE_BRK = 0x11,
	MNE_BRL = 0x12,
	MNE_BVC = 0x13,
	MNE_BVS = 0x14,

	MNE_CLC = 0x15,
	MNE_CLD = 0x16,
	MNE_CLI = 0x17,
	MNE_CLV = 0x18,
	MNE_CMP = 0x19,
	MNE_COP = 0x1a,
	MNE_CPX = 0x1b,
	MNE_CPY = 0x1c,

	MNE_DCP = 0x1d,
	MNE_DEC = 0x1e,
	MNE_DEX = 0x1f,
	MNE_DEY = 0x20,

	MNE_EOR = 0x21,

	MNE_INC = 0x22,
	MNE_INX = 0x23,
	MNE_INY = 0x24,
	MNE_ISC = 0x25,

	MNE_JML = 0x26,
	MNE_JMP = 0x27,
	MNE_JSL = 0x28,
	MNE_JSR = 0x29,

	MNE_KI0 = 0x2a,
	MNE_KI1 = 0x2b,
	MNE_KI2 = 0x2c,
	MNE_KI3 = 0x2d,
	MNE_KI4 = 0x2e,
	MNE_KI5 = 0x2f,
	MNE_KI6 = 0x30,
	MNE_KI7 = 0x31,
	MNE_KI9 = 0x32,
	MNE_KIB = 0x33,
	MNE_KID = 0x34,
	MNE_KIF = 0x35,

	MNE_LAS = 0x36,
	MNE_LAX = 0x37,
	MNE_LDA = 0x38,
	MNE_LDX = 0x39,
	MNE_LDY = 0x3a,
	MNE_LSR = 0x3b,

	MNE_MVN = 0x3c,
	MNE_MVP = 0x3d,

	MNE_NO0 = 0x3e,
	MNE_NO2 = 0x3f,
	MNE_NO4 = 0x40,
	MNE_NO6 = 0x41,
	MNE_NO8 = 0x42,
	MNE_NOC = 0x43,
	MNE_NOE = 0x44,
	MNE_NOP = 0x45,
	MNE_NOX = 0x46,
	MNE_NOY = 0x47,

	MNE_ORA = 0x48,

	MNE_PEA = 0x49,
	MNE_PEI = 0x4a,
	MNE_PER = 0x4b,
	MNE_PHA = 0x4c,
	MNE_PHB = 0x4d,
	MNE_PHD = 0x4e,
	MNE_PHK = 0x4f,
	MNE_PHP = 0x50,
	MNE_PHX = 0x51,
	MNE_PHY = 0x52,
	MNE_PLA = 0x53,
	MNE_PLB = 0x54,
	MNE_PLD = 0x55,
	MNE_PLP = 0x56,
	MNE_PLX = 0x57,
	MNE_PLY = 0x58,

	MNE_REP = 0x59,
	MNE_RLA = 0x5a,
	MNE_ROL = 0x5b,
	MNE_ROR = 0x5c,
	MNE_RRA = 0x5d,
	MNE_RTI = 0x5e,
	MNE_RTL = 0x5f,
	MNE_RTS = 0x60,

	MNE_SAX = 0x61,
	MNE_SBC = 0x62,
	MNE_SBI = 0x63,
	MNE_SEC = 0x64,
	MNE_SED = 0x65,
	MNE_SEI = 0x66,
	MNE_SEP = 0x67,
	MNE_SHX = 0x68,
	MNE_SLO = 0x69,
	MNE_SRE = 0x6a,
	MNE_STA = 0x6b,
	MNE_STP = 0x6c,
	MNE_STX = 0x6d,
	MNE_STY = 0x6e,
	MNE_STZ = 0x6f,

	MNE_TAS = 0x70,
	MNE_TAX = 0x71,
	MNE_TAY = 0x72,
	MNE_TCD = 0x73,
	MNE_TCS = 0x74,
	MNE_TDC = 0x75,
	MNE_TRB = 0x76,
	MNE_TSB = 0x77,
	MNE_TSC = 0x78,
	MNE_TSX = 0x79,
	MNE_TXA = 0x7a,
	MNE_TXS = 0x7b,
	MNE_TXY = 0x7c,
	MNE_TYA = 0x7d,
	MNE_TYX = 0x7e,

	MNE_WAI = 0x7f,
	MNE_WDM = 0x80,

	MNE_XAA = 0x81,
	MNE_XBA = 0x82,
	MNE_XCE = 0x83
} OPCODE;


typedef enum
{
	AK_NONE		= 0x0000,
	AK_CARRO	= 0x0001,
	AK_BOPEN	= 0x0002,
	AK_SBOPEN	= 0x0004,
	AK_NUMZP	= 0x0008,
	AK_NUMABS	= 0x0010,
	AK_NUMLONG	= 0x0020,
	AK_COMMAS	= 0x0040,
	AK_COMMAZP	= 0x0080,
	AK_BCLOSE1	= 0x0100,
	AK_SBCLOSE	= 0x0200,
	AK_COMMAY	= 0x0400,
	AK_COMMAX	= 0x0800,
	AK_BCLOSE2	= 0x1000,

	AK_ERROR	= 0xffff
} ADRESSKEY;


typedef enum
{
	AM_None		= 0x00,		/*  -         */
	AM_CaZp		= 0x01,		/*  #Zp       */
	AM_CaAbs	= 0x02,		/*  #Abs      */
	AM_Zp		= 0x03,		/*  Zp        */
	AM_ZpCoS	= 0x04,		/*  Zp,s      */
	AM_ZpCoX	= 0x05,		/*  Zp,x      */
	AM_ZpCoY	= 0x06,		/*  Zp,y      */
	AM_ZpCoZp	= 0x07,		/*  Zp,Zp     */
	AM_BrZpBr	= 0x08,		/*  (Zp)      */
	AM_BrZpCoXBr	= 0x09,		/*  (Zp,x)    */
	AM_BrZpCoSBrCoY	= 0x0a,		/*  (Zp,s),y  */
	AM_BrZpBrCoY	= 0x0b,		/*  (Zp),y    */
	AM_SbrZpSbr	= 0x0c,		/*  [Zp]      */
	AM_SbrZpSbrCoY	= 0x0d,		/*  [Zp],y    */
	AM_Abs		= 0x0e,		/*  Abs       */
	AM_AbsCoX	= 0x0f,		/*  Abs,x     */
	AM_AbsCoY	= 0x10,		/*  Abs,y     */
	AM_BrAbsBr	= 0x11,		/*  (Abs)     */
	AM_BrAbsCoXBr	= 0x12,		/*  (Abs,x)   */
	AM_Long		= 0x13,		/*  Long      */
	AM_LongCoX	= 0x14,		/*  Long,x    */
	AM_Rel		= 0x15,		/*  Rel       */
	AM_RelLong	= 0x16,		/*  RelLong   */
    AM_length	= 0x17,			/*  no real addressmode, this shows the length of the amode_offs table  */

	AM_Error	= 0xff		/*  An Error occured  */
} ADRESSMODE;


typedef enum
{
	AMB_Empty		= 0x00000000,
	AMB_None		= 0x00000001,
	AMB_CaZp		= 0x00000002,
	AMB_CaAbs		= 0x00000004,
	AMB_Zp			= 0x00000008,
	AMB_ZpCoS		= 0x00000010,
	AMB_ZpCoX		= 0x00000020,
	AMB_ZpCoY		= 0x00000040,
	AMB_ZpCoZp		= 0x00000080,
	AMB_BrZpBr		= 0x00000100,
	AMB_BrZpCoXBr		= 0x00000200,
	AMB_BrZpCoSBrCoY	= 0x00000400,
	AMB_BrZpBrCoY		= 0x00000800,
	AMB_SbrZpSbr		= 0x00001000,
	AMB_SbrZpSbrCoY		= 0x00002000,
	AMB_Abs			= 0x00004000,
	AMB_AbsCoX		= 0x00008000,
	AMB_AbsCoY		= 0x00010000,
	AMB_BrAbsBr		= 0x00020000,
	AMB_BrAbsCoXBr		= 0x00040000,
	AMB_Long		= 0x00080000,
	AMB_LongCoX		= 0x00100000,
	AMB_Rel			= 0x00200000,
	AMB_RelLong		= 0x00400000,
	AMB_CaAsize		= 0x00800000,
	AMB_CaXysize		= 0x01000000
} ADRESSMODEBITS;


typedef enum
{
	AC_GREEN	= 0,
	AC_BROWN	= 1,
	AC_RED		= 2,
	AC_SPC0		= 3,
	AC_SPC1		= 4,
	AC_SPC2		= 5,
	AC_DIRECT	= 6
} ADRESSCLASS;


extern const uint16_t mne_keys[132];

typedef struct
{
	ADRESSCLASS adressclass;
	uint8_t offset;
} amode_offset_s;

extern const amode_offset_s amode_offset[132];
extern const uint8_t amode_offs_groups[AM_length*6];

extern const uint8_t amode_length[23];


typedef enum
{
	CPUTYPE_6510		= 0x00,
	CPUTYPE_6510Ill		= 0x01,
	CPUTYPE_65816		= 0x02,

	CPUTYPE_UNKNOWN		= 0xff
} CPUTYPE;

typedef struct
{
	const char *name;
	CPUTYPE id;
	const uint32_t *validmodes;
} cpuTypeNamePair_t;

extern const cpuTypeNamePair_t cputype_name[3];


#endif		/* __OPCODES_H__ */

