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

#include <stdint.h>
#include "opcodes.h"


const uint16_t mne_keys[132] =
{
	MNE_CODE('A','C','N'),
	MNE_CODE('A','D','C'),
	MNE_CODE('A','H','X'),
	MNE_CODE('A','L','R'),
	MNE_CODE('A','N','C'),
	MNE_CODE('A','N','D'),
	MNE_CODE('A','R','R'),
	MNE_CODE('A','S','L'),
	MNE_CODE('A','X','S'),

	MNE_CODE('B','C','C'),
	MNE_CODE('B','C','S'),
	MNE_CODE('B','E','Q'),
	MNE_CODE('B','I','T'),
	MNE_CODE('B','M','I'),
	MNE_CODE('B','N','E'),
	MNE_CODE('B','P','L'),
	MNE_CODE('B','R','A'),
	MNE_CODE('B','R','K'),
	MNE_CODE('B','R','L'),
	MNE_CODE('B','V','C'),
	MNE_CODE('B','V','S'),

	MNE_CODE('C','L','C'),
	MNE_CODE('C','L','D'),
	MNE_CODE('C','L','I'),
	MNE_CODE('C','L','V'),
	MNE_CODE('C','M','P'),
	MNE_CODE('C','O','P'),
	MNE_CODE('C','P','X'),
	MNE_CODE('C','P','Y'),

	MNE_CODE('D','C','P'),
	MNE_CODE('D','E','C'),
	MNE_CODE('D','E','X'),
	MNE_CODE('D','E','Y'),

	MNE_CODE('E','O','R'),

	MNE_CODE('I','N','C'),
	MNE_CODE('I','N','X'),
	MNE_CODE('I','N','Y'),
	MNE_CODE('I','S','C'),

	MNE_CODE('J','M','L'),
	MNE_CODE('J','M','P'),
	MNE_CODE('J','S','L'),
	MNE_CODE('J','S','R'),

	MNE_CODE('K','I','0'),
	MNE_CODE('K','I','1'),
	MNE_CODE('K','I','2'),
	MNE_CODE('K','I','3'),
	MNE_CODE('K','I','4'),
	MNE_CODE('K','I','5'),
	MNE_CODE('K','I','6'),
	MNE_CODE('K','I','7'),
	MNE_CODE('K','I','9'),
	MNE_CODE('K','I','B'),
	MNE_CODE('K','I','D'),
	MNE_CODE('K','I','F'),

	MNE_CODE('L','A','S'),
	MNE_CODE('L','A','X'),
	MNE_CODE('L','D','A'),
	MNE_CODE('L','D','X'),
	MNE_CODE('L','D','Y'),
	MNE_CODE('L','S','R'),

	MNE_CODE('M','V','N'),
	MNE_CODE('M','V','P'),

	MNE_CODE('N','O','0'),
	MNE_CODE('N','O','2'),
	MNE_CODE('N','O','4'),
	MNE_CODE('N','O','6'),
	MNE_CODE('N','O','8'),
	MNE_CODE('N','O','C'),
	MNE_CODE('N','O','E'),
	MNE_CODE('N','O','P'),
	MNE_CODE('N','O','X'),
	MNE_CODE('N','O','Y'),

	MNE_CODE('O','R','A'),

	MNE_CODE('P','E','A'),
	MNE_CODE('P','E','I'),
	MNE_CODE('P','E','R'),
	MNE_CODE('P','H','A'),
	MNE_CODE('P','H','B'),
	MNE_CODE('P','H','D'),
	MNE_CODE('P','H','K'),
	MNE_CODE('P','H','P'),
	MNE_CODE('P','H','X'),
	MNE_CODE('P','H','Y'),
	MNE_CODE('P','L','A'),
	MNE_CODE('P','L','B'),
	MNE_CODE('P','L','D'),
	MNE_CODE('P','L','P'),
	MNE_CODE('P','L','X'),
	MNE_CODE('P','L','Y'),

	MNE_CODE('R','E','P'),
	MNE_CODE('R','L','A'),
	MNE_CODE('R','O','L'),
	MNE_CODE('R','O','R'),
	MNE_CODE('R','R','A'),
	MNE_CODE('R','T','I'),
	MNE_CODE('R','T','L'),
	MNE_CODE('R','T','S'),

	MNE_CODE('S','A','X'),
	MNE_CODE('S','B','C'),
	MNE_CODE('S','B','I'),
	MNE_CODE('S','E','C'),
	MNE_CODE('S','E','D'),
	MNE_CODE('S','E','I'),
	MNE_CODE('S','E','P'),
	MNE_CODE('S','H','X'),
	MNE_CODE('S','L','O'),
	MNE_CODE('S','R','E'),
	MNE_CODE('S','T','A'),
	MNE_CODE('S','T','P'),
	MNE_CODE('S','T','X'),
	MNE_CODE('S','T','Y'),
	MNE_CODE('S','T','Z'),

	MNE_CODE('T','A','S'),
	MNE_CODE('T','A','X'),
	MNE_CODE('T','A','Y'),
	MNE_CODE('T','C','D'),
	MNE_CODE('T','C','S'),
	MNE_CODE('T','D','C'),
	MNE_CODE('T','R','B'),
	MNE_CODE('T','S','B'),
	MNE_CODE('T','S','C'),
	MNE_CODE('T','S','X'),
	MNE_CODE('T','X','A'),
	MNE_CODE('T','X','S'),
	MNE_CODE('T','X','Y'),
	MNE_CODE('T','Y','A'),
	MNE_CODE('T','Y','X'),

	MNE_CODE('W','A','I'),
	MNE_CODE('W','D','M'),

	MNE_CODE('X','A','A'),
	MNE_CODE('X','B','A'),
	MNE_CODE('X','C','E'),
};


