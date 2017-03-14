/*
 * closeconf.c - close a configuration file
 * Copyright (C) 1993,1994,1995 Uwe Ohse
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
CL_FUNCTION(Function,void,cl_openconf,
[[(cl_file_t *@var{hadle})]],[[
Closes the configurations file with the handle @var{handle}
and frees all resources of @var{handle} (this means that 
you really should not access @var{handle} and its data 
any longer).
]]) */

void
cl_closeconf (cl_file_t * handle)
{
	if (handle == NULL)
		return;
	cl_invalidate_stanzalist(handle);
	if (handle->fhandle)
	{
		fclose (handle->fhandle);
		handle->fhandle = NULL;
	}
#define MEMFREE(x) if (x) { free(x); x=NULL; }
	MEMFREE (handle->buf);
	MEMFREE (handle->fname);
	MEMFREE (handle->stanza);
#undef MEMFREE
	free(handle);
}

