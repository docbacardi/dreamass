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

extern bool allBytesResolved;

/*-----------------------------------*/
/* locals                            */


/*
 * current cpu (6510, 6510Ill or 65816)
 */
CPUTYPE currentCpu;

/*
 * current register sizes (8 or 16)
 */
uint8_t regsize_a;
uint8_t regsize_xy;

const uint32_t *validModes;

bool getNextOp(OPERAND *op);
ADRESSMODE getAdrMode(uint32_t adrKeys, bool varSizeKnown, OPCODE opc);
ADRESSMODE validateAddressmode(ADRESSMODE adrMode, uint32_t validAdrModes);
ADRESSMODE AdrKey2Mode(uint32_t adrkey);
ADRESSKEY scanAdrKeys(uint8_t sidx, uint8_t eidx);

/*-----------------------------------*/


void mne_init(void)
{
	/* default is 6510 */
	setCpuType(CPUTYPE_6510);
}


/*-----------------------------------*/


elength_t parse_mne(void)
{
	lineelement_t *mneelem;
	lineelement_t *termelem;
	const lineelement_t *lelem;
	linesize_t linepos_term;
	lineelement_t nlelem;
	uint32_t adrKeys=AK_NONE;		/* combination of multiple ADRESSKEY's */
	ADRESSMODE adrMode;
	VARIABLE var_numArg1, var_numArg2;
	bool varSizeKnown = true;
	const length_t length1 = { true, 1 };
	elength_t length = { true, { false, 0 }};
	OPCODE opc;
	amode_offset_s amode_offs;
	uint8_t mne_byte = 0;
	int16_t rel;
	int32_t relLong;
	char c;
	OPERAND op;


	var_numArg1.defined = var_numArg2.defined = false;
	var_numArg1.final = var_numArg2.final = false;

	/* get the opcode */
	mneelem = (lineelement_t*)pp_peek();
	if( mneelem==NULL ) {
		return length;
	}
	opc = mneelem->data.mne;
	if( !pp_eat() ) {
		return length;
	}

	/* test for '#', '(' and '[' */
	adrKeys |= scanAdrKeys(0, 3);
	if( adrKeys==AK_ERROR ) {
		return length;
	}

	/* remember term's position */
	linepos_term = pp_getPos();
	termelem = (lineelement_t*)pp_peek();
	if( termelem==NULL ) {
		return length;
	}
	switch( termelem->typ )
	{
	case LE_EMPTY:
	case LE_SRC:
	case LE_VARDEF:
	case LE_VARTERM:
	case LE_SEGMENTENTER:
	case LE_SEGELEMENTER:
	case LE_LOCALBLOCK:
	case LE_PHASE:
	case LE_DEPHASE:
	case LE_PREPROC:
	case LE_SETCODE:
	case LE_ASIZE:
	case LE_XYSIZE:
	case LE_CPUTYPE:
	case LE_MACRODEF:
	case TE_1BYTE:
	case TE_3BYTE:
	case TE_4BYTE:
	case TE_RBYTE:
	case TE_RLBYTE:
	case TE_nBYTE:
	case TE_JIADR:
	case TE_FLEX:
	case BE_1BYTE:
	case BE_2BYTE:
	case BE_3BYTE:
	case BE_4BYTE:
	case BE_DSB:
	case BE_nBYTE:
		/* Should never happen */
		assert( false );

	case LE_OPERAND:
		/* ':' after an opcode is a soft-lineend */
		if( termelem->data.op==OP_Colon )
		{
			break;
		}
		/* fallthrough */
	case LE_NUMBER:
	case TE_2BYTE:
	case LE_STRING:
	case LE_TEXT:
	case LE_TEXTNUM1:
	case LE_TEXTNUM2:
	case LE_TEXTNUM3:
	case LE_TEXTNUM4:
	/*
	 * Next two are not really startelements for a term,
	 * but read_term has a nice errorhandling for them :)
	 */
	case LE_PSYOPC:
	case LE_MNE:
		/* try to read in term */
		if( !read_term(TS_FLEX, &var_numArg1, true) )
		{
			/* serious error -> abort parsing */
			return length;
		}
		else if( var_numArg1.defined && var_numArg1.final )
		{
			/* set the address keys for the number according to the size */
			switch( var_numArg1.valt.byteSize )
			{
			case 1:
				adrKeys |= AK_NUMZP;
				break;
			case 2:
				adrKeys |= AK_NUMABS;
				break;
			case 3:
				adrKeys |= AK_NUMLONG;
				break;
			default:
				/* strange length, error! */
				error(EM_Overflow_d, var_numArg1.valt.value.num);
				return length;
			}
		}
		else
		{
			/* variable not defined yet -> default to worst case : absolute */
			adrKeys |= AK_NUMABS;
			varSizeKnown = false;
		}
		break;

	case LE_EOL:
	case LE_EOF:
		break;
	};

	segment_addLength(length1);

	/*
	 * All further addresskeys start with an operand.
	 * Ignore other lineelements, especially ':' which is soft_cr.
	 */
	if( !getNextOp(&op) ) {
		return length;
	}
	if( op!=OP_Colon )
	{
		/*
		 * first test for ',s'
		 * Note: this can not be joined with the checks for ',x', ',y' and ',Zp' later as
		 * we need the combination 'mne (Zp,s),y'.
		 */
		if( op==OP_Comma )
		{
			lelem = pp_peek();
			if( lelem==NULL ) {
				return length;
			}
			if( lelem->typ==LE_TEXT && *lelem->data.txt==1 )
			{
				c = tolower( *((char*)(lelem->data.txt+1)) );
				if( c=='s' )
				{
					adrKeys |= AK_COMMAS;
					/* free 's' */
					if( !pp_eat() )
					{
						return length;
					}
					/* get next operand */
					if( !getNextOp(&op) )
					{
						return length;
					}
				}
			}
		}

		/* test for ')' and ']' */
		if( op==OP_Bc )
		{
			adrKeys |= AK_BCLOSE1;
			if( !getNextOp(&op) )
			{
				return length;
			}
		}
		else if( op==OP_Sbc )
		{
			adrKeys |= AK_SBCLOSE;
			if( !getNextOp(&op) )
			{
				return length;
			}
		}

		/* test for ',x', ',y' and ',Zp' */
		if( op==OP_Comma )
		{
			lelem = pp_peek();
			if( lelem==NULL ) {
				return length;
			}
			if( lelem->typ==LE_TEXT && *lelem->data.txt==1 )
			{
				c = tolower( *((char*)(lelem->data.txt+1)) );
				if( c=='x' )
				{
					adrKeys |= AK_COMMAX;
					if( !pp_eat() )
					{
						return length;
					}
					/* get next operand */
					if( !getNextOp(&op) )
					{
						return length;
					}
				}
				else if( c=='y' )
				{
					adrKeys |= AK_COMMAY;
					if( !pp_eat() )
					{
						return length;
					}
					/* get next operand */
					if( !getNextOp(&op) )
					{
						return length;
					}
				}
				/* a textelement with size 1 can also be a variable or the start of a term, like m+2*n */
				else if( read_term(TS_1BYTE, &var_numArg2, true) )
				{
					adrKeys |= AK_COMMAZP;
					segment_addLength(length1);
					/* get next operand */
					if( !getNextOp(&op) )
					{
						return length;
					}
				}
				else
				{
					error(EM_UnknownAdressmode);
					return length;
				}
			}
			else if( read_term(TS_1BYTE, &var_numArg2, true) )
			{
				adrKeys |= AK_COMMAZP;
				segment_addLength(length1);
				/* get next operand */
				if( !getNextOp(&op) )
				{
					return length;
				}
			}
			else
			{
				error(EM_UnknownAdressmode);
				return length;
			}
		}

		if( op==OP_Bc )
		{
			adrKeys |= AK_BCLOSE2;
			/* no need to get next operand, this is the last element */
		}
	}

	/* translate addresskeys to addressmode */
	adrMode = getAdrMode(adrKeys, varSizeKnown, opc);
	if( adrMode==AM_Error )
	{
		error(EM_IllAdressmodeForOpcode);
		return length;
	}

	/* test the special cases */

	/*
	 * The adressclass defines how to calculate the byte value for this
	 * mne / adressmode combination. AC_GREEN and AC_BROWN depend on regular
	 * patterns in the opcode table, AC_DIRECT is for the rest
	 */
	amode_offs = amode_offset[opc];
	if( amode_offs.adressclass==AC_DIRECT ) {
		mne_byte = amode_offs.offset;
	}
	else {
		mne_byte = amode_offs.offset^amode_offs_groups[amode_offs.adressclass*AM_length + adrMode];
	}

	length.len.defined = true;
	length.len.len = amode_length[adrMode];

	/* it is ok to modify the lineelement directly */
	mneelem->typ = BE_1BYTE;
	mneelem->data.b_1byte = mne_byte;

	if( adrMode==AM_Rel )
	{
		/*
		 * Note: it does not matter if the value in 'var_numArg1' is final. Moving the PC
		 * does not affect relative brances.
		 */
		if( var_numArg1.defined && segment_isPCDefined() )
		{
			if( (rel=adr2rel(var_numArg1.valt.value.num, segment_getPC()))==-1 )
				return length;
			/*
			 * The lineelement can't be modified directly here. A non-final
			 * value is still a term.
			 */
			pp_delItems(linepos_term, linepos_term+1);
			nlelem.typ=BE_1BYTE;
			nlelem.data.b_1byte = (uint8_t)rel;
			pp_replaceItem(linepos_term, &nlelem);
		}
		else
		{
			allBytesResolved = false;

			/* Modify term type */
			termelem->typ = TE_RBYTE;
		}
	}
	else if( adrMode==AM_RelLong )
	{
		/*
		 * Note: it does not matter if the value in 'var_numArg1' is final. Moving the PC
		 * does not affect relative brances.
		 */
		if( var_numArg1.defined && segment_isPCDefined() )
		{
			if( (relLong=adr2relLong(var_numArg1.valt.value.num, segment_getPC()))==-1 )
				return length;
			/*
			 * The lineelement can't be modified directly here. A non-final
			 * value is still a term.
			 */
			pp_delItems(linepos_term, linepos_term+1);
			nlelem.typ = BE_2BYTE;
			nlelem.data.b_2byte = (uint16_t)relLong;
			pp_replaceItem(linepos_term, &nlelem);
		}
		else
		{
			allBytesResolved = false;

			/* Modify term type */
			termelem->typ = TE_RLBYTE;
		}
	}
	else if( length.len.len>0 )
	{
		/* Modify length according to adressmode */
		if( adrMode==AM_BrAbsBr && !checkJmpInd(&var_numArg1) )
		{
			termelem->typ = TE_JIADR;
			allBytesResolved = false;
		}
		else if( var_numArg1.defined && var_numArg1.final )
		{
			/* set the size of the binary value to the correct size for the addressmode */
			assert( termelem->typ==BE_1BYTE || termelem->typ==BE_2BYTE || termelem->typ==BE_3BYTE );
			switch(length.len.len)
			{
			case 1:
				termelem->typ = BE_1BYTE;
				termelem->data.b_1byte = (uint8_t)var_numArg1.valt.value.num;
				break;
			case 2:
				termelem->typ = BE_2BYTE;
				termelem->data.b_2byte = (uint16_t)var_numArg1.valt.value.num;
				break;
			case 3:
				termelem->typ = BE_3BYTE;
				termelem->data.b_3byte = (uint32_t)var_numArg1.valt.value.num;
				break;
			}
		}
		else
		{
			/* length must be between 1 and 4 bytes */
			assert( length.len.len>=1 && length.len.len<=4 );
			/* set the size of the term to the correct size for the addressmode */
			termelem->typ = (LINEELEMENT_TYP)(TE_1BYTE + (length.len.len-1));
			allBytesResolved = false;
		}
	}

	length.err = false;
	return length;
}


