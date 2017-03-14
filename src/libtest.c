/*
    libtest.c - main() to test conflib
    Copyright (C) 1993,1994,1995  Uwe Ohse, 1991, 1993 Wolfram Roesler

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

*/

#include "ownstd.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "conflib.h"
#include "tinterp.h"

short numshort;
short twodigits;
long num;
long aliasziel;
int  bool_true;
int  bool_false;
int value_1;
int value_5;
unsigned long decision;
char *string;
char *tmpdir;
char *function_result;
time_t statement_result=(time_t) -1;
char *array[CL_MAXARRAY]={NULL};
cl_list_t *sliste;
cl_list_t *mliste;
char character;

cl_enum_t zahlen[]=
{ 
{	"Null",   0 }, {	"Zero",   0 },
{	"Eins",   1 }, {	"One",    1 },
{	"Zwei",   2 }, {	"Two",    2 },
{	"Drei",   3 }, {    "Three",  3 },
{	"Vier",   4 }, {	"Four",   4 },
{	"Fuenf",  5 }, {	"Five",   5 },
{	"Sechs",  6 }, {	"Six",    6 },
{	"Sieben", 7 }, {	"Seven",  7 },
{	"Acht",   8 }, {	"Eight",  8 },
{	"Neun",   9 }, {	"Nine",   9 },
{	NULL,     0 }
};

cl_enum_t bitfield[]=
{ 
{	"ja",   0 },
{	"nein",   1 },
{	"vielleicht",   2 },
{	NULL,     0 }
};

int value1;
int value5;
cl_var_t Tab[] =
{
	{ "numshort",  NULL,       CL_NUMSHORT,&numshort   ,0          ,NULL },
	{ "twodigits", NULL,       CL_NUMSHORT,&twodigits  ,CL_RANGE_WILDCARD,
		"[0-9][0-9]" },
	{ "num",	   NULL,       CL_NUM,     &num        ,0          ,NULL },
	{ "char",	   NULL,       CL_CHAR,    &character  ,0          ,NULL },
	{ "bool_false",NULL,       CL_BOOLEAN, &bool_false ,0          ,NULL },
	{ "bool_true", NULL,       CL_BOOLEAN, &bool_true  ,0          ,NULL },
	{ "decision",  bitfield,   CL_BITFIELD,&decision   ,0          ,NULL },
	{ "value-1",   zahlen,     CL_ENUM,    &value1     ,0          ,NULL },
	{ "value-5",   zahlen,     CL_ENUM,    &value5     ,0          ,NULL },
	{ "string",    "string",   CL_STRING,  &string     ,CL_POSTENV ,NULL },
	{ "dummy",     "dummy",    CL_DUMMY,   NULL        ,CL_POSTENV ,NULL },
	{ "tmpdir",    "TMP",      CL_DIRNAME, &tmpdir     ,CL_PREENV  ,NULL },
	{ "aliasziel", NULL,       CL_NUM,     &aliasziel  ,0          ,NULL },
	{ "alias",     "aliasziel",CL_ALIAS,   NULL        ,0          ,NULL },
	{ "array",     "array",    CL_ARRAY,   array       ,CL_POSTENV ,NULL },
	{ "space-list"," ",        CL_LIST,    &sliste     ,0          ,NULL },
	{ "multi-list",": ,\t",    CL_LIST,    &mliste     ,0          ,NULL },
	{ NULL,NULL,0,NULL,0 }
};

static void interactive(void);

static tip_t mytiptab[]={
	{ '\0', "${literal",   0, tip_literal },
	{ '\\', NULL,        0, tip_backslash },
	{ '~',  NULL,        0, tip_tilde },
	{ '\0', "$[",        0, tip_conditional },
	{ '\0', "${strip",   0, tip_strip },
	{ '\0', "${force",   0, tip_force },
	{ '\0', "${macro",   0, tip_macro },
	{ '\0', "${defmacro",   0, tip_defmacro },
	{ '\0', "${undefmacro",   0, tip_undefmacro },
	{ '$',  NULL,        0, tip_dollar },
	{ '\0', NULL,        0, NULL }
};

static void interactive(void)
{
	char *erg;
	char rbuf[8192]="";
	char *rpos;
	int in_cont=0;

	rpos=rbuf;
	while (fgets(rpos,sizeof(rbuf)-(rpos-rbuf),stdin))
	{
		char *start;
		char *p;
		if (in_cont)
		{
			p=rpos;
			while (isspace(*p))
				p++;
			memmove(rpos,p,strlen(p)+1);
			in_cont=0;
		}
		p=strpbrk(rpos,"\r\n");
		if (p==rbuf || *rbuf==0)
			break;
		if (p)
		{
			*p=0;
			/* Backslash am Zeilenende? */
			if (p>rpos && p[-1]=='\\')
			{
				/* Zwei Backslashs am Ende entwerten die Entwertung */
				if (p-1<=rpos || p[-2]!='\\')
				{
					rpos=p-1;
					*rpos=0;
					/* Zeile wird fortgesetzt */
					in_cont=1;
					continue;
				}
				p--;
				*p=0;
			}
		}
		/*
		 * Was hier ankommt kann interpretiert und ausgegeben werden.
		 */
		rpos=rbuf;
		erg=tip_ainterpret(mytiptab,rbuf);
		if (!erg)
		{
			printf("tip-error: %s\n",tip_errtext);
			continue;
		}

		start=erg;
		while (start && *start)
		{
			char *next;
			p=strpbrk(start,"\r\n");
			if (p)
			{
				if (*p=='\n')
					next=p+1;
				else
					next=p+2;
				*p=0;
			}
			else
				next=NULL;
			puts(start);
			start=next;
		}
		free(erg);
	}
}

