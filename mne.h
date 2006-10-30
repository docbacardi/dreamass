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

#ifndef __MNE_H__
#define __MNE_H__


void mne_init(void);

elength_t parse_mne(void);
int16_t adr2rel(uint32_t dstadr, uint32_t opcadr);
int32_t adr2relLong(uint32_t dstadr, uint32_t opcadr);
bool checkJmpInd(VARIABLE *param);

CPUTYPE getCurrentCpu(void);
const char *getCurrentCpuName(void);
CPUTYPE getCpuIdx(const char *cpuName);
void setCpuType(CPUTYPE cpuType);

void setRegisterSize_A(uint8_t regSize);
void setRegisterSize_XY(uint8_t regSize);

#endif		/* __MNE_H__ */

