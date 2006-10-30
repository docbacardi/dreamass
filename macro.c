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
/* locals                            */

static NALLOC macros;

/*-----------------------------------*/

void macro_zero(void)
{
	memset(&macros, 0, sizeof(NALLOC));
}


bool macro_init(macro_cnt nmemb)
{
	return nalloc_init(&macros, sizeof(macro_t), nmemb);
}


void macro_cleanUp(void)
{
	macro_t *ms, *me;
	mline_t *ls, *le;


	ms = (macro_t*)macros.buf;
	me = ms + macros.elemCount;
	while(ms<me) {
		free(ms->sname);
		ls = ms->mlines;
		le = ls + ms->mlines_count;
		while(ls<le) {
			if( ls->textLength ) {
				free( ls->data.line );
			}
			++ls;
		}
		free( ms->mlines );
		++ms;
	}
	nalloc_free(&macros);
}


macro_cnt macro_add(char *mname, linesize_t mname_len)
{
	macro_t *mt;
	const psyopc_s *pscnt;
	macro_cnt mcnt;


	/* macro name must not be one of the psyopcodes */
	pscnt = psyopc;
	while( pscnt<psyopc+arraysize(psyopc) ) {
		if( mname_len==pscnt->keylen && !strncasecmp(pscnt->key, mname, mname_len) ) {
			break;
		}
		else {
			pscnt++;
		}
	}
	/* found psy opcode? */
	if( pscnt<psyopc+arraysize(psyopc) ) {
		error(EM_ReservedMacroName);
		return (macro_cnt)-1;
	}

	if( nalloc_size(&macros, macros.elemCount+1)==false ) {
		return (macro_cnt)-1;
	}

	mt  = (macro_t*)macros.buf;
	mt += macros.elemCount;

	mt->isActive = false;
	mt->sname = cstr2string(mname,mname_len);
	if( mt->sname==NULL ) {
		systemError(EM_OutOfMemory);
		return (macro_cnt)-1;
	}

	mt->mlines_count = 0;
	mt->parameter_count = 0;
	mt->mlines = (mline_t*)malloc(16*sizeof(mline_t));
	if( mt->mlines==NULL ) {
		systemError(EM_OutOfMemory);
		return (macro_cnt)-1;
	}

	mcnt = macros.elemCount;
	++macros.elemCount;

	return mcnt;
}


linescnt_t macro_addLine(macro_cnt macroIdx, mline_data_t data, linesize_t line_len, linescnt_t mlines_buflen)
{
	macro_t *mac;
	linescnt_t newlen;
	mline_t *ml;


	/* check index */
	assert( macroIdx<macros.elemCount );

	/* get pointer to the macro */
	mac  = (macro_t*)macros.buf;
	mac += macroIdx;

	if( mac->mlines_count >= mlines_buflen )
	{
		if( mlines_buflen==((linescnt_t)-1) )
		{
			error(EM_TooManyMacros);
			return (linescnt_t)-1;
		}

		if( (newlen=mlines_buflen<<1)<mlines_buflen )
			newlen=((linescnt_t)-1);
		if( (ml=(mline_t*)realloc(mac->mlines, newlen*sizeof(mline_t)))==NULL )
		{
			systemError(EM_OutOfMemory);
			return (linescnt_t)-1;
		}
		mac->mlines = ml;
		mlines_buflen = newlen;
	}

	if( ((ml=mac->mlines+mac->mlines_count)->textLength=line_len)!=0 )
	{
		/*
		 * Textlen>0 -> its a real line
		 */
		if( (ml->data.line=(char*)malloc(line_len))==NULL )
		{
			systemError(EM_OutOfMemory);
			return (linescnt_t)-1;
		}
		memcpy(ml->data.line, data.line, line_len);
	}
	else
		ml->data.pidx = data.pidx;

	++mac->mlines_count;
	return mlines_buflen;
}


bool macro_finalize(macro_cnt macroIdx, linescnt_t mlines_buflen)
{
	macro_t *mac;
	mline_t *ml;


	/* check index */
	assert( macroIdx<macros.elemCount );

	/* get pointer to the macro */
	mac  = (macro_t*)macros.buf;
	mac += macroIdx;

	/* were more lines allocated than neccessary? */
	if( mac->mlines_count<mlines_buflen )
	{
		/* yes -> free the unused lines */
		if( (ml=(mline_t*)(realloc(mac->mlines, mac->mlines_count*sizeof(mline_t))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return false;
		}
		mac->mlines = ml;
	}

	return true;
}


/*
   test macro name, must be unique
   on success activate macro
*/
bool macro_activate(macro_cnt macroIdx)
{
	macro_t *mac;
	macro_cnt m;
	const char *mac_name;
	linesize_t mac_name_len;
	char *mac_cname;


	/* check index */
	assert( macroIdx<macros.elemCount );

	/* get pointer to the macro */
	mac  = (macro_t*)macros.buf;
	mac += macroIdx;

	/* look for other macro with the same name */
	mac_name = (const char*)(mac->sname+1);
	mac_name_len = *(mac->sname);
	m = macro_find(mac_name, mac_name_len);
	if( m!=(macro_cnt)-1 ) {
		mac_cname = string2cstr(mac->sname);
		if( mac_cname==NULL ) {
			systemError(EM_OutOfMemory);
		}
		else {
			error(EM_MacroRedefinition_s, mac_cname);
			free(mac_cname);
		}
		return false;
	}


	/* activate macro */
	mac->isActive = true;

	return true;
}


macro_cnt macro_find(const char *name, linesize_t name_len)
{
	macro_cnt mcnt;
	macro_t *ms, *me;


	mcnt = 0;
	ms = (macro_t*)macros.buf;
	me = ms+macros.elemCount;
	while(ms<me) {
		if(
			/* is the macro active? */
			ms->isActive &&
			/* is the length equal? */
			name_len==*ms->sname &&
			/* are the strings equal? */
			memcmp(name, ms->sname+1, name_len)==0
		  )
		{
			return mcnt;
		}
		++ms;
		++mcnt;
	}

	return (macro_cnt)-1;
}


macro_t *macro_get(macro_cnt macroIdx)
{
	if( macroIdx<macros.elemCount )
	{
		return ((macro_t*)macros.buf) + macroIdx;
	}
	else
	{
		return NULL;
	}
}

