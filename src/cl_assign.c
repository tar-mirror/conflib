/*
    cl_assign.c - variable assignments for conflib
    Copyright (C) 1993, 1994, 1995  Uwe Ohse,  1991, 1992 Wolfram Roesler

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


	For questions contact:
	e-mail: (prefered)
		uwe@tirka.gun.de
		Uwe_Ohse@du3.maus.de
	snail-mail:
	    Uwe Ohse, Drosselstrasse 2, 47055 Duisburg, Germany

	$Id: cl_assign.c,v 1.2 1998/08/15 09:43:50 uwe Exp $
	$Date: 1998/08/15 09:43:50 $
	$Name:  $
	$Log: cl_assign.c,v $
	Revision 1.2  1998/08/15 09:43:50  uwe
	*** empty log message ***
	
	Revision 1.1.1.1  1998/07/09 21:57:52  uwe
	import
	
	Revision 1.1.1.3  1997/11/02 17:11:55  uwe
	import nachträglich
	
	Revision 1.1.1.1  1996/06/15 08:55:27  uwe
	Imported sources

	Revision 1.1.1.1  1996/02/18 22:45:23  uwe
	Import Sourcen

	Revision 1.4  1995/05/28 21:00:58  quark
	cl_assign(): bei unbekannten Statements, also wenn space_given != 0,
	wird 1 returniert, nicht 0 wie bei unbekannten Zuweisungen

	Revision 1.3  1995/05/07 19:33:35  quark
	RCS-Keywords hinzugefügt

*/

#include "ownstd.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_REGCOMP
#ifdef HAVE_RX_H
#include <rx.h>
#elif defined(HAVE_REGEX_H)
#include <regex.h>
#endif
#endif

#include "conflib.h"
#include "intern.h"
#include "tinterp.h"
#include "cl_strtol.h"

void (*cl_rangecheck_hook) 
	(cl_file_t *, cl_var_t *, const char *, const char *);

static void
cl_as_dontmatch(cl_file_t *conf, cl_var_t *var, const char *what, 
	const char *val)
{
	if (cl_rangecheck_hook!=NULL)
		(cl_rangecheck_hook)(conf, var, what, val);
	else
		cl_warning(CL_WARN(conf,var),
			"Value %s doesn't match %s %s.",
			val,what,var->rangeexp);
}
	
