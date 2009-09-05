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

elength_t psy_align(linesize_t linepos_start);
elength_t psy_binclude(linesize_t linepos_start);
elength_t psy_dsb(linesize_t linepos_start);
elength_t psy_regsize(linesize_t linepos_start, int reg);
elength_t psy_cpu(linesize_t linepos_start);
elength_t readData(linesize_t linepos_start, TERMSIZE maxSize, bool lPetMode);
elength_t psy_debug(void);

bool PetMode;

/*-----------------------------------*/

const psyopc_s psyopc[24] =
{
	{ "db",		2,	PSY_DB },
	{ "dw",		2,	PSY_DW },
	{ "dt",		2,	PSY_TEXT },
	{ "dp",		2,	PSY_PET },
	{ "ds",		2,	PSY_SCR },
	{ "setpet",	6,	PSY_SETPET },
	{ "setscr",	6,	PSY_SETSCR },
	{ "dsb",	3,	PSY_DSB },
	{ "align",	5,	PSY_ALIGN },
	{ "(",		1,	PSY_BLOCKSTART },
	{ ")",		1,	PSY_BLOCKEND },
	{ "binclude",	8,	PSY_BINCLUDE },
	{ "segment",	7,	PSY_SEGMENT },
	{ "pseudopc",	8,	PSY_PSEUDOPC },
	{ "realpc",	6,	PSY_REALPC },
	{ "asize",	5,	PSY_ASIZE },
	{ "xysize",	6,	PSY_XYSIZE },
	{ "cpu",	3,	PSY_CPU },
	{ "debugcmd",	8,	PSY_DEBUGCMD },

	/*   some aliases  */
	{ "byte",	4,	PSY_DB },
	{ "word",	4,	PSY_DW },
	{ "text",	4,	PSY_TEXT },
	{ "pet",	3,	PSY_PET },
	{ "scr",	3,	PSY_SCR }
};


/* Set Pet/Screen Mode to the default */
void reset_TextMode(void)
{
	PetMode = true;
}


