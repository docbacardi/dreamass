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
/* Locals                            */

typedef struct
{
	localdepth_t start;
	localdepth_t length;
} PIDXLEN;

typedef struct
{
	localdepth_t idx;
	bool f_exists;
	VARIABLE var;
} VARIDX;

typedef struct
{
	varcnt_t left;
	varcnt_t right;
	varcnt_t lweight;
	varcnt_t rweight;
	stringsize_t *vname;
	localdepth_t varidx_count, varidx_buflen;
	VARIDX *varidx;
} VARNAME;

typedef struct
{
	bool err;
	varcnt_t pi;
	localdepth_t lb_best;
	localdepth_t lb_idx;
	localdepth_t levelidx;
	varcnt_t *connector;
} VARFIND;

/*-----------------------------------*/

uint8_t makeHash(const char* name, const stringsize_t namelen);

void r_dump(FILE *dfh, varcnt_t pi, localdepth_t level, bool useHtml);

/*  void tdump(FILE *dfh, varcnt_t pi, varcnt_t phere);  */
/*  void tdump_r(FILE *dfh, varcnt_t pi, varcnt_t phere);  */

VARFIND find_var(const stringsize_t *name);
VARADR assumeVariable(const stringsize_t *name);

varcnt_t treeSearch(const char* name, const stringsize_t namelen, const bool addweight, varcnt_t **connector);

VARIABLE *getVarByAdr(const VARADR adr);

/*-----------------------------------*/

varcnt_t *hashtbl;

localdepth_t blockidx;

localdepth_t *parentidx, parentidx_count, parentidx_buflen;
PIDXLEN *pliststart;
localdepth_t pliststart_count, pliststart_buflen;

VARNAME *vars;
varcnt_t vars_count, vars_buflen;

varcnt_t cnt_undefine;
varcnt_t cnt_resolves;

extern FILE *debugLog;

/*-----------------------------------*/

/*   uninitialize variable list  */
void variable_zero(void)
{
	vars_count = vars_buflen = 0;

	hashtbl = NULL;
	vars = NULL;
	parentidx = NULL;
	pliststart = NULL;
}

/*   allocate new variable list  */
bool new_variable(varcnt_t nmemb)
{
	uint8_t cnt = 0;
	varcnt_t *hashp;


	if( (hashp=hashtbl=(varcnt_t*)(malloc(256*sizeof(varcnt_t))))!=NULL &&
	    (vars=(VARNAME*)(malloc((vars_buflen=nmemb)*sizeof(VARNAME))))!=NULL &&
	    (parentidx=(localdepth_t*)(malloc((parentidx_buflen=32)*sizeof(localdepth_t))))!=NULL &&
	    (pliststart=(PIDXLEN*)(malloc((pliststart_buflen=16)*sizeof(PIDXLEN))))!=NULL )
	{
		/*   Clear the Hashtable  */
		do
			*hashp++ = no_var;
		while( ++cnt!=0 );

		vars_count = 0;

		/*   generate index to searchlist for this block  */
		pliststart->start = 0;
		pliststart->length = 1;
		pliststart_count = 1;

		/*   first block is the global one. it has no parent, just search itself  */
		parentidx[0] = 0;
		parentidx_count = 1;

		cnt_undefine = cnt_resolves = 0;

		blockidx = 0;

		return true;
	}
	else
	{
		if( hashtbl!=NULL )
			free( hashtbl );
		if( vars!=NULL )
			free( vars );
		if( parentidx!=NULL )
			free( parentidx );
		if( pliststart!=NULL )
			free( pliststart );

		systemError(EM_OutOfMemory);
		return false;
	}
}

/*   delete variable list  */
void del_variable(void)
{
	VARNAME *vnc, *vne;
	VARIDX *vic, *vie;


	/*   Free Hashtable  */
	if( hashtbl!=NULL )
		free( hashtbl );

	/*   Free Tree Table  */
	vnc = vars;
	if( vnc!=NULL )
	{
		vne = vnc + vars_count;
		/*   Free Name for every entry  */
		while( vnc<vne )
		{
			/*   free all variables  */
			vic = vnc->varidx;
			vie = vic + vnc->varidx_count;
			while( vic<vie )
			{
				deleteVariable(&vic->var);
				++vic;
			}
			free( vnc->varidx );
			free( vnc->vname );
			++vnc;
		}

		/*   Free the complete Table  */
		free( vars );
	}

	if( parentidx!=NULL )
		free( parentidx );
	if( pliststart!=NULL )
		free( pliststart );
}


