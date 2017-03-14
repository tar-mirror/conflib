/*
   cl_free_unknowns.c - free a list of unknown-type variables.
   Copyright (C) 1995 Uwe Ohse

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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "conflib.h"
#include "intern.h"

/* CL_FUNCTION(Function,void,cl_free_unknowns,[[cl_ulist_t *@var{list}]],[[
cl_free_unknowns frees all memory allocated to the list @var{list},
which should be a list of unknown variables created from 
@code{cl_getstanza}. @xref{reading and writing,cl_getstanza}.
]])*/

void
cl_free_unknowns(cl_ulist_t *list)
{
	while (list)
	{
		cl_ulist_t *t=list;
		list=list->next;
		free(t->varname);
		free(t->inhalt);
		free(t);
	}
}

