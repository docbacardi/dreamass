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

bool allBytesResolved = false;
bool allTermsFinal;

uint32_t errorcnt = 0;
uint32_t warningcnt = 0;

bool dumpPass;

/*
 * return value for char conversion if no suitable equivalent could be found
 * in the destination charset
 */
#define NO_CHAR '.'

/*-----------------------------------*/
/* locals                            */

bool pass_eparse(void);

/*-----------------------------------*/
/* initialize a new assembly pass    */

void newPass(void)
{
	errorcnt = 0;
	warningcnt = 0;
}


bool pass_parse(sourcefile_t *src)
{
	const lineelement_t *lelem;


	segment_reset();

	allBytesResolved = true;
	allTermsFinal = true;

	dumpPass = false;
	/* Set Pet/Screen Mode to the default */
	reset_TextMode();

	src_reset(src);
	if( !pp_init(src) ) {
		return false;
	}
	resetResolveCounter();

	while( (lelem=pp_peek())!=NULL && lelem->typ!=LE_EOF )
	{
		if( !pass_eparse() ) {
			pp_skipGarbage();
		}

		/* Change SegElem if PC was redefined in this line */
		segment_processChange();

		/* if the number of errors reached the maximum value, stop parsing now */
		if( errorcnt>=cfg_maxerrors ) {
			printf("maximum number of errors reached. aborting!\n");
			break;
		}
	};

	if( errorcnt==0 )
	{
		if( segment_getPhaseIdx()!=0 ) {
			error(EM_PseudoPCStillOpenAtEnd);
			return false;
		}

		if( getLocalBlock()!=0 ) {
			error(EM_LocalBlockStillOpenAtEnd);
			return false;
		}
	}

	return true;
}


bool pass_eparse(void)
{
	const lineelement_t *le;


	if( !pass_line() )
		return false;
	if( (le=pp_get())==NULL )
		return false;
	if( le->typ!=LE_EOL )
	{
		error(EM_GarbageAtEndOfLine);
		return false;
	}
	return true;
}