int 
cl_assign(cl_file_t *conf, cl_var_t *KonfTab,
	const char *varname, const char *value,cl_ulist_t **unknowns)
{
	register cl_var_t *Vp;
	const cl_enum_t *enump;
	int override=0;
	int append=0;
	if (0==strncmp(varname,"override ",9)) {
		varname+=9;
		override=1;
	}
	if (0==strncmp(varname,"append ",7)) {
		varname+=7;
		append=1;
	}

	for(Vp=KonfTab;Vp->varname;Vp++)
	{
		char c1=tolower(*varname);
		char c2=tolower(*Vp->varname);
		if (c1=='-')
			c1='_';
		if (c2=='-')
			c2='_';
		if (c1==c2 && !cl_varnamecmp(varname+1,Vp->varname + 1))
		{
			if (Vp->typ==CL_ALIAS)
			{
				Vp=cl_unalias(conf,KonfTab,Vp);
				if (!Vp)
					return 1;
			}
			if (Vp->flags & CL_WARNING) {
				cl_warning(CL_WARN(conf,Vp),"variable %s should not be used",
						   Vp->varname);
			}
			if (Vp->rangeexp)
			{
				/* we must do a typecheck */
				if (Vp->flags & CL_RANGE_WILDCARD)
				{
					if (cl_fnmatch(Vp->rangeexp,value)!=0)
					{
						cl_as_dontmatch(conf,Vp,"wildcard", value);
						return 0;
					}
				}
				else 
				{
#ifdef HAVE_REGCOMP
					regex_t exp;
					if (regcomp(&exp,Vp->rangeexp,REG_NOSUB)!=0)
					{
						cl_warning(CL_ERR(conf,Vp),
							"cannot compile regular expression %s",
							Vp->rangeexp);
						return -1;
					}
					if (regexec(&exp,value,0,NULL,0)!=0)
					{
						cl_as_dontmatch(conf,Vp,"regular expression", value);
						return 0;
					}
#else
					/* ### was hier zurückliefern? */
#endif
				}
			}
			switch(Vp->typ)
			{
			case CL_ENUM:
				if (!Vp->secdata)
				{
					cl_warning(CL_ERR(conf,Vp),
						"CL_ENUM nullpointer dereference!");
					return 1;
				}
				Vp->flags |= CL_MAY_SAVE;
				for (enump=(const cl_enum_t *) Vp->secdata; enump->magic; enump++)
				{
					if (!strcasecmp(enump->magic,value))
					{
						*(long*)Vp->adr = enump->value;
						return 0;
					}
				}
				cl_warning(CL_WARN(conf,Vp),
					"unknown value %s for enumeration",value);
				*(long*)Vp->adr = (long) 0;
				return 0; /* ### is this good? */
				break;
			case CL_BITFIELD:
				{
					const char *p;
					if (!Vp->secdata) {
						cl_warning(CL_ERR(conf,Vp),
							"CL_BITFIELD nullpointer dereference!");
						return 1;
					}
					Vp->flags |= CL_MAY_SAVE;
					if (override)
						*(unsigned long*)Vp->adr = 0;
					for (p=value;p && *p;) {	
						const char *q;
						int found=0;
						int not=0;
						while (1) {
							while (*p && (isspace(*p) || *p=='|' || *p==';' || *p==','))
								p++;
							if (*p=='!') {
								not=!not;
								p++;
								continue;
							}
							break;
						}
						if (!*p)
							break;
						q=p;
						/* can't use strtok, i want possibly localized isspace */
						while (*q && !isspace(*q) && *q!='|' && *q!=';' && *q!=',')
							q++;
						/* Ok, between p and q is a value */
						for (enump=(const cl_enum_t *) Vp->secdata; enump->magic; enump++)
						{
							if (!strncasecmp(enump->magic,p,q-p) && enump->magic[q-p]==0) {
								if (not)
									*(unsigned long*)Vp->adr &= ~(enump->value);
								else 
									*(unsigned long*)Vp->adr |= enump->value;
								found=1;
							}
						}
						if (!found) {
							cl_warning(CL_WARN(conf,Vp),
								"unknown value %s for enumeration",p);
						}
						p=q;
					}
					return 0;
					break;
				}
			case CL_STRING:	/* Stringvariable */
				if (append && (*(char **)Vp->adr)) {
					size_t l1=strlen(*(char **)Vp->adr);
					size_t l2=strlen(value);
					char *s=malloc(l1+l2+2);
					if (!s)
						return -1;
					Vp->flags |= CL_MAY_SAVE;
					memcpy(s,*(char **)Vp->adr,l1);
					s[l1]=' ';
					memcpy(s+l1+1,value,l2+1);
					if (Vp->flags & CL_MALLOCED)
						free (*(char **)Vp->adr);
					*(char **)Vp->adr=s;
					Vp->flags|=CL_MALLOCED;
					return 0;
				}
				if (0==cl_dup_to(Vp,value))
				{
					Vp->flags |= CL_MAY_SAVE;
					return 0;
				}
				return -1;
			case CL_DIRNAME:	/* Stringvariable */
				{
					size_t l;
					char *buf;
					l=strlen(value);
					buf=malloc(l+2);
					if (!buf)
						return -1;
					memcpy(buf,value,l+1);
					if (buf[l-1]!='/') {
						buf[l]='/';
						buf[l+1]='\0';
						l++;
					}
					if (append && (*(char **)Vp->adr)) {
						size_t l1=strlen(*(char **)Vp->adr);
						char *s=malloc(l1+l+1);
						if (!s)
							return -1;
						Vp->flags |= CL_MAY_SAVE;
						memcpy(s,*(char **)Vp->adr,l1);
						memcpy(s+l1,buf,l+1);
						if (Vp->flags & CL_MALLOCED)
							free (*(char **)Vp->adr);
						*(char **)Vp->adr=s;
						free(buf);
						Vp->flags|=CL_MALLOCED;
						return 0;
					}
					if (0==cl_dup_to(Vp,buf))
					{
						Vp->flags |= CL_MAY_SAVE;
						free(buf);
						return 0;
					}
					free(buf);
				}
				return -1;
			case CL_CHAR:
				if (value[1]!='\0')
					cl_warning(CL_WARN(conf,Vp),"char desired, got string %s", value);
				Vp->flags |= CL_MAY_SAVE;
				*(char *)Vp->adr = *value;
				return 0;
			case CL_NUM:
				Vp->flags |= CL_MAY_SAVE;
				{
					strtol_error s_err;
					s_err=cl_strtol(value,NULL,0,Vp->adr,"bckmw");
					if (s_err!=LONGINT_OK) {
						CL_STRTOL_WARNING(s_err,conf,Vp,value,Vp->varname);
						*(long*)Vp->adr = strtol(value,NULL,0);
					}
				}
				return 0;
			case CL_NUMSHORT:
				Vp->flags |= CL_MAY_SAVE;
				{
					strtol_error s_err;
					long lo;
					int warned=0;
					s_err=cl_strtol(value,NULL,0,&lo,"bckmw");
					if (s_err!=LONGINT_OK) {
						CL_STRTOL_WARNING(s_err,conf,Vp,value,Vp->varname);
						lo=strtol(value,NULL,0);
						warned++;
					} 
					if (lo > SHRT_MAX || lo < SHRT_MIN) {
						if (!warned)
							CL_STRTOL_WARNING(LONGINT_INT_OVERFLOW,conf,Vp,value,Vp->varname);
						lo=0;
					}
					*(short*)Vp->adr = (int) lo;
				}
				return 0;
			case CL_BOOLEAN:
				Vp->flags |= CL_MAY_SAVE;
				*(int *)Vp->adr=cl_convert2bool(value);
				return 0;
			case CL_ARRAY:	/* Stringarray */
				{
					register int Cnt;
					register char **Ap;
					for(Ap=(char**)Vp->adr,Cnt=0;*Ap && Cnt<CL_MAXARRAY;Ap++,Cnt++);
					if (Cnt<CL_MAXARRAY)
					{
						*Ap = malloc(strlen(value)+1);
						if (*Ap)
							strcpy(*Ap,value);
						Ap[1]=0;
					}
					else
					{
						cl_warning(CL_WARN(conf,Vp),"Array %s is full",Vp->varname);
						return -1;
					}
					Vp->flags |= CL_MAY_SAVE;
					return 0;
				}
				break;
			case CL_ALIAS:
				break;
			case CL_DUMMY:
				if (conf->report_unknown)
				{
					cl_warning(CL_WARN(conf,Vp),
						"Warning: dummy %s used, please remove", Vp->varname);
				}
				break;
			case CL_LIST:
				{
					const char *isep;
					const char *this;
					char *psep;
					cl_list_t *last;
					cl_list_t *new;
					if (override) {
						/* we can't free the list as we don't know 
						 * - if they are needed
						 * - if they really have been dynamically allocated
						 */
						*(cl_list_t **)Vp->adr=NULL;
					}
					last=*(cl_list_t **)Vp->adr;
					while (last && last->next)
						last=last->next;

					isep=(const char *)Vp->secdata;
					if (!isep)
						isep=":\t ";

					this=value;
					while (this && *this && strchr(isep,*this))
						this++;
					while (this && *this)
					{
						psep=strpbrk(this,isep);
						if (psep)
							*psep++='\0';
						new=malloc(sizeof(cl_list_t));
						if (new)
							new->inhalt=strdup(this);
						if (!new || !new->inhalt)
						{
							if (new)
								free(new);
							cl_warning(CL_ERR(conf,Vp),"out of memory");
							return(-1);
						}
						new->next=NULL;
						if (last)
							last->next=new;
						else
							*(cl_list_t **)Vp->adr=new;
						last=new;
						this=psep;
						while (this && *this && strchr(isep,*this))
							this++;
						Vp->flags|=CL_MALLOCED;
					} /* while sep */
					Vp->flags |= CL_MAY_SAVE;
					return 0;
				}
				break;
			} /* switch */
			return 0;
		}
	} /* for */

	/* wenn wir unbekannte Variablen sichern sollen. */
	if (unknowns)
	{
		cl_ulist_t *dieser;
		cl_ulist_t *last;
		for (dieser=*unknowns, last=NULL; dieser; 
			last=dieser, dieser=dieser->next)
		{
			if (!strcasecmp(dieser->varname,varname))
			{
				char *old=dieser->inhalt;
				dieser->inhalt=strdup(value);
				if (!dieser->inhalt)
				{
					cl_warning(CL_ERR(conf,Vp),"out of memory");
					dieser->inhalt=old;
					return -1;
				}
				free(old);
				return 0;
			}
		}
		dieser=malloc(sizeof(cl_ulist_t));
		if (!dieser)
		{
			cl_warning(CL_ERR(conf,Vp),"out of memory");
			return 0;
		}
		dieser->varname=strdup(varname);
		dieser->inhalt=strdup(value);
		if (!dieser->varname || !dieser->inhalt)
		{
			free(dieser->varname);
			free(dieser->inhalt);
			free(dieser);
			cl_warning(CL_ERR(conf,Vp),"out of memory");
			return -1;
		}
		dieser->next=NULL;
		if (!last)
			*unknowns=dieser;
		else
			last->next=dieser;
			
		return 0;
	}
	
	if (conf->report_unknown)
		cl_warning(CL_WARN(conf,Vp),
			"unknown variable '%s'",varname);
	return 0;
}