/*
 * Look for variable and create it if it does
 * not exist yet
 */
VARADR assumeVariable(const stringsize_t *name)
{
	VARIDX *vi_cnt, *newvibuf;
	localdepth_t newvilen;
	VARADR vret = { no_var, 0 };
	VARNAME *newbuf;
	varcnt_t newlen;
	VARNAME *pp;
	VARFIND vf;
	const char *cname;
	stringsize_t namelen;


	if( (vf=find_var(name)).err ) {
		return vret;
	}

	if( vf.pi!=no_var )
	{
		if( vf.lb_best==(localdepth_t)-1 )
		{
			/*   No definition found in this block or it's parents -> insert a new one at the end of the array  */

			/*   is enough space in the array left?  */
			if( (pp=vars+vf.pi)->varidx_count>=pp->varidx_buflen )
			{
				/*   can the array be expanded or is it's maximal size already reached?  */
				if( pp->varidx_buflen==((localdepth_t)-1) )
				{
					error(EM_TooManyLocalBlocks);
					return vret;
				}
				
				/*   double the array's size or use the maximal length  */
				if( (newvilen=pp->varidx_buflen<<1)<pp->varidx_buflen )
					newvilen=((localdepth_t)-1);
				/*   resize the array  */
				if( (newvibuf=(VARIDX*)(realloc(pp->varidx, newvilen*sizeof(VARIDX))))==NULL )
				{
					systemError(EM_OutOfMemory);
					return vret;
				}
				pp->varidx = newvibuf;
				pp->varidx_buflen = newvilen;
			}

			/*   insert new item  */
			vi_cnt = pp->varidx+pp->varidx_count;
			vi_cnt->idx = vf.levelidx;
			vi_cnt->f_exists = false;
			vi_cnt->var.defined = vi_cnt->var.used = vi_cnt->var.final = vi_cnt->var.readwrite = false;
			vi_cnt->var.segidx = (seglistsize_t)-1;
			vi_cnt->var.valt.typ = VALTYP_NUM;
			vi_cnt->var.valt.byteSize = 0;
			vi_cnt->var.valt.value.num = 0;

			/*   a new variable was requested which is not defined yet -> we have another undefined var  */
			++cnt_undefine;

			vret.varidx = vf.pi;
			vret.localidx = pp->varidx_count++;

			return vret;
		}
		else
		{
			vret.varidx = vf.pi;
			vret.localidx = vf.lb_idx;
			return vret;
		}
	}
	else
	{
		/*   The variable was not found. Create it now.  */

		/*   Erase the '@...'  */
		namelen = *name;
		cname = (const char*)(name+1);
		if( namelen>1 && *cname=='@' )
		{
			do
				++cname;
			while( --namelen>1 && isdigit(*cname) );
		}

		/*   set the connector from the upper element (or the hashtable) to this idx  */
		/*   do it before the realloc as the pointer might become invalid  */
		*vf.connector = vars_count;

		/*   is the array full?  */
		if( vars_count >= vars_buflen )
		{
			/*   can the array be expanded or is it's maximal size already reached?  */
			if( vars_buflen==((varcnt_t)-1) )
			{
				error(EM_TooManyVariables);
				return vret;
			}

			/*   double the array's size or use the maximal length  */
			if( (newlen=vars_buflen<<1)<vars_buflen )
				newlen=((varcnt_t)-1);
			/*   resize the array  */
			if( (newbuf=(VARNAME*)(realloc(vars, newlen*sizeof(VARNAME))))==NULL )
			{
				systemError(EM_OutOfMemory);
				return vret;
			}
			vars = newbuf;
			vars_buflen = newlen;
		}

		/*   get a pointer to the next free element in the "vars" array  */
		pp=vars+vars_count;
		/*   The new element has no left and right children  */
		pp->left = pp->right = no_var;
		/*   no children means a weight of '0'  */
		pp->lweight = pp->rweight = 0;
		/*   allocate space for the name string  */
		if( (pp->vname=cstr2string(cname,namelen))==NULL )
			return vret;

		/*  Create a new VARIDX list  */
		if( (pp->varidx=(VARIDX*)malloc(4*sizeof(VARIDX)))==NULL )
		{
			systemError(EM_OutOfMemory);
			return vret;
		}
		pp->varidx_buflen=4;
		/*   Fill the first entry of the new array with the index of this local block and an empty variable  */
		pp->varidx[0].idx = vf.levelidx;
		pp->varidx[0].f_exists = false;
		pp->varidx[0].var.defined = pp->varidx[0].var.used = pp->varidx[0].var.final = pp->varidx[0].var.readwrite = false;
		pp->varidx[0].var.segidx = (seglistsize_t)-1;
		pp->varidx[0].var.valt.typ = VALTYP_NUM;
		pp->varidx[0].var.valt.byteSize = 0;
		pp->varidx[0].var.valt.value.num = 0;
		pp->varidx_count = 1;

		/*   a new variable was requested which is not defined yet -> we have another undefined var  */
		++cnt_undefine;

		/*   return the adress of the variable and increase the array's length counter "vars_count"  */
		vret.varidx = vars_count++;
		vret.localidx = 0;

		/*   increase the weight on the whole path. vf.connector is just a dummy here  */
		treeSearch(cname,namelen, true, &vf.connector);

		return vret;
	}
}