bool getNextOp(OPERAND *op)
{
	const lineelement_t *lelem;


	lelem = pp_peek();
	if( lelem==NULL )
	{
		return false;
	}
	if( lelem->typ==LE_OPERAND && (*op=lelem->data.op)!=OP_Colon )
	{
		if( !pp_eat() )
		{
			return false;
		}
	}
	else
	{
		*op = OP_Colon;
	}
	return true;
}


/*-----------------------------------*/


typedef struct
{
	ADRESSMODE oldmode;
	ADRESSMODE newmode;
	bool warn;
} amode_replace_t;

amode_replace_t amode_replace[] =
{
	{ AM_Zp,	AM_Rel,		false },
	{ AM_Abs,	AM_Rel,		false },
	{ AM_Zp,	AM_RelLong,	false },
	{ AM_Abs,	AM_RelLong,	false },
	{ AM_BrZpBr,	AM_BrAbsBr,	false },

	{ AM_Zp,	AM_Abs,		true },
	{ AM_Zp,	AM_Long,	true },
	{ AM_Abs,	AM_Long,	true },
	{ AM_ZpCoX,	AM_AbsCoX,	true },
	{ AM_ZpCoX,	AM_LongCoX,	true },
	{ AM_AbsCoX,	AM_LongCoX,	true },
	{ AM_ZpCoY,	AM_AbsCoY,	true }
};