bool pass_line(void)
{
	lineelement_t *lelem;
	lineelement_t *vle;
	lineelement_t nlelem;
	length_t len;
	int16_t rel;
	int32_t relLong;

	VARIABLE var;
	elength_t ve;
	linesize_t linepos_start;
	seglistsize_t segelemidx;
	const lineelement_t pcd = { LE_OPERAND, { OP_StarEq }};


	while( 1 )
	{
		vle=(lineelement_t*)pp_peek();
		if( vle==NULL ) {
			return false;
		}
		if( vle->typ==LE_EOL ) {
			break;
		}
		linepos_start = pp_getPos();

		len.len = 0;
		len.defined = true;

		switch( vle->typ )
		{
		case LE_NUMBER:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
			error(EM_NumberWithoutOperand);
			return false;
		case LE_OPERAND:
			/* soft lineend? */
			if( vle->data.op==OP_Colon ) {
				/* replace ':' with LE_EOL */
				vle->typ = LE_EOL;
				break;
			}
			else if( vle->data.op==OP_Carro ) {
				/*
				 * if a single '#' ends up here which was not interpreted as a
				 * preproc command or a 'mne #number', it's an unknown preproc error!
				 */
				error(EM_UnknownPreproc);
				return false;
			}
			else if( vle->data.op==OP_Dot ) {
				/*
				 * if a single '.' ends up here which was not interpreted as a
				 * psyopcode or a macro, it's an unknown psyopcode error!
				 */
				error(EM_UnknownPsyOpcodeOrMacro);
				return false;
			}
			else if( !pp_skip() ) {
				return false;
			}

			lelem = (lineelement_t*)pp_peek();
			if( lelem==NULL ) {
				return false;
			}
			if( vle->data.op==OP_StarEq ||
			    ( vle->data.op==OP_Star && lelem->typ==LE_OPERAND && lelem->data.op==OP_Eq ) )
			{
				if( vle->data.op==OP_Star && !pp_skip() ) {
					return false;
				}
				lelem = (lineelement_t*)pp_peek();
				if( lelem==NULL ) {
					return false;
				}
				if( lelem->typ==LE_EOL )
				{
					segelemidx = segment_newSegElem(false, 0);
					pp_delItems(linepos_start, pp_getPos());
					vle->typ = LE_SEGELEMENTER;
					vle->data.segelemidx = segelemidx;
				}
				else if( !term_readVardef(&pcd, linepos_start) )
				{
					error(EM_AssignmentHasNoRightSide);
					return false;
				}
			}
			else if( !term_readVardef(vle, linepos_start) )
			{
				error(EM_AssignmentHasNoRightSide);
				return false;
			}
			break;
		case LE_PSYOPC:
			if( (ve=pass_psyopc()).err )
				return false;
			len = ve.len;
			break;
		case LE_MNE:
			if( (ve=parse_mne()).err )
				return false;
			len = ve.len;
			break;
		case LE_STRING:
			error(EM_StringWithoutOperand);
			return false;
		case LE_TEXT:
			if( !pp_skip() )
				return false;

			lelem = (lineelement_t*)pp_peek();
			if( lelem==NULL ) {
				return false;
			}
			switch( lelem->typ )
			{
			case LE_OPERAND:
				if( lelem->data.op!=OP_Colon )
				{
					/* and parse all together as a term */
					if( !term_readVardef(vle, linepos_start) )
					{
						error(EM_AssignmentHasNoRightSide);
						return false;
					}
					break;
				}
				else
				{
					/* clear ':' */
					lelem->typ = LE_EMPTY;
					/* and fallthrough */
				}
			case LE_EOF:
			case LE_EOL:
			case LE_PSYOPC:
			case LE_MNE:
				if( !term_makePCVar(vle->data.txt, linepos_start) )
					return false;
				break;
			case LE_NUMBER:
			case LE_STRING:
			case LE_TEXT:
			case LE_TEXTNUM1:
			case LE_TEXTNUM2:
			case LE_TEXTNUM3:
			case LE_TEXTNUM4:
				error(EM_ExpectingVarAssignment);
				return false;

			case BE_1BYTE:
			case BE_2BYTE:
			case BE_3BYTE:
			case BE_4BYTE:
			case BE_nBYTE:
			case TE_1BYTE:
			case TE_2BYTE:
			case TE_3BYTE:
			case TE_4BYTE:
			case TE_RBYTE:
			case TE_RLBYTE:
			case TE_FLEX:
			case TE_nBYTE:
			case TE_JIADR:
			case BE_DSB:
			case LE_EMPTY:
			case LE_SRC:
			case LE_VARDEF:
			case LE_VARTERM:
			case LE_SEGMENTENTER:
			case LE_SEGELEMENTER:
			case LE_LOCALBLOCK:
			case LE_PHASE:
			case LE_DEPHASE:
			case LE_SETCODE:
			case LE_ASIZE:
			case LE_XYSIZE:
			case LE_CPUTYPE:
			case LE_MACRODEF:
			case LE_PREPROC:
				/* This should never happen! */
				assert( false );
				break;
			};

			break;
		case BE_1BYTE:
		case BE_2BYTE:
		case BE_3BYTE:
		case BE_4BYTE:
			if( !pp_skip() )
				return false;
			len.len = (vle->typ&LINEELEMENT_TYPMASK)+1;
			break;
		case BE_DSB:
			if( !pp_skip() )
				return false;
			len.len = vle->data.dsb.length;
			break;
		case BE_nBYTE:
			if( !pp_skip() )
				return false;
			len.len = *(vle->data.b_nbyte);
			break;
		case TE_1BYTE:
		case TE_2BYTE:
		case TE_3BYTE:
		case TE_4BYTE:
			if( !read_term((TERMSIZE)(vle->typ&LINEELEMENT_TYPMASK),&var, true) )
				return false;
			allBytesResolved &= var.defined;
			if( var.defined )
				allTermsFinal &= var.final;
			len.len = (vle->typ&LINEELEMENT_TYPMASK)+1;
			break;
		case TE_RBYTE:
			if( !read_term(TS_RBYTE,&var, false) )
				return false;
			allBytesResolved &= var.defined;
			if( var.defined && segment_isPCDefined() )
			{
				pp_delItems(linepos_start, pp_getPos());
				nlelem.typ = BE_1BYTE;
				if( (rel=adr2rel(var.valt.value.num,segment_getPC()))==-1 )
					return false;
				nlelem.data.b_1byte = (uint8_t)rel;
				pp_replaceItem(linepos_start, &nlelem);
			}
			len.len = 1;
			break;
		case TE_RLBYTE:
			if( !read_term(TS_RLBYTE,&var, false) )
				return false;
			allBytesResolved &= var.defined;
			if( var.defined && segment_isPCDefined() )
			{
				pp_delItems(linepos_start, pp_getPos());
				nlelem.typ = BE_2BYTE;
				if( (relLong=adr2relLong(var.valt.value.num,segment_getPC()))==-1 )
					return false;
				nlelem.data.b_2byte = (uint8_t)relLong;
				pp_replaceItem(linepos_start, &nlelem);
			}
			len.len = 1;
			break;
		case TE_FLEX:
			if( !read_term(TS_FLEX,&var, true) )
				return false;
			if( (allBytesResolved&=(len.defined=var.defined)) )
				len.len = (vle->typ&LINEELEMENT_TYPMASK)+1;
			if( var.defined )
				allTermsFinal &= var.final;
			break;
		case TE_nBYTE:
			if( !read_term(TS_nBYTE, &var, true) )
				return false;
			if( (allBytesResolved&=(len.defined=var.defined)) )
				len.len = var.valt.byteSize;
			if( var.defined )
				allTermsFinal &= var.final;
			deleteVariable( &var );
			break;
		case TE_JIADR:
			if( !read_term(TS_2BYTE,&var, true) )
				return false;
			allBytesResolved &= var.defined;
			checkJmpInd(&var);
			len.len = 2;
			break;
		case LE_PREPROC:
		case LE_EOF:
		case LE_EOL:
		case LE_EMPTY:
		case LE_SRC:
		case LE_VARDEF:
		case LE_VARTERM:
		case LE_SEGMENTENTER:
		case LE_SEGELEMENTER:
		case LE_LOCALBLOCK:
		case LE_PHASE:
		case LE_DEPHASE:
		case LE_SETCODE:
		case LE_ASIZE:
		case LE_XYSIZE:
		case LE_CPUTYPE:
		case LE_MACRODEF:
			/* should never happen */
			assert( false );
		};

		segment_addLength(len);
	};
	return true;
}