/*
 * Find variable in Tree and local block
 */
VARFIND find_var(const stringsize_t *name)
{
	stringsize_t namelen;
	localdepth_t level;
	char *cname;
	bool digread;
	bool fixedlevel;
	localdepth_t *pi_start, *pi_cnt, *pi_end;
	VARIDX *vi_cnt, *vi_end;
	VARNAME *pp;
	VARFIND vret = { true, 0, 0, 0, 0, NULL };


	/*  start and endpointer to the list of parent indices  */
	pi_end=(pi_start=parentidx+pliststart[blockidx].start)+pliststart[blockidx].length;

	cname=(const char*)(name+1);
	if( (namelen=*name)>1 && *cname=='@' )
	{
		level=0;					/*  the level  */
		digread=false;					/*  no digits read for now  */
		while( --namelen>0 && isdigit(*++cname) )
		{
			level *= 10;
			level += *cname -'0';
			digread = true;				/*  now we have read a digit  */
		}
		if( namelen==0 )				/*  no text after '@' / number -> error!  */
		{
			error(EM_AtWithoutLabelname);
			return vret;
		}
		if(digread)
		{
			if( (pi_cnt=pi_start+level)>=pi_end )	/*  level exceeds local block depth  */
			{
				error(EM_LevelExceedsDepth);
				return vret;
			}
			vret.levelidx = *pi_cnt;		/*  get index of this level  */
		}
		else
			vret.levelidx = blockidx;		/*  no number after the '@' means local, which is the actual blockidx  */
		fixedlevel = true;
	}
	else
	{
		vret.levelidx = blockidx;
		fixedlevel = false;
	}

	if( (vret.pi=treeSearch(cname,namelen, false, &vret.connector))!=no_var )
	{
		/*   The variable was found. Search the VARIDX list for definition in the deepest local block  */

		pp = vars+vret.pi;
		vret.lb_best = (localdepth_t)-1;

		vi_cnt = pp->varidx;
		vi_end = vi_cnt + pp->varidx_count;
		/*   search the list  */
		if( fixedlevel )
		{
			/*   Only search for exact match  */
			while( vi_cnt<vi_end )
			{
				if( vi_cnt->idx==vret.levelidx )
				{
					vret.lb_best= 0;
					vret.lb_idx = vi_cnt-pp->varidx;
					break;
				}
				++vi_cnt;
			};
		}
		else
		{
			/*   somehow I don't like this... But I can't sort the list either. Nah, maybe nobody sees it ;)  */
			while( vi_cnt<vi_end )
			{
				for( pi_cnt=pi_start; pi_cnt<pi_end; ++pi_cnt)
				{
					if( *pi_cnt==vi_cnt->idx && (vret.lb_best<*pi_cnt || vret.lb_best==(localdepth_t)-1) )
					{
						vret.lb_best = *pi_cnt;
						vret.lb_idx = vi_cnt-pp->varidx;
					}
				}
				++vi_cnt;
			};
		}
	}

	vret.err = false;
	return vret;
}


