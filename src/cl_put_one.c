/*
    cl_put_one.c - printf one configuration variable
    Copyright (C) 1993,1994,1995 Uwe Ohse

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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "conflib.h"
#include "intern.h"

#define NULLSTR(str) (str ? str : "(null)")

void
cl_put_one(FILE *file, const cl_var_t *v)
{
	switch(v->typ)
	{
		case CL_STRING:
		case CL_DIRNAME:
			fprintf(file,"%s=%s\n",v->varname,NULLSTR(*(char**)v->adr));
			break;
		case CL_CHAR:
			if (*(char *)v->adr) {
				fprintf(file,"%s=%c\n",v->varname,*(char *)v->adr);
			} else {
				fprintf(file,"%s=\n",v->varname);
			}
			break;
		case CL_NUM:
			fprintf(file,"%s=%ld\n",v->varname,*(long*)v->adr);
			break;
		case CL_NUMSHORT:
			fprintf(file,"%s=%d\n",v->varname,*(short*)v->adr);
			break;
		case CL_BOOLEAN:
			fprintf(file,"%s=%s\n",v->varname,*(int*)v->adr? "true":"false");
			break;
		case CL_ENUM:
			{
				const cl_enum_t *p;
				long numvalue;
				const char *strvalue;
				numvalue=*(long *)v->adr;
				strvalue=NULL;

				for(p = (const cl_enum_t *)v->secdata;p->magic;p++)
				{
					if (p->value==numvalue)
					{
						strvalue=p->magic;
						break;
					}
				}
				if (!strvalue)
					strvalue=((const cl_enum_t *)v->secdata)->magic;
				fprintf(file,"%s=%s\n",v->varname,NULLSTR(strvalue));
			}
			break;
		case CL_BITFIELD:
			{
				const cl_enum_t *p;
				unsigned long numvalue;
				int cnt=0;
				numvalue=*(unsigned long *)v->adr;

				fprintf(file,"%s=",v->varname);
				for(p = (const cl_enum_t *)v->secdata;p->magic;p++)
				{
					if (p->value & numvalue) {
						if (cnt++)
							fputs("|",file);
						fprintf(file,"%s",p->magic);
						numvalue-=p->value;
					}
				}
				fputs("\n",file);
			}
			break;
		case CL_ARRAY:	/* Stringarray */
			{
				char **p;
				for(p = (char**)v->adr;*p;p++)
					fprintf(file,"%s=%s\n",v->varname,NULLSTR(*p));
				break;
			}
		case CL_DUMMY:
		case CL_ALIAS:
			break;
		case CL_LIST:
			{
				cl_list_t *akt;
				const char *sep;
				akt = *(cl_list_t **) v->adr;
				sep = (const char *) v->secdata;
				if (!sep)
					sep=":";
				fprintf(file,"%s=%s",v->varname,
					akt ? "" : "\n");
				while(akt)
				{
					fprintf(file,"%s%c",NULLSTR(akt->inhalt),
						(akt->next) ? *sep : '\n');
					akt=akt->next;
				}
			}
			break;
	}
}

