/*
 * cl_openconf.c - open a configuration file
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


#if defined(F_WRLCK) && defined(F_RDLCK) && defined(F_SETLK)
#ifndef HAVE_BROKEN_F_SETLK
#define DO_LOCK
#endif
#endif

#ifdef DO_LOCK
static int
cl_lock_file (int fd, int for_write)
{
	struct flock lock;

	if (for_write != 0)
		lock.l_type = F_WRLCK;
	else
		lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 1;
	lock.l_whence = SEEK_SET;
	return fcntl (fd, F_SETLK, &lock);
}
#endif

/*
CL_FUNCTION(Function,cl_file_t,cl_openconf,
[[(const char *@var{fname}, const char *@var{mode})]],[[
Open the configuration file @var{fname} for reading 
(@var{mode} == @code{r}) or writing (@var{mode} == @code{r}).
The normal return value is the handle needed to access 
@var{fname} with other @code{conflib}-functions.

n the case of an error, a value of @code{NULL} is returned
instead. In addition to the usual file open errors 
(@pxref{Opening and Closing Files,opening a file descriptor,
Opening and Closing Files,libc,The GNU C Library Reference Manual}), 
the following @code{errno} error conditions are defined
for this function:

@table @code
@item ENOMEM
no more memory available.
@item EINVAL
@var{mode} is not equal to @code{r} or @code{w}.
@end table

]]) */

cl_file_t *
cl_openconf (const char *fname, const char *mode)
{
	cl_file_t *c;
	if (*mode != 'r' && *mode != 'w') {
		errno=EINVAL;
		return NULL;
	}
	c = malloc (sizeof (cl_file_t));
	if (!c) {
		errno=ENOMEM;
		return NULL;
	}
	c->mode = *mode;
	c->fhandle = NULL;
	c->buf = NULL;
	c->buflen = 0;
	c->in_comment = 0;
	c->in_stanza = 0;
	c->did_backup = 0;
	c->report_unknown = 0;
	c->fname = NULL;
	c->stanza = NULL;
	c->line_nr = 0;
	c->stanzas = NULL;
	c->last_ele = NULL;
	c->in_scanmode = 0;
	c->dont_scan = 0;
	c->interptab = NULL;

	if (*mode == 'w')
		c->fhandle = fopen (fname, "r+");
	else
		c->fhandle = fopen (fname, "r");

	if (!c->fhandle)
	{
		int e=errno;
		free(c);
		errno=e;
		return NULL;
	}

#ifdef DO_LOCK
	if (0 != cl_lock_file (fileno (c->fhandle), *mode == 'w'))
	{
		int e=errno;
		fclose(c->fhandle);
		free(c);
		errno=e;
		return NULL;
	}
#endif

	c->fname = strdup (fname);
	if (!c->fname)
	{
		fclose(c->fhandle);
		free(c);
		errno=ENOMEM;
		return NULL;
	}

	return c;
}