/*
 * Tree search
 * looks for the variable in the nametree
 * In Parameter:
 *   char* name = pointer to the name
 *   stringsize_t namelen = length of the name
 *   bool addweight = change weights in the path
 * Out Parameter:
 *   Index of found Variable or 'no_var'
 */
varcnt_t treeSearch(const char* name, const stringsize_t namelen, const bool addweight, varcnt_t **connector)
{
	varcnt_t *up;
	size_t minlen;
	stringsize_t pl;
	VARNAME *lp, *pp=NULL, *rp;
	varcnt_t pi;
	int cmpres;
	bool checkwheight;


	checkwheight=addweight;

	pi=*(up=hashtbl+makeHash(name,namelen));

	while( pi!=no_var )
	{
		pp = vars+pi;

		/*   Compare  */
		minlen=( namelen<(pl=*pp->vname) ) ? namelen : pl;

		if( (cmpres=strncmp( name, (char*)(pp->vname+1), minlen ))==0 && namelen!=pl )
			cmpres = (namelen<pl)?-1:1;

		if( cmpres==0 )
			break;
		else if( cmpres<0 )
		{
			if( checkwheight && (++pp->lweight>pp->rweight+8) )
			{
				/*   swap pp and pp->left  */
				lp = vars+(*up=pp->left);

				pp->left = lp->right;
				lp->right = pi;

				pp->lweight = lp->rweight;
				lp->rweight += pp->rweight + 1;

				pi = *up;

				checkwheight=false;
			}
			else
			{
				up = &pp->left;
				pi=pp->left;
				checkwheight=addweight;
			}
		}
		else
		{
			if( checkwheight && (++pp->rweight>pp->lweight+8) )
			{
				/*   swap pp and pp->right  */
				rp = vars+(*up=pp->right);

				pp->right = rp->left;
				rp->left = pi;

				pp->rweight = rp->lweight;
				rp->lweight += pp->lweight + 1;

				pi = *up;

				checkwheight=false;
			}
			else
			{
				up = &pp->right;
				pi=pp->right;
				checkwheight=addweight;
			}
		}
	};

	*connector=up;
	return pi;
}


VARADR writeVariable(const stringsize_t *name, const VARIABLE invar, const bool allowRedefine)
{
	VARIABLE *var;
	char *sname;
	VARADR vget, vret = { no_var, 0 };


	/*   try to find variable, if it doesn't exist yet, create it  */
	if( (vget=assumeVariable(name)).varidx==no_var )
		return vret;

	/*   get the variable's address for direct modification  */
	var = &vars[vget.varidx].varidx[vget.localidx].var;

	/*   only allow modification of an already defined variable if  */
	/*   redefinition is requested or the variable is non-final  */
	if( var->defined )
	{
		if( !allowRedefine && var->final )
		{
			/*   redefinition not allowed, show error  */
			if( (sname=string2cstr(name))==NULL )
			{
				error(EM_VarRedefinition_s, "");
				return vret;
			}
			error(EM_VarRedefinition_s, sname);
			free(sname);
			return vret;
		}

		/*   free old value of variable  */
		switch( invar.valt.typ ) {
		case VALTYP_NUM:
			/*   nothing to do for a number...  */
			break;
		case VALTYP_STR:
			/*   free the old string  */
			free(var->valt.value.str);
			break;
		}
	}

	/*   resolved another undefined variable?  */
	if( !var->defined && invar.defined )
	{
		--cnt_undefine;
		++cnt_resolves;
	}

	/*   set the variable to the new value  */
	/*   first do a simple copy of all elements  */
	*var = invar;
	copyVariable(var);

	return vget;
}


void resetResolveCounter(void)
{
	cnt_resolves = 0;
}


bool resolvedSomething(void)
{
	return cnt_resolves>0;
}


uint8_t makeHash(const char* name, const stringsize_t namelen)
{
	/* no good hash function, but keeps all labels in sorted order -> no need to sort at label dump time */
	return *name;
}


/*
 * Dump all labels up to a level
 * Output should be reuseable.
 */