uint8_t Asc2Pet(uint8_t c)
{
	if(c==0xa3)
		return 0x5c;				/*  pound -> 5c  */
	else if(c <0x20)
		return NO_CHAR;				/*  00 - 1f : nothing  */
	else if(c <0x41)
		return c;				/*  20 - 40 : same  */
	else if(c <0x5b)
		return c+0x80;				/*  41 - 5a -> c0 - da  */
	else if(c==0x5b || c==0x5d)  return c;          /*  5b, 5d -> 5b, 5d  */
	else if(c==0x5c)  return 0x6d;                  /*  backslash -> shift M  */
	else if(c==0x5e)  return c;                     /*  ^ -> arrow up  */
	else if(c==0x5f)  return 0xa4;                  /*  _ -> C= @  */
	else if(c==0x60)  return NO_CHAR;               /*  ` -> nothing  */
	else if(c <0x7b)  return c-0x20;                /*  61 - 7a -> 41 - 5a  */
	else if(c==0x7c)  return 0xdd;                  /*  | -> C= -  */
	else              return NO_CHAR;               /*  7b, 7d, 7e, 7f -> nothing  */
}


uint8_t Asc2Scr(uint8_t c)
{
	if(c==0xa3)      return 0x1c;			/*  pound -> 1c  */
	else if(c <0x20)  return NO_CHAR;		/*  00 - 1f : nothing  */
	else if(c==0x40)  return 0;			/*  @ -> 00  */
	else if(c <0x5b)  return c;			/*  20 - 5a : same  */
	else if(c==0x5b || c==0x5d)  return c-0x40;	/*  5b, 5d -> 1b, 1d  */
	else if(c==0x5c)  return 0x4d;			/*  backslash -> shift M  */
	else if(c==0x5e)  return 0x1e;			/*  ^ -> arrow up  */
	else if(c==0x5f)  return 0x64;			/*  _ -> C= @  */
	else if(c==0x60)  return NO_CHAR;		/*  ` -> nothing  */
	else if(c <0x7b)  return c-0x60;		/*  61 - 7a -> 01 - 1a  */
	else if(c==0x7c)  return 0x5d;			/*  | -> C= -  */
	else              return NO_CHAR;		/*  7b, 7d, 7e, 7f -> nothing  */
}