const uint32_t validModes_6510[132] =
{
	AMB_Empty,										/*  acn  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  adc  */
	AMB_Empty,										/*  ahx  */
	AMB_Empty,										/*  alr  */
	AMB_Empty,										/*  anc  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  and  */
	AMB_Empty,										/*  arr  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  asl  */
	AMB_Empty,										/*  axs  */

	AMB_Rel,										/*  bcc  */
	AMB_Rel,										/*  bcs  */
	AMB_Rel,										/*  beq  */
	AMB_Zp|AMB_Abs,										/*  bit  */
	AMB_Rel,										/*  bmi  */
	AMB_Rel,										/*  bne  */
	AMB_Rel,										/*  bpl  */
	AMB_Empty,										/*  bra  */
	AMB_None|AMB_CaZp,									/*  brk  */
	AMB_Empty,										/*  brl  */
	AMB_Rel,										/*  bvc  */
	AMB_Rel,										/*  bvs  */

	AMB_None,										/*  clc  */
	AMB_None,										/*  cld  */
	AMB_None,										/*  cli  */
	AMB_None,										/*  clv  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  cmp  */
	AMB_Empty,										/*  cop  */
	AMB_CaZp|AMB_Zp|AMB_Abs,								/*  cpx  */
	AMB_CaZp|AMB_Zp|AMB_Abs,								/*  cpy  */

	AMB_Empty,										/*  dcp  */
	AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,							/*  dec  */
	AMB_None,										/*  dex  */
	AMB_None,										/*  dey  */

	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  eor  */

	AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,							/*  inc  */
	AMB_None,										/*  inx  */
	AMB_None,										/*  iny  */
	AMB_Empty,										/*  isc  */

	AMB_Empty,										/*  jml  */
	AMB_Abs|AMB_BrAbsBr,									/*  jmp  */
	AMB_Empty,										/*  jsl  */
	AMB_Abs,										/*  jsr  */

	AMB_Empty,										/*  ki0  */
	AMB_Empty,										/*  ki1  */
	AMB_Empty,										/*  ki2  */
	AMB_Empty,										/*  ki3  */
	AMB_Empty,										/*  ki4  */
	AMB_Empty,										/*  ki5  */
	AMB_Empty,										/*  ki6  */
	AMB_Empty,										/*  ki7  */
	AMB_Empty,										/*  ki9  */
	AMB_Empty,										/*  kib  */
	AMB_Empty,										/*  kid  */
	AMB_Empty,										/*  kif  */

	AMB_Empty,										/*  las  */
	AMB_Empty,										/*  lax  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  lda  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoY|AMB_Abs|AMB_AbsCoY,						/*  ldx  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  ldy  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  lsr  */

	AMB_Empty,										/*  mvn  */
	AMB_Empty,										/*  mvp  */

	AMB_Empty,										/*  no0  */
	AMB_Empty,										/*  no2  */
	AMB_Empty,										/*  no4  */
	AMB_Empty,										/*  no6  */
	AMB_Empty,										/*  no8  */
	AMB_Empty,										/*  noc  */
	AMB_Empty,										/*  noe  */
	AMB_None,										/*  nop  */
	AMB_Empty,										/*  nox  */
	AMB_Empty,										/*  noy  */
	
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  ora  */

	AMB_Empty,										/*  pea  */
	AMB_Empty,										/*  pei  */
	AMB_Empty,										/*  per  */
	AMB_None,										/*  pha  */
	AMB_Empty,										/*  phb  */
	AMB_Empty,										/*  phd  */
	AMB_Empty,										/*  phk  */
	AMB_None,										/*  php  */
	AMB_Empty,										/*  phx  */
	AMB_Empty,										/*  phy  */
	AMB_None,										/*  pla  */
	AMB_Empty,										/*  plb  */
	AMB_Empty,										/*  pld  */
	AMB_None,										/*  plp  */
	AMB_Empty,										/*  plx  */
	AMB_Empty,										/*  ply  */

	AMB_Empty,										/*  rep  */
	AMB_Empty,										/*  rla  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  rol  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  ror  */
	AMB_Empty,										/*  rra  */
	AMB_None,										/*  rti  */
	AMB_Empty,										/*  rtl  */
	AMB_None,										/*  rts  */

	AMB_Empty,										/*  sax  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  sbc  */
	AMB_Empty,										/*  sbi  */
	AMB_None,										/*  sec  */
	AMB_None,										/*  sed  */
	AMB_None,										/*  sei  */
	AMB_Empty,										/*  sep  */
	AMB_Empty,										/*  shx  */
	AMB_Empty,										/*  slo  */
	AMB_Empty,										/*  sre  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  sta  */
	AMB_Empty,										/*  stp  */
	AMB_Zp|AMB_ZpCoY|AMB_Abs,								/*  stx  */
	AMB_Zp|AMB_ZpCoX|AMB_Abs,								/*  sty  */
	AMB_Empty,										/*  stz  */

	AMB_Empty,										/*  tas  */
	AMB_None,										/*  tax  */
	AMB_None,										/*  tay  */
	AMB_Empty,										/*  tcd  */
	AMB_Empty,										/*  tcs  */
	AMB_Empty,										/*  tdc  */
	AMB_Empty,										/*  trb  */
	AMB_Empty,										/*  tsb  */
	AMB_Empty,										/*  tsc  */
	AMB_None,										/*  tsx  */
	AMB_None,										/*  txa  */
	AMB_None,										/*  txs  */
	AMB_Empty,										/*  txy  */
	AMB_None,										/*  tya  */
	AMB_Empty,										/*  tyx  */

	AMB_Empty,										/*  wai  */
	AMB_Empty,										/*  wdm  */

	AMB_Empty,										/*  xaa  */
	AMB_Empty,										/*  xba  */
	AMB_Empty										/*  xce  */
};


