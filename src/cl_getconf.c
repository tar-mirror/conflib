/*
 * getconf.c - get configuration table from file
 * Copyright (C) 1993,1994,1995 Uwe Ohse,  1991, 1992 Wolfram Roesler
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * 
 * For questions contact:
 * e-mail: (prefered)
 * uwe@tirka.gun.de
 * uo@du3.maus.de
 * snail-mail:
 * Uwe Ohse, Drosselstrasse 2, 47055 Duisburg, Germany
 */

#include "ownstd.h"

#include <stdlib.h>
#include <stdio.h>

#include "conflib.h"
#include "intern.h"

/*
CL_FUNCTION(Function,int,cl_getconf,
[[(const char *@var{fname}, cl_var_t *@var{conftab})]],[[
Reads the whole configuration file @var{fname} into
@var{conftab}. 

This function is identical with calling 
@code{cl_getconfstanza(@var{fname},@var{conftab},0)}.

Returns 0 if successful, and other values if not.
]]) */

int 
cl_getconf (const char *FName, cl_var_t * KonfTab)
{
	return cl_getconfstanza (FName, KonfTab, (char *) 0);
}

