/*
   cllib.c - general routines for conflib
   Copyright (C) 1993,1994,1995 Uwe Ohse,  1991, 1992 Wolfram Roesler

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
 */

#include "ownstd.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "conflib.h"
#include "intern.h"

int
cl_varnamecmp (const char *p1, const char *p2)
{
	char c1;
	char c2;
	while (*p1!='\0')
	{
		c1=*p1++;
		c1=tolower(c1); /* tolower makro? */
		if (c1=='_')
			c1='-';

		c2=*p2++;
		c2=tolower(c2); /* tolower makro? */
		if (c2=='_')
			c2='-';

		if (c1 != c2)
			return c1-c2;
	}
	return *p1 - *p2;
} /* cl_varnamecmp */

char *
cl_strtrim (char *s)
{
	char *p;
	size_t l;

	p = s;
	while (*p == ' ' || *p == '\t')
		p++;
	l=strlen(p);
	if (!l)
		return p;
	s = p + l - 1;
	while (s >= p && isspace((unsigned char)*s))
	{ 
		*s-- = '\0';
	}
	return (p);
} /* cl_strtrim */


int cl_dup_to(cl_var_t *confvar, const char *p)
{
	size_t l=strlen(p);
	char *ad;
	if (confvar->flags & CL_MALLOCED)
	{
		confvar->flags -=CL_MALLOCED;
		free(*(char **)confvar->adr);
	}
	ad=malloc(l+1);
	if (!ad)
	{
		cl_warning(CL_ERR(NULL,confvar),"out of memory");
		return 1;
	}
	confvar->flags|=CL_MALLOCED;
	*(char **)confvar->adr=ad;
	strcpy((char *)ad,p);
	return 0;
}

int cl_convert2bool(const char *Ptr)
{
	/* no nice code, indeed */
	switch(*Ptr)
	{
	case 't':   /* true */
	case 'T':
	case 'Y':   /* yes */
	case 'y':
	case 'J':   /* ja */
	case 'j':
	case 'w':   /* wahr */
	case 'W':
		return 1;
		break;
	case 'F':   /* False, Falsch */
	case 'f':
	case 'N':   /* no, nein */
	case 'n':
		return 0;
		break;
	default:
		/* 
         * everything else is assumed numeric,
		 * 0 is false, !=0 means true
		 */
		if (*Ptr=='+' || *Ptr=='-')
			Ptr++;
		if (!isdigit(*Ptr))
			return 0;
		return strtol(Ptr,NULL,10)!=0;
		break;
	}
}

void
cl_invalidate_stanzalist(cl_file_t *conf)
{
	cl_stanza_list_t *st=conf->stanzas;
	conf->stanzas=NULL;
	conf->last_ele=NULL;
	while (st)
	{
		cl_stanza_list_t *next;

		next = st->next;
		if (st->stanzaname)
			free (st->stanzaname);
		free (st);
		st = next;
	}
}

#ifdef NEED_FNMATCH_CASEFOLD
int 
cl_fnmatch_casefold (const char *pattern, const char *string)
{
	char *pat;
	char *str;
	char *p;
	int r;
	pat=strdup(pattern);
	if (!pat)
		return -1;
	str=strdup(string);
	if (!str)
		return -1;
	for (p=pat;*p;p++)
		*p=tolower(*p);
	for (p=str;*p;p++)
		*p=tolower(*p);
	r=cl_fnmatch(pattern,string);
	free(pat);
	free(str);
	return r;
}
#endif

