/*
    macro.c - Routinen um die Makros
    Copyright (C) 1995  Uwe Ohse

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
		Uwe_Ohse@mausdu3.gun.de
		uwe@tirka.gun.de
	snail-mail:
	    Uwe Ohse, Drosselstrasse 2, 47055 Duisburg, Germany

$Id: cl_macro.c,v 1.1.1.1 1998/07/09 21:57:52 uwe Exp $
$Date: 1998/07/09 21:57:52 $
$Name:  $
$Log: cl_macro.c,v $
Revision 1.1.1.1  1998/07/09 21:57:52  uwe
import

Revision 1.2  1997/11/02 18:41:00  uwe
Einen Bug bereinigt und einige Warnungen beseitigt

Revision 1.1.1.2  1997/11/02 17:11:39  uwe
import nachträglich

Revision 1.1.1.1  1996/06/15 08:55:27  uwe
Imported sources

Revision 1.1.1.1  1996/02/18 22:45:23  uwe
Import Sourcen

Revision 1.2  1995/05/08 19:02:12  quark
Falschen Rückgabewert bei undef eines nicht existierenden Makros (ERROR)
nach 0 (OK) geändert.

Revision 1.1  1995/05/08 18:34:47  quark
Initial revision

*/

#include "ownstd.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "tinterp.h"
#include "conflib.h"

#define PARANOIA_STRING(fn_name,start,this_magic) \
do { \
	if (strncmp(start,this_magic,strlen(this_magic))) \
	{ \
		tip_errtext = fn_name " called without proper magic " this_magic ; \
		return TIP_ERROR; \
	} \
} while (0)

#define PARANOIA_CHAR(fn_name,start,this_magic) \
do { \
	if (*start!=*this_magic) \
	{ \
		tip_errtext = fn_name " called without proper magic " this_magic ; \
		return TIP_ERROR; \
	} \
} while (0)

#define LENCHECK(l) if (l>=maxlen) { tip_errtext="buffer shortage"; return TIP_ERROR; }
#define ADD_CHAR(c) { *to=c; to[1]='\0'; return 1; } 

typedef struct  tip_macro_t
{
	char *name;
	char *value;
	struct tip_macro_t *next;
} tip_macro_t;
static tip_macro_t *anker;


int 
tip_defmacro_intern(const char *name, const char *value)
{
	tip_macro_t *akt=anker;
	tip_macro_t *last=NULL;
	tip_macro_t *neu;

	if (!value)
	{
		/* undef ... */
		akt=anker;
		while (akt)
		{
			if (!strcmp(akt->name,name))
			{
				if (last)
					last->next=akt->next;
				else
					anker=akt->next;
				free(akt->name);
				free(akt->value);
				free(akt);
				return 0;
			}
			last=akt;
			akt=akt->next;
		}
		return 0;
	}


	akt=anker;
	while (akt)
	{
		int vgl;
		vgl=strcmp(akt->name,name);
		if (vgl == 0)
		{
			char *n=strdup(value);
			if (!n)
				return TIP_ERROR;
			free(akt->value);
			akt->value=n;
			return 0;
		}
		else if (vgl > 0)
		{
			neu=malloc(sizeof(tip_macro_t));
			if (!neu)
				return TIP_ERROR;
			neu->name=strdup(name);
			if (!neu->name)
			{	
				free(neu);
				return TIP_ERROR;
			}
			neu->value=strdup(value);
			if (!neu->value)
			{
				free(neu->name);
				free(neu);
				return TIP_ERROR;
			}
			neu->next=akt;
			if (!last)
				anker=neu;
			else
				last->next=neu;
			return 0;
		}
		last=akt;
		akt=akt->next;
	}
	neu=malloc(sizeof(tip_macro_t));
	if (!neu)
		return TIP_ERROR;
	neu->name=strdup(name);
	if (!neu->name)
	{	
		free(neu);
		return TIP_ERROR;
	}
	neu->value=strdup(value);
	if (!neu->value)
	{
		free(neu->name);
		free(neu);
		return TIP_ERROR;
	}
	neu->next=NULL;
	if (last)
		last->next=neu;
	else
		anker=neu;
	return 0;
}

/*
 * ${macro any} -> "macro value", interpretiert!
 */
size_t 
tip_macro(tip_t *tab, const char *format, 
	const char **next, char *to, size_t maxlen)
{
	const char *ende=NULL;
	const char *start=NULL;
	tip_macro_t *akt;
	size_t vgllen;
#define MAGIC "${macro "

	PARANOIA_STRING("tip_macro",format, MAGIC );

	*to='\0';
	start=format+sizeof(MAGIC)-1;
#undef MAGIC

	ende=tip_getpart(start,"}");
	if (!ende)
		return TIP_ERROR;

	vgllen=ende-start;
	*next=ende+1;

	akt=anker;
	while (akt)
	{
		int di;
		di=strncmp(akt->name,start,vgllen);
		if (di>0)	
			break;
		if (di==0)
		{
			size_t len=tip_interpret(tab,akt->value,to,maxlen);
			if (len!=TIP_ERROR)
				return len-1;
			return TIP_ERROR;
		}
		akt=akt->next;
	}
	return tip_interpret(tab,"undefined",to,maxlen);
}

/*
 * ${defmacro test HALLO} -> "test" hat Wert "HALLO"
 */
size_t 
tip_defmacro(tip_t *tab __CL_ATTRIB_UNUSED, const char *format, 
	const char **next, char *to, size_t maxlen __CL_ATTRIB_UNUSED)
{
	const char *ende=NULL;
	const char *start=NULL;
	char *kopie;
	char *p;
	size_t vgllen;
	int ret;
#define MAGIC "${defmacro "

	PARANOIA_STRING("tip_defmacro",format, MAGIC );

	*to='\0';
	start=format+sizeof(MAGIC)-1;
#undef MAGIC

	ende=tip_getpart(start,"}");
	if (!ende)
		return TIP_ERROR;

	vgllen=ende-start;

	kopie=malloc(ende-start+1);
	if (!kopie)
		return TIP_ERROR;
	strncpy(kopie,start,ende-start);
	kopie[ende-start]=0;

	p=strpbrk(kopie,"\t ");
	if (p)
		*p++=0;

	ret = tip_defmacro_intern(kopie, p ? p : "");
	free(kopie);
	if (ret)
		return ret;

	*next=ende+1;
	return 0;
}

/*
 * ${undefmacro test}
 */
size_t 
tip_undefmacro(tip_t *tab __CL_ATTRIB_UNUSED, const char *format, 
	const char **next, char *to, size_t maxlen __CL_ATTRIB_UNUSED)
{
	const char *ende=NULL;
	const char *start=NULL;
	char *kopie;
	size_t vgllen;
	int ret;
#define MAGIC "${undefmacro "

	PARANOIA_STRING("tip_undefmacro",format, MAGIC );

	*to='\0';
	start=format+sizeof(MAGIC)-1;
#undef MAGIC

	ende=tip_getpart(start,"}");
	if (!ende)
		return TIP_ERROR;

	vgllen=ende-start;

	kopie=malloc(ende-start+1);
	if (!kopie)
		return TIP_ERROR;
	strncpy(kopie,start,ende-start);
	kopie[ende-start]=0;

	ret=tip_defmacro_intern(kopie, NULL);
	free(kopie);
	if (ret)
		return ret;
	*next=ende+1;
	return 0;
}

