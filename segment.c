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


#include "globals.h"

/*-----------------------------------*/
/* locals                            */

seglistsize_t segment_count;
seglistsize_t segment_buflen;
segment_t *segment;

seglistsize_t outfile_count;
seglistsize_t outfile_buflen;
outfile_t *outfile;

seglistsize_t act_segment;

bool memlist_dump(segment_t* seg);

/*-----------------------------------*/

void segment_zero(void)
{
	segment_count = 0;
	outfile_count = 0;

	segment = NULL;
	outfile = NULL;
}


bool segment_init(seglistsize_t nmemb)
{
	seglistsize_t firstseg;

	const char* fnc = "seg1";
	stringsize_t *firstname = NULL;


	/* Create the list for the Segments */
	segment_count = 0;
	if( (segment=(segment_t*)(malloc((segment_buflen=nmemb)*sizeof(segment_t))))==NULL ) {
		systemError(EM_OutOfMemory);
		return false;
	}

	/* Create the list for the Outfiles */
	outfile_count = 0;
	if( (outfile=(outfile_t*)(malloc((outfile_buflen=nmemb)*sizeof(outfile_t))))==NULL ) {
		systemError(EM_OutOfMemory);
		return false;
	}

	/* convert the charstring to a h-field */
	if( (firstname=cstr2string(fnc,strlen(fnc)))==NULL ) {
		return false;
	}

	if( (firstseg=segment_new(stringClone(firstname), 0, 0x10000, false, 0x00, false, SEGTYP_ABS))==(seglistsize_t)-1 ) {
		free( firstname );
		return false;
	}

	free( firstname );
	return true;
}


void segment_del(void)
{
	segment_t *sc, *se;
	memelem_t *mc, *me;
	outfile_t *oc, *oe;
	stringsize_t **fc, **fe;


	if( segment!=NULL ) {
		for(se=(sc=segment)+segment_count; sc<se; ++sc) {
			if( sc->name!=NULL ) {
				free(sc->name);
			}
			if( sc->seglist!=NULL ) {
				free(sc->seglist);
			}
			if( sc->memlist!=NULL ) {
				for( me=(mc=sc->memlist)+sc->memlist_count; mc<me; ++mc ) {
					if( mc->Memarea!=NULL )
						free(mc->Memarea);
				}
				free(sc->memlist);
			}
		}
		free(segment);
	}

	if( outfile!=NULL ) {
		for(oe=(oc=outfile)+outfile_count; oc<oe; ++oc) {
			if( oc->filename!=NULL ) {
				free(oc->filename);
			}
			for(fe=(fc=oc->segnames)+oc->segnames_count; fc<fe; ++fc) {
				if( *fc!=NULL )
					free(*fc);
			}
			free(oc->segnames);
		}
		free(outfile);
	}
}


seglistsize_t segment_new(stringsize_t *name, uint32_t startadr, uint32_t endadr, bool fillup, uint8_t fillbyte, bool force, SEGMENT_T typ)
{
	seglistsize_t newlen;
	segment_t *sm;


	if( segment_count >= segment_buflen ) {
		if( segment_buflen==((seglistsize_t)-1) ) {
			error(EM_TooManySegments);
			return ((seglistsize_t)-1);
		}

		if( (newlen=segment_buflen<<1)<segment_buflen ) {
			newlen=((seglistsize_t)-1);
		}
		if( (sm=(segment_t*)(realloc(segment, newlen*sizeof(segment_t))))==NULL ) {
			systemError(EM_OutOfMemory);
			return ((seglistsize_t)-1);
		}
		segment = sm;
		segment_buflen = newlen;
	}

	sm=segment+segment_count;
	if( (sm->name=stringClone(name))==NULL ) {
		return ((seglistsize_t)-1);
	}

	/* name is no longer needed */
	free(name);

	/* set the segments attributes */
	sm->startadr = startadr;
	sm->endadr = endadr;
	sm->fillup = fillup;
	sm->fillbyte = fillbyte;
	sm->force = force;
	sm->typ = typ;

	sm->memlist = NULL;
	sm->memlist_count = 0;

	/* Create the list for the SegElems */
	sm->seglist_count = 1;
	if( (sm->seglist=(segelem_t*)(malloc((sm->seglist_buflen=16)*sizeof(segelem_t))))==NULL ) {
		systemError(EM_OutOfMemory);
		return ((seglistsize_t)-1);
	}
	sm->seglist->fixed = false;
	sm->seglist->phaseidx = 0;
	sm->seglist->pc_defined = true;
	sm->seglist->adr = sm->seglist->pc = 0;
	sm->seglist->memelemidx = 0;

	/* actual SegElem is the first entry */
	sm->act_segelem = sm->next_segelem = 0;

	/* actual Segment is the new created */
	return (act_segment=segment_count++);
}