const uint32_t validModes_6510Ill[132] =
{
	AMB_CaZp,										/*  acn  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  adc  */
	AMB_BrZpBrCoY|AMB_AbsCoY,								/*  ahx  */
	AMB_CaZp,										/*  alr  */
	AMB_CaZp,										/*  anc  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  and  */
	AMB_CaZp,										/*  arr  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  asl  */
	AMB_CaZp,										/*  axs  */

	AMB_Rel,										/*  bcc  */
	AMB_Rel,										/*  bcs  */
	AMB_Rel,										/*  beq  */
	AMB_Zp|AMB_Abs,										/*  bit  */
	AMB_Rel,										/*  bmi  */
	AMB_Rel,										/*  bne  */
	AMB_Rel,										/*  bpl  */
	AMB_Empty,										/*  bra  */
	AMB_None|AMB_CaZp,									/*  brk  */
	AMB_Empty,										/*  brl  */
	AMB_Rel,										/*  bvc  */
	AMB_Rel,										/*  bvs  */

	AMB_None,										/*  clc  */
	AMB_None,										/*  cld  */
	AMB_None,										/*  cli  */
	AMB_None,										/*  clv  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  cmp  */
	AMB_Empty,										/*  cop  */
	AMB_CaZp|AMB_Zp|AMB_Abs,								/*  cpx  */
	AMB_CaZp|AMB_Zp|AMB_Abs,								/*  cpy  */

	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  dcp  */
	AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,							/*  dec  */
	AMB_None,										/*  dex  */
	AMB_None,										/*  dey  */

	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  eor  */

	AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,							/*  inc  */
	AMB_None,										/*  inx  */
	AMB_None,										/*  iny  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  isc  */

	AMB_Empty,										/*  jml  */
	AMB_Abs|AMB_BrAbsBr,									/*  jmp  */
	AMB_Empty,										/*  jsl  */
	AMB_Abs,										/*  jsr  */

	AMB_None,										/*  ki0  */
	AMB_None,										/*  ki1  */
	AMB_None,										/*  ki2  */
	AMB_None,										/*  ki3  */
	AMB_None,										/*  ki4  */
	AMB_None,										/*  ki5  */
	AMB_None,										/*  ki6  */
	AMB_None,										/*  ki7  */
	AMB_None,										/*  ki9  */
	AMB_None,										/*  kib  */
	AMB_None,										/*  kid  */
	AMB_None,										/*  kif  */

	AMB_AbsCoY,										/*  las  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoY,		/*  lax  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  lda  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoY|AMB_Abs|AMB_AbsCoY,						/*  ldx  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  ldy  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  lsr  */

	AMB_Empty,										/*  mvn  */
	AMB_Empty,										/*  mvp  */

	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  no0  */
	AMB_None|AMB_ZpCoX|AMB_AbsCoX,								/*  no2  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_AbsCoX,							/*  no4  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_AbsCoX,							/*  no6  */
	AMB_CaZp|AMB_AbsCoX,									/*  no8  */
	AMB_None|AMB_CaZp|AMB_ZpCoX|AMB_AbsCoX,							/*  noc  */
	AMB_None|AMB_CaZp|AMB_ZpCoX|AMB_AbsCoX,							/*  noe  */
	AMB_None,										/*  nop  */
	AMB_CaZp,										/*  nox  */
	AMB_CaZp,										/*  noy  */

	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  ora  */

	AMB_Empty,										/*  pea  */
	AMB_Empty,										/*  pei  */
	AMB_Empty,										/*  per  */
	AMB_None,										/*  pha  */
	AMB_Empty,										/*  phb  */
	AMB_Empty,										/*  phd  */
	AMB_Empty,										/*  phk  */
	AMB_None,										/*  php  */
	AMB_Empty,										/*  phx  */
	AMB_Empty,										/*  phy  */
	AMB_None,										/*  pla  */
	AMB_Empty,										/*  plb  */
	AMB_Empty,										/*  pld  */
	AMB_None,										/*  plp  */
	AMB_Empty,										/*  plx  */
	AMB_Empty,										/*  ply  */

	AMB_Empty,										/*  rep  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  rla  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  rol  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  ror  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  rra  */
	AMB_None,										/*  rti  */
	AMB_Empty,										/*  rtl  */
	AMB_None,										/*  rts  */

	AMB_Zp|AMB_ZpCoY|AMB_BrZpCoXBr|AMB_Abs,							/*  sax  */
	AMB_CaZp|AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,	/*  sbc  */
	AMB_CaZp,										/*  sbi  */
	AMB_None,										/*  sec  */
	AMB_None,										/*  sed  */
	AMB_None,										/*  sei  */
	AMB_Empty,										/*  sep  */
	AMB_AbsCoY,										/*  shx  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  slo  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  sre  */
	AMB_Zp|AMB_ZpCoX|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY,		/*  sta  */
	AMB_Empty,										/*  stp  */
	AMB_Zp|AMB_ZpCoY|AMB_Abs,								/*  stx  */
	AMB_Zp|AMB_ZpCoX|AMB_Abs,								/*  sty  */
	AMB_Empty,										/*  stz  */

	AMB_AbsCoY,										/*  tas  */
	AMB_None,										/*  tax  */
	AMB_None,										/*  tay  */
	AMB_Empty,										/*  tcd  */
	AMB_Empty,										/*  tcs  */
	AMB_Empty,										/*  tdc  */
	AMB_Empty,										/*  trb  */
	AMB_Empty,										/*  tsb  */
	AMB_Empty,										/*  tsc  */
	AMB_None,										/*  tsx  */
	AMB_None,										/*  txa  */
	AMB_None,										/*  txs  */
	AMB_Empty,										/*  txy  */
	AMB_None,										/*  tya  */
	AMB_Empty,										/*  tyx  */

	AMB_Empty,										/*  wai  */
	AMB_Empty,										/*  wdm  */

	AMB_CaZp,										/*  xaa  */
	AMB_Empty,										/*  xba  */
	AMB_Empty										/*  xce  */
};


