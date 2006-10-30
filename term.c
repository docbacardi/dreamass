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

extern bool PetMode;
extern bool allTermsFinal;
extern uint16_t pass_cnt;

/*-----------------------------------*/
/* local routines                    */

bool testAssign(term *t);
bool testSize(VARIABLE *pvar, TERMSIZE size);
bool term2const(lineelement_t *lelem, VARIABLE *pvar, TERMSIZE size);
termcnt_t new_term(size_t size);
bool term_parse(term *that);
int8_t term_parseElem(term *that, const lineelement_t *lelem);

/*   void term_makePic(termcnt_t termidx);  */
/*   void term_debug(termcnt_t termidx, FILE *dfh);  */

uint32_t rMakeGraph(term *t, uint32_t *xpos, uint32_t *size, uint32_t pxpos, termsize_t tidx);
bool rDrawTable(FILE *dfh, term *t, uint32_t *xpos, uint32_t *size, termsize_t *level, termsize_t level_count, uint32_t xmax);


bool term_insConst(term *that, uint32_t num);
bool term_insStr(term *that, stringsize_t *varname );
bool term_insVar(term *that, stringsize_t *varname);
bool term_insOp(term *that, OPERAND op);
bool term_add(term *that, termelem_s *te);

static NALLOC termlist;

/*-----------------------------------*/

bool read_term(TERMSIZE size, VARIABLE *pvar, bool toBinary)
{
	const lineelement_t *lelem;
	lineelement_t nlelem;
	term *t;
	termcnt_t termidx;
	linesize_t linepos_start;
	VARERR ve;
	LINEELEMENT_TYP term_typ[] =
	{
		TE_1BYTE,
		TE_2BYTE,
		TE_3BYTE,
		TE_4BYTE,
		TE_RBYTE,
		TE_RLBYTE,
		TE_FLEX,
		TE_nBYTE,
		TE_nBYTE
	};


	pvar->defined = false;

	lelem = pp_peek();
	if( lelem==NULL ) {
		return false;
	}
	if( lelem->typ==LE_EOL )
	{
		error(EM_TermParse);
		return false;
	}

	linepos_start = pp_getPos();

	if( (lelem->typ & LINEELEMENT_CLASSMASK) == LINEELEMENT_TERM )
	{
		t=getTerm(termidx=lelem->data.termidx);
		if( !pp_skip() )
		{
			error(EM_TermParse);
			return false;
		}

#ifdef __DUMP_TERMS__
		if( cfg_debug )
		{
			fprintf( debugLog, "<a name=\"term%x_p%d\"></a><h2>Term %d</h2>\nBefore eval:<br>", termidx, pass_cnt, termidx);
			term_makeGraph(debugLog, termidx);
		}
#endif	/*   __DUMP_TERMS__  */

		if( (ve=term_eval(t)).err )
		{
			error(EM_TermParse);
			return false;
		}

#ifdef __DUMP_TERMS__
		if( cfg_debug )
		{
			fprintf( debugLog, "<br>After eval:<br>");
			term_makeGraph(debugLog, termidx);
		}
#endif	/*   __DUMP_TERMS__  */

		if( !testSize(&ve.var, size) )
		{
			error(EM_TermParse);
			deleteVariable( &ve.var );
			return false;
		}
		if( ve.var.defined && ve.var.final && toBinary )
		{
			if( !term2const(&nlelem, &ve.var, size) )
			{
				deleteVariable( &ve.var );
				return false;
			}
			pp_delItems(linepos_start, pp_getPos());
			pp_replaceItem(linepos_start, &nlelem);
#ifdef __DUMP_TERMS__
			if( cfg_debug ) {
				fprintf( debugLog, "Term was completely solved. It will be reused.<br>\n");
			}
#endif	/*   __DUMP_TERMS__  */
		}
		if( ve.var.defined ) {
			allTermsFinal &= ve.var.final;
		}
	}
	else {
		if( (termidx=new_term(16))==(termcnt_t)-1 )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
		t  = ((term*)termlist.buf);
		t += termidx;

		t->root = 0;
		t->inspos = no_term;
		t->bracketcnt = 0;

		/*   Remember position in linebuffer to insert the var definitions  */
		t->vls = linepos_start+1;

		if( !term_parse(t) )
		{
			/*   delete the term from the list  */
			free( t->t );
			--termlist.elemCount;
/*  			free( termlist[--termlist_count].t );  */
			error(EM_TermParse);
			return false;
		}

#ifdef __DUMP_TERMS__
		if( cfg_debug )
		{
			fprintf( debugLog, "<a name=\"term%x_p%d\"></a><h2>Term %d</h2>\nBefore eval:<br>", termidx, pass_cnt, termidx);
			term_makeGraph(debugLog, termidx);
		}
#endif	/*   __DUMP_TERMS__  */

		pp_delItems(linepos_start, pp_getPos());
		if( (ve=term_eval(t)).err )
		{
			error(EM_TermParse);
			return false;
		}

#ifdef __DUMP_TERMS__
		if( cfg_debug )
		{
			fprintf( debugLog, "<br>After eval:<br>");
			term_makeGraph(debugLog, termidx);
		}
#endif	/*  __DUMP_TERMS__  */

		if( ve.var.defined && !testSize(&ve.var, size) )
		{
			deleteVariable( &ve.var );
			return false;
		}
		if( ve.var.defined && ve.var.final && toBinary )
		{
			if( !term2const(&nlelem, &ve.var, size) )
			{
				deleteVariable( &ve.var );
				return false;
			}
			/*   delete the term from the list  */
			t  = (term*)termlist.buf;
			t += --termlist.elemCount;
			del_term(t);
#ifdef __DUMP_TERMS__
			if( cfg_debug )
				fprintf( debugLog, "Term was completely solved. It will be reused.<br>\n");
#endif	/*   __DUMP_TERMS__  */
		}
		else
		{
			nlelem.typ = *(term_typ+size);
			nlelem.data.termidx = termidx;
		}
		pp_replaceItem(linepos_start, &nlelem);

		if( ve.var.defined ) {
			allTermsFinal &= ve.var.final;
		}
	}

	/*
	 * Sanity check (Are the booleans really 0 or 1 ?)
	 */
	if( ((ve.var.defined&0xfe)!=0) || ((ve.var.used&0xfe)!=0) || ((ve.var.final&0xfe)!=0) || ((ve.var.readwrite&0xfe)!=0) )
	{
		printf("\nSanity Error: Bogus Boolean Term Values!\n\tDefined: $%02x\n\tUsed: $%02x\n\tFinal: $%02x\n\tReadWrite: $%02x\n",ve.var.defined,ve.var.used,ve.var.final,ve.var.readwrite);
		term_makeGraph(stdout, termidx);
		printf("We're near line: %d\n", pp_getLineNr() );
		assert( false );
	}
	*pvar=ve.var;
	return true;
}