ADRESSMODE getAdrMode(uint32_t adrKeys, bool varSizeKnown, OPCODE opc)
{
	ADRESSMODE adrMode;
	uint32_t validAdrModes;		/* multiple combinations of ADRESSMODEBITS */
	bool found;


	validAdrModes = validModes[opc];

	if( (validAdrModes&AMB_CaAsize)!=0 )
	{
		validAdrModes &= ~AMB_CaAsize;
		validAdrModes |= (regsize_a==8) ? AMB_CaZp : AMB_CaAbs;
	}
	else if( (validAdrModes&AMB_CaXysize)!=0 )
	{
		validAdrModes &= ~AMB_CaXysize;
		validAdrModes |= (regsize_xy==8) ? AMB_CaZp : AMB_CaAbs;
	}


	/*
	 * try find suitable addressmode
	 * is the variable's size unknown?
	 */
	if( varSizeKnown ) {
		/* varsize is known, the addressmode must exist */
		if( (adrMode=AdrKey2Mode(adrKeys))==AM_Error )
		{
			error(EM_UnknownAdressmode);
			adrMode = AM_Error;
		}
		else
		{
			/* now just validate the addressmode */
			adrMode = validateAddressmode(adrMode, validAdrModes);
		}
	}
	else {
		/* varsize is unknown. The default size is abs, try to get an addressmode with this size. */
		found = (
			(adrMode=AdrKey2Mode(adrKeys))!=AM_Error &&
			(adrMode=validateAddressmode(adrMode,validAdrModes))!=AM_Error
		);

		if( !found )
		{
			/* failed, first try zp as a replacement */
			adrKeys &= ~(AK_NUMZP|AK_NUMABS|AK_NUMLONG);
			adrKeys |= AK_NUMZP;

			if(
				(adrMode=AdrKey2Mode(adrKeys))!=AM_Error &&
				(adrMode=validateAddressmode(adrMode,validAdrModes))!=AM_Error
			  )
			{
				warning(WM_ForcedLabelSize, 8);
			}
			else {
				/* now try long */
				adrKeys &= ~(AK_NUMZP|AK_NUMABS|AK_NUMLONG);
				adrKeys |= AK_NUMLONG;

				if( (adrMode=AdrKey2Mode(adrKeys))==AM_Error ) {
					error(EM_UnknownAdressmode);
					adrMode = AM_Error;
				}
				else if( (adrMode=validateAddressmode(adrMode,validAdrModes))==AM_Error ) {
					error(EM_IllAdressmodeForOpcode);
					adrMode = AM_Error;
				}
				else {
					warning(WM_ForcedLabelSize, 24);
				}
			}
		}
	}


	return adrMode;
}