elength_t pass_psyopc(void)
{
	lineelement_t *lelem;
	elength_t elength = { true, { true, 0 }};
	VARIABLE datavar;
	linesize_t linepos_start;
	localdepth_t newindex;
	seglistsize_t segidx;
	char *cstr;
	length_t undefSegmentLength = { false, 0 };


	linepos_start = pp_getPos();
	if( (lelem=(lineelement_t*)pp_get())==NULL )
		return elength;

	switch( lelem->data.psyopc )
	{
	case PSY_DSB:
		elength = psy_dsb(linepos_start);
		break;
	case PSY_DB:
		elength = readData(linepos_start, TS_1BYTE, PetMode);
		break;
	case PSY_DW:
		elength = readData(linepos_start, TS_2BYTE, PetMode);
		break;
	case PSY_ALIGN:
		elength = psy_align(linepos_start);
		break;
	case PSY_TEXT:
		elength = readData(linepos_start, TS_nBYTE, PetMode);
		break;
	case PSY_PET:
		elength = readData(linepos_start, TS_nBYTE, true);
		break;
	case PSY_SCR:
		elength = readData(linepos_start, TS_nBYTE, false);
		break;

	case PSY_SETPET:
		lelem->typ = LE_SETCODE;
		lelem->data.code = PetMode = true;
		elength.err = false;
		break;
	case PSY_SETSCR:
		lelem->typ = LE_SETCODE;
		lelem->data.code = PetMode = false;
		elength.err = false;
		break;

	case PSY_BLOCKSTART:
		pp_delItems(linepos_start, pp_getPos());
		if( (newindex=localDive())==(localdepth_t)-1 )
			return elength;
		lelem->typ=LE_LOCALBLOCK;
		lelem->data.blockidx=newindex;
		elength.err = false;
		break;
	case PSY_BLOCKEND:
		pp_delItems(linepos_start, pp_getPos());
		if( (newindex=localUp())==(localdepth_t)-1 )
		{
			error(EM_NoOpeningLocalBlock);
			return elength;
		}
		lelem->typ=LE_LOCALBLOCK;
		lelem->data.blockidx=newindex;
		elength.err = false;
		break;
	case PSY_BINCLUDE:
		elength = psy_binclude(linepos_start);
		break;
	case PSY_SEGMENT:
		if( !read_term(TS_STRING,&datavar, false) )
		{
			error(EM_NoSegmentName);
			return elength;
		}
		if( !datavar.defined )
		{
			/* no undef segments yet */
			error(EM_UndefVarInSegSwitch);
			return elength;
		}
		if( (segidx=segment_findSegment(datavar.valt.value.str))==((seglistsize_t)-1) )
		{
			/* no unknown segments yet */
			if( (cstr=string2cstr(datavar.valt.value.str))!=NULL )
			{
				error(EM_UndefSegment_s, cstr);
				free(cstr);
			} else {
				error(EM_UndefSegment_s, "?");
			}
			free(datavar.valt.value.str);
			return elength;
		}

		free(datavar.valt.value.str);
		pp_delItems(linepos_start, pp_getPos());
		lelem->typ = LE_SEGMENTENTER;
		lelem->data.segmentidx = segidx;
		segment_enterSegment(segidx);
		elength.err = false;
		break;
	case PSY_PSEUDOPC:
		if( !read_term(TS_2BYTE,&datavar, false) )
		{
			error(EM_PseudopcWithoutArg);
			return elength;
		}

		if( datavar.defined )
		{
			pp_delItems(linepos_start, pp_getPos());
			lelem->typ = LE_PHASE;
			lelem->data.phasepc = datavar.valt.value.num;
			elength.err = !segment_phase( datavar.valt.value.num );
		}
		else {
			/* var not defined -> enter new phase and undefine pc */
			elength.err = !segment_phase(0);
			segment_addLength(undefSegmentLength);
		}
		break;
	case PSY_REALPC:
		pp_delItems(linepos_start, pp_getPos());
		lelem->typ = LE_DEPHASE;
		elength.err = !segment_dephase();
		break;
	case PSY_ASIZE:
		elength = psy_regsize(linepos_start, 0);
		break;
	case PSY_XYSIZE:
		elength = psy_regsize(linepos_start, 1);
		break;
	case PSY_CPU:
		elength = psy_cpu(linepos_start);
		break;
	case PSY_DEBUGCMD:
		elength = psy_debug();
		break;
	};

	return elength;
}


elength_t psy_align(linesize_t linepos_start)
{
	elength_t elength = { true, { true, 0 }};
	VARIABLE align_mask, dsb_byte;
	lineelement_t *lelem;
	linesize_t linepos_end;
	lineelement_t nlelem;
	uint16_t dsb_len;


	/* set default byte for fill */
	dsb_byte.valt.typ = VALTYP_NUM;
	dsb_byte.defined = true;
	dsb_byte.valt.value.num = 0;
	
	dsb_len = 0;

	if( !read_term(TS_2BYTE,&align_mask, false) )
	{
		error(EM_AlignWithoutArg);
		return elength;
	}

	lelem = (lineelement_t*)pp_peek();
	if( lelem==NULL ) {
		return elength;
	}
	if( lelem->typ==LE_OPERAND && lelem->data.op==OP_Comma )
	{
		if( !pp_skip() )
			return elength;

		if( !read_term(TS_1BYTE,&dsb_byte, false) )
		{
			error(EM_AlignWithoutArg);
			return elength;
		}
	}
	linepos_end = pp_getPos();

	if( align_mask.defined && dsb_byte.defined && segment_isPCDefined() )
	{
		pp_delItems(linepos_start, linepos_end);
		if( (dsb_len=segment_getPC()%align_mask.valt.value.num)!=0 )
		{
			dsb_len = align_mask.valt.value.num-dsb_len;
			nlelem.typ = BE_DSB;
			nlelem.data.dsb.length = dsb_len;
			nlelem.data.dsb.fillbyte = dsb_byte.valt.value.num;
			pp_replaceItem(linepos_start, &nlelem);
		}
	}
	else
		allBytesResolved = false;

	elength.len.len = dsb_len;

	elength.err=false;
	return elength;
}