bool dump(FILE *dfh, localdepth_t level, bool useHtml)
{
	uint8_t hash;

	if( useHtml )
	{
		fprintf(dfh, "<table><tbody>\n");
	}

	hash = 0;
	do
		r_dump( dfh, *(hashtbl+hash), level, useHtml );
	while( ++hash!=0 );

	if( useHtml )
	{
		fprintf(dfh, "</tbody></table>\n");
	}

	return !ferror(dfh);
}


void r_dump(FILE *dfh, varcnt_t pi, localdepth_t level, bool useHtml)
{
	VARNAME *pp;
	VARIABLE var;
	localdepth_t cnt;


	if( pi!=no_var )
	{
		pp = vars + pi;
		r_dump( dfh, pp->left, level, useHtml );

		for( cnt=0; cnt<pp->varidx_count; cnt++ )
		{
			if( pp->varidx[cnt].idx<=level )
			{
				if( useHtml )
				{
					fprintf(dfh, "<tr><td>");
				}

				printString(dfh, pp->vname);
				if( useHtml )
				{
					fprintf(dfh, "</td><td>");
				}
				else
				{
					fprintf(dfh," = ");
				}
				var = pp->varidx[cnt].var;
				switch( var.valt.typ )
				{
				case VALTYP_NUM:
					switch( var.valt.byteSize )
					{
					case 1:
						fprintf(dfh, "$%02x", var.valt.value.num);
						break;
					case 2:
						fprintf(dfh, "$%04x", var.valt.value.num);
						break;
					case 3:
						fprintf(dfh, "$%06x", var.valt.value.num);
						break;
					default:
						fprintf(dfh, "$%08x", var.valt.value.num);
						break;
					}
					break;
				case VALTYP_STR:
					fprintf(dfh, "\"");
					printString(dfh, var.valt.value.str);
					fputc('\"', dfh);
					break;
				};

				if( useHtml )
				{
					fprintf(dfh, "</td><td>");
				}

				if( !var.used )
				{
					if( useHtml )
					{
						fprintf(dfh, "unused");
					}
					else
					{
						fprintf(dfh, "\t\t;unused" );
					}
				}
				else if( useHtml )
				{
					fprintf(dfh, "&nbsp;");
				}

				if( useHtml )
				{
					fprintf(dfh, "</td></tr>");
				}

				fputc('\n', dfh);
			}
		}

		r_dump( dfh, pp->right, level, useHtml );
	}
}

/*
void tdump(FILE *dfh, varcnt_t pi, varcnt_t phere)
{
	//Write a VCG file to filehandle dfh


	fprintf(dfh, "\n---------------------------------------\n");
	fprintf(dfh, "Variable Tree dump\n\n");
	fprintf(dfh, "graph:\n{\n");
	fprintf(dfh, "\tdisplay_edge_labels: yes\n");
	fprintf(dfh, "\tstraight_phase: yes\n");
	fprintf(dfh, "\tpriority_phase: yes\n");
	fprintf(dfh, "\tport_sharing: no\n");
	fprintf(dfh, "\tcrossing_weight: median\n");
	fprintf(dfh, "\tsplines: yes\n");

	tdump_r(dfh, pi, phere);

	fprintf(dfh, "}\n");
	fprintf(dfh, "\n---------------------------------------\n\n");
}


void tdump_r(FILE *dfh, varcnt_t pi, varcnt_t phere)
{
	const char *edgecolor = "blue";
	const char *labelcolor= "black";
	VARNAME *pp;
	stringsize_t nlen;
	char *nc;


	pp = vars + pi;

	fprintf(dfh, "\tnode:\n\t{\n\t\ttitle: \"%d\"\n\t\tlabel: %c\"", pi, (pi==phere)?'*':' ');
	//Dirty hack to dump name
	nlen = *(pp->vname);
	nc = (char*)(pp->vname+1);
	while( nlen-->0 )
		fprintf(dfh, "%c", *(nc++) );
	fprintf(dfh, "\"\n\t}\n");

	if( pp->left!=no_var )
	{
		fprintf(dfh, "\tedge:\n");
		fprintf(dfh, "\t{\n");
		fprintf(dfh, "\t\tsourcename: \"%d\"\n", pi);
		fprintf(dfh, "\t\ttargetname: \"%d\"\n", pp->left);
		fprintf(dfh, "\t\tlabel: \"left\"\n");
		fprintf(dfh, "\t\tcolor: %s\n", edgecolor);
		fprintf(dfh, "\t\ttextcolor: %s\n", labelcolor);
		fprintf(dfh, "\t}\n");
	}
	if( pp->right!=no_var )
	{
		fprintf(dfh, "\tedge:\n");
		fprintf(dfh, "\t{\n");
		fprintf(dfh, "\t\tsourcename: \"%d\"\n", pi);
		fprintf(dfh, "\t\ttargetname: \"%d\"\n", pp->right);
		fprintf(dfh, "\t\tlabel: \"right\"\n");
		fprintf(dfh, "\t\tcolor: %s\n", edgecolor);
		fprintf(dfh, "\t\ttextcolor: %s\n", labelcolor);
		fprintf(dfh, "\t}\n");
	}

	if( pp->left!=no_var )
		tdump_r(dfh, pp->left, phere);
	if( pp->right!=no_var )
		tdump_r(dfh, pp->right, phere);
}
*/