bool pass_dump(sourcefile_t *src)
{
	const lineelement_t *lelem;
	uint16_t dsb_cnt;
	stringsize_t nb_cnt;
	uint8_t *memptr;
	length_t vlen;
	bool final;
	VARIABLE var;


	vlen.defined = true;
	vlen.len = 0;

	dumpPass = true;

	if( !segment_memDump() )
		return false;

	/*
	 * finalize all terms
	 */
	final = allTermsFinal;

	if( !final && cfg_verbose )
	{
		printf("Finalizing Terms\n");
		/*  fflush(stdout);  */
	}
	while( !final )
	{
		if( cfg_verbose )
		{
			printf(".\n");
		}
		final = true;

		src_reset(src);
		segment_reset();
		if( !pp_init(src) )
			return false;

		while( 1 )
		{
			lelem = pp_peek();
			if( lelem==NULL ) {
				return false;
			}
			if( lelem->typ==LE_EOF ) {
				break;
			}
			switch( lelem->typ )
			{
			/* Add the elements length to the PC */
			case BE_1BYTE:
				pp_skip();
				vlen.len=1;
				break;
			case BE_2BYTE:
				pp_skip();
				vlen.len=2;
				break;
			case BE_3BYTE:
				pp_skip();
				vlen.len=3;
				break;
			case BE_4BYTE:
				pp_skip();
				vlen.len=4;
				break;
			case BE_DSB:
				pp_skip();
				vlen.len=lelem->data.dsb.length;
				break;
			case BE_nBYTE:
				pp_skip();
				vlen.len=*lelem->data.b_nbyte;
				break;

			case LE_NUMBER:
			case LE_TEXTNUM1:
			case LE_TEXTNUM2:
			case LE_TEXTNUM3:
			case LE_TEXTNUM4:
			case LE_OPERAND:
			case LE_PREPROC:
			case LE_PSYOPC:
			case LE_MNE:
			case LE_STRING:
			case LE_TEXT:
			case TE_RBYTE:
			case TE_RLBYTE:
				/* this should never happen as all of these symbols should have been parsed */
				assert( false );
				break;

			case TE_1BYTE:
			case TE_2BYTE:
			case TE_3BYTE:
			case TE_4BYTE:
				if( !read_term((TERMSIZE)(lelem->typ&LINEELEMENT_TYPMASK),&var, true) )
					return false;
				assert( var.defined );
				final &= var.final;
				vlen.len = (lelem->typ&LINEELEMENT_TYPMASK)+1;
				break;
			case TE_FLEX:
				if( !read_term(TS_FLEX,&var, true) )
					return false;
				assert( var.defined );
				final &= var.final;
				vlen.len = (lelem->typ&LINEELEMENT_TYPMASK)+1;
				break;
			case TE_nBYTE:
				if( !read_term(TS_nBYTE,&var, true) )
					return false;
				assert( var.defined );
				final &= var.final;
				vlen.len = var.valt.byteSize;
				deleteVariable( &var );
				break;
			case TE_JIADR:
				if( !read_term(TS_2BYTE,&var, true) )
					return false;
				assert( var.defined );
				final &= var.final;
				checkJmpInd(&var);
				vlen.len = 2;
				break;

			case LE_EMPTY:
			case LE_EOF:
			case LE_SRC:
			case LE_VARDEF:
			case LE_VARTERM:
			case LE_SEGMENTENTER:
			case LE_SEGELEMENTER:
			case LE_LOCALBLOCK:
			case LE_PHASE:
			case LE_DEPHASE:
			case LE_SETCODE:
			case LE_ASIZE:
			case LE_XYSIZE:
			case LE_CPUTYPE:
			case LE_MACRODEF:
				/* should never happen */
				assert( false );
			case LE_EOL:
				/* Change SegElem if PC was redefined in this line */
				pp_skip();
				segment_processChange();
				vlen.len = 0;
				break;
			};
			segment_addLength(vlen);
		};
		if( cfg_verbose )
			putc('\n', stdout);
	};
	printf("I'm done\n");


	/*
	 * Ugly but quick hack to dump all bytes
	 */
	src_reset(src);
	segment_reset();
	if( !pp_init(src) )
	{
		return false;
	}

	while( (lelem=pp_get())->typ!=LE_EOF )
	{
		memptr = segment_getMemPtr();

		switch( lelem->typ )
		{
		case BE_1BYTE:
			assert(memptr!=NULL);
			*(memptr++) =  lelem->data.b_1byte;
			vlen.len=1;
			segment_addLength(vlen);
			break;
		case BE_2BYTE:
			assert(memptr!=NULL);
			*(memptr++) =  lelem->data.b_2byte     &0xff;
			*(memptr++) = (lelem->data.b_2byte>>8 )&0xff;
			vlen.len=2;
			segment_addLength(vlen);
			break;
		case BE_3BYTE:
			assert(memptr!=NULL);
			*(memptr++) =  lelem->data.b_3byte     &0xff;
			*(memptr++) = (lelem->data.b_3byte>>8 )&0xff;
			*(memptr++) = (lelem->data.b_3byte>>16)&0xff;
			vlen.len=3;
			segment_addLength(vlen);
			break;
		case BE_4BYTE:
			assert(memptr!=NULL);
			*(memptr++) =  lelem->data.b_4byte     &0xff;
			*(memptr++) = (lelem->data.b_4byte>>8 )&0xff;
			*(memptr++) = (lelem->data.b_4byte>>16)&0xff;
			*(memptr++) = (lelem->data.b_4byte>>24)&0xff;
			vlen.len=4;
			segment_addLength(vlen);
			break;
		case BE_DSB:
			assert(memptr!=NULL);
			for(dsb_cnt=0; dsb_cnt<lelem->data.dsb.length; dsb_cnt++)
			{
				*(memptr++) = lelem->data.dsb.fillbyte;
			}
			vlen.len=lelem->data.dsb.length;
			segment_addLength(vlen);
			break;
		case BE_nBYTE:
			assert(memptr!=NULL);
			for(nb_cnt=0; nb_cnt<*lelem->data.b_nbyte; nb_cnt++)
			{
				*(memptr++) = *((uint8_t*)lelem->data.b_nbyte+sizeof(stringsize_t)+nb_cnt);
			}
			vlen.len=*lelem->data.b_nbyte;
			segment_addLength(vlen);
			break;

		case LE_NUMBER:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
		case LE_OPERAND:
		case LE_PSYOPC:
		case LE_PREPROC:
		case LE_MNE:
		case LE_STRING:
		case LE_TEXT:
		case TE_1BYTE:
		case TE_2BYTE:
		case TE_3BYTE:
		case TE_4BYTE:
		case TE_RBYTE:
		case TE_RLBYTE:
		case TE_FLEX:
		case TE_nBYTE:
		case TE_JIADR:
			/* this should never happen as all of these symbols should have been parsed */
			assert( false );
			break;

		case LE_EMPTY:
		case LE_EOF:
		case LE_SRC:
		case LE_VARDEF:
		case LE_VARTERM:
		case LE_SEGMENTENTER:
		case LE_SEGELEMENTER:
		case LE_LOCALBLOCK:
		case LE_PHASE:
		case LE_DEPHASE:
		case LE_SETCODE:
		case LE_ASIZE:
		case LE_XYSIZE:
		case LE_CPUTYPE:
		case LE_MACRODEF:
			/* should never happen */
			assert( false );
		case LE_EOL:
			/* Change SegElem if PC was redefined in this line */
			segment_processChange();
			break;
		};
	};
	return true;
}