ADRESSMODE validateAddressmode(ADRESSMODE adrMode, uint32_t validAdrModes)
{
	uint8_t cnt0;


	if( (validAdrModes&(1<<adrMode))==0 ) {
		cnt0=0;
		while( cnt0<arraysize(amode_replace) ) {
			if( adrMode==amode_replace[cnt0].oldmode && (validAdrModes&(1<<amode_replace[cnt0].newmode))!=0 ) {
				if( amode_replace[cnt0].warn ) {
					warning(WM_ByteToWordExpansion);
				}
				adrMode = amode_replace[cnt0].newmode;
				break;
			}
			else {
				cnt0++;
			}
		}
		if( cnt0>=arraysize(amode_replace) ) {
			adrMode = AM_Error;
		}
	}

	return adrMode;
}


/*-----------------------------------*/


/* typedef for an addresskey - addressmode pair */
typedef struct
{
	uint32_t adressbits;		/* combination of multiple ADRESSKEY's */
	ADRESSMODE adressmode;
} adressKeyModePair;

/*
 * translation table from addresskey to addressmode
 * used in function 'AdrKey2Mode'
 */
adressKeyModePair adrKeyMode[] =
{
	{ AK_NONE,						AM_None },
	{ AK_CARRO|AK_NUMZP,					AM_CaZp },
	{ AK_CARRO|AK_NUMABS,					AM_CaAbs },
	{ AK_NUMZP,						AM_Zp },
	{ AK_NUMZP|AK_COMMAS,					AM_ZpCoS },
	{ AK_NUMZP|AK_COMMAX,					AM_ZpCoX },
	{ AK_NUMZP|AK_COMMAY,					AM_ZpCoY },
	{ AK_NUMZP|AK_COMMAZP,					AM_ZpCoZp },
	{ AK_BOPEN|AK_NUMZP|AK_BCLOSE1,				AM_BrZpBr },
	{ AK_BOPEN|AK_NUMZP|AK_COMMAX|AK_BCLOSE2,		AM_BrZpCoXBr },
	{ AK_BOPEN|AK_NUMZP|AK_COMMAS|AK_BCLOSE1|AK_COMMAY,	AM_BrZpCoSBrCoY },
	{ AK_BOPEN|AK_NUMZP|AK_BCLOSE1|AK_COMMAY,		AM_BrZpBrCoY },
	{ AK_SBOPEN|AK_NUMZP|AK_SBCLOSE,			AM_SbrZpSbr },
	{ AK_SBOPEN|AK_NUMZP|AK_SBCLOSE|AK_COMMAY,		AM_SbrZpSbrCoY },
	{ AK_NUMABS,						AM_Abs },
	{ AK_NUMABS|AK_COMMAX,					AM_AbsCoX },
	{ AK_NUMABS|AK_COMMAY,					AM_AbsCoY },
	{ AK_BOPEN|AK_NUMABS|AK_BCLOSE1,			AM_BrAbsBr },
	{ AK_BOPEN|AK_NUMABS|AK_COMMAX|AK_BCLOSE2,		AM_BrAbsCoXBr },
	{ AK_NUMLONG,						AM_Long },
	{ AK_NUMLONG|AK_COMMAX,					AM_LongCoX }
};


