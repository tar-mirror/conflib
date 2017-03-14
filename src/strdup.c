/* strdup.c : strdup replacement for conflib */
/*
    Copyright (C) 1994 1995 1996 Frank Baumgart, Uwe Ohse

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

    Contact: uwe@tirka.gun.de, Uwe Ohse @ DU3 (mausnet)
       Snail Mail (don't expect me to answer):
             Uwe Ohse
             Drosselstraﬂe 2
             47055 Duisburg
             Germany
*/
#include "ownstd.h"

#include <stdlib.h>

#ifndef HAVE_STRDUP
char *
cl_strdup(char *s)
{
	size_t l;
	char *p;
	l=strlen(s);
	p=malloc(l+1);
	if (!p)
		return NULL;
	strcpy(p,s);
	return p;
}
#endif
