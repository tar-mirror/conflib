/*
    intern.h
    Copyright (C) 1994 Uwe Ohse

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
		Uwe_Ohse@me.maus.de
		uwe@tirka.gun.de
	snail-mail:
	    Uwe Ohse, Drosselstrasse 2, 47055 Duisburg, Germany

$Id: intern.h,v 1.1.1.1 1998/07/09 21:57:52 uwe Exp $
$Date: 1998/07/09 21:57:52 $
$Name:  $
$Log: intern.h,v $
Revision 1.1.1.1  1998/07/09 21:57:52  uwe
import

Revision 1.1.1.3  1997/11/02 17:11:56  uwe
import nachträglich

Revision 1.1.1.1  1996/06/15 08:55:27  uwe
Imported sources

Revision 1.1.1.1  1996/02/18 22:45:23  uwe
Import Sourcen

Revision 1.2  1995/05/07 20:30:47  quark
$Id: intern.h,v 1.1.1.1 1998/07/09 21:57:52 uwe Exp $
$Date: 1998/07/09 21:57:52 $
$Name:  $
$Log: intern.h,v $
Revision 1.1.1.1  1998/07/09 21:57:52  uwe
import

Revision 1.1.1.3  1997/11/02 17:11:56  uwe
import nachträglich

Revision 1.1.1.1  1996/06/15 08:55:27  uwe
Imported sources

Revision 1.1.1.1  1996/02/18 22:45:23  uwe
Import Sourcen

eingesetzt.
cl_assign: zusätzlicher Parameter "space_given"


*/

#ifndef INTERN_H
#define INTERN_H

	/* cllib.c */
int cl_varnamecmp (const char *p1,const char *p2);
char *cl_strtrim (char *s);
int cl_assign (cl_file_t *, cl_var_t *conf_array, 
	const char *varname, const char *value, cl_ulist_t **unknowns);
int cl_dup_to (cl_var_t *var, const char *string);
int cl_convert2bool (const char *Ptr);
cl_var_t *cl_unalias (cl_file_t *,cl_var_t *tab, cl_var_t *entry);
void cl_invalidate_stanzalist (cl_file_t *handle);
void cl_put_one (FILE *file, const cl_var_t *v);
int cl_zeile_auswerten (cl_file_t * conf, cl_var_t * tab, char *zeile,
                 long flags, cl_ulist_t ** unknowns);


void cl_warning (int error, int errnum, cl_file_t *, cl_var_t *, 
	const char *, long, const char *, ...);
#define CL_WARN(c,v) 0,0,c,v,__FILE__,__LINE__
#define CL_ERR(c,v) 1,0,c,v,__FILE__,__LINE__
#define CL_WARN3(e,c,v) 0,e,c,v,__FILE__,__LINE__
#define CL_ERR3(e,c,v) 1,e,c,v,__FILE__,__LINE__

#if defined(LEAK_CHECK) && !defined(LEAK_CHECK_MACROS_DEFINED)
#define LEAK_CHECK_MACROS_DEFINED
#define GC_DEBUG
#define FIND_LEAK
#include "gc.h"
#define malloc(x) GC_MALLOC(x)
#define realloc(x,y) GC_REALLOC(x,y)
#define calloc(m,n) GC_MALLOC((m)*(n))
#define free(x) GC_FREE(x)
#define strdup(x) libquark_xstrdup(x,__FILE__,__LINE__)
#endif

#ifndef _
# define _(String) String
#endif

#endif /* INTERN_H */