bool term_readVardef(const lineelement_t *preelem, linesize_t linepos_start)
{
	lineelement_t nlelem;
	term *t;
	termcnt_t termidx;
	VARERR ve;


	if( (termidx=new_term(16))==(termcnt_t)-1 )
	{
		systemError(EM_OutOfMemory);
		return false;
	}
	t  = (term*)termlist.buf;
	t += termidx;

	t->root = 0;
	t->inspos = no_term;
	t->bracketcnt = 0;

	/*   Remember position in linebuffer to insert the var definitions  */
	t->vls = linepos_start;

	if( preelem!=NULL && term_parseElem(t, preelem)!=0 )
		return false;
	if( !term_parse(t) )
	{
		/*   delete the term from the list  */
		free(t->t);
		--termlist.elemCount;
/*  		free( termlist[--termlist_count].t );  */
		return false;
	}

#ifdef __DUMP_TERMS__
	if( cfg_debug )
	{
		fprintf( debugLog, "<a name=\"term%x_p%d\"></a><h2>Term %d</h2>\nBefore eval:<br>", termidx, pass_cnt, termidx);
		term_makeGraph(debugLog, termidx);
	}
#endif	/*   __DUMP_TERMS__  */

	if( !testAssign(t) )
		return false;

	pp_delItems(linepos_start, pp_getPos());
	if( (ve=term_eval(t)).err )
		return false;

#ifdef __DUMP_TERMS__
	if( cfg_debug )
	{
		fprintf( debugLog, "<br>After eval:<br>");
		term_makeGraph(debugLog, termidx);
	}
#endif	/*   __DUMP_TERMS__  */

	if( !ve.var.defined || !ve.var.final )
	{
		nlelem.typ = LE_VARTERM;
		nlelem.data.termidx = termidx;
		pp_replaceItem(linepos_start, &nlelem);
	}

	if( ve.var.defined )
		allTermsFinal &= ve.var.final;

	/*
	 * Sanity check (Are the booleans really 0 or 1 ?)
	 */
	if( ((ve.var.defined&0xfe)!=0) || ((ve.var.used&0xfe)!=0) || ((ve.var.final&0xfe)!=0) || ((ve.var.readwrite&0xfe)!=0) )
	{
		printf("\nSanity Error: Bogus Boolean Term Values!\n\tDefined: $%02x\n\tUsed: $%02x\n\tFinal: $%02x\n\tReadWrite: $%02x\n",ve.var.defined,ve.var.used,ve.var.final,ve.var.readwrite);
		term_makeGraph(stdout, termidx);
		printf("We're near line: %d\n", pp_getLineNr() );
		assert( false );
	}

	deleteVariable( &ve.var );

	return true;
}


bool term_makePCVar(stringsize_t *varname, linesize_t varpos)
{
	term *t;
	termcnt_t termidx;
	VARERR ve;
	lineelement_t nlelem;


	if( (termidx=new_term(16))==(termcnt_t)-1 )
	{
		systemError(EM_OutOfMemory);
		return false;
	}
	t  = (term*)termlist.buf;
	t += termidx;

	/*
	 * Create the term '<varname>=*' with the current position
	 * to insert the vardef. So the term will be overwritten automatically
	 * when the PC is final
	 */
	t->root = 0;
	t->inspos = no_term;
	t->bracketcnt = 0;
/*  	t->vle = varelem;  */
	t->vls = varpos;

	/*   Insert <varname>  */
	/*  term_insVar(t, varelem->data.txt);  */
	term_insVar(t, varname);
	/*   Insert '='  */
	term_insOp(t, OP_Eq);
	/*   Insert '*'  */
	term_insOp(t, OP_Star);

	pp_delItems(varpos, varpos+1);

#ifdef __DUMP_TERMS__
	if( cfg_debug )
	{
		fprintf( debugLog, "<a name=\"term%x_p%d\"></a><h2>Term %d</h2>\nBefore eval:<br>", termidx, pass_cnt, termidx);
		term_makeGraph(debugLog, termidx);
	}
#endif	/*   __DUMP_TERMS__  */

	if( (ve=term_eval(t)).err )
		return false;

#ifdef __DUMP_TERMS__
	if( cfg_debug )
	{
		fprintf( debugLog, "<br>After eval:<br>");
		term_makeGraph(debugLog, termidx);
	}
#endif	/*   __DUMP_TERMS__  */

	/*
	 * If the term ( i.e. the PC) is not final, no vardef was inserted.
	 * create a LE_VARTERM element
	 */
	if( !ve.var.final )
	{
		nlelem.typ = LE_VARTERM;
		nlelem.data.termidx = termidx;
		pp_replaceItem(varpos, &nlelem);
	}

	if( ve.var.defined )
		allTermsFinal &= ve.var.final;

	/*   ok, that's not really necessary. A pc def must be numeric...  */
	deleteVariable( &ve.var );

	return true;
}

/*
 * Name:
 *   testAssign
 *
 * Description:
 *   Test if the top operation of a term is an assignment. This is necessary
 *   to determine if the term can stand alone.
 *   Example: The term (2+3)*4 can not stand alone, it is no assignment, just a
 *   number. a=(2+3)*4 can stand alone as it assigns a value to the variable 'a'.
 *
 * Parameters:
 *   term *t = pointer to the term to test
 *
 * Returns:
 *   bool true  = the term's top operation is an assignment
 *        false = the term's top operation is not an assignment
 */