const uint32_t validModes_65816[132] =
{
	AMB_Empty,										/*  acn  */
	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  adc  */
	AMB_Empty,										/*  ahx  */
	AMB_Empty,										/*  alr  */
	AMB_Empty,										/*  anc  */
	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  and  */
	AMB_Empty,										/*  arr  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  asl  */
	AMB_Empty,										/*  axs  */

	AMB_Rel,										/*  bcc  */
	AMB_Rel,										/*  bcs  */
	AMB_Rel,										/*  beq  */
	AMB_CaAsize|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,					/*  bit  */
	AMB_Rel,										/*  bmi  */
	AMB_Rel,										/*  bne  */
	AMB_Rel,										/*  bpl  */
	AMB_Rel,										/*  bra  */
	AMB_None|AMB_CaZp,									/*  brk  */
	AMB_RelLong,										/*  brl  */
	AMB_Rel,										/*  bvc  */
	AMB_Rel,										/*  bvs  */

	AMB_None,										/*  clc  */
	AMB_None,										/*  cld  */
	AMB_None,										/*  cli  */
	AMB_None,										/*  clv  */
	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  cmp  */
	AMB_CaZp,										/*  cop  */
	AMB_CaXysize|AMB_Zp|AMB_Abs,								/*  cpx  */
	AMB_CaXysize|AMB_Zp|AMB_Abs,								/*  cpy  */

	AMB_Empty,										/*  dcp  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  dec  */
	AMB_None,										/*  dex  */
	AMB_None,										/*  dey  */

	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  eor  */

	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  inc  */
	AMB_None,										/*  inx  */
	AMB_None,										/*  iny  */
	AMB_Empty,										/*  isc  */

	AMB_BrAbsBr,										/*  jml  */
	AMB_Abs|AMB_BrAbsBr|AMB_BrAbsCoXBr|AMB_Long,						/*  jmp  */
	AMB_Long,										/*  jsl  */
	AMB_Abs|AMB_BrAbsCoXBr,									/*  jsr  */

	AMB_Empty,										/*  ki0  */
	AMB_Empty,										/*  ki1  */
	AMB_Empty,										/*  ki2  */
	AMB_Empty,										/*  ki3  */
	AMB_Empty,										/*  ki4  */
	AMB_Empty,										/*  ki5  */
	AMB_Empty,										/*  ki6  */
	AMB_Empty,										/*  ki7  */
	AMB_Empty,										/*  ki9  */
	AMB_Empty,										/*  kib  */
	AMB_Empty,										/*  kid  */
	AMB_Empty,										/*  kif  */

	AMB_Empty,										/*  las  */
	AMB_Empty,										/*  lax  */
	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  lda  */
	AMB_CaXysize|AMB_Zp|AMB_ZpCoY|AMB_Abs|AMB_AbsCoY,					/*  ldx  */
	AMB_CaXysize|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,					/*  ldy  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  lsr  */

	AMB_ZpCoZp,										/*  mvn  */
	AMB_ZpCoZp,										/*  mvp  */

	AMB_Empty,										/*  no0  */
	AMB_Empty,										/*  no2  */
	AMB_Empty,										/*  no4  */
	AMB_Empty,										/*  no6  */
	AMB_Empty,										/*  no8  */
	AMB_Empty,										/*  noc  */
	AMB_Empty,										/*  noe  */
	AMB_None,										/*  nop  */
	AMB_Empty,										/*  nox  */
	AMB_Empty,										/*  noy  */

	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  ora  */

	AMB_CaAbs,										/*  pea  */
	AMB_BrZpBr,										/*  pei  */
	AMB_RelLong,										/*  per  */
	AMB_None,										/*  pha  */
	AMB_None,										/*  phb  */
	AMB_None,										/*  phd  */
	AMB_None,										/*  phk  */
	AMB_None,										/*  php  */
	AMB_None,										/*  phx  */
	AMB_None,										/*  phy  */
	AMB_None,										/*  pla  */
	AMB_None,										/*  plb  */
	AMB_None,										/*  pld  */
	AMB_None,										/*  plp  */
	AMB_None,										/*  plx  */
	AMB_None,										/*  ply  */

	AMB_CaZp,										/*  rep  */
	AMB_Empty,										/*  rla  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  rol  */
	AMB_None|AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,						/*  ror  */
	AMB_Empty,										/*  rra  */
	AMB_None,										/*  rti  */
	AMB_None,										/*  rtl  */
	AMB_None,										/*  rts  */

	AMB_Empty,										/*  sax  */
	AMB_CaAsize|AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,	/*  sbc  */
	AMB_Empty,										/*  sbi  */
	AMB_None,										/*  sec  */
	AMB_None,										/*  sed  */
	AMB_None,										/*  sei  */
	AMB_CaZp,										/*  sep  */
	AMB_Empty,										/*  shx  */
	AMB_Empty,										/*  slo  */
	AMB_Empty,											/*  sre  */
	AMB_Zp|AMB_ZpCoS|AMB_ZpCoX|AMB_BrZpBr|AMB_BrZpCoSBrCoY|AMB_BrZpCoXBr|AMB_BrZpBrCoY|AMB_SbrZpSbr|AMB_SbrZpSbrCoY|AMB_Abs|AMB_AbsCoX|AMB_AbsCoY|AMB_Long|AMB_LongCoX,		/*  sta  */
	AMB_None,										/*  stp  */
	AMB_Zp|AMB_ZpCoY|AMB_Abs,								/*  stx  */
	AMB_Zp|AMB_ZpCoX|AMB_Abs,								/*  sty  */
	AMB_Zp|AMB_ZpCoX|AMB_Abs|AMB_AbsCoX,							/*  stz  */

	AMB_Empty,										/*  tas  */
	AMB_None,										/*  tax  */
	AMB_None,										/*  tay  */
	AMB_None,										/*  tcd  */
	AMB_None,										/*  tcs  */
	AMB_None,										/*  tdc  */
	AMB_Zp|AMB_Abs,										/*  trb  */
	AMB_Zp|AMB_Abs,										/*  tsb  */
	AMB_None,										/*  tsc  */
	AMB_None,										/*  tsx  */
	AMB_None,										/*  txa  */
	AMB_None,										/*  txs  */
	AMB_None,										/*  txy  */
	AMB_None,										/*  tya  */
	AMB_None,										/*  tyx  */

	AMB_None,										/*  wai  */
	AMB_None,										/*  wdm  */

	AMB_Empty,										/*  xaa  */
	AMB_None,										/*  xba  */
	AMB_None										/*  xce  */
};


