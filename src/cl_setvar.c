/*
 * cl_setvar.c - set variable from string.
 * Copyright (C) 1996 Uwe Ohse
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

#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "conflib.h"
#include "intern.h"
#include "tinterp.h"

/*
CL_FUNCTION(Function,int,cl_setvar,
[[(cl_var_t *@var{tab}, char *@var{zeile}, long @var{flags}, 
cl_ulist_t ** @var{unknowns})]],[[
cl_setvar works as if @var{zeile} was a line in a configuration
file. It looks in @var{tab} for the variable name.

@var{flags} is a bitfield consisting of the following bits:
@table @bullet
@item CL_GET_NOINCLUDE
If this bit is set @code{include}- and @code{includestanza}-statements
in @var{handle} will be ignored.
@end table

If @var{unknowns} is not @code{NULL} then the pointer it points to
will be the start of a list of unknown variables (basically this
are variables set in the stanza which are not found in @var{tab}).
The list is dynamically allocated, use @code{cl_free_unknowns} to
deallocate the memory. @xref{cleaning up, cl_free_unknowns}.

This function returns 0 if successful and some other value if not.
]]) */

int 
cl_setvar(cl_var_t *tab, char *zeile, long flags, cl_ulist_t **unknowns)
{
	char *buf;
	int ret;
	cl_file_t dummy= {
		NULL, /* FILE */
		NULL, /* see below */
		0, /* see below */
		0, /* in_comment */
		1, /* in_stanza */
		0, /* report_unknown */
		"direct", /* fname */
		"none",   /* stanza name */
		1, /* line number */
		NULL, /* stanzas */
		NULL, /* last_ele */
		0, /* scanmode */
		1, /* dont scan */
		0, /* did backup */
		'r' /* read/write mode */
	};
	dummy.buflen=strlen(zeile)+1024;
	buf=malloc(dummy.buflen);
	if (!buf) {
		errno=ENOMEM;
		return -1;
	}
	dummy.buf=buf;
	strcpy(buf,zeile);
	ret=cl_zeile_auswerten(&dummy,tab,zeile, flags, unknowns);
	free(buf);
	return ret;
}