bool testAssign(term *t)
{
	termelem_s *telem;


	return(
		t->t_size>1 &&
		(telem=t->t+t->root)->typ==TT_FUNCTION &&
		(telem->data.function & FN_IdxMask)>=FN_AssigningStart &&
		(telem->data.function & FN_IdxMask)<=FN_AssigningEnd
	);
}


/*
 * Name:
 *   testSize
 *
 * Description:
 *   Test the size of a variable in bytes. It must not exceed a specified maximum.
 *
 * Parameters:
 *   VARIABLE *pvar = pointer to the variable to be checked
 *   TERMSIZE size  = maximum size this variable may have
 *
 * Returns:
 *   bool true  = variable does not exceed the specified size
 *        false = variable is larger
 */
bool testSize(VARIABLE *pvar, TERMSIZE size)
{
	bool ok;


	ok = true;
	if( pvar->valt.typ==VALTYP_STR )
	{
		/*   a string variable fits only for the types nByte or string  */
		if( size!=TS_nBYTE && size!=TS_STRING ) {
			error(EM_TypeMismatch);
			ok = false;
		}
	}
        /*   check value for 1-3 byte size  */
	else if( pvar->valt.typ==VALTYP_NUM )
	{
		switch( size )
		{
		case TS_1BYTE:
			ok = (pvar->valt.byteSize <= 1);
			if( !ok ) {
				error(EM_Overflow_d, pvar->valt.value.num);
			}
			break;
		case TS_2BYTE:
			ok = (pvar->valt.byteSize <= 2);
			if( !ok ) {
				error(EM_Overflow_d, pvar->valt.value.num);
			}
			break;
		case TS_3BYTE:
			ok = (pvar->valt.byteSize <= 3);
			if( !ok ) {
				error(EM_Overflow_d, pvar->valt.value.num);
			}
			break;
		case TS_4BYTE:
		case TS_FLEX:
		case TS_nBYTE:
		/*   nBYTE can act as a flex number too if it's small enough  */
			ok = (pvar->valt.byteSize <= 4);
			if( !ok )
			{
				error(EM_Overflow_d, pvar->valt.value.num);
			}
			break;
		case TS_STRING:
			/*   read in a number but a string was requested  */
			error(EM_TypeMismatch);
			ok = false;
			break;
		case TS_RBYTE:
		case TS_RLBYTE:
			break;
		}
	}

	return ok;
}


/*
 * Name:
 *   term2const
 *
 * Description:
 *   convert a variable to a constant value
 *
 * Parameters:
 *   lineelement_t *lelem = lineelement hold the constant element
 *   VARIABLE *pvar       = variable to convert
 *   TERMSIZE size        =
 *
 * Returns:
 *   bool true  =
 *        false =
 */
bool term2const(lineelement_t *lelem, VARIABLE *pvar, TERMSIZE size)
{
	int le_size;
	char *sc, *se;


	assert( size!=TS_RBYTE && size!=TS_RLBYTE );

	if( pvar->valt.typ==VALTYP_STR )
	{
		lelem->typ = BE_nBYTE;

		se=(sc=((char*)(pvar->valt.value.str+1)))+*pvar->valt.value.str;
		/*   Convert to pet/scr  */
		if( PetMode )
		{
			while( sc<se )
			{
				*sc = Asc2Pet(*sc);
				++sc;
			}
		}
		else
		{
			while( sc<se )
			{
				*sc = Asc2Scr(*sc);
				++sc;
			}
		}

		lelem->data.b_nbyte = stringClone(pvar->valt.value.str);
		return ( lelem->data.b_nbyte!=NULL );
	}
	else
	{
		if( size==TS_FLEX || size==TS_nBYTE )
		{
			/*   valid bytesize for flex and nByte is 1-4  */
			if( pvar->valt.byteSize>0 && pvar->valt.byteSize<=4 )
			{
				le_size = pvar->valt.byteSize-1;
			}
			else
			{
				error(EM_Overflow_d, pvar->valt.byteSize);
				return false;
			}
		}
		else if( size==TS_1BYTE || size==TS_2BYTE || size==TS_3BYTE || size==TS_4BYTE )
		{
			le_size = size;
		}
		else
		{
			/*   illegal type  */
			le_size = -1;
		}

		switch( le_size )
		{
		case 0:
			lelem->data.b_1byte = pvar->valt.value.num;
			lelem->typ = BE_1BYTE;
			break;
		case 1:
			lelem->data.b_2byte = pvar->valt.value.num;
			lelem->typ = BE_2BYTE;
			break;
		case 2:
			lelem->data.b_3byte = pvar->valt.value.num;
			lelem->typ = BE_3BYTE;
			break;
		case 3:
			lelem->data.b_4byte = pvar->valt.value.num;
			lelem->typ = BE_4BYTE;
			break;
		default:
			assert( false );
		}

		return true;
	}
}


/*
 * Name:
 *   termlist_zero
 *
 * Description:
 *   Set the termlist to uninitialized state.
 *
 * Parameters:
 *   -
 *
 * Returns:
 *   -
 */
void termlist_zero(void)
{
	memset(&termlist, 0, sizeof(NALLOC));
}


/*
 * Name:
 *   termlist_init
 *
 * Description:
 *   Init the termlist. Memory for a number of entries is allocated.
 *
 * Parameters:
 *   termcnt_t nmemb = initial number of entries in the termlist
 *
 * Returns:
 *   bool true  = init succeeded
 *        false = init failed
 */
bool termlist_init(termcnt_t nmemb)
{
	return nalloc_init(&termlist, sizeof(term), nmemb);
}


/*
 * Name:
 *   termlist_delete
 *
 * Description:
 *   Free the termlist and all elements.
 *
 * Parameters:
 *   -
 *
 * Returns:
 *   -
 */
void termlist_delete(void)
{
	term *pc, *pe;


	pc = (term*)termlist.buf;
	pe = pc+termlist.elemCount;
	while(pc<pe) {
		del_term(pc++);
	}
	nalloc_free(&termlist);
}


/*
 * Name:
 *   new_term
 *
 * Description:
 *   Create a new, empty term and insert it into the termlist. The termlist will be
 *   reallocated and expanded if it is full.
 *
 * Parameters:
 *   size_t size = initial number of elements in this term
 *
 * Returns:
 *   termcnt_t -1   = error adding the new term
 *             else = index of the new term in the termlist
 */
