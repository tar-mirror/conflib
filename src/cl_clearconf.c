/*
   cl_clearconf.c - clean up a configuration table
   Copyright (C) 1994, 1995 Uwe Ohse

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

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "conflib.h"
#include "intern.h"

/* CL_FUNCTION(Function,void,cl_clearconf,
[[cl_var_t *@var{tab}]],[[
cl_clearconf walks through @var{tab} and 

@itemize @bullet
@item sets all numeric variables to 0.
@item sets boolean variables to FALSE.
@item frees all array contents.
@item frees all dynamically allocted memory (the values of
all variables and all lists which have the @code{CL_MALLOCED} flag set).
@end itemize
]]) */

void
cl_clearconf (cl_var_t * tab)
{
	cl_var_t *Vp;

	for (Vp = tab; Vp->varname; Vp++)
	{
		switch (Vp->typ)
		{
		case CL_ENUM:
			*(long *) Vp->adr = (long) 0;
			break;
		case CL_BITFIELD:
			*(unsigned long *) Vp->adr = (unsigned long) 0;
			break;
		case CL_STRING:		/* Stringvariable */
		case CL_DIRNAME:
			if (*(char **) Vp->adr && Vp->flags & CL_MALLOCED)
			{
				Vp->flags -= CL_MALLOCED;
				free (*(char **) Vp->adr);
				*(char **) Vp->adr = NULL;
			}
			break;
		case CL_CHAR:
			*(char *) Vp->adr = '\0';
			break;
		case CL_NUM:
			*(long *) Vp->adr = (long) 0;
			break;
		case CL_NUMSHORT:
			*(short *) Vp->adr = (short) 0;
			break;
		case CL_BOOLEAN:
			*(int *) Vp->adr = (int) 0;
			break;
		case CL_ARRAY:			/* Stringarray */
			{
				char **atab;
				int i;

				i = 0;
				atab = (char **) Vp->adr;
				while (*atab && i < CL_MAXARRAY)
				{
					free (*atab);
					*atab = NULL;
					atab++;
					i++;
				}
			}
			break;
		case CL_ALIAS:
		case CL_DUMMY:
			break;
		case CL_LIST:
			{
				cl_list_t *akt;
				cl_list_t *next;

				if (!Vp->flags & CL_MALLOCED)
					break;
				akt = *(cl_list_t **) Vp->adr;
				while (akt)
				{
					if (akt->inhalt)
						free (akt->inhalt);
					next = akt->next;
					free (akt);
					akt = next;
				}
				*(cl_list_t **) Vp->adr = NULL;
				Vp->flags -= CL_MALLOCED;
			}
			break;
		}						/* switch */
	}							/* for */
} /* cl_clearconf */