static void rewrite(void)
{
	int erg;
	cl_file_t *fi;
	cl_ulist_t *unknowns=NULL;

	fi=cl_openconf("rewrite.cnf","w");
	if (!fi)
	{
		fputs("cannot open rewrite.cnf\n",stderr);
		exit(1);
	}
	
	erg=cl_getstanza(fi,Tab,"rewrite1",CL_GET_NOINCLUDE,&unknowns);
	if (erg!=0)
	{
		fputs("cannot read stanza rewrite1\n",stderr);
		exit(1);
	}
	num++;

	erg=cl_putstanza(fi, Tab, "rewrite1",0,unknowns);
	if (erg!=0)
	{
		fputs("cannot write stanza rewrite1\n",stderr);
		exit(1);
	}

	erg=cl_getstanza(fi,Tab,"rewrite2",CL_GET_NOINCLUDE,&unknowns);
	if (erg!=0)
	{
		fputs("cannot read stanza rewrite2\n",stderr);
		exit(1);
	}

	character='X';
	Tab[2].flags|=CL_MAY_SAVE;

	erg=cl_putstanza(fi, Tab, "rewrite2",0,unknowns);
	if (erg!=0)
	{
		fputs("cannot write stanza rewrite2\n",stderr);
		exit(1);
	}

	erg=cl_getstanza(fi,Tab,"rewrite3",CL_GET_NOINCLUDE,&unknowns);
	if (erg!=0)
	{
		fputs("cannot read stanza rewrite3\n",stderr);
		exit(1);
	}

	character='Y';
	Tab[2].flags|=CL_MAY_SAVE;

	erg=cl_putstanza(fi, Tab, "rewrite3",0,unknowns);
	if (erg!=0)
	{
		fputs("cannot write stanza rewrite3\n",stderr);
		exit(1);
	}

	character='Z';
	Tab[2].flags|=CL_MAY_SAVE;

	erg=cl_putstanza(fi, Tab, "append",0,unknowns);
	if (erg!=0)
	{
		fputs("cannot write stanza append\n",stderr);
		exit(1);
	}

	cl_closeconf(fi);
	exit(0);

}

int main(int argc, char **argv)
{
	int i;
	int automode=0;
	int autocount=0;
	int interactive_flag=0;
	int rewrite_flag=0;
	const char *fname="test.cnf";
	char st[200];
	int Erg;

	if (argc==2)
	{
		if (!strcmp(argv[1],"--auto") || !strcmp(argv[1],"-a"))
		{
			automode=1;
			autocount=100;
		}
		else if (!strcmp(argv[1],"--rewrite")
			||	!strcmp(argv[1],"-r"))
			rewrite_flag=1;
		else if (!strcmp(argv[1],"--interactive")
			||	!strcmp(argv[1],"-i"))
			interactive_flag=1;
		else
			fname=argv[1];
	}
	else if (argc>2)
	{
		fprintf(stderr,"usage: %s [configfilename]\n",argv[0]);
		exit(1);
	}
	fprintf(stderr,"Version %d\n",cl_getversion());

	tip_defmacro_intern("testmakro1", "inhalt makro 1");
	tip_defmacro_intern("testmakro2", "inhalt makro 2 - killed");
	tip_defmacro_intern("testmakro2", NULL);
	tip_defmacro_intern("testmakro3", "testmakro 2 is \"$[${macro testmakro2}?defined:undefined]\"");

	if (interactive_flag)
	{
		interactive();
		exit(0);
	}
	if (rewrite_flag)
	{
		rewrite();
		exit(0);
	}

	for(;;)
	{
		cl_clearconf(Tab);
		if (automode) {
			Erg = cl_getconfstanza(fname,Tab, NULL);
		}
		else
		{
			char *p;
			printf("Stanza? (ENTER=Ende, 0=NULL=all) ");fflush(stdout);
			if (!fgets(st,sizeof(st)-1,stdin))
				break;
			p=strpbrk(st,"\r\n");
			if (p)
				*p=0;
			if (!*st || feof(stdin))
				break;
			if (0==strncmp(st,"cmd",3)) {
				Erg = cl_setvar(Tab,st+3,0,NULL);
				if (Erg) {
					printf("cl_setvar: %d\n",Erg);
					continue;
				}
			}
			else
				Erg = cl_getconfstanza(fname,Tab,strcmp(st,"0") ? st : NULL);
		}

		if (Erg!=0)
			printf("cl_getconfstanza: %d\n",Erg);
		else
	    	cl_writeconf(stdout,Tab);
	    for (i=0;i<CL_MAXARRAY;i++)
	    	array[i] = NULL;
	    if (automode && !--autocount)
	    	break;
	}
	if (automode) {
		Erg=cl_setvar(Tab,strdup("$PATH=/usr/local/bin:$PATH"),0,NULL);
		if (Erg!=0)
			printf("cl_setvar: %d\n",Erg);
		else {
			Erg=cl_setvar(Tab,strdup("string=$PATH"),0,NULL);
			if (Erg!=0)
				printf("cl_setvar: %d\n",Erg);
			else {
				cl_writeconf(stdout,Tab);
			}
		}
	}
	return 0;
}