termcnt_t new_term(size_t size)
{
	term *t;


	if( nalloc_size(&termlist, termlist.elemCount+1)==false ) {
		return (termcnt_t)-1;
	}

	t  = (term*)termlist.buf;
	t += termlist.elemCount;

	if( (t->t=(termelem_s*)(malloc(sizeof(termelem_s)*size)))==NULL )
		return (termcnt_t)-1;

	t->t_size = 0;
	t->t_alloc = size;

	return termlist.elemCount++;
}


/*
 * Name:
 *   del_term
 *
 * Description:
 *   Free a term and all elements.
 *
 * Parameters:
 *   term *that = the term to delete
 *
 * Returns:
 *   -
 */
void del_term(term *that)
{
	termelem_s *c, *e;


	for( e=(c=that->t)+that->t_size; c<e; ++c) {
		clear_termElement(c);
	}
	free( that->t );
}


/*
 * Name:
 *   clear_termElement
 *
 * Description:
 *   Clear a term element. If the element allocated additional memory, it is freed.
 *
 * Parameters:
 *   termelem_s *telem = the term element to clear
 *
 * Returns:
 *   -
 */
void clear_termElement(termelem_s *telem)
{
	if( telem->typ==TT_CONST_STR ) {
		free( telem->data.string );
	}
	else if( telem->typ==TT_VARIABLE ) {
		free( telem->data.varname );
	}
}


bool term_parse(term *that)
{
	const lineelement_t *lelem;
	termsize_t lastPos;
	bool neot = true;


	while( neot ) {
		lelem=pp_peek();
		if( lelem==NULL ) {
			return false;
		}
		if( lelem->typ==LE_EOL ) {
			break;
		}
		switch( term_parseElem(that, lelem) )
		{
		case -1:
			return false;
		case 1:
			neot = false;
			break;
		case 0:
			if( !pp_skip() )
				return false;
		};

/*   Show how the term is built up. Don't enable for bigger sources or you'll get some MB's of debuginfo  */
/*  		term_makeGraph(debugLog, that-termlist );  */
	}

	if( that->bracketcnt!=0 )
	{
		error(EM_NoClosingBracket);
		return false;
	}
	/*   some sanity tests  */
	else if( (lastPos=that->inspos)==no_term )
	{
		error(EM_TermParse);
		return false;
	}
	else if( lastPos>=that->t_size )
	{
		error(EM_TermParse);
		return false;
	}
	else
		return (that->t_size>0);
}


/*
 * Return: -1 = Error
 *          0 = Ok
 *          1 = End Of Term
 */
int8_t term_parseElem(term *that, const lineelement_t *lelem)
{
	uint8_t cnt0;
	uint32_t num;


	switch( lelem->typ )
	{
	case LE_NUMBER:
		if( !term_insConst(that, lelem->data.num) )
		{
			error(EM_NumberWithoutOperand);
			return -1;
		}
		return 0;
	case LE_TEXTNUM1:
	case LE_TEXTNUM2:
	case LE_TEXTNUM3:
	case LE_TEXTNUM4:
		num = 0;
		cnt0=(uint8_t)(lelem->typ-LE_TEXTNUM1);
		do
		{
			num <<=8;
			if( PetMode )
				num |= Asc2Pet( (lelem->data.txtnum>>(cnt0<<3))&0xff );
			else
				num |= Asc2Scr( (lelem->data.txtnum>>(cnt0<<3))&0xff );
		} while( --cnt0<4 );
		if( !term_insConst(that, num) )
		{
			error(EM_NumberWithoutOperand);
			return -1;
		}
		return 0;
	case LE_OPERAND:
		switch( lelem->data.op )
		{
		case OP_Colon:
		case OP_Comma:
		case OP_Sbc:
			/*  comma, colon and closing square bracket are all end of term signs  */
			return 1;
			break;
		case OP_Bc:
			/*  a closing bracket is end_of_term if we did not get a matching opening one  */
			if( that->bracketcnt==0 )
				return 1;
			else
			{
				--that->bracketcnt;
				/*  Move up to the first opening bracket  */
				while( that->inspos!=no_term && ( that->t[that->inspos].typ!=TT_FUNCTION || that->t[that->inspos].data.function!=FN_BracketOpen ) )
					that->inspos = that->t[that->inspos].up;

				/*  Should never happen: matching opening bracket not found in term tree  */
				if( that->inspos==no_term )
				{
					if( cfg_debug ) {
						term_makeGraph(debugLog, that-((term*)termlist.buf));
					}
					assert( false );
				}
				that->t[that->inspos].data.function = FN_BracketClosed;
			}
			break;
		case OP_Bo:
			that->bracketcnt++;
			/*  no break! this must fall through to the call to term_insOp  */
		default:
			if( !term_insOp(that, lelem->data.op) )
			{
				error(EM_NumberWithoutOperand);
				return -1;
			}
			break;
		};
		return 0;
	case LE_STRING:
		if( !term_insStr(that, lelem->data.str) )
		{
			error(EM_NumberWithoutOperand);
			return -1;
		}
		return 0;
	case LE_TEXT:
		if( !term_insVar(that, lelem->data.txt) )
		{
			error(EM_NumberWithoutOperand);
			return -1;
		}
		return 0;
	case LE_PSYOPC:
		error(EM_PsyInTerm);
		return -1;
	case LE_MNE:
		error(EM_OpcInTerm);
		return -1;

	case BE_1BYTE:
	case BE_2BYTE:
	case BE_3BYTE:
	case BE_4BYTE:
	case BE_DSB:
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
	case LE_PREPROC:
	case LE_EMPTY:
	case LE_EOL:
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
		/*  should never happen  */
		assert( false );
	};

	return 0;
}