const amode_offset_s amode_offset[132] =
{
	{ AC_DIRECT, 0x2b },	/*  acn  */
	{ AC_GREEN,  0x61 },	/*  adc  */
	{ AC_BROWN,  0x83 },	/*  ahx  */
	{ AC_DIRECT, 0x4b },	/*  alr  */
	{ AC_DIRECT, 0x0b },	/*  anc  */
	{ AC_GREEN,  0x21 },	/*  and  */
	{ AC_DIRECT, 0x6b },	/*  arr  */
	{ AC_BROWN,  0x02 },	/*  asl  */
	{ AC_DIRECT, 0xcb },	/*  axs  */

	{ AC_DIRECT, 0x90 },	/*  bcc  */
	{ AC_DIRECT, 0xb0 },	/*  bcs  */
	{ AC_DIRECT, 0xf0 },	/*  beq  */
	{ AC_SPC0,   0x20 },	/*  bit  */
	{ AC_DIRECT, 0x30 },	/*  bmi  */
	{ AC_DIRECT, 0xd0 },	/*  bne  */
	{ AC_DIRECT, 0x10 },	/*  bpl  */
	{ AC_DIRECT, 0x80 },	/*  bra  */
	{ AC_SPC1,   0xeb },	/*  brk  */
	{ AC_DIRECT, 0x82 },	/*  brl  */
	{ AC_DIRECT, 0x50 },	/*  bvc  */
	{ AC_DIRECT, 0x70 },	/*  bvs  */

	{ AC_DIRECT, 0x18 },	/*  clc  */
	{ AC_DIRECT, 0xd8 },	/*  cld  */
	{ AC_DIRECT, 0x58 },	/*  cli  */
	{ AC_DIRECT, 0xb8 },	/*  clv  */
	{ AC_GREEN,  0xc1 },	/*  cmp  */
	{ AC_DIRECT, 0x02 },	/*  cop  */
	{ AC_BROWN,  0xe0 },	/*  cpx  */
	{ AC_BROWN,  0xc0 },	/*  cpy  */

	{ AC_GREEN,  0xc3 },	/*  dcp  */
	{ AC_SPC0,   0xc2 },	/*  dec  */
	{ AC_DIRECT, 0xca },	/*  dex  */
	{ AC_DIRECT, 0x88 },	/*  dey  */

	{ AC_GREEN,  0x41 },	/*  eor  */

	{ AC_SPC0,   0xe2 },	/*  inc  */
	{ AC_DIRECT, 0xe8 },	/*  inx  */
	{ AC_DIRECT, 0xc8 },	/*  iny  */
	{ AC_GREEN,  0xe3 },	/*  isc  */

	{ AC_DIRECT, 0xdc },	/*  jml  */
	{ AC_BROWN,  0x40 },	/*  jmp  */
	{ AC_DIRECT, 0x22 },	/*  jsl  */
	{ AC_SPC0,   0x2c },	/*  jsr  */

	{ AC_DIRECT, 0x02 },	/*  ki0  */
	{ AC_DIRECT, 0x12 },	/*  ki1  */
	{ AC_DIRECT, 0x22 },	/*  ki2  */
	{ AC_DIRECT, 0x32 },	/*  ki3  */
	{ AC_DIRECT, 0x42 },	/*  ki4  */
	{ AC_DIRECT, 0x52 },	/*  ki5  */
	{ AC_DIRECT, 0x62 },	/*  ki6  */
	{ AC_DIRECT, 0x72 },	/*  ki7  */
	{ AC_DIRECT, 0x92 },	/*  ki9  */
	{ AC_DIRECT, 0xb2 },	/*  kib  */
	{ AC_DIRECT, 0xd2 },	/*  kid  */
	{ AC_DIRECT, 0xf2 },	/*  kif  */

	{ AC_DIRECT, 0xbb },	/*  las  */
	{ AC_RED,    0xa3 },	/*  lax  */
	{ AC_GREEN,  0xa1 },	/*  lda  */
	{ AC_BROWN,  0xa2 },	/*  ldx  */
	{ AC_BROWN,  0xa0 },	/*  ldy  */
	{ AC_BROWN,  0x42 },	/*  lsr  */

	{ AC_DIRECT, 0x54 },	/*  mvn  */
	{ AC_DIRECT, 0x44 },	/*  mvp  */

	{ AC_GREEN,  0x00 },	/*  no0  */
	{ AC_GREEN,  0x20 },	/*  no2  */
	{ AC_GREEN,  0x40 },	/*  no4  */
	{ AC_GREEN,  0x60 },	/*  no6  */
	{ AC_SPC1,   0x62 },	/*  no8  */
	{ AC_SPC2,   0xc2 },	/*  noc  */
	{ AC_SPC2,   0xe2 },	/*  noe  */
	{ AC_DIRECT, 0xea },	/*  nop  */
	{ AC_DIRECT, 0x82 },	/*  nox  */
	{ AC_DIRECT, 0x80 },	/*  noy  */

	{ AC_GREEN,  0x01 },	/*  ora  */

	{ AC_DIRECT, 0xf4 },	/*  pea  */
	{ AC_DIRECT, 0xd4 },	/*  pei  */
	{ AC_DIRECT, 0x62 },	/*  per  */
	{ AC_DIRECT, 0x48 },	/*  pha  */
	{ AC_DIRECT, 0x8b },	/*  phb  */
	{ AC_DIRECT, 0x0b },	/*  phd  */
	{ AC_DIRECT, 0x4b },	/*  phk  */
	{ AC_DIRECT, 0x08 },	/*  php  */
	{ AC_DIRECT, 0xda },	/*  phx  */
	{ AC_DIRECT, 0x5a },	/*  phy  */
	{ AC_DIRECT, 0x68 },	/*  pla  */
	{ AC_DIRECT, 0xab },	/*  plb  */
	{ AC_DIRECT, 0x2b },	/*  pld  */
	{ AC_DIRECT, 0x28 },	/*  plp  */
	{ AC_DIRECT, 0xfa },	/*  plx  */
	{ AC_DIRECT, 0x7a },	/*  ply  */

	{ AC_DIRECT, 0xc2 },	/*  rep  */
	{ AC_GREEN,  0x23 },	/*  rla  */
	{ AC_BROWN,  0x22 },	/*  rol  */
	{ AC_BROWN,  0x62 },	/*  ror  */
	{ AC_GREEN,  0x63 },	/*  rra  */
	{ AC_DIRECT, 0x40 },	/*  rti  */
	{ AC_DIRECT, 0x6b },	/*  rtl  */
	{ AC_DIRECT, 0x60 },	/*  rts  */

	{ AC_RED,    0x83 },	/*  sax  */
	{ AC_GREEN,  0xe1 },	/*  sbc  */
	{ AC_DIRECT, 0xeb },	/*  sbi  */
	{ AC_DIRECT, 0x38 },	/*  sec  */
	{ AC_DIRECT, 0xf8 },	/*  sed  */
	{ AC_DIRECT, 0x78 },	/*  sei  */
	{ AC_DIRECT, 0xe2 },	/*  sep  */
	{ AC_DIRECT, 0x9e },	/*  shx  */
	{ AC_GREEN,  0x03 },	/*  slo  */
	{ AC_GREEN,  0x43 },	/*  sre  */
	{ AC_GREEN,  0x81 },	/*  sta  */
	{ AC_DIRECT, 0xdb },	/*  stp  */
	{ AC_BROWN,  0x82 },	/*  stx  */
	{ AC_BROWN,  0x80 },	/*  sty  */
	{ AC_SPC1,   0x60 },	/*  stz  */

	{ AC_DIRECT, 0x9b },	/*  tas  */
	{ AC_DIRECT, 0xaa },	/*  tax  */
	{ AC_DIRECT, 0xa8 },	/*  tay  */
	{ AC_DIRECT, 0x5b },	/*  tcd  */
	{ AC_DIRECT, 0x1b },	/*  tcs  */
	{ AC_DIRECT, 0x7b },	/*  tdc  */
	{ AC_BROWN,  0x10 },	/*  trb  */
	{ AC_BROWN,  0x00 },	/*  tsb  */
	{ AC_DIRECT, 0x3b },	/*  tsc  */
	{ AC_DIRECT, 0xba },	/*  tsx  */
	{ AC_DIRECT, 0x8a },	/*  txa  */
	{ AC_DIRECT, 0x9a },	/*  txs  */
	{ AC_DIRECT, 0x9b },	/*  txy  */
	{ AC_DIRECT, 0x98 },	/*  tya  */
	{ AC_DIRECT, 0xbb },	/*  tyx  */

	{ AC_DIRECT, 0xcb },	/*  wai  */
	{ AC_DIRECT, 0x42 },	/*  wdm  */

	{ AC_DIRECT, 0x8b },	/*  xaa  */
	{ AC_DIRECT, 0xeb },	/*  xba  */
	{ AC_DIRECT, 0xfb }	/*  xce  */
};