bool pass_showUndefs(sourcefile_t *src)
{
	const lineelement_t *lelem;


	dumpPass = true;

	src_reset(src);
	segment_reset();
	if( !pp_init(src) )
		return false;

	while( (lelem=pp_get())->typ!=LE_EOF )
	{
		switch( lelem->typ )
		{
		/*
		 * already parsed elements -> nothing undefined here
		 * ignore them
		 */
		case BE_1BYTE:
		case BE_2BYTE:
		case BE_3BYTE:
		case BE_4BYTE:
		case BE_DSB:
		case BE_nBYTE:

		/*
		 * not parsed yet, maybe an error before
		 * ignore them
		 */
		case LE_NUMBER:
		case LE_TEXTNUM1:
		case LE_TEXTNUM2:
		case LE_TEXTNUM3:
		case LE_TEXTNUM4:
		case LE_OPERAND:
		case LE_PSYOPC:
		case LE_PREPROC:
		case LE_MNE:
		case LE_STRING:
		case LE_TEXT:
			break;

		case TE_1BYTE:
		case TE_2BYTE:
		case TE_3BYTE:
		case TE_4BYTE:
		case TE_RBYTE:
		case TE_RLBYTE:
		case TE_FLEX:
		case TE_nBYTE:
		case TE_JIADR:
			term_findUndefs(lelem->data.termidx);
			break;

		case LE_EMPTY:
		case LE_EOF:
		case LE_SRC:
		case LE_VARDEF:
		case LE_VARTERM:
		case LE_SEGMENTENTER:
		case LE_SEGELEMENTER:
		case LE_LOCALBLOCK:
		case LE_PHASE:
		case LE_DEPHASE:
		case LE_SETCODE:
		case LE_ASIZE:
		case LE_XYSIZE:
		case LE_CPUTYPE:
		case LE_MACRODEF:
			/* should never happen */
			assert( false );
		case LE_EOL:
			/* Change SegElem if PC was redefined in this line */
			segment_processChange();
			break;
		};
	};
	return true;
}