ADRESSMODE AdrKey2Mode(uint32_t adrkey)
{
	uint8_t cnt=0;
	ADRESSMODE adrMode;


	cnt = 0;
	adrMode = AM_Error;

	do
	{
		if( adrKeyMode[cnt].adressbits==adrkey )
		{
			adrMode = adrKeyMode[cnt].adressmode;
			break;
		}
	} while( ++cnt<arraysize(adrKeyMode) );

	return adrMode;
}


/*-----------------------------------*/


/* typedef for an operand - addresskey pair */
typedef struct
{
	OPERAND operand;
	ADRESSKEY adresskey;
} operandAdrKeyPair;

/*
 * translation table from operands to addresskeys
 * used in function scanAdrKeys
 */
operandAdrKeyPair operandToAdresskey[] =
{
	{ OP_Carro,	AK_CARRO },
	{ OP_Bo,	AK_BOPEN },
	{ OP_Sbo,	AK_SBOPEN },
	{ OP_Bc,	AK_BCLOSE1 },
	{ OP_Sbc,	AK_SBCLOSE },
	{ OP_Bc,	AK_BCLOSE2 }
};

ADRESSKEY scanAdrKeys(uint8_t sidx, uint8_t eidx)
{
	uint8_t cnt0;
	ADRESSKEY adrKeys = AK_NONE;
	const lineelement_t *lelem;


	lelem = pp_peek();
	if( lelem==NULL ) {
		return AK_ERROR;
	}
	if( lelem->typ==LE_OPERAND )
	{
		for(cnt0=sidx; cnt0<eidx; cnt0++)
			if( lelem->data.op==operandToAdresskey[cnt0].operand )
			{
				adrKeys = operandToAdresskey[cnt0].adresskey;
				if( !pp_eat() )
					return AK_ERROR;
			}
	}

	return adrKeys;
}