const uint8_t amode_offs_groups[6*AM_length] =
{
/*  amode_offs_green  */
	0x1a,		/*  AM_None         */
	0x08,		/*  AM_CaZp         */
	0x08,		/*  AM_CaAbs        */
	0x04,		/*  AM_Zp           */
	0x02,		/*  AM_ZpCoS        */
	0x14,		/*  AM_ZpCoX        */
	0xff,		/*  AM_ZpCoY        */
	0xff,		/*  AM_ZpCoZp       */
	0x13,		/*  AM_BrZpBr       */
	0x00,		/*  AM_BrZpCoXBr    */
	0x12,		/*  AM_BrZpCoSBrY   */
	0x10,		/*  AM_BrZpBrCoY    */
	0x06,		/*  AM_SbrZpSbr     */
	0x16,		/*  AM_SbrZpSbrCoY  */
	0x0c,		/*  AM_Abs          */
	0x1c,		/*  AM_AbsCoX       */
	0x18,		/*  AM_AbsCoY       */
	0xff,		/*  AM_BrAbsBr      */
	0xff,		/*  AM_BrAbsBrCoX   */
	0x0e,		/*  AM_Long         */
	0x1e,		/*  AM_LongCoX      */
	0xff,		/*  AM_Rel          */
	0xff,		/*  AM_RelLong      */

/*  amode_offs_brown  */
	0x08,		/*  AM_None         */
	0x00,		/*  AM_CaZp         */
	0x00,		/*  AM_CaAbs        */
	0x04,		/*  AM_Zp           */
	0xff,		/*  AM_ZpCoS        */
	0x14,		/*  AM_ZpCoX        */
	0x14,		/*  AM_ZpCoY        */
	0xff,		/*  AM_ZpCoZp       */
	0xff,		/*  AM_BrZpBr       */
	0xff,		/*  AM_BrZpCoXBr    */
	0xff,		/*  AM_BrZpCoSBrY   */
	0x10,		/*  AM_BrZpBrCoY    */
	0xff,		/*  AM_SbrZpSbr     */
	0xff,		/*  AM_SbrZpSbrCoY  */
	0x0c,		/*  AM_Abs          */
	0x1c,		/*  AM_AbsCoX       */
	0x1c,		/*  AM_AbsCoY       */
	0x2c,		/*  AM_BrAbsBr      */
	0x3c,		/*  AM_BrAbsCoXBr   */
	0x1c,		/*  AM_Long         */
	0xff,		/*  AM_LongCoX      */
	0xff,		/*  AM_Rel          */
	0xff,		/*  AM_RelLong      */

/*  amode_offs_red  */
	0x1a,		/*  AM_None         */
	0x08,		/*  AM_CaZp         */
	0xff,		/*  AM_CaAbs        */
	0x04,		/*  AM_Zp           */
	0xff,		/*  AM_ZpCoS        */
	0x14,		/*  AM_ZpCoX        */
	0x14,		/*  AM_ZpCoY        */
	0xff,		/*  AM_ZpCoZp       */
	0xff,		/*  AM_BrZpBr       */
	0x00,		/*  AM_BrZpCoXBr    */
	0xff,		/*  AM_BrZpCoSBrY   */
	0x10,		/*  AM_BrZpBrCoY    */
	0xff,		/*  AM_SbrZpSbr     */
	0xff,		/*  AM_SbrZpSbrCoY  */
	0x0c,		/*  AM_Abs          */
	0x1c,		/*  AM_AbsCoX       */
	0x1c,		/*  AM_AbsCoY       */
	0xff,		/*  AM_BrAbsBr      */
	0xff,		/*  AM_BrAbsBrCoX   */
	0xff,		/*  AM_Long         */
	0xff,		/*  AM_LongCoX      */
	0xff,		/*  AM_Rel          */
	0xff,		/*  AM_RelLong      */

/*  amode_offs_spc0  */
	0xf8,		/*  AM_None         */
	0xa9,		/*  AM_CaZp         */
	0xa9,		/*  AM_CaAbs        */
	0x04,		/*  AM_Zp           */
	0xff,		/*  AM_ZpCoS        */
	0x14,		/*  AM_ZpCoX        */
	0xff,		/*  AM_ZpCoY        */
	0xff,		/*  AM_ZpCoZp       */
	0xff,		/*  AM_BrZpBr       */
	0xff,		/*  AM_BrZpCoXBr    */
	0xff,		/*  AM_BrZpCoSBrY   */
	0xff,		/*  AM_BrZpBrCoY    */
	0xff,		/*  AM_SbrZpSbr     */
	0xff,		/*  AM_SbrZpSbrCoY  */
	0x0c,		/*  AM_Abs          */
	0x1c,		/*  AM_AbsCoX       */
	0xff,		/*  AM_AbsCoY       */
	0xff,		/*  AM_BrAbsBr      */
	0xd0,		/*  AM_BrAbsBrCoX   */
	0xff,		/*  AM_Long         */
	0xff,		/*  AM_LongCoX      */
	0xff,		/*  AM_Rel          */
	0xff,		/*  AM_RelLong      */

/*  amode_offs_spc1  */
	0xeb,		/*  AM_None         */
	0xeb,		/*  AM_CaZp         */
	0xff,		/*  AM_CaAbs        */
	0x04,		/*  AM_Zp           */
	0xff,		/*  AM_ZpCoS        */
	0x14,		/*  AM_ZpCoX        */
	0xff,		/*  AM_ZpCoY        */
	0xff,		/*  AM_ZpCoZp       */
	0xff,		/*  AM_BrZpBr       */
	0xff,		/*  AM_BrZpCoXBr    */
	0xff,		/*  AM_BrZpCoSBrY   */
	0xff,		/*  AM_BrZpBrCoY    */
	0xff,		/*  AM_SbrZpSbr     */
	0xff,		/*  AM_SbrZpSbrCoY  */
	0xfc,		/*  AM_Abs          */
	0xfe,		/*  AM_AbsCoX       */
	0xff,		/*  AM_AbsCoY       */
	0xff,		/*  AM_BrAbsBr      */
	0xff,		/*  AM_BrAbsBrCoX   */
	0xff,		/*  AM_Long         */
	0xff,		/*  AM_LongCoX      */
	0xff,		/*  AM_Rel          */
	0xff,		/*  AM_RelLong      */

/*  amode_offs_spc2  */
	0x18,		/*  AM_None         */
	0x00,		/*  AM_CaZp         */
	0xff,		/*  AM_CaAbs        */
	0xff,		/*  AM_Zp           */
	0xff,		/*  AM_ZpCoS        */
	0x16,		/*  AM_ZpCoX        */
	0xff,		/*  AM_ZpCoY        */
	0xff,		/*  AM_ZpCoZp       */
	0xff,		/*  AM_BrZpBr       */
	0xff,		/*  AM_BrZpCoXBr    */
	0xff,		/*  AM_BrZpCoSBrY   */
	0xff,		/*  AM_BrZpBrCoY    */
	0xff,		/*  AM_SbrZpSbr     */
	0xff,		/*  AM_SbrZpSbrCoY  */
	0xff,		/*  AM_Abs          */
	0x1e,		/*  AM_AbsCoX       */
	0xff,		/*  AM_AbsCoY       */
	0xff,		/*  AM_BrAbsBr      */
	0xff,		/*  AM_BrAbsBrCoX   */
	0xff,		/*  AM_Long         */
	0xff,		/*  AM_LongCoX      */
	0xff,		/*  AM_Rel          */
	0xff		/*  AM_RelLong      */
};