stringsize_t *segment_getNameOf(const seglistsize_t segidx)
{
	return (segment+segidx)->name;
}


stringsize_t *segment_getName(void)
{
	return (segment+act_segment)->name;
}


void segment_reset(void)
{
	segment_t *sc, *se;
	segelem_t *ec, *ee;


	for( se=(sc=segment)+segment_count; sc<se; ++sc )
	{
		sc->next_segelem = 0;
		sc->act_segelem = 0;
		for( ee=(ec=sc->seglist)+sc->seglist_count; ec<ee; ++ec )
		{
			ec->pc = ec->adr;
			ec->pc_defined = true;
			ec->phaseidx = 0;
		}
	}
	act_segment = 0;
}


void segment_enterSegment(const seglistsize_t segidx)
{
	act_segment = segidx;
}


seglistsize_t segment_findSegment(const stringsize_t *name)
{
	segment_t *sc, *se;


	for( se=(sc=segment)+segment_count; sc<se; ++sc )
	{
		if( stringCmp(sc->name, name)==0 )
			break;
	}

	return (sc<se)?sc-segment:((seglistsize_t)-1);
}


/*
 * segment_newSegElem creates a new listentry in the SegElem table, but
 * does not enter it. That's necessary for sth like this: '.DB *=$c000'
 * Return:  0 = no need to switch SegElem (because the actual one is empty)
 *         -1 = Error
 *         rest = index of the new SegElem
 */
seglistsize_t segment_newSegElem(bool fixed, uint32_t adr)
{
	seglistsize_t newlen;
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	/* If the actual SegElem is empty, there's no need to create a new one */
	sl=seg->seglist+seg->act_segelem;
	if( sl->adr==sl->pc && sl->pc_defined ) {
/*
		printf("SegElem: no new SegElem for idx %d\n", act_segelem);
*/

		sl->fixed = fixed;
		sl->pc_defined = true;
		sl->adr = sl->pc = fixed?adr:0;
		sl->phaseidx = 0;

		return 0;
	}
	else {
		if( seg->seglist_count >= seg->seglist_buflen ) {
			if( seg->seglist_buflen==((seglistsize_t)-1) ) {
				error(EM_TooManySegElems);
				return ((seglistsize_t)-1);
			}

			if( (newlen=seg->seglist_buflen<<1)<seg->seglist_buflen )
				newlen=((seglistsize_t)-1);
			if( (sl=(segelem_t*)(realloc(seg->seglist, newlen*sizeof(segelem_t*))))==NULL )
			{
				systemError(EM_OutOfMemory);
				return ((seglistsize_t)-1);
			}
			seg->seglist = sl;
			seg->seglist_buflen = newlen;
		}

		(sl=seg->seglist+seg->seglist_count)->fixed = fixed;
		sl->pc_defined = true;
		sl->adr = sl->pc = fixed?adr:0;
		sl->phaseidx = 0;
/*
		printf("SegElem: new SegElem %x\n", seg->seglist_count);
*/
		return seg->next_segelem=seg->seglist_count++;
	}
}


seglistsize_t segment_getActSegIdx(void)
{
	return act_segment;
}


void segment_enterSegElem(seglistsize_t segidx)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	sl = seg->seglist+seg->act_segelem;
/*
	printf("SegElem: EnterReq to SegElem %x\n", segidx);
*/

	if( sl->phaseidx>0 )
	{
		/*
		 * TODO: warning instead of this
		 * something like "PC still phased at end of Segment!"
		 */
		assert(false);
	}
	seg->next_segelem = segidx;
}