/*-----------------------------------*/


/*
 * generate a relative branch parameter from the opcode
 * address and the destination address
 */
int16_t adr2rel(uint32_t dstadr, uint32_t opcadr)
{
	int16_t diff;


	diff = dstadr-opcadr-1;
	if( diff<-0x80 || diff>0x80 )
	{
		error(EM_BranchOutOfRange);
		diff = -1;
	}
	else if( diff<0 )
	{
		diff += 0x0100;
		return diff;
	}

	return diff;
}


int32_t adr2relLong(uint32_t dstadr, uint32_t opcadr)
{
	int32_t diff;


	diff = dstadr-opcadr-1;
	if( diff<-0x8000 || diff>0x8000 )
	{
		error(EM_BranchOutOfRange);
		diff = -1;
	}
	else if( diff<0 )
	{
		diff += 0x010000;
		return diff;
	}

	return diff;
}

/*-----------------------------------*/


/*
 * This checks the parameter of a jmp(x) opcode.
 * The 6502/6510 has a bug which wraps the adress
 * for x from $..ff back to $..00 of the same page
 * instead of going th the next.
 */
bool checkJmpInd(VARIABLE *param)
{
	/*
	 * the parameter can only be checked, if it's defined and
	 * it is final.
	 */
	if( param->defined && param->final )
	{
		/* this bug is fixed for 65816 */
		if( currentCpu!=CPUTYPE_65816 )
		{
			/* the parameter must be a number. parse_mne should have checked this */
			assert( param->valt.typ==VALTYP_NUM );

			if( (param->valt.value.num&0x00ff)==0x00ff )
				warning(WM_JmpIndAtFF_d, param->valt.value.num);
		}
		return true;
	}
	else
		return false;
}


/*-----------------------------------*/


CPUTYPE getCurrentCpu(void)
{
	return currentCpu;
}

const char *getCurrentCpuName(void)
{
	return cputype_name[currentCpu].name;
}

CPUTYPE getCpuIdx(const char *cpuName)
{
	const cpuTypeNamePair_t *cc, *ce;
	CPUTYPE cpuIdx;


	cc = cputype_name;
	ce = cc + arraysize(cputype_name);
	cpuIdx = CPUTYPE_UNKNOWN;

	while( cc<ce )
	{
		if( strcasecmp(cpuName, cc->name)==0 )
		{
			cpuIdx = cc->id;
			break;
		}
		else
		{
			++cc;
		}
	}

	return cpuIdx;
}


void setCpuType(CPUTYPE cpuType)
{
	/* take over new value */
	currentCpu = cpuType;

	/*
	 * set register sizes to 8
	 * TODO: maybe it's better to remember last values from 65816 mode and restore here?
	 */
	regsize_a = regsize_xy = 8;

	/* set valid mode table */
	validModes = (cputype_name+cpuType)->validmodes;
}


void setRegisterSize_A(uint8_t regSize)
{
	regsize_a = regSize;
}


void setRegisterSize_XY(uint8_t regSize)
{
	regsize_xy = regSize;
}


/*-----------------------------------*/

