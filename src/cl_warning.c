/*
   cl_error.c - errormessages and warnings
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
#include <stdarg.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "conflib.h"
#include "intern.h"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#ifdef HAVE_VPRINTF
#define CAN_HOOK
#endif

#if HAVE_VPRINTF || HAVE_DOPRNT
# if __STDC__
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
# else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
# endif
#else
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
#endif


#if STDC_HEADERS || _LIBC
# include <stdlib.h>
# include <string.h>
#else
void exit ();
#endif

#if HAVE_STRERROR
#  ifndef strerror       /* On some systems, strerror is a macro */
char *strerror ();
#  endif
#else
extern char *sys_errlist[];
extern int sys_nerr;
static const char *
private_strerror (int errnum)
{

	if (errnum > 0 && errnum <= sys_nerr)
		return sys_errlist[errnum];
	return _("Unknown system error");
}
#  define strerror private_strerror
#endif  /* HAVE_STRERROR */


void (*cl_warning_hook)
    (int, cl_file_t *, cl_var_t *, const char *sourcefile, long sourceline,
	const char *message);
void (*cl_error_hook)
    (int, cl_file_t *, cl_var_t *, const char *sourcefile, long sourceline,
    const char *errormessage);

char *cl_program_name=NULL;

void 
#if defined(VA_START) && __STDC__
cl_warning(int error, int errnum, cl_file_t *file, cl_var_t *var, const char *source, 
	long sline, const char *format, ...)
#else
cl_warning(error, errnum,file, var, source, sline, format, va_alist)
	int error; 
	int errnum; 
	cl_file_t *file; 
	cl_var_t *var; 
	const char *source; 
	long sline; 
	const char *format; 
	va_dcl)
#endif
{
#ifdef CAN_HOOK
	void (*hook) (int, cl_file_t *, cl_var_t *, const char *, long , const char *);
#endif
#ifdef VA_START
	va_list ap;
#endif

#ifdef CAN_HOOK
	if (error)
		hook=cl_error_hook;
	else
		hook=cl_warning_hook;

	if (hook)
	{
		char *buf;
		buf=malloc(4096);
		if (!buf)
		{
			(hook)(0,file,var,source,sline,"out of memory");
			return;
		}
		VA_START (ap, format);
		vsprintf (buf, format, ap);
		va_end (ap);

		(hook)(errnum,file,var,source,sline,buf);
		free(buf);
		return;
	}
#endif
	fflush (stdout);
	if (cl_program_name)
		fprintf(stderr,"%s: ",cl_program_name);
	else
		fprintf (stderr, "conflib: ");


	fprintf(stderr,"%s: ", (error ? "error" : "warning"));
#ifdef VA_START
	VA_START (ap, format);
# if HAVE_VPRINTF || _LIBC
	vfprintf (stderr, format, ap);
# else
	_doprnt (format, ap, stderr);
# endif
	va_end (ap);
#else
	fprintf (stderr, format, a1, a2, a3, a4, a5, a6, a7, a8);
#endif

	if (errnum)
	{
		fprintf(stderr,": %s\n",strerror(errnum));
	}
	else
		fputs("\n",stderr);
	if (file)
	{
		if (file->line_nr>=0)
			fprintf(stderr,"  file %s, line %ld.\n",
				file->fname,file->line_nr);
		else
			fprintf(stderr,"  file %s (no line number)\n",
				file->fname);
	}
	if (var)
		fprintf(stderr,"  variable %s\n", var->varname);
	if (error)
		fprintf(stderr,"  at %s, line %ld.\n",
			source,sline);
}

