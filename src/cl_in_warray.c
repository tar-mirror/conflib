/*
   cl_in_warray.c - does the array contain a string matching a wildcard?
   Copyright (C) 1993, 1994, 1995 Uwe Ohse,  1991, 1992 Wolfram Roesler

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

/* CL_FUNCTION(Function,int,cl_isinwarray,
[[(const char **Array, const char *St)]],[[
cl_isinarray searches the elements of array @var{Array} for
a string matching @var{st}. This function uses wildcard 
comparision (with @var{st} being the pattern) and 
works casesensitive).

The function returns 0 if @var{st} found and one otherwise.
]]) */


int
cl_isinwarray (const char **Array, const char *St)
{
	while (*Array)
	{
		if (0==cl_fnmatch (St, *Array))
			return 0;
		Array++;
	}
	return 1;
} /* cl_is_in_warray */

