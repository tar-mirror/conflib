/*
   cl_in_array.c - check whether an array contains a string
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

/* CL_FUNCTION(Function,int,cl_isinarray,
[[(const char **Array, const char *St, int insensitive)]],[[
cl_isinarray searches the elements of array @var{Array} for
a string equal to @var{st}. If @var{insensitive} is not zero
then the string comparision is caseinsensitive.

The function returns 0 if @var{st} found and one otherwise.
]]) */
int
cl_isinarray (const char **Array, const char *St, int insensitive)
{
	if (!Array)
		return 0;
	while (*Array)
	{
		if (insensitive!=0)
		{
			if (0==strcasecmp (St, *Array))
				return 0;
		}
		else
		{
			if (0==strcmp (St, *Array))
				return 0;
		}
		Array++;
	}
	return 1;
} /* cl_is_in_array */