void segment_addLength(const length_t len)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	sl = seg->seglist+seg->act_segelem;
	if( len.defined ) {
		sl->pc += len.len;
	}
	else {
		sl->pc_defined = false;
	}
}


bool segment_isPCDefined(void)
{
	segment_t *seg;

	seg = segment+act_segment;
	return (seg->seglist+seg->act_segelem)->pc_defined;
}


bool segment_isPCFinal(void)
{
	segment_t *seg;

	seg = segment+act_segment;
	return (seg->seglist+seg->act_segelem)->fixed;
}


uint32_t segment_getPC(void)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	sl = seg->seglist+seg->act_segelem;
	if( sl->phaseidx>0 ) {
		return sl->pc+*(sl->pcoffset+sl->phaseidx-1);
	}
	else {
		return sl->pc;
	}
}


void segment_processChange(void)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	if( seg->next_segelem ) {
/*
		printf("SegElem: Changed to SegElem %x\n", next_segelem);
*/

		seg->act_segelem = seg->next_segelem;

		sl = seg->seglist+seg->act_segelem;
		sl->pc = sl->adr;
		sl->pc_defined = true;
		sl->phaseidx = 0;

		seg->next_segelem = 0;
	}
}


bool segment_phase(uint32_t phasepc)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	sl = seg->seglist+seg->act_segelem;
	if( sl->phaseidx>=16 ) {
		/* Too many Phases without a dephase */
		error(EM_TooManyNestedPhases);
		return false;
	}
	if( sl->pc_defined )
		*(sl->pcoffset+sl->phaseidx) = ((int32_t)phasepc)-((int32_t)sl->pc);

	++sl->phaseidx;
/*
	printf("PC phased to $%04x\n", segment_getPC() );
*/
	return true;
}


bool segment_dephase(void)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	sl = seg->seglist+seg->act_segelem;
	if( sl->phaseidx==0 ) {
		/* Hey, not phased! */
		error(EM_UnboundDephase);
		return false;
	}
	--sl->phaseidx;
/*
	printf("PC dephased back to $%04x\n", segment_getPC() );
*/
	return true;
}


uint8_t segment_getPhaseIdx(void)
{
	segment_t *seg;
	segelem_t *sl;


	seg = segment+act_segment;
	sl = seg->seglist+seg->act_segelem;
	return sl->phaseidx;
}


void segment_debug(FILE *dfh)
{
	seglistsize_t seg_cnt, sl_cnt;
	segment_t *seg;
	segelem_t *sl;


	for(seg_cnt=0; seg_cnt<segment_count; seg_cnt++) {
		seg=segment+seg_cnt;

		fprintf(dfh, "<hr width=\"100%%\">\n<h2>Segment %u (", seg_cnt);
		printString(dfh, seg->name);
		fprintf(dfh, ")</h2>\nDump: %u SegElem%s\n", seg->seglist_count, (seg->seglist_count==1)?"":"s");

		fprintf(dfh, "<table border=\"1\"><tbody>\n");
		for(sl_cnt=0; sl_cnt<seg->seglist_count; sl_cnt++) {
			sl = seg->seglist+sl_cnt;
			fprintf(dfh, "<tr><td>%u</td><td>", sl_cnt);
			if( sl->fixed ) {
				fprintf(dfh, "Fixed</td><td>Start: $%04x</td><td>End: ", sl->adr);
				if( sl->pc_defined )
					fprintf(dfh, "$%04x", sl->pc);
				else
					fprintf(dfh, "undefined");
			}
			else
				fprintf(dfh, "Flex<td>Len: $%04x</td><td>&nbsp;", sl->pc);
			fprintf(dfh, "</td></tr>\n");
		}
		fprintf(dfh, "</tbody></table>\n");
	}
}


bool segment_memDump(void)
{
	seglistsize_t segcnt;


	for(segcnt=0; segcnt<segment_count; ++segcnt) {
		if( !memlist_dump(segment+segcnt) ) {
			return false;
		}
	}

	return true;
}


