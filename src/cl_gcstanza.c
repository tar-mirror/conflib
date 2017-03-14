/*
 * cl_cgstanza.c - open config file, get stanza, close file.
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
CL_FUNCTION(Function,int,cl_getconfstanza,
[[(const char *@var{fname}, cl_var_t *@var{conftab}, const char *stanza)]],[[
cl_getconfstanza reads the stanza @var{stanza} from the configuration
file @var{fname} into @var{conftab}. If @var{stanza} is @code{NULL} then 
the whole configuration file is read.

Returns 0 if successful and other values if not.
]]) */


int
cl_getconfstanza (const char *fname, cl_var_t * tab, const char *stanza)
{
	cl_file_t *handle;
	int ret;

	handle = cl_openconf (fname, "r");

	if (!handle)
		return 1;

	((cl_file_t *) handle)->dont_scan = 1;

	ret = cl_getstanza (handle, tab, stanza, 0, NULL);

	cl_closeconf (handle);

	return ret;
}								/* cl_getconfstanza */