bool term_insConst(term *that, uint32_t num)
{
	termelem_s te, *ti;


	te.typ = TT_CONST_NUM;
	te.data.constant = num;
	te.up = te.left = te.right = no_term;
	te.byteSize = value_getRealSize(num);

	if( that->inspos!=no_term )
	{
		ti = that->t + that->inspos;

		/*   a constant can not follow a post unary operator as this would result in 'number after number'  */
		if( ti->typ==TT_FUNCTION && (ti->data.function & FN_TypMask)!=FN_PostUnary )
		{
			/*   insert number at right side of this operand  */
			ti->right = that->t_size;
			te.up = that->inspos;
		}
		else
			return false;
	}

	return term_add(that, &te);
}


bool term_insVar(term *that, stringsize_t *varname )
{
	termelem_s te, *ti;


	te.typ = TT_VARIABLE;
	if( (te.data.varname=stringClone(varname))==NULL )
		return false;
	te.up = te.left = te.right = no_term;
	te.byteSize = 0;

	if( that->inspos!=no_term )
	{
		ti = that->t + that->inspos;

		/*   a variable can not follow a post unary operator as this would result in 'number after number'  */
		if( ti->typ==TT_FUNCTION && (ti->data.function & FN_TypMask)!=FN_PostUnary )
		{
			/*   insert number at right side of this operand  */
			ti->right = that->t_size;
			te.up = that->inspos;
		}
		else
			return false;
	}

	return term_add(that, &te);
}


bool term_insStr(term *that, stringsize_t *string )
{
	termelem_s te, *ti;


	te.typ = TT_CONST_STR;
	te.data.string = stringClone(string);
	te.up = te.left = te.right = no_term;
	te.byteSize = *string;

	if( that->inspos!=no_term )
	{
		ti = that->t + that->inspos;

		/*   a string can not follow a post unary operator as this would result in 'string after string'  */
		if( ti->typ==TT_FUNCTION && (ti->data.function & FN_TypMask)!=FN_PostUnary )
		{
			/*   insert string at right side of this operand  */
			ti->right = that->t_size;
			te.up = that->inspos;
		}
		else
			return false;
	}

	return term_add(that, &te);
}


bool term_insOp(term *that, OPERAND op)
{
	termelem_s te, *ti, *tiu=NULL;
	FUNCTION ti_fn;


	te.typ = TT_FUNCTION;
	te.up = te.left = te.right = no_term;
	te.byteSize = 0;

	if( that->inspos==no_term )
	{
		/*  term starts with an operand, this is only allowed for *(==PC) or unary ones  */
		if( op==OP_Star || op==OP_StarEq )
		{
			te.typ = TT_PC;
			if( op==OP_StarEq )
			{
				/*  we know that we'll insert the assign operator right after this  */
				te.up = 1;
				if( !term_add(that,&te) )
					return false;
				te.up = no_term;
				te.typ = TT_FUNCTION;
				te.data.function = FN_Assign;
				te.left = 0;
				that->root = 1;
			}
		}
		else if( (te.data.function=op2fn[op].preUnary)==FN_no )
			return false;
	}
	else
	{
		/*  ti is the place to insert the new operand  */
		ti = that->t+that->inspos;
		switch( ti->typ )
		{
		case TT_FUNCTION:
			ti_fn = ti->data.function;
			/*  an operand at the left side is only allowed if this is *(==PC) or that is a bracket or if this is an unary one  */

			/*
			 * '<OP> *' evals to '<OP> <PC>' if <OP> is binary or preunary.
			 * Note that this rules out both opening and closing brackets.
			 * If <OP> is not binary or preunary, the sequence will eval to '<OP> <MULT>' in  the else branch
			 */
			if( op==OP_Star && ((ti_fn&FN_TypMask)==FN_Binary || (ti_fn&FN_TypMask)==FN_PreUnary) )
			{
				/*   insert number at right side of this operand  */
				te.typ = TT_PC;
				ti->right = that->t_size;
				te.up = that->inspos;
			}
			else
			{
				/*   '( <OP>' is only possible for a preunary <OP>  */
				if( ti_fn==FN_BracketOpen )
				{
					if( (te.data.function=(op2fn+op)->preUnary)==FN_no )
						return false;
					/*   Insert <OP> below '('  */
					ti->right = that->t_size;
					te.up = that->inspos;
				}
				/*
				 * ') <OP>' is only valid for a binary <OP>.
				 * a preUnary <OP> would result in 'number after number' and
				 * a postUnary <OP> (which is only ++ and -- ) requires a plain variable on the left side.
				 */
				else if( ti_fn==FN_BracketClosed )
				{
					if( (te.data.function=(op2fn+op)->binary)==FN_no )
						return false;

					/*   go up until root node is reached or above priority is higher than this operand (priority 0 is the highest)  */
					while( ti->up!=no_term && fn_pri[(tiu=that->t+ti->up)->data.function & FN_IdxMask]<fn_pri[te.data.function & FN_IdxMask] )
					{
						that->inspos = ti->up;
						ti = tiu;
					}

					/*  insert te above ti  */
					if( (te.up=ti->up)==no_term )
						that->root = that->t_size;
					else if( tiu->left==that->inspos )
						tiu->left = that->t_size;
					else
						tiu->right = that->t_size;
					te.left = that->inspos;
					ti->up = that->t_size;

				}
				/*   a bracket after an operator is only valid for a binary or pre unary  */
				else if( op==OP_Bo )
				{
					te.data.function = FN_BracketOpen;

					if( (ti_fn & FN_TypMask)!=FN_Binary && (ti_fn & FN_TypMask)!=FN_PreUnary )
						return false;

					/*  insert bracket always below operator  */
					te.up = that->inspos;
					ti->right = that->t_size;
				}
				else
				{
					if( (ti_fn & FN_TypMask)==FN_PostUnary && (te.data.function=(op2fn+op)->binary)!=FN_no )
					{
						/*   insert a binary operator after a post unary one  */

						/*   (same as ti_fn == variable or constant)  */

						/*   go up until root node is reached or above priority is higher than this operand (priority 0 is the highest)  */
						while( ti->up!=no_term && fn_pri[(tiu=that->t+ti->up)->data.function & FN_IdxMask]<fn_pri[te.data.function & FN_IdxMask] )
						{
							that->inspos = ti->up;
							ti = tiu;
						}

						/*  insert te above ti  */
						if( (te.up=ti->up)==no_term )
							that->root = that->t_size;
						else if( tiu->left==that->inspos )
							tiu->left = that->t_size;
						else
							tiu->right = that->t_size;
						te.left = that->inspos;
						ti->up = that->t_size;
					}
					else if( (ti_fn & FN_TypMask)==FN_Binary && (te.data.function=(op2fn+op)->preUnary)!=FN_no )
					{
						/*   insert preunary operator after a binary one always below  */
						te.up = that->inspos;
						ti->right = that->t_size;
					}
					else if( (ti_fn & FN_TypMask)==FN_PreUnary && (te.data.function=(op2fn+op)->preUnary)!=FN_no )
					{
						/*   insert preunary operator after a preunary one always below  */
						te.up = that->inspos;
						ti->right = that->t_size;
					}
					else if( (ti_fn & FN_TypMask)==FN_PostUnary && (te.data.function=(op2fn+op)->postUnary)!=FN_no )
					{
						/*   (same as ti_fn == variable or constant)  */

						/*   go up until root node is reached or above priority is higher than this operand (priority 0 is the highest)  */
						while( ti->up!=no_term && fn_pri[(tiu=that->t+ti->up)->data.function & FN_IdxMask]<fn_pri[te.data.function & FN_IdxMask] )
						{
							that->inspos = ti->up;
							ti = tiu;
						}

						/*  insert te above ti  */
						if( (te.up=ti->up)==no_term )
							that->root = that->t_size;
						else if( tiu->left==that->inspos )
							tiu->left = that->t_size;
						else
							tiu->right = that->t_size;
						te.left = that->inspos;
						ti->up = that->t_size;
					}
					else
						return false;
				}
			}
			break;
		case TT_CONST_NUM:
		case TT_CONST_STR:
		case TT_VARIABLE:
		case TT_PC:
			/*   an operator after a constant, string or variable must be binary or post unary. a pre unary would result in 'number after number'  */
			if( (te.data.function=(op2fn+op)->binary)==FN_no && (te.data.function=(op2fn+op)->postUnary)==FN_no )
				return false;

			/*   go up until root node is reached or above priority is higher than this operand (priority 0 is the highest)  */
			while( ti->up!=no_term && fn_pri[(tiu=that->t+ti->up)->data.function & FN_IdxMask]<=fn_pri[te.data.function & FN_IdxMask] )
			{
				that->inspos = ti->up;
				ti = tiu;
			}

			/*  insert te above ti  */
			if( (te.up=ti->up)==no_term )
				that->root = that->t_size;
			else if( tiu->left==that->inspos )
				tiu->left = that->t_size;
			else
				tiu->right = that->t_size;
			te.left = that->inspos;
			ti->up = that->t_size;

			break;
		};
	}

	return term_add(that, &te);
}


