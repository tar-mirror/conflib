/*
   cl_getvers.c - contains a function to inquire the version number.
   Copyright (C) 1995

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

/* CL_FUNCTION(Function,int,cl_getversion,
[[(void)]],[[
cl_getversion returns an integer representing the version number of
the library. It's @math{10000 * major + 100 * minor + patchlevel}.
]]) */


const char *
cl_getversionstr (void) 
{
	return VERSION;
}

int
cl_getversion (void)
{
	const char *p;
	int i;
	i=strtol(VERSION,NULL,10) * 10000;
	p=strchr(VERSION,'.');
	if (!p)
		return i;
	p++;
	i+=strtol(p,NULL,10) * 100;
	p=strchr(p,'.');
	if (!p)
		return i;
	p++;
	i+=strtol(p,NULL,10);
	return i;
} /* cl_getversion */