bool memlist_dump(segment_t* seg)
{
	seglistsize_t segelemcnt;
	segelem_t *selem;
	memelem_t *newmlist;

	memelem_t *memlist_end, *memlist_cnt;
	memelem_t *melem_bestfit;
	uint32_t adr, needed, rest, bestrest, beststartadr;
	char *csegname0;


	if( (seg->memlist=(memelem_t*)(malloc(seg->seglist_count*sizeof(memelem_t))))==NULL ) {
		systemError(EM_OutOfMemory);
		return false;
	}
	memlist_end = seg->memlist;

	/*
	 * Insert all fixed SegElems into the memlist
	 */
	for( segelemcnt=0; segelemcnt<seg->seglist_count; ++segelemcnt )
	{
		if( (selem=seg->seglist+segelemcnt)->fixed && selem->adr!=selem->pc )
		{
			if( selem->adr<seg->startadr || selem->pc>seg->endadr )
			{
				if( (csegname0=string2cstr(seg->name))==NULL )
					systemError(EM_OutOfMemory);
				else
				{
					systemError(EM_SegElemMem_s, csegname0);
					/* print some detailed info */
					fprintf(stdout, "Segment range: $%04x - $%04x\n", seg->startadr, seg->endadr);
					fprintf(stdout, "Element range: $%04x - $%04x\n", selem->adr, selem->pc);
					free(csegname0);
				}
				return false;
			}
			memlist_cnt=seg->memlist;
			while( memlist_cnt<memlist_end && selem->pc>memlist_cnt->Start ) {
				++memlist_cnt;
			}
			if( memlist_cnt!=seg->memlist && selem->adr<(memlist_cnt-1)->End ) {
				error(EM_SegmentsOverlapping_d_d_d_d, selem->adr, selem->pc, (memlist_cnt-1)->Start, (memlist_cnt-1)->End);
				return false;
			}
			if( memlist_cnt<memlist_end ) {
				memmove( memlist_cnt+1, memlist_cnt, (memlist_end-memlist_cnt)*sizeof(memelem_t));
			}

			memlist_cnt->segelemidx = segelemcnt;
			memlist_cnt->Start = selem->adr;
			memlist_cnt->End = selem->pc;
			memlist_cnt->Memarea = NULL;
			if( (memlist_cnt->Memarea=(uint8_t*)(malloc(selem->pc-selem->adr)))==NULL ) {
				systemError(EM_OutOfMemory);
				return false;
			}
			++memlist_end;
		}
	}

	/*
	 * Insert all flex SegElems into the memlist
	 * define the pc to the fixed value
	 */
	for( segelemcnt=0; segelemcnt<seg->seglist_count; ++segelemcnt )
	{
		if( !(selem=seg->seglist+segelemcnt)->fixed && selem->adr!=selem->pc ) {
			/* For now just a basic best fit, not sorted */
			bestrest = (uint32_t)-1;
			melem_bestfit = NULL;
			beststartadr = 0;
			adr = seg->startadr;
			/* Flex Elems all start at 0 (I hope :) */
			assert( selem->adr==0 );
			needed = selem->pc;	/*   So the length is equal to the PC  */
			/* test if flex elem fits before the entry */
			for( memlist_cnt=seg->memlist; memlist_cnt<memlist_end; ++memlist_cnt ) {
				/* Does it fit in and is it smaller than the last fit? */
				if( (rest=memlist_cnt->Start-adr)>=needed && rest<bestrest ) {
					bestrest = rest;
					melem_bestfit = memlist_cnt;
					beststartadr = adr;
				}
				adr = memlist_cnt->End;
			}
			/* Does it fit behind the last entry and is it smaller than the last fit? */
			if( (rest=seg->endadr-adr)>=needed && rest<bestrest ) {
				bestrest=rest;
				melem_bestfit = memlist_end;
				beststartadr = adr;
			}
			/* Something found? */
			if( bestrest==(uint32_t)-1 ) {
				/*
				 * No -> Flex Segment does not fit between the fixed segments
				 * Whaaa! Error with something like a linenumber, where this flexseg started
				 */
				assert( false );
			}
			/* Yes, something found -> insert new element */
			if( melem_bestfit<memlist_end ) {
				memmove( melem_bestfit+1, melem_bestfit, (memlist_end-melem_bestfit)*sizeof(memelem_t));
			}

			melem_bestfit->segelemidx = segelemcnt;
			melem_bestfit->Start = beststartadr;
			melem_bestfit->End = beststartadr+needed;
			melem_bestfit->Memarea = NULL;
			if( (melem_bestfit->Memarea=(uint8_t*)(malloc(needed)))==NULL ) {
				systemError(EM_OutOfMemory);
				return false;
			}
			++memlist_end;

			/* Now set adr and pc */
			selem->adr = beststartadr;
			selem->pc_defined = true;
			selem->pc = beststartadr+needed;
			selem->fixed = true;
		}
	}

	seg->memlist_count = memlist_end-seg->memlist;
	if( seg->memlist_count==0 ) {
		free(seg->memlist);
		seg->memlist = NULL;
	}
	else if( seg->memlist_count<seg->seglist_count ) {
		if( (newmlist=(memelem_t*)realloc(seg->memlist,seg->memlist_count*sizeof(memelem_t)))!=NULL )
			seg->memlist = newmlist;
	}

	printf("Segment '");
	printString(stdout, seg->name);
	printf("' sorted MemElems:\n");
	for( segelemcnt=0; segelemcnt<seg->memlist_count; ++segelemcnt ) {
		memlist_cnt = seg->memlist+segelemcnt;
		printf("%u : $%04x - $%04x, id: %u\n", segelemcnt, memlist_cnt->Start, memlist_cnt->End, memlist_cnt->segelemidx );
	}


	/*
	 * Make the links from the Segelems back to the Memlist
	 */
	for( segelemcnt=0; segelemcnt<seg->memlist_count; ++segelemcnt ) {
		(seg->seglist+((seg->memlist+segelemcnt)->segelemidx))->memelemidx = segelemcnt;
	}

	return true;
}


