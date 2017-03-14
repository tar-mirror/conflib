/*
   cl_unalias.c - dereference an alias variable
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

/* CL_FUNCTION(Function,cl_var_t *,cl_unalias,
[[(cl_file_f *@var{file}, cl_var_t *@var{tab}, cl_var_t *@var{entry})]],[[
This function follows an alias chain until it finds an entry in @var{tab}
which is not an alias, and returns it's address.

@var{FILE} is needed for error messages.

@var{ENTRY} is the initial variable. If it is not an alias this function
returns @var{ENTRY}.

cl_unalias returns @code{NULL} in case of any error.
]]) */

cl_var_t *
cl_unalias(cl_file_t *f, cl_var_t *tab, cl_var_t *entry)
{
	int derefed=0;
	int count=0;
	cl_var_t *orig=entry;
	const char *name;
	while (entry->typ == CL_ALIAS)
	{
		cl_var_t *akt;
		akt=tab;
		name=(const char *) entry->secdata;
		if (++count==10)
			break;
		if (!name)
		{
			cl_warning(CL_ERR(f,entry),
				"alias points to NULL (program error)");
			return NULL;
		}
		while (akt->varname)
		{
			char c1=tolower(*name);
			char c2=tolower(*akt->varname);
			if (c1=='-')
				c1='_';
			if (c2=='-')
				c2='_';
			if (c1==c2 && 0==cl_varnamecmp(name+1,akt->varname + 1))
				break;
			akt++;
		}
		if (!akt->varname)
		{
			cl_warning(CL_ERR(f,entry),
				"alias points to %s, not dereferenced (program error)",
				name);
			return NULL;
		}
		derefed++;
		entry=akt;
	}
	if (derefed!=0)
	{
		if (orig->flags & CL_WARNING)
			cl_warning(CL_WARN(f,orig),"Variable is alias, use %s instead",
				entry->varname);
		return entry;
	}
	else if (count==10)
	{
		cl_warning(CL_ERR(f,orig),"alias loop suspected");
		return NULL;
	}
	else
	{
		cl_warning(CL_ERR(f,orig),"alias points to unknown variable %s",
			orig->secdata);
		return NULL;
	}
}
	
