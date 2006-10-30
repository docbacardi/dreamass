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

/*
 * This is a collection of little helper routines.
 * The confusing name 'Navi' comes from the game
 * "The legend of Zelda - Ocarina of time". Navi is a little fairy supporting
 * the hero Link in his quest.
 */


/*
 * Name: stringcmp
 * Desc: Compare two h-field strings in strcmp style.
 * Par:  stringsize_t *s1  - first String to compare
 *       stringsize_t *s1  - second String to compare
 * Ret:  -1: s1 <  s2
 *        0: s1 == s2
 *        1: s1 >  s2
 */
int stringCmp(const stringsize_t *s1, const stringsize_t *s2)
{
	stringsize_t minlen;
	int res;


	minlen = (*s1<*s2)?*s1:*s2;
	if( (res=strncmp((char*)(s1+1),(char*)(s2+1),minlen))==0 && *s1!=*s2 )
		res=(*s1<*s2)?-1:1;
	return res;
}


/*
 * Name: stringclone
 * Desc: make a copy of a h-field string in new allocated memory
 * Par:  stringsize_t *str - string to clone
 * Ret:  NULL : malloc failed
 *       other: Pointer to new string
 */
stringsize_t *stringClone(const stringsize_t *str)
{
	stringsize_t *newstr;


	if( (newstr=(stringsize_t*)malloc(sizeof(stringsize_t)+*str))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}
	memcpy( newstr, str, sizeof(stringsize_t)+*str );
	return newstr;
}


/*
 * Name: cstr2string
 * Desc: create a h-field string from a normal char* string
 * Par:  char *cstr - char string
 *       stringsize_t clen - length of char string
 * Ret:  NULL : malloc failed
 *       other: Pointer to new string
 */
stringsize_t *cstr2string(const char *cstr, const stringsize_t clen)
{
	stringsize_t *newstr;


	if( (newstr=(stringsize_t*)malloc(sizeof(stringsize_t)+clen))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}
	*newstr = clen;
	memcpy( (char*)(newstr+1), cstr, clen );
	return newstr;
}


/*
 * Name: string2cstr
 * Desc: create a normal char* string from a h-field string
 * Par:  stringsize_t *str - h-field string
 * Ret:  NULL : malloc failed
 *       other: Pointer to new char string
 */
char *string2cstr(const stringsize_t *str)
{
	char *newstr;


	if( (newstr=(char*)malloc(*str+1))==NULL )
	{
		systemError(EM_OutOfMemory);
		return NULL;
	}
	memcpy( newstr, (char*)(str+1), *str );
	*(newstr+*str) = 0;
	return newstr;
}


/*
 * Name: printString
 * Desc: print a h-field string to a file descriptor
 * Par:  FILE *f - the filedescriptor to print the string to
 *       const stringsize_t *str - the h-field string to print
 * Ret:  nothing
 */
void printString(FILE *f, const stringsize_t *str)
{
	stringsize_t nl;
	char *np;

	if( str!=NULL ) {
		nl=*str;
		np= (char*)(str+1);
		fwrite(np, nl, 1, f);
	} else {
		fwrite("(null)", 6, 1, f);
	}
}



bool readFile(int filedsc, char *pc, size_t length)
{
	ssize_t readLen;
	char *pe;


	/* get pointer to end of buffer */
	pe = pc + length;
	/* read in the data */
	do {
		readLen = read(filedsc, pc, pe-pc);
		if( readLen<1 ) {
			break;
		}
		pc += readLen;
	} while( pc<pe );

	return (pc==pe);
}





bool nalloc_init(NALLOC *ptNAlloc, size_t elemSize, size_t initialLen)
{
	register void *pvBuf = NULL;
	register size_t sMaxLen = ((size_t)-1);


	assert( ptNAlloc!=NULL && elemSize!=0 );

	/* get maximum array size */
	sMaxLen /= elemSize;

	/* is the initial length valid? */
	if( initialLen>sMaxLen ) {
		/* no -> this length can not be allocated */
		systemError(EM_OutOfMemory);
		return false;
	}

	if( initialLen!=0 ) {
		/* allocate initial array */
		pvBuf = malloc(elemSize*initialLen);
		if( pvBuf==NULL ) {
			systemError(EM_OutOfMemory);
			return false;
		}
	}

	/* init the struct */
	ptNAlloc->elemSize = elemSize;
	ptNAlloc->maxLen = sMaxLen;
	ptNAlloc->buf = pvBuf;
	ptNAlloc->elemCount = 0;
	ptNAlloc->bufLen = initialLen;

	return true;
}


bool nalloc_size(NALLOC *ptNAlloc, size_t reqLen)
{
	size_t sBufLen;
	size_t sMaxLen;
	size_t sNewLen;
	void *pvNewBuf;


	assert( ptNAlloc!=NULL );

	/* get current size */
	sBufLen = ptNAlloc->bufLen;
	/* does the requested length 'reqLen' fit into the buffer? */
	if( sBufLen<reqLen ) {
		/* no -> the buffer must be enlarged */

		/* get the maximum length */
		sMaxLen = ptNAlloc->maxLen;

		/* is the new length invalid or is the max size already reached?
		   sBufLen must not be equal to sMaxLen as it will be enlarged.
		   (sBufLen<reqLen -> reqLen>sBufLen>=sMaxLen -> reqLen>sMaxLen -> reqLen is invalid) */
		if( sBufLen>=sMaxLen ) {
			/* yes -> the buffer can not be enlarged */
			systemError(EM_OutOfMemory);
			return false;
		}

		/* get new length, just double the old length */
		sNewLen = sBufLen<<1;
		/* restrict length to max length and check for overflow */
		if( sNewLen>sMaxLen || sNewLen<sBufLen ) {
			/* set to maximum length */
			sNewLen = sMaxLen;
		}

		/* try to reallocate the buffer */
		pvNewBuf = realloc(ptNAlloc->buf, ptNAlloc->elemSize*sNewLen);
		if( pvNewBuf==NULL ) {
			systemError(EM_OutOfMemory);
			return false;
		}

		/* set new pointer and length */
		ptNAlloc->buf = pvNewBuf;
		ptNAlloc->bufLen = sNewLen;
	}

	return true;
}


void nalloc_free(NALLOC *ptNAlloc)
{
	assert( ptNAlloc!=NULL );

	if( ptNAlloc->buf!=NULL ) {
		free(ptNAlloc->buf);
		ptNAlloc->buf = NULL;
	}

	ptNAlloc->elemCount = 0;
	ptNAlloc->bufLen = 0;
}