uint8_t *segment_getMemPtr(void)
{
	segment_t *seg;
	segelem_t *selem;
	memelem_t *melem;


	seg = segment+act_segment;
	selem = seg->seglist+seg->act_segelem;
	if( (melem=seg->memlist+selem->memelemidx)==NULL ) {
		return NULL;
	}
	else {
		return melem->Memarea+(selem->pc-selem->adr);
	}
}


bool memSort(memtab_t *memlist, memtab_t **memlist_end, seglistsize_t segidx)
{
	memtab_t *mc;
	uint32_t start, end;
	segment_t *seg;


	if( segidx>=segment_count ) {
		return false;
	}

	seg = segment+segidx;

	/* Get the boundaries for this segment */
	if( (seg->memlist_count==0 && seg->force) || seg->fillup ) {
		/* Dump the complete range */
		start = seg->startadr;
		end = seg->endadr;
	}
	else if( seg->memlist_count!=0 ) {
		/* Dump from start of first segment to then end of the last */
		start = seg->memlist->Start;
		end = (seg->memlist+seg->memlist_count-1)->End;
	}
	else {
		return true;
	}

	mc = memlist;
	while( mc<*memlist_end && end>mc->Start ) {
		++mc;
	}
	/* if new element is not at the first position, check if it overlaps with the previous one */
	if( mc!=memlist && start<(mc-1)->End ) {
		error(EM_SegmentsOverlapping_d_d_d_d, start, end, (mc-1)->Start, (mc-1)->End);
		return false;
	}
	/* if new element is not at the end of the list, move rest 1 up */
	if( mc<*memlist_end ) {
		memmove( mc+1, mc, (*memlist_end-mc)*sizeof(memtab_t));
	}

	/*   Fill new element with values  */
	mc->Start = start;
	mc->End = end;
	mc->segelemidx = segidx;
	++*memlist_end;
	return true;
}


