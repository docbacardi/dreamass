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

typedef struct
{
	sourcefile_t *srcFile;
	uint8_t phaseIdx;
	localdepth_t localBlockIdx;
} stackelem;

static NALLOC srcstack;

/*-----------------------------------*/


void srcstack_zero(void)
{
	memset(&srcstack, 0, sizeof(stackelem));
}


bool srcstack_init(srcstackcnt_t nmemb)
{
	return nalloc_init(&srcstack, sizeof(stackelem), nmemb);
}


void srcstack_cleanUp(void)
{
	nalloc_free(&srcstack);
}


bool srcstack_push(sourcefile_t *src)
{
	stackelem *se;


	if( nalloc_size(&srcstack, srcstack.elemCount+1)==false ) {
		return false;
	}

	se  = (stackelem*)srcstack.buf;
	se += srcstack.elemCount++;

	se->srcFile = src;
	se->phaseIdx = segment_getPhaseIdx();
	se->localBlockIdx = getLocalBlock();

	return true;
}


sourcefile_t *srcstack_pull(void)
{
	stackelem *se;


	if( srcstack.elemCount==0 ) {
		return NULL;
	}
	else
	{
		se  = (stackelem*)srcstack.buf;
		se += --srcstack.elemCount;
		if( se->phaseIdx!=segment_getPhaseIdx() )
		{
			warning(WM_PhaseIdxShift);
		}
		if( se->localBlockIdx!=getLocalBlock() ) {
			warning(WM_LocalBlockOpen);
		}
		return se->srcFile;
	}
}


sourcefile_t *srcstack_peek(void)
{
	stackelem *se;


	if( srcstack.elemCount==0 ) {
		return NULL;
	}
	else {
		se  = (stackelem*)srcstack.buf;
		se += srcstack.elemCount-1;
		return se->srcFile;
	}
}