elength_t psy_binclude(linesize_t linepos_start)
{
	elength_t elength = { true, { true, 0 }};
	const lineelement_t *lelem2;
	stringsize_t *filename, *fnameclone;
	VARIABLE binc_offs = { true, false, true, false, (seglistsize_t)-1, { VALTYP_NUM, 0, { 0 }}};
	VARIABLE binc_len = { false, false, false, false, (seglistsize_t)-1, { VALTYP_NUM, 0, { 0 }}};
	bool lendef = false;
	int bincfile;
	struct stat statbuf;
	off_t filesize;
	lineelement_t nlelem;
	char *cfname;


	lelem2 = pp_get();
	if( lelem2->typ!=LE_STRING )
	{
		error(EM_MissingFilename);
		return elength;
	}

	filename = lelem2->data.str;

	lelem2 = pp_peek();
	if( lelem2==NULL )
	{
		return elength;
	}
	if( lelem2->typ==LE_OPERAND && lelem2->data.op==OP_Comma )
	{
		if( !pp_skip() )
		{
			return elength;
		}

		if( !read_term(TS_4BYTE,&binc_offs, false) )
		{
			error(EM_NoArgAfterComma);
			return elength;
		}

		lelem2 = pp_peek();
		if( lelem2==NULL )
		{
			return elength;
		}
		if( lelem2->typ==LE_OPERAND && lelem2->data.op==OP_Comma )
		{
			if( !pp_skip() )
			{
				return elength;
			}

			if( !read_term(TS_2BYTE,&binc_len, false) )
			{
				error(EM_NoArgAfterComma);
				return elength;
			}
			lendef=true;
		}
	}

	if( binc_offs.defined && ( !lendef || ( lendef && binc_len.defined ) ) )
	{
		fnameclone = stringClone(filename);

		pp_delItems(linepos_start, pp_getPos());
		if( (cfname=string2cstr(fnameclone))==NULL )
		{
			free(fnameclone);
			return elength;
		}

		/* Open the file for reading */
		if( (bincfile=filelist_ropen( fnameclone ))==-1 )
		{
			error(EM_FileNotFound_s, cfname);
			free( cfname );
			free( fnameclone );
			return elength;
		}

		free( fnameclone );

		/* Get Filesize */
		if( fstat(bincfile, &statbuf)==0 && S_ISREG(statbuf.st_mode) )
		{
			filesize=statbuf.st_size;
		}
		else
		{
			error(EM_FileNotFound_s, cfname);
			free( cfname );
			close(bincfile);
			return elength;
		}

		/* Test for valid parameters. */
		if( (off_t)binc_offs.valt.value.num>=filesize )
		{
			error(EM_OffsExcFilelen);
			free( cfname );
			close(bincfile);
			return elength;
		}
		if( lendef && (off_t)(binc_offs.valt.value.num+binc_len.valt.value.num)>filesize )
		{
			error(EM_LenExcFilelen);
			free( cfname );
			close(bincfile);
			return elength;
		}
		else if( !lendef && (binc_len.valt.value.num=filesize-binc_offs.valt.value.num)>0x10000 )
		{
			warning(WM_BIncOver64k);
			binc_len.valt.value.num = 0x10000;
		}

		/* Allocate buffer for data and length info */
		nlelem.typ = BE_nBYTE;
		nlelem.data.b_nbyte = (stringsize_t*)malloc(binc_len.valt.value.num+sizeof(stringsize_t));
		if( nlelem.data.b_nbyte==NULL )
		{
			systemError(EM_OutOfMemory);
			free( cfname );
			close(bincfile);
			return elength;
		}
		*(nlelem.data.b_nbyte) = binc_len.valt.value.num;

		/* seek to the desired position */
		if( binc_offs.valt.value.num!=0 && lseek(bincfile, binc_offs.valt.value.num, SEEK_SET)==((off_t)-1) )
		{
			error(EM_ReadError_s, cfname);
			free( cfname );
			close(bincfile);
			free(nlelem.data.b_nbyte);
			return elength;
		}

		/* read in the data */
		if( readFile(bincfile, (char*)(nlelem.data.b_nbyte+1), binc_len.valt.value.num)==false )
		{
			error(EM_ReadError_s, cfname);
			free( cfname );
			close(bincfile);
			free(nlelem.data.b_nbyte);
			return elength;
		}

		pp_replaceItem(linepos_start, &nlelem);
		close(bincfile);

		free( cfname );

		/* the length is set now */
		binc_len.defined = true;
	}

	elength.len.defined = binc_len.defined;
	elength.len.len = binc_len.valt.value.num;

	elength.err=false;
	return elength;
}