bool sortedListDump(char *fname, memtab_t *memlist, memtab_t *me, bool writesadr, uint8_t fillbyte )
{
	FILE *fh = NULL;
	uint32_t dumpadr;
	memtab_t *mc;
	bool wsadr;


	wsadr = writesadr;
	/* Open the file */
	if( (fh=fopen(fname,"wb"))==NULL ) {
		error(EM_CreateFile_s, fname);
		return false;
	}

	/* process all memlist elements */
	dumpadr = memlist->Start;
	for( mc=memlist; mc<me; ++mc ) {
		/* fill the space between the segments */
		while( dumpadr<mc->Start ) {
			fputc( (int)(fillbyte), fh);
			++dumpadr;
		}
		if( ferror(fh) ) {
			error(EM_WriteError_s, fname);
			fclose( fh );
			return false;
		}

		if( !segment_writeSegment( fh, mc->segelemidx, wsadr ) ) {
			error(EM_WriteError_s, fname);
			fclose( fh );
			return false;
		}
		/* Only write the startadress of the first segment */
		wsadr = false;

		dumpadr=mc->End;
	};
	fclose( fh );
	return true;
}


bool segment_writeFiles(char *ofile)
{
	outfile_t *oc, *oe;
	bool writesadr;
	uint8_t fillbyte;
	seglistsize_t segcnt;
	segment_t *segm;

	seglistsize_t *seglist = NULL;
	memtab_t *memlist = NULL;
	char *cname = NULL;
	FILE *fh = NULL;

	memtab_t *me;


	if( outfile_count==0 ) {
		/*
		 * No #outfile present
		 * default action: dump all abs segments in one file, with a startadress, sorted and with fillbyte $00
		 */

		/* Alloc the table for the memtab */
		if( (memlist=(memtab_t*)(malloc(segment_count*sizeof(memtab_t))))==NULL ) {
			systemError(EM_OutOfMemory);
			goto __ERROR_EXIT__;
		}
		me = memlist;

		/* Sort all segments */
		for( segcnt=0; segcnt<segment_count; ++segcnt ) {
			segm = segment+segcnt;
			/* only dump abs segments by default */
			if( segm->typ==SEGTYP_ABS ) {
				if( !memSort( memlist, &me, segcnt ) ) {
					goto __ERROR_EXIT__;
				}
			}
		}
		if( !sortedListDump(ofile, memlist, me, true, 0x00 ) ) {
			goto __ERROR_EXIT__;
		}
	}
	else {
		for( oe=(oc=outfile)+outfile_count; oc<oe; ++oc ) {
			/* First get the filename in a uniform way (out beloved h-fields ;) */
			if( oc->filename!=NULL ) {
				cname = string2cstr( oc->filename );
			}
			else {
				cname = ofile;
			}

			/* write a startadress to this file? */
			writesadr = oc->writesadr;
			/* fillbyte for the gaps between the segments */
			fillbyte = oc->fillbyte;

			/* show the info */
			printf("outfile: %s\n", cname);
			for( segcnt=0; segcnt<oc->segnames_count; ++segcnt ) {
				printf("segment %u: \"", segcnt);
				printString(stdout, *(oc->segnames+segcnt) );
				printf("\"\n");
			}

			/* Alloc space for the list of segment indices */
			if( (seglist=(seglistsize_t*)malloc(oc->segnames_count*sizeof(seglistsize_t)))==NULL ) {
				systemError(EM_OutOfMemory);
				goto __ERROR_EXIT__;
			}
			/*
			 * get the indices for the segment names
			 * (error if a name does not exist in the segment table)
			 * and
			 */
			for( segcnt=0; segcnt<oc->segnames_count; ++segcnt ) {
				if( (*(seglist+segcnt)=segment_findSegment(*(oc->segnames+segcnt)))==((seglistsize_t)-1) ) {
					error(EM_UnknownSegment);
					goto __ERROR_EXIT__;
				}
			}

			if( oc->sort ) {
				/* Alloc the table for the memtab */
				if( (memlist=(memtab_t*)(malloc(oc->segnames_count*sizeof(memtab_t))))==NULL ) {
					systemError(EM_OutOfMemory);
					goto __ERROR_EXIT__;
				}
				me = memlist;
				for( segcnt=0; segcnt<oc->segnames_count; ++segcnt ) {
					if( !memSort( memlist, &me, *(seglist+segcnt) ) )
						goto __ERROR_EXIT__;
				}

				if( !sortedListDump(cname, memlist, me, oc->writesadr, oc->fillbyte ) ) {
					goto __ERROR_EXIT__;
				}
			}
			else {
				/* Open the file */
				if( (fh=fopen(cname,"wb"))==NULL ) {
					error(EM_CreateFile_s, cname);
					goto __ERROR_EXIT__;
				}

				for( segcnt=0; segcnt<oc->segnames_count; ++segcnt ) {
					if( !segment_writeSegment( fh, *(seglist+segcnt), writesadr ) ) {
						error(EM_WriteError_s, ofile);
						goto __ERROR_EXIT__;
					}
					/* Only write the startadress of the first segment */
					writesadr=false;
				}
				fclose( fh );
				if( cname!=ofile ) {
					free( cname );
				}
			}
		}
	}

	if( memlist!=NULL ) {
		free( memlist );
	}
	if( seglist!=NULL ) {
		free( seglist );
	}

	return true;

__ERROR_EXIT__:
	if( memlist!=NULL ) {
		free( memlist );
	}
	if( seglist!=NULL ) {
		free( seglist );
	}
	if( cname!=NULL && cname!=ofile ) {
		free( cname );
	}
	if( fh!=NULL ) {
		fclose( fh );
	}

	return false;
}