/*   Add a term element to a term.  */
/*   The term is reallocaed if it's list is full.  */
bool term_add(term *that, termelem_s *te)
{
	termelem_s *newarr = NULL;
	termsize_t nlen;


	if( that->t_size>=that->t_alloc )
	{
		if( that->t_alloc==(termsize_t)-1 )
		{
			error(EM_TermTooLong);
			return false;
		}

		if( (nlen=that->t_alloc<<1)<that->t_alloc )
			nlen=(termsize_t)-1;
		if( (newarr=(termelem_s*)(realloc(that->t,nlen*sizeof(termelem_s))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
		that->t = newarr;
		that->t_alloc = nlen;
	}

	*(that->t + (that->inspos=that->t_size++)) = *te;
	return true;
}

/*-----------------------------------*/


bool term_makeGraph(FILE *dfh, termcnt_t termidx)
{
	term *t;
	uint32_t *xpos, *size;
	size_t len;
	termsize_t cnt;
	uint32_t xmax;


	t  = (term*)termlist.buf;
	t += termidx;
	len = t->t_size*sizeof(uint32_t);

	if( (xpos=(uint32_t*)malloc(len))==NULL )
	{
		systemError(EM_OutOfMemory);
		return false;
	}
	if( (size=(uint32_t*)malloc(len))==NULL )
	{
		free(xpos);
		systemError(EM_OutOfMemory);
		return false;
	}
	memset(xpos, 0, len);
	memset(size, 0, len);

	rMakeGraph(t,xpos,size,0,t->root);
	xmax = 0;
	for( cnt=0; cnt<t->t_size; ++cnt)
	{
		if( xmax<xpos[cnt]+size[cnt] )
			xmax=xpos[cnt]+size[cnt];
	}

	fprintf(dfh, "<table border=\"0\">\n<colgroup align=\"center\" span=\"%u\" width=\"%2.2f%%\"></colgroup>\n<tbody align=\"center\">\n", xmax, 100.0/xmax);
	if( !rDrawTable(dfh, t,xpos,size,&t->root,1,xmax) )
	{
		free(xpos);
		free(size);
		return false;
	}
	fprintf(dfh, "</tbody></table>\n");

	free(xpos);
	free(size);
	return true;
}


bool rDrawTable(FILE *dfh, term *t, uint32_t *xpos, uint32_t *size, termsize_t *level, termsize_t level_count, uint32_t xmax)
{
	termsize_t tidx, lr, lw;
	termsize_t *nextlevel;
	termelem_s *telem;
	uint32_t xp,xd;
	const char *fn2text[64] =
	{
		"ForceSize",
		"IsDef",
		"SegmentOf",
		"SizeOf",
		"BitNot",
		"PostInc",
		"PreInc",
		"PostDec",
		"PreDec",
		"PosSign",
		"NegSign",
		"Mult",
		"Div",
		"Modulo",
		"Add",
		"Sub",
		"ShiftLeft",
		"ShiftRight",
		"Smaller",
		"LessEqual",
		"GreaterEqual",
		"Greater",
		"Equal",
		"Unequal",
		"BitAnd",
		"HiByte",
		"LoByte",
		"BitEor",
		"BitOr",
		"BoolAnd",
		"BoolOr",
		"Assign",
		"ReAssign",
		"MultReAssign",
		"DivReAssign",
		"ModReAssign",
		"AddReAssign",
		"SubReAssign",
		"ShiftLeftReAssign",
		"ShiftRightReAssign",
		"AndReAssign",
		"EorReAssign",
		"OrReAssign",

		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",

		"bracket"
	};


	if( (nextlevel=(termsize_t*)malloc((level_count<<1)*sizeof(termsize_t)))==NULL )
	{
		systemError(EM_OutOfMemory);
		return false;
	}
	fprintf(dfh, "<tr>");
	xp = 0;
	for(lr=lw=0; lr<level_count; ++lr)
	{
		telem = t->t+(tidx=*(level+lr));

		xd = (*(xpos+tidx)-xp) + ((*(size+tidx)-2)>>1);
		if( xd==1 )
			fprintf(dfh, "<td>&nbsp;</td>");
		else if( xd>1 )
			fprintf(dfh, "<td colspan=\"%u\">&nbsp;</td>", xd);
		fprintf(dfh, "<td colspan=\"2\">");
		switch( telem->typ )
		{
		case TT_CONST_NUM:
			switch( telem->byteSize )
			{
			case 1:
				fprintf(dfh, "$%02x", telem->data.constant);
				break;
			case 2:
				fprintf(dfh, "$%04x", telem->data.constant);
				break;
			case 3:
				fprintf(dfh, "$%06x", telem->data.constant);
				break;
			default:
				fprintf(dfh, "$%08x", telem->data.constant);
				break;
			}
			break;
		case TT_CONST_STR:
			fprintf(dfh, "<pre>\"");
			printString(dfh, telem->data.string);
			fprintf(dfh, "\"</pre>");
			break;
		case TT_VARIABLE:
			printString(dfh, telem->data.varname);
			break;
		case TT_PC:
			fprintf(dfh, "PC");
			break;
		case TT_FUNCTION:
			fprintf(dfh, "%s", fn2text[telem->data.function & FN_IdxMask] );
			break;
		};
		fprintf(dfh, "</td>");

		xp += xd+2;

		if( telem->left!=no_term )
		{
			*(nextlevel+lw)=telem->left;
			++lw;
		}
		if( telem->right!=no_term )
		{
			*(nextlevel+lw)=telem->right;
			++lw;
		}
	}
	xd = xmax - xp;
	if( xd==1 )
		fprintf(dfh, "<td>&nbsp;</td>");
	else if( xd>1 )
		fprintf(dfh, "<td colspan=\"%u\">&nbsp;</td>", xd);
	fprintf(dfh, "</tr>\n");

	if( lw!=0 )
	{
		xp = 0;
		fprintf(dfh, "<tr>");

		for(lr=0; lr<level_count; ++lr)
		{
			telem = t->t+(tidx=*(level+lr));

			if( telem->left!=no_term && telem->right!=no_term)
			{
				xd = (*(xpos+telem->left)-xp) + ((*(size+telem->left)-2)>>1);
				if( xd==1 )
					fprintf(dfh, "<td>&nbsp;</td>");
				else if( xd>1 )
					fprintf(dfh, "<td colspan=\"%u\">&nbsp;</td>", xd);
				xp += xd;

				xd = (*(xpos+tidx)-xp) + ((*(size+tidx)-2)>>1) + 1;
				if( xd==1 )
					fprintf(dfh, "<td");
				else
					fprintf(dfh, "<td colspan=\"%u\"", xd);
				fprintf(dfh, ">/</td>");
				xp += xd;

				xd = (*(xpos+telem->right)-xp) + ((*(size+telem->right)-2)>>1) + 1;
				if( xd==1 )
					fprintf(dfh, "<td");
				else
					fprintf(dfh, "<td colspan=\"%u\"", xd);
				fprintf(dfh, ">\\</td>");
				xp += xd;
			}
			else if( telem->right!=no_term )
			{
				xd = (*(xpos+telem->right)-xp) + ((*(size+telem->right)-2)>>1);
				if( xd==1 )
					fprintf(dfh, "<td>");
				else if( xd>1 )
					fprintf(dfh, "<td colspan=\"%u\">", xd);
				xp += xd;

				fprintf(dfh, "&nbsp;</td><td colspan=\"2\">|</td>");
				xp += 2;
			}
		}

		xd = xmax - xp;
		if( xd==1 )
			fprintf(dfh, "<td>&nbsp;</td>");
		else if( xd>1 )
			fprintf(dfh, "<td colspan=\"%u\">&nbsp;</td>", xd);
		fprintf(dfh, "</tr>\n");

		if( !rDrawTable(dfh, t,xpos,size,nextlevel,lw,xmax) )
			return false;
	}

	free( nextlevel );
	return true;
}


uint32_t rMakeGraph(term *t, uint32_t *xpos, uint32_t *size, uint32_t pxpos, termsize_t tidx)
{
	uint32_t s;
	termelem_s *telem;


	telem = t->t+tidx;

	/*   My x pos is the xpos which my parent told me  */
	*(xpos+tidx) = pxpos;

	/*   add the size of the left child  */
	s  = ( telem->left !=no_term )?rMakeGraph(t,xpos,size,pxpos  ,telem->left ):0;
	/*   add the size of the right child  */
	s += ( telem->right!=no_term )?rMakeGraph(t,xpos,size,pxpos+s,telem->right):0;
	/*   no children? at least space for me  */
	if( s==0 )
		s=2;

	/*   remember and return my size  */
	return ( *(size+tidx) = s );
}

#if 0	/* don't make graph */
void term_makePic(termcnt_t termidx)
{
	pid_t pid_vcg, pid_cnv;
	char *fname;
	char *picname = NULL;
	int p[2];
	FILE *pfh;
	int of;
	const char *tmpname = "/tmp/dreamass_vcg.ppm";


	/*   don't generate pics by default  */
	if( 1 )
		return;

	unlink(tmpname);
	pipe(p);
	fname = filelist_getName(pp_getFileIdx());
	if( (picname=(char*)malloc(strlen(fname)+18))==NULL )
		assert( false );
	sprintf(picname, "%s_term%04d.png", fname, termidx);

	if( (pid_vcg=fork())==0 )
	{
		/*  child connects pipe output ([1]) to stdin (0)  */
		dup2( p[0], 0 );
		close( p[1] );
		execlp("xvcg","xvcg","-silent","-ppmoutput",tmpname, "-", NULL );
		free( picname );
		assert(false);
	}
	else
	{
		/*  parent connects pipe input ([0]) to the vcg description  */
		if( (pfh=fdopen( p[1],"w"))==NULL )
			assert( false );
		term_debug(termidx, pfh);
		fclose( pfh );
		close( p[0] );
		close( p[1] );
		waitpid(pid_vcg, NULL, 0);
	}

	if( (of=open(picname,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))==-1 )
	{
		printf("%s\n", picname);
		free( picname );
		assert(false);
	}
	if( (pid_cnv=fork())==0 )
	{
		/*  child  */
		dup2( of, 1 );
		execlp("pnmtopng","pnmtopng", tmpname, NULL );
		free( picname );
		assert(false);
	}
	else
	{
		/*  parent  */
		waitpid(pid_cnv, NULL, 0);
		close( p[0] );
		close( p[1] );
		close( of );
	}

	free( picname );
}


void term_debug(termcnt_t termidx, FILE *dfh)
{
	/*  Write a VCG file to filehandle dfh  */
	term *t;
	const char *edgecolor = "blue";
	const char *labelcolor= "black";

	const char *fn2text[] =
	{
		"ForceSize",
		"IsDef",
		"SegmentOf",
		"SizeOf",
		"BitNot",
		"PostInc",
		"PreInc",
		"PostDec",
		"PreDec",
		"PosSign",
		"NegSign",
		"Mult",
		"Div",
		"Modulo",
		"Add",
		"Sub",
		"ShiftLeft",
		"ShiftRight",
		"Smaller",
		"LessEqual",
		"GreaterEqual",
		"Greater",
		"Equal",
		"Unequal",
		"BitAnd",
		"HiByte",
		"LoByte",
		"BitEor",
		"BitOr",
		"BoolAnd",
		"BoolOr",
		"Assign",
		"ReAssign",
		"MultReAssign",
		"DivReAssign",
		"ModReAssign",
		"AddReAssign",
		"SubReAssign",
		"ShiftLeftReAssign",
		"ShiftRightReAssign",
		"AndReAssign",
		"EorReAssign",
		"OrReAssign"
	};

	termsize_t cnt0;
	FUNCTION fn;


/*  	fprintf(dfh, "\n---------------------------------------\n\n");  */
	fprintf(dfh, "graph:\n{\n");
	fprintf(dfh, "\tdisplay_edge_labels: yes\n");
	fprintf(dfh, "\tstraight_phase: yes\n");
	fprintf(dfh, "\tpriority_phase: yes\n");
	fprintf(dfh, "\tport_sharing: no\n");
	fprintf(dfh, "\tcrossing_weight: median\n");
	fprintf(dfh, "\tsplines: yes\n");

	t = termlist+termidx;
	for(cnt0=0; cnt0<t->t_size; cnt0++)
	{
		fprintf(dfh, "\tnode:\n\t{\n\t\ttitle: \"%d\"\n\t\tlabel: \"", cnt0);
		switch( t->t[cnt0].typ )
		{
		case TT_CONST_NUM:
			fprintf(dfh, "Constant : $%04x", t->t[cnt0].data.constant );
			break;
		case TE_CONST_STR:
			fprintf(dfh, "String : '" );
			printString(dfh, t->t[cnt0].data.string);
			fprintf(dfh, "'");
			break;
		case TT_VARIABLE:
			fprintf(dfh, "Variable  : ");
			printString(dfh, t->t[cnt0].data.varname);
			break;
		case TT_PC:
			fprintf(dfh, "PC  : ");
			break;
		case TT_FUNCTION:
			fprintf(dfh, "Function : ");
			if( (fn=t->t[cnt0].data.function)==FN_BracketClosed )
				fprintf(dfh, "( )");
			else if( fn==FN_no )
				fprintf(dfh, "no");
			else
				fprintf(dfh, "%s", fn2text[fn & FN_IdxMask] );
			break;
		};
		fprintf(dfh, "\"\n\t}\n");
		if( t->t[cnt0].right!=no_term )
		{
			fprintf(dfh, "\tedge:\n");
			fprintf(dfh, "\t{\n");
			fprintf(dfh, "\t\tsourcename: \"%d\"\n", cnt0);
			fprintf(dfh, "\t\ttargetname: \"%d\"\n", t->t[cnt0].right);
			fprintf(dfh, "\t\tlabel: \"right\"\n");
			fprintf(dfh, "\t\tcolor: %s\n", edgecolor);
			fprintf(dfh, "\t\ttextcolor: %s\n", labelcolor);
			fprintf(dfh, "\t}\n");
		}
		if( t->t[cnt0].left!=no_term )
		{
			fprintf(dfh, "\tedge:\n");
			fprintf(dfh, "\t{\n");
			fprintf(dfh, "\t\tsourcename: \"%d\"\n", cnt0);
			fprintf(dfh, "\t\ttargetname: \"%d\"\n", t->t[cnt0].left);
			fprintf(dfh, "\t\tlabel: \"left\"\n");
			fprintf(dfh, "\t\tcolor: %s\n", edgecolor);
			fprintf(dfh, "\t\ttextcolor: %s\n", labelcolor);
			fprintf(dfh, "\t}\n");
		}
	}
	fprintf(dfh, "}\n");
/*  	fprintf(dfh, "\n---------------------------------------\n\n");  */
}
#endif	/* don't make graph */

term *getTerm(termcnt_t termidx)
{
	assert( termidx<termlist.elemCount );
	return ((term*)termlist.buf)+termidx;
}


void term_findUndefs(termcnt_t termidx)
{
	term *t;
	termelem_s *tc, *te;
	char *cname;


	assert( termidx<termlist.elemCount );
	t  = (term*)termlist.buf;
	t += termidx;
	for( te=(tc=t->t)+t->t_size; tc<te; ++tc )
	{
		if( tc->typ==TT_VARIABLE )
		{
			if( (cname=string2cstr(tc->data.varname))==NULL ) {
				return;
			}

			/* does the variable exist in general? */
			if( !existAndDefinedVariable(tc->data.varname) )
			{
				error(EM_VarUndef_s, cname);
			}

			free(cname);
		}
	}
}