elength_t psy_dsb(linesize_t linepos_start)
{
	elength_t elength = { true, { true, 0 }};
	VARIABLE dsb_len, dsb_byte;
	lineelement_t *lelem;
	linesize_t linepos_end;
	lineelement_t nlelem;


	/* set default byte for fill */
	dsb_byte.valt.typ = VALTYP_NUM;
	dsb_byte.defined = true;
	dsb_byte.valt.value.num = 0;

	if( !read_term(TS_2BYTE,&dsb_len, false) )
	{
		error(EM_DSBWithoutArg);
		return elength;
	}

	lelem = (lineelement_t*)pp_peek();
	if( lelem==NULL ) {
		return elength;
	}
	if( lelem->typ==LE_OPERAND && lelem->data.op==OP_Comma )
	{
		if( !pp_skip() )
			return elength;

		if( !read_term(TS_1BYTE,&dsb_byte, false) )
		{
			error(EM_DSBWithoutArg);
			return elength;
		}
	}
	linepos_end = pp_getPos();

	if( dsb_len.defined && dsb_byte.defined )
	{
		pp_delItems(linepos_start, linepos_end);
		nlelem.typ = BE_DSB;
		nlelem.data.dsb.length = dsb_len.valt.value.num;
		nlelem.data.dsb.fillbyte = dsb_byte.valt.value.num;
		pp_replaceItem(linepos_start, &nlelem);
	}
	else
		allBytesResolved = false;

	elength.len.defined = dsb_len.defined;
	elength.len.len = dsb_len.valt.value.num;

	elength.err=false;
	return elength;
}


/*
 * Read in Data Elements and translate the Strings with the
 * CharPage cp. That is a 256 byte array with:
 *   cp[_pc_char_] = _c64_char_
 * Of course also other mappings than pc->c64 can be done
 * with this.
 */
/*
 * Sooo for now just scr/pet
 */
elength_t readData(linesize_t linepos_start, TERMSIZE maxSize, bool lPetMode)
{
	const lineelement_t *lelem;
	bool neot = true;
	bool oPetMode;
	VARIABLE datavar;
	elength_t elen = { true, { true, 0 }};


	oPetMode = PetMode;
	PetMode = lPetMode;
	do
	{
		/* delete '.text' or ',' */
		pp_delItems(linepos_start, pp_getPos());

		if( !read_term(maxSize,&datavar, true) )
		{
			error(EM_NoArgAfterComma);
			PetMode = oPetMode;
			return elen;
		}

		if( datavar.defined )
		{
			switch( datavar.valt.typ )
			{
			case VALTYP_NUM:
				switch( maxSize )
				{
				case TS_1BYTE:
					elen.len.len += 1;
					break;
				case TS_2BYTE:
					elen.len.len += 2;
					break;
				case TS_3BYTE:
					elen.len.len += 3;
					break;
				case TS_4BYTE:
					elen.len.len += 4;
					break;
				case TS_nBYTE:
				case TS_FLEX:
					elen.len.len += datavar.valt.byteSize;
					break;
				case TS_RBYTE:
				case TS_RLBYTE:
				case TS_STRING:
					/* should never happen */
					assert( false );
				};
				break;

			case VALTYP_STR:
				/* it's a String, set the length */
				elen.len.len += *datavar.valt.value.str;
				break;
			}
			deleteVariable( &datavar );
		}
		else
		{
			allBytesResolved = false;

			/* The term is undefined, but for fixed sizes we can get the length */
			switch( maxSize )
			{
			case TS_RBYTE:
			case TS_1BYTE:
				elen.len.len += 1;
				break;
			case TS_RLBYTE:
			case TS_2BYTE:
				elen.len.len += 2;
				break;
			case TS_3BYTE:
				elen.len.len += 3;
				break;
			case TS_4BYTE:
				elen.len.len += 4;
				break;
			case TS_nBYTE:
			case TS_FLEX:
			case TS_STRING:
				elen.len.defined = false;
				break;
			};
		}

		linepos_start = pp_getPos();
		lelem = pp_peek();
		if( lelem==NULL ) {
			return elen;
		}
		if( lelem->typ==LE_OPERAND && lelem->data.op==OP_Comma )
		{
			if( !pp_skip() )
			{
				PetMode = oPetMode;
				return elen;
			}
		}
		else
			neot = false;
	} while( neot );

	PetMode = oPetMode;
	elen.err = false;
	return elen;
}