bool segment_writeSegment(FILE *ofh, seglistsize_t segidx, bool startadr)
{
	seglistsize_t melemcnt;
	segment_t *seg;
	memelem_t *melem;
	uint32_t dumpadr, seglen;
	uint8_t fillbyte;
	bool fillup;


	seg = segment+segidx;
	fillbyte = seg->fillbyte;
	if( seg->memlist_count!=0 ) {
		dumpadr = seg->startadr;
		fillup = seg->fillup;
		/* maybe we can dump the startadress of the segment here */
		for(melemcnt=0; melemcnt<seg->memlist_count; ++melemcnt) {
			melem = seg->memlist+melemcnt;
			if( !melemcnt && startadr ) {
				fputc( (int)( melem->Start     & 0xff), ofh);
				fputc( (int)((melem->Start>>8) & 0xff), ofh);
			}
			/* No fillup of the segment by default */
			if( melemcnt || fillup ) {
				while(dumpadr<melem->Start) {
					fputc( (int)fillbyte, ofh);
					++dumpadr;
				}
			}
			else {
				dumpadr=melem->Start;
			}
			seglen = melem->End-melem->Start;
			if( fwrite( melem->Memarea, 1, seglen, ofh )!=seglen ) {
				return false;
			}
			dumpadr=melem->End;
		}
		if( fillup ) {
			while(dumpadr<seg->endadr) {
				fputc( (int)fillbyte, ofh);
				++dumpadr;
			}
		}
	}
	else if( seg->force ) {
		for( dumpadr=seg->startadr; dumpadr<seg->endadr; ++dumpadr ) {
			fputc( (int)fillbyte, ofh);
		}
	}

	return true;
}


bool segment_newOutfile( stringsize_t *filename, bool sort, bool writesadr, uint8_t fillbyte, stringsize_t **segnames, seglistsize_t segnames_count )
{
	outfile_t *of;
	outfile_t *newbuf;
	seglistsize_t newlen;


	if( outfile_count >= outfile_buflen ) {
		if( outfile_buflen==((seglistsize_t)-1) ) {
			error(EM_TooManyOutfiles);
			return false;
		}

		if( (newlen=outfile_buflen<<1)<outfile_buflen ) {
			newlen=((seglistsize_t)-1);
		}
		if( (newbuf=(outfile_t*)(realloc(outfile, newlen*sizeof(outfile_t))))==NULL ) {
			systemError(EM_OutOfMemory);
			return false;
		}
		outfile = newbuf;
		outfile_buflen = newlen;
	}

	/* fill new entry with the values */
	(of=outfile+outfile_count)->filename = filename;
	of->sort = sort;
	of->writesadr = writesadr;
	of->fillbyte = fillbyte;
	of->segnames = segnames;
	of->segnames_count = segnames_count;

	++outfile_count;
	return true;
}

