/*
    ownstd.h
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
		uwe@tirka.gun.de
		Uwe_Ohse@mausdu3.gun.de 
		Uwe_Ohse@du3.maus.sub.org
	snail-mail:
	    Uwe Ohse, Drosselstrasse 2, 47055 Duisburg, Germany
*/

#ifndef OWNSTD_H
#define OWNSTD_H

#include "config.h"

#if STDC_HEADERS
#  include <string.h>
#else /* not STDC_HEADERS and not HAVE_STRING_H */
#ifdef HAVE_STRING_H
#  include <string.h>
#else
   /* we need size_t */
#  include <stddef.h>
#endif
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#ifndef __P
# if PROTOTYPES
#  define __P(Args) Args
# else
#  define __P(Args) ()
# endif
#endif

#ifndef HAVE_STRCASECMP
#  ifdef HAVE_STRICMP
#    define strcasecmp stricmp
#  else
     extern int cl_strcasecmp __P((const char *s1, const char *s2));
#    define strcasecmp cl_strcasecmp
#  endif
#endif

#ifndef HAVE_STRNCASECMP
#  ifdef HAVE_STRNICMP
#    define strnicmp strncasecmp
#  else
     extern int strncasecmp __P(const char *s1, const char *s2,size_t n));
#    define strncasecmp cl_strncasecmp
#  endif
#endif

#ifndef HAVE_STRDUP
#define strdup cl_strdup
extern char *cl_strdup __P(const char *));
#endif
#ifndef HAVE_STRTOL
#define strtol cl_strtol
extern char *cl_strtol __P(const char *, char **endptr, int base));
#endif

#ifdef HAVE_FNMATCH
#  include <fnmatch.h>
#  ifdef FNM_NOMATCH
#    define CL_FNM_NOMATCH FNM_NOMATCH
#  else
#    define CL_FNM_NOMATCH 1
#  endif
#  define cl_fnmatch(x,y) fnmatch(x,y,0)
#  ifndef FNM_CASEFOLD
#    define NEED_FNMATCH_CASEFOLD
#  else
#    define cl_fnmatch_casefold(x,y) fnmatch(x,y,FNM_CASEFOLD)
#  endif
#else
#  define CL_FNM_NOMATCH 1
#  define NEED_FNMATCH_CASEFOLD
extern int cl_fnmatch (const char *pattern, const char *string);
#endif
#ifdef NEED_FNMATCH_CASEFOLD
extern int cl_fnmatch_casefold (const char *pattern, const char *string);
#endif

#endif /* OWNSTD_H */
