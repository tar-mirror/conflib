/*
    cl_writeconf.c - save configuration table
    Copyright (C) 1993,1994 Uwe Ohse, 1991, 1992 Wolfram Roesler

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

#include <stdio.h>

#include "conflib.h"
#include "intern.h"

/* CL_FUNCTION(Function,void,cl_writeconf,
[[(FILE *@var{file},const cl_var_t *@var{tab})]],[[
This function may be used to dump a programs configuration
to file @var{file}. It should not be used to overwrite
a configuration file.
]]) */

void cl_writeconf(FILE *file,const cl_var_t *var)
{
	if (!file || !var)
		return;
	
	while(var->varname)
	{
		cl_put_one(file,var);

		var++;
	}
	return;
}


