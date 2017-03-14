/*
   cl_alist.c - build an array of all stanzas
   Copyright (C) 1995 Uwe Ohse

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
   uo@du3.maus.de
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

/*
CL_FUNCTION(Function,[[char **]],cl_build_stanza_array,
[[(const char *@var{pattern}, cl_file_t *var{file})]],[[
Generates an array containing the names of all stanzas
matching @var{pattern} in the configuration file 
@var{file}. The last element is a @code{NULL} pointer.

@var{pattern} must not be a @code{NULL} pointer. Use @code{*}
instead.

This function returns the array or @code{NULL} if an
error occured.

If no stanza name matches @var{pattern} the function returns 
an empty array with the first element beeing the @code{NULL}
pointer.
]]) */


char **
cl_build_stanza_array(const char *pattern, cl_file_t *conf)
{
	cl_stanza_list_t *st=conf->stanzas;
	int count;
	char **array;

	if (!st)
		return NULL;
	for (count=0,st=conf->stanzas;st!=NULL;st=st->next)
	{
		if (0==cl_fnmatch_casefold(pattern, st->stanzaname))
		{
			count++;
		}
	}
	array=malloc(sizeof(char *) * (count +1));
	if (!array)
	{
		cl_warning(CL_ERR(conf,NULL),"out of memory");
		return NULL;
	}

	for (count=0,st=conf->stanzas;st!=NULL;st=st->next)
	{
		if (0==cl_fnmatch_casefold(pattern, st->stanzaname))
		{
			array[count]=strdup(st->stanzaname);
			if (!array[count])
			{
				cl_delete_stanza_array(array);
				cl_warning(CL_ERR(conf,NULL),"out of memory");
				return NULL;
			}
			count++;
		}
	}
	array[count]=NULL;
	return array;
	
}

/*
CL_FUNCTION(Function,void,cl_delete_stanza_array,
[[(char **@var{array})]],[[
Frees all memory used by @var{array} and its elements.
Sets *@var{array} to @code{NULL}.
]]) */

void 
cl_delete_stanza_array(char **array)
{
	int count=0;
	if (!array)
		return;
	while (array[count]!=NULL)
	{
		free(array[count]);
		count++;
	}
	free(array);
	*array=NULL;
}

/* CL_INCLUDE([[
An example, with error handling left out:

@smallexample
cl_file_t *f;
char **ar;
f=cl_openconf("something.cnf","r");
ar=cl_build_stanza_array("*",f);
if (*ar) @{
    char **p;
	for (p=ar;*p;p++)
	    puts(*p);
@}
cl_closeconf(f);
cl_delete_stanza_array(ar);
@end smallexample
]])
*/