localdepth_t localDive(void)
{
	localdepth_t depth, newpilen, *newpibuf;
	localdepth_t newpslen;
	localdepth_t *pcnt, *pend;
	PIDXLEN* newpsbuf;


	depth = pliststart[blockidx].length;

	/*   need to resize the parentidx array?  */
	if( parentidx_count+depth+1 > parentidx_buflen )
	{
		/*   can the array be expanded or is it's maximal size already reached?  */
		if( parentidx_buflen==((localdepth_t)-1) )
		{
			error(EM_TooManyVariables);
			return (localdepth_t)-1;
		}

		/*   double the array's size or use the maximal length  */
		if( (newpilen=parentidx_buflen<<1)<parentidx_buflen )
			newpilen=((localdepth_t)-1);
		/*   resize the array  */
		if( (newpibuf=(localdepth_t*)(realloc(parentidx, newpilen*sizeof(localdepth_t))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return (localdepth_t)-1;
		}
		parentidx = newpibuf;
		parentidx_buflen = newpilen;
	}

	/*   need to resize the pliststart array?  */
	if( pliststart_count >= pliststart_buflen )
	{
		/*   can the array be expanded or is it's maximal size already reached?  */
		if( pliststart_buflen==((localdepth_t)-1) )
		{
			error(EM_TooManyVariables);
			return (localdepth_t)-1;
		}

		/*   double the array's size or use the maximal length  */
		if( (newpslen=pliststart_buflen<<1)<pliststart_buflen )
			newpilen=((localdepth_t)-1);
		/*   resize the array  */
		if( (newpsbuf=(PIDXLEN*)(realloc(pliststart, newpslen*sizeof(PIDXLEN))))==NULL )
		{
			systemError(EM_OutOfMemory);
			return (localdepth_t)-1;
		}
		pliststart = newpsbuf;
		pliststart_buflen = newpslen;
	}

	/*   get start and end of the parent list of this local block  */
	/*   this will be the base of the list for the new block  */
	/*   the depth of the new local block is the depth of the current +1  */
	pend = (pcnt=parentidx+pliststart[blockidx].start) + depth;

	/*   the new index of the local block is the number of used local blocks which happens to be the same as pliststart_count  */
	blockidx = pliststart_count;

	/*   generate index to searchlist for this block  */
	pliststart[blockidx].start = parentidx_count;
	pliststart[blockidx].length = depth + 1;
	++pliststart_count;

	/*   copy the parent's parents to the list  */
	while( pcnt<pend )
		parentidx[parentidx_count++] = *(pcnt++);
	/*   and this block too  */
	parentidx[parentidx_count++] = blockidx;

	return blockidx;
}


localdepth_t localUp(void)
{
	if( blockidx!=0 )
	{
		return ( blockidx = *(parentidx + pliststart[blockidx].start + (pliststart[blockidx].length-2) ) );
	} else {
		return (localdepth_t)-1;
	}
}


localdepth_t getLocalBlock(void)
{
	return blockidx;
}


void setLocalBlock(localdepth_t bidx)
{
	blockidx = bidx;
}


VARIABLE *getVarByAdr(const VARADR adr)
{
	return &vars[adr.varidx].varidx[adr.localidx].var;
}


bool existVariable(const stringsize_t *name)
{
	VARFIND vf;


	return ( !(vf=find_var(name)).err && vf.pi!=no_var && vf.lb_best!=(localdepth_t)-1 );
}


bool existAndDefinedVariable(const stringsize_t *name)
{
	VARFIND vf;
	VARADR vadr;
	VARIABLE *pVar;


	vf = find_var(name);
	if( !vf.err && vf.pi!=no_var && vf.lb_best!=(localdepth_t)-1 ) {
		vadr.varidx = vf.pi;
		vadr.localidx = vf.lb_idx;
		pVar = getVarByAdr(vadr);
		return pVar->defined;
	} else {
		return false;
	}
}


void writeVarAdr(const VARADR adr, const VARIABLE invar)
{
	VARIABLE *var;


	(var=getVarByAdr(adr))->defined = true;
	/*   delete old data structure  */
	deleteVariable(var);
	/*   copy new data  */
	*var = invar;
	copyVariable(var);
}


VARERR readVariable(const stringsize_t *name)
{
	VARERR vret;
	VARADR vadr;
	VARFIND vf;
	VARIABLE *vp;


	vret.err = true;
	vret.var.defined = vret.var.used = vret.var.final = vret.var.readwrite = false;
	vret.var.segidx = (seglistsize_t)-1;
	vret.var.valt.typ = VALTYP_NUM;
	vret.var.valt.byteSize = 0;
	vret.var.valt.value.num = 0;

	if( !(vf=find_var(name)).err )
	{
		vret.err = false;
		if( vf.pi!=no_var && vf.lb_best!=(localdepth_t)-1 )
		{
			vadr.varidx = vf.pi;
			vadr.localidx = vf.lb_idx;
			(vp=getVarByAdr(vadr))->used = true;
			vret.var = *vp;
		}
	}
	return vret;
}


bool getRMWPointer(const stringsize_t *name, VARADR *adr, VARIABLE **var)
{
	VARFIND vf;


	if( (vf=find_var(name)).err )
		return false;

	if( vf.pi!=no_var && vf.lb_best!=(localdepth_t)-1 )
	{
		adr->varidx = vf.pi;
		adr->localidx = vf.lb_idx;
		(*var=getVarByAdr(*adr))->used = true;
	}
	else
		*var = NULL;
	return true;
}


const stringsize_t *getVarName(varcnt_t varidx)
{
	return (vars+varidx)->vname;
}


/*
 * Name:
 *   variable_getRealSize
 *
 * Description:
 *   Get the real (not forced) size of the variable in bytes.
 *
 * Parameters:
 *   int32_t val = value to examine
 *
 * Returns:
 *   uint32_t = size of the value in bytes
 *
 * TODO:
 *   size of all negative values is 4, is this ok?
 */
uint32_t value_getRealSize(int32_t val)
{
	uint32_t len;


	if( val>=0 && val<0x00000100 )
	{
		len = 1;
	}
	else if( val>=0x00000100 && val<0x00010000 )
	{
		len = 2;
	}
	else if( val>=0x00010000 && val<0x01000000 )
	{
		len = 3;
	}
	else
	{
		len = 4;
	}

	return len;
}



/*
 * Name:
 *   deleteVariable
 *
 * Description:
 *   delete the datastructures of a variable, (e.g. for strings the string array)
 *
 * Parameters:
 *   VARIABLE *var = pointer to the variable
 *
 * Returns:
 *   -
 *
 */
void deleteVariable(VARIABLE *var)
{
	if( var->defined )
	{
		switch( var->valt.typ )
		{
		case VALTYP_NUM:
			/*   nothing to free for numbers  */
			break;
		case VALTYP_STR:
			free(var->valt.value.str);
			var->valt.value.str = NULL;
			break;
		}

		var->defined = false;
	}
}


/*
 * Name:
 *   copyVariable
 *
 * Description:
 *   copy a variable with respects to the datastructures (e.g. for strings the string array)
 *
 * Parameters:
 *   VARIABLE *var = pointer to the variable
 *
 * Returns:
 *   -
 *
 */
void copyVariable(VARIABLE *var)
{
	if( var->defined )
	{
		switch( var->valt.typ )
		{
		case VALTYP_NUM:
			/*   nothing to free for numbers  */
			break;
		case VALTYP_STR:
			/*   clone the string  */
			var->valt.value.str = stringClone(var->valt.value.str);
			break;
		}
	}
}