const uint8_t amode_length[23] =
{
	0,		/*  AM_None         */
	1,		/*  AM_CaZp         */
	2,		/*  AM_CaAbs        */
	1,		/*  AM_Zp           */
	1,		/*  AM_ZpCoS        */
	1,		/*  AM_ZpCoX        */
	1,		/*  AM_ZpCoY        */
	1,		/*  AM_ZpCoZp       */
	1,		/*  AM_BrZpBr       */
	1,		/*  AM_BrZpCoXBr    */
	1,		/*  AM_BrZpCoSBrY   */
	1,		/*  AM_BrZpBrCoY    */
	1,		/*  AM_SbrZpSbr     */
	1,		/*  AM_SbrZpSbrCoY  */
	2,		/*  AM_Abs          */
	2,		/*  AM_AbsCoX       */
	2,		/*  AM_AbsCoY       */
	2,		/*  AM_BrAbsBr      */
	2,		/*  AM_BrAbsBrCoX   */
	3,		/*  AM_Long         */
	3,		/*  AM_LongCoX      */
	1,		/*  AM_Rel          */
	2		/*  AM_RelLong      */
};


const cpuTypeNamePair_t cputype_name[3] =
{
	{ "6510",	CPUTYPE_6510,		validModes_6510 },
	{ "6510ill",	CPUTYPE_6510Ill,	validModes_6510Ill },
	{ "65816",	CPUTYPE_65816,		validModes_65816 }
};


