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


#ifndef __SEGMENT_H__
#define __SEGMENT_H__

typedef enum { SEGTYP_ABS, SEGTYP_BSS, SEGTYP_ZP } SEGMENT_T;

typedef struct
{
	bool fixed;
	uint32_t adr;
	bool pc_defined;
	uint32_t pc;
	uint8_t phaseidx;
	int32_t pcoffset[16];
	seglistsize_t memelemidx;
} segelem_t;

typedef struct
{
	seglistsize_t segelemidx;
	uint32_t Start;
	uint32_t End;
} memtab_t;

typedef struct
{
	seglistsize_t segelemidx;
	uint32_t Start;
	uint32_t End;
	uint8_t *Memarea;
} memelem_t;

typedef struct
{
	stringsize_t *name;
	seglistsize_t seglist_count;
	seglistsize_t seglist_buflen;
	segelem_t *seglist;

	seglistsize_t next_segelem;
	seglistsize_t act_segelem;

	memelem_t *memlist;
	seglistsize_t memlist_count;

	uint32_t startadr;
	uint32_t endadr;
	bool fillup;
	bool force;
	uint8_t fillbyte;
	SEGMENT_T typ;
} segment_t;


typedef struct
{
	stringsize_t *filename;
	bool sort;
	bool writesadr;
	uint8_t fillbyte;
	stringsize_t **segnames;
	seglistsize_t segnames_count;
} outfile_t;


void segment_zero(void);
bool segment_init(seglistsize_t nmemb);
seglistsize_t segment_new(stringsize_t *name, uint32_t startadr, uint32_t endadr, bool fillup, uint8_t fillbyte, bool force, SEGMENT_T typ);
void segment_del(void);

stringsize_t *segment_getNameOf(const seglistsize_t segidx);
stringsize_t *segment_getName(void);

bool segelem_init(segment_t *seg, seglistsize_t nmemb);

void segment_reset(void);
void segment_enterSegment(const seglistsize_t segidx);
seglistsize_t segment_findSegment(const stringsize_t *name);
seglistsize_t segment_newSegElem(bool fixed, uint32_t adr);
seglistsize_t segment_getActSegIdx(void);
void segment_enterSegElem(seglistsize_t segidx);
void segment_addLength(const length_t len);
bool segment_isPCDefined(void);
bool segment_isPCFinal(void);
uint32_t segment_getPC(void);
void segment_processChange(void);

bool segment_phase(uint32_t phasepc);
bool segment_dephase(void);
uint8_t segment_getPhaseIdx(void);

void segment_debug(FILE *dfh);

bool segment_memDump(void);
uint8_t *segment_getMemPtr(void);
bool segment_writeFiles(char *ofile);
bool segment_writeSegment(FILE *ofh, seglistsize_t segidx, bool startadr);

bool segment_newOutfile( stringsize_t *filename, bool sort, bool writesadr, uint8_t fillbyte, stringsize_t **segnames, seglistsize_t segnames_count );


#endif		/* __SEGMENT_H__ */

