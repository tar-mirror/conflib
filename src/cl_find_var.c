/*
   cl_find_var.c - find a configuration variable in a table
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

/* CL_FUNCTION(Function,cl_var_t *,cl_find_confvar,
[[(cl_var_t *@var{tab}, const char *@var{name})]],[[
This functions searches the array @var{tab} for the variable called
@var{name} and returnes a pointer to the array element containing
@var{name}, or @code{NULL} if @var{name} is not found. 
]]) */
cl_var_t *
cl_find_confvar(cl_var_t *tab, const char *name)
{
	cl_var_t *otab=tab;
	char c1;
	c1=tolower(*name);
	if (c1=='_')
		c1='-';
	name++;
	for (; tab->varname; tab++)
	{
		char c2;
		c2=tolower(*tab->varname);
		if (c2=='_')
			c2='-';
		if (c1==c2 && 0==cl_varnamecmp(tab->varname+1,name)) {
			if (tab->typ==CL_ALIAS) {
				return cl_find_confvar(otab,tab->secdata);
			}
			return tab;
		}
	}
	return NULL;
}