elength_t psy_regsize(linesize_t linepos_start, int reg)
{
	elength_t elen = { true, { true, 0 }};
	VARIABLE datavar;
	lineelement_t nlelem;


	/* read in argument */
	if( !read_term(TS_1BYTE,&datavar, false) )
	{
		error(EM_RegSizeWithoutArg);
	}
	/*
	 * the argument must be defined (i.e. no preuse) and
	 * it must be final. Non final vars depend on the pc
	 * in a code segment wthout fixed pc address.
	 */
	else if( !datavar.defined || !datavar.final )
	{
		error(EM_RegSizeNotDefinedOrNotFinal);
	}
	/* test argument, must be 8 or 16, 16 is only allowed for 65816 cpus */
	else if( datavar.valt.value.num!=8 && datavar.valt.value.num!=16 )
	{
		error(EM_IllegalRegSizeArg_d, datavar.valt.value.num);
	}
	else if( datavar.valt.value.num==16 && getCurrentCpu()!=CPUTYPE_65815 )
	{
		error(EM_RegSize16NotAllowed_s, getCurrentCpuName());
	}
	else
	{
		pp_delItems(linepos_start, pp_getPos());
		nlelem.data.regsize = (uint8_t)datavar.valt.value.num;
		if( reg==0 )
		{
			nlelem.typ = LE_ASIZE;
			setRegisterSize_A(nlelem.data.regsize);
		}
		else
		{
			nlelem.typ = LE_XYSIZE;
			setRegisterSize_XY(nlelem.data.regsize);
		}
		pp_replaceItem(linepos_start, &nlelem);
		elen.err = false;
	}

	return elen;
}


elength_t psy_cpu(linesize_t linepos_start)
{
	elength_t elen = { true, { true, 0 }};
	VARIABLE datavar;
	lineelement_t nlelem;
	char *cpuName;
	CPUTYPE cpuType;


	/* read in argument */
	if( !read_term(TS_STRING,&datavar, false) )
	{
		error(EM_CpuWithoutArg);
	}
	/*
	 * the argument must be defined (i.e. no preuse) and
	 * it must be final. Non final vars depend on the pc
	 * in a code segment wthout fixed pc address.
	 */
	else if( !datavar.defined || !datavar.final )
	{
		deleteVariable( &datavar );
		error(EM_RegSizeNotDefinedOrNotFinal);
	}
	/* convert hstring to cstring (for easy compare) */
	else if( (cpuName=string2cstr(datavar.valt.value.str))!=NULL )
	{
		/* compare the cpu name with the known types */
		cpuType = getCpuIdx(cpuName);
		if( cpuType==CPUTYPE_UNKNOWN )
		{
			error(EM_UnknownCpuType_s, cpuName);
		}
		else
		{
			pp_delItems(linepos_start, pp_getPos());
			nlelem.typ = LE_CPUTYPE;
			nlelem.data.cputype = cpuType;
			pp_replaceItem(linepos_start, &nlelem);
			setCpuType(cpuType);
			elen.err = false;
		}
		free(cpuName);
		deleteVariable( &datavar );
	}

	return elen;
}


/*
 * This Command is just for debugging
 * to execute some code at a special line in a sourcefile
 */
elength_t psy_debug(void)
{
	elength_t elen = { true, { true, 0 }};


	printf("DebugCmd: PC ");
	if( segment_isPCDefined() )
		printf("$%04x\n", segment_getPC() );
	else
		printf("undefined\n");
	elen.err = false;

	return elen;
}


