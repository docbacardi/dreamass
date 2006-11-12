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

static NALLOC filelist;

int ropen(char *name);

/*-----------------------------------*/

void filelist_zero(void)
{
	memset(&filelist, 0, sizeof(NALLOC));
}


bool filelist_init(filescnt_t nmemb)
{
	return nalloc_init(&filelist, sizeof(SRCNAME), nmemb);
}


void filelist_cleanUp(void)
{
	SRCNAME *pc, *pe;


	/*   only free the names, the sourcefiles were created somewhere else  */
	pc = (SRCNAME*)filelist.buf;
	pe = pc + filelist.elemCount;
	while(pc<pe) {
		free((pc++)->name);
	}
	nalloc_free(&filelist);
}


filescnt_t filelist_addFile(sourcefile_t *src, stringsize_t *filename)
{
	SRCNAME *ptSrcName;


	if( nalloc_size(&filelist, filelist.elemCount+1)==false ) {
		return (filescnt_t)-1;
	}

	ptSrcName  = (SRCNAME*)(filelist.buf);
	ptSrcName += filelist.elemCount;
	if( (ptSrcName->name=stringClone(filename))==NULL ) {
		return (filescnt_t)-1;
	}

	ptSrcName->src = src;

	return filelist.elemCount++;
}


stringsize_t *filelist_getName(filescnt_t fileidx)
{
	assert( fileidx<filelist.elemCount );
	return ((SRCNAME*)filelist.buf)[fileidx].name;
}


sourcefile_t *filelist_getSrc(filescnt_t fileidx)
{
	assert( fileidx<filelist.elemCount );
	return ((SRCNAME*)filelist.buf)[fileidx].src;
}


/*
 * Try to open a file for reading in the current
 * directory and all include paths
 */
int filelist_ropen(const stringsize_t *name)
{
	includePathscnt_t cnt0;
	stringsize_t namelen;
	stringsize_t inclen;
	int fdes;
	char *cbuf, *cpos;


	namelen = *name;
	if( (cbuf=(char*)malloc(includeMaxLen+namelen+1))==NULL ) {
		systemError(EM_OutOfMemory);
		return -1;
	}
	/* copy name to cbuf */
	memcpy( cbuf+includeMaxLen, (char*)(name+1), namelen );
	/* terminate the string */
	*(cbuf+includeMaxLen+namelen) = '\0';

	if( (fdes=ropen(cbuf+includeMaxLen))!=-1 ) {
		free(cbuf);
		return fdes;
	}

	for( cnt0=0; cnt0<includePaths_count; ++cnt0 )
	{
		cpos= cbuf+includeMaxLen-(inclen=*(*(includePaths+cnt0)));
		memcpy( cpos, *(includePaths+cnt0)+1, inclen );
		if( (fdes=ropen(cpos))!=-1 )
		{
			break;
		}
	}

	free(cbuf);
	return fdes;
}


int ropen(char *name)
{
	struct stat statbuf;
	int fdes;
	int iOpenFlags;


	iOpenFlags = O_RDONLY;
#ifdef O_BINARY
	iOpenFlags |= O_BINARY;
#endif

	return (
			!stat(name, &statbuf) &&
			S_ISREG(statbuf.st_mode) &&
			(fdes=open(name, iOpenFlags))!=-1
		) ? fdes : -1;
}

