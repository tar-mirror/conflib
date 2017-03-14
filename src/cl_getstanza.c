/*
 * getconf.c - get configuration table from file
 * Copyright (C) 1993,1994,1995 Uwe Ohse,  1991, 1992 Wolfram Roesler
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
#include "cl_strtol.h"

static tip_t cl_tiptab[]={
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

/* The following is from pathmax.h.  */
/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
#if defined(_POSIX_VERSION) || (defined(HAVE_LIMITS_H) && !defined(__GNUC__))
# include <limits.h>
#endif

#ifndef _POSIX_PATH_MAX
# define _POSIX_PATH_MAX 255
#endif

#if !defined(PATH_MAX) && defined(_PC_PATH_MAX)
# define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

/* Don't include sys/param.h if it already has been.  */
#if defined(HAVE_SYS_PARAM_H) && !defined(PATH_MAX) && !defined(MAXPATHLEN)
# include <sys/param.h>
#endif

#if !defined(PATH_MAX) && defined(MAXPATHLEN)
# define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif


static int env_auswertung (cl_file_t *, cl_var_t * Konftab, long pass);
static int zeile_scannen (cl_file_t * conf, char *zeile);
static int cl_putenv(cl_file_t *conf, char *p);

/*
CL_FUNCTION(Function,int,cl_getstanza,
[[(cl_file_t *@var{handle}, cl_var_t *@var{tab}, const char *var{stanza},
			  long @var{flags}, cl_ulist_t ** @var{unknowns})]],[[
cl_getstanza reads the stanza @var{stanza} (or all stanzas, if 
@var{stanza} is @code{NULL}) from the already opened configuration
file pointed to by @var{handle} into @var{tab}.

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
cl_getstanza (cl_file_t * conf, cl_var_t * tab, const char *stanza,
			  long flags, cl_ulist_t ** unknowns)
{
	char *p;
	int ret = 0;
	int stanza_found=0;
	int nach_liste = 0;
	cl_stanza_list_t *akt_position = NULL;
	int orig_line_nr=conf->line_nr;

	if (!conf->in_scanmode)
	{
		conf->line_nr = 0;
		conf->in_comment = 0;

		if (stanza)
		{
			conf->in_stanza = 0;
			conf->stanza = strdup (stanza);
			if (!conf->stanza)
				return -1;
			if (conf->stanzas)
			{
				nach_liste = 1;
			}
			if (conf->stanzas == NULL && !conf->dont_scan)
			{
				int erg;

				conf->in_scanmode = 1;
				erg = cl_getstanza (conf, tab, NULL, flags, NULL);
				conf->in_scanmode = 0;
				if (erg == 0)
					nach_liste = 1;
				conf->line_nr = 0;
				conf->in_comment = 0;
			}
		}
		else
			conf->in_stanza = 1;
	}

	if (nach_liste)
	{
		akt_position = conf->stanzas;
		while (akt_position 
			   && cl_fnmatch_casefold (stanza, akt_position->stanzaname))
			akt_position = akt_position->next;
		if (akt_position)
		{
			fseek (conf->fhandle, akt_position->pos, SEEK_SET);
			conf->in_stanza = 1;
			conf->line_nr=akt_position->lineno;
		}
		else
			rewind (conf->fhandle);
	}
	else
		rewind (conf->fhandle);

	if (!conf->buf)
	{
		conf->buf = malloc (1024);
		if (!conf->buf)
			return 1;
		conf->buflen = 1024;
	}

   /* Env-Variablen, Preauswertung */
	if (!conf->in_scanmode)
		ret |= env_auswertung (conf,tab, CL_PREENV);

	while ((!nach_liste || akt_position)
		   && fgets (conf->buf, (int) conf->buflen, conf->fhandle))
	{
		stanza_found|=conf->in_stanza;
		conf->line_nr++;
		p = conf->buf + strlen (conf->buf) - 1;
		while (*p == '\r' || *p == '\n')
		{
			*p = '\0';
			if (p == conf->buf)
				break;
			else
				p--;
		}

		while (*p == '\\')
		{
			char *p2;
			int noch_frei;

		   /* Zeile ist noch nicht zu Ende */
			*p-- = '\0';
			noch_frei = (int) (conf->buflen - (p - conf->buf) - 1);
			if (noch_frei <= 0 || !fgets (p + 1, noch_frei, conf->fhandle))
				break;
			conf->line_nr++;
			p2 = strpbrk (p, "\r\n");
			if (p2)
			{
				*p2 = '\0';
				p = p2 - 1;
			}
			else
				p = p + strlen (p) - 1;
		}
		if (conf->in_scanmode)
		{
			if (zeile_scannen (conf, conf->buf))
				ret = 1;
		}
		else
		{
			if (0 != cl_zeile_auswerten (conf, tab, conf->buf, flags, unknowns))
				ret = -1;
		}
		if (nach_liste && !conf->in_stanza)
		{
			akt_position = akt_position->next;
			while (akt_position 
				   && cl_fnmatch_casefold (stanza, akt_position->stanzaname))
				akt_position = akt_position->next;
			if (akt_position)
			{
				fseek (conf->fhandle, akt_position->pos, SEEK_SET);
				conf->line_nr=akt_position->lineno;
			}
		}
	}

	if (!conf->in_scanmode)
		ret |= env_auswertung (conf,tab, CL_POSTENV);

	if (stanza)
	{
		free (conf->stanza);
		conf->stanza = NULL;
	}
	if (ret && conf->in_scanmode)
	{
		cl_stanza_list_t *akt;
		cl_stanza_list_t *next;

		akt = conf->stanzas;
		while (akt)
		{
			next = akt->next;
			free (akt->stanzaname);
			free (akt);
			akt = next;
		}
		conf->last_ele = NULL;
		conf->stanzas = NULL;
	}
	if (!ret && !stanza_found && stanza && strcmp(stanza,"*")) {
		int bk=conf->line_nr;
		if (orig_line_nr)
			conf->line_nr=orig_line_nr;
		cl_warning (CL_WARN(conf,NULL),
			"stanza %s not found\n", stanza);
		conf->line_nr=bk;
		ret=1;
	}
	return ret;
}

static int
zeile_scannen (cl_file_t * conf, char *zeile)
{
	char *p;
	size_t l;

	p = cl_strtrim (zeile);
	if (*p == '\0' || *p == '#')
	{
		if (*p == '#' && p[1] == '#' && p[2] != '#')
			conf->in_comment = !conf->in_comment;
		return 0;
	}
	if (conf->in_comment)
		return 0;
	l=strlen(p);
	if (conf->in_stanza)
	{
		char *q=NULL; /* keep gcc quiet */
		if (!strchr (p, '=') && ((*p=='[' && p[l-1]==']') || (q = strchr (p, ':')) != NULL))
		{
			if ((*p=='[' && p[l-1]==']') || q[1]=='\0')
				conf->in_stanza = 0;
		}
	}
	if (!conf->in_stanza)
	{
		/* vor einer Stanza */
		if (!strchr (p, '=') && ((*p=='[' && p[l-1]==']') || p[l-1] == ':'))	/* Label? */
		{
			char *p2;
			long pos = ftell (conf->fhandle);
			
			for (p2 = strtok (*p=='[' ? p+1 : p, " \t:]"); p2; p2 = strtok (NULL, " \t:]"))
			{
				cl_stanza_list_t *ele;

				ele = malloc (sizeof (cl_stanza_list_t));
				if (!ele)
					return 1;
				ele->stanzaname = strdup (p2);
				if (!ele->stanzaname)
				{
					free (ele);
					return -1;
				}
				ele->lineno=conf->line_nr;
				ele->next = 0;
				ele->pos = pos;
				if (conf->last_ele)
					conf->last_ele->next = ele;
				else
					conf->stanzas = ele;
				conf->last_ele = ele;
			}
		}
	}
	return 0;
}

int
cl_zeile_auswerten (cl_file_t * conf, cl_var_t * tab, char *zeile,
				 long flags, cl_ulist_t ** unknowns)
{
	char *p;
	char *q=NULL; /* keep gcc quiet */
	size_t l;

	p = cl_strtrim (zeile);
	if (*p == '\0' || *p == '#')
	{
		if (*p == '#' && p[1] == '#' && p[2] != '#') 
			conf->in_comment = !conf->in_comment;
		return 0;
	}
	if (conf->in_comment)
		return 0;
	l=strlen(p);
	if (conf->in_stanza)
	{
		if (strcmp (p, "end") == 0)
		{
			conf->in_stanza = 0;
			return 0;
		}
		if (!strchr (p, '=') && ((*p=='[' && p[l-1]==']') || (q = strchr (p, ':')) != NULL))
		{
			if ((*p=='[' && p[l-1]==']') || q[1]=='\0')
				conf->in_stanza = 0;
		}
	}
	if (!conf->in_stanza)
	{
		/* vor einer Stanza */
		if (!strchr (p, '=') && ((*p=='[' && p[l-1]==']') || p[l-1] == ':'))	/* Label? */
		{
			char *p2;
			
			if (!conf->stanza)	/* Keine Stanza angegeben -> alles */
				conf->in_stanza = 1;
			else
			{
				for (p2 = strtok (*p=='[' ? p+1 : p, " \t:]"); p2; p2 = strtok (NULL, " \t:]"))
				{
					if (!cl_fnmatch_casefold(conf->stanza, p2))
					{
						conf->in_stanza = 1;
						break;
					}
				}
			}
		}
		return 0;
	}
   /* '=' finden */
	q = strchr (p, '=');
	if (q == NULL)
	{
		if ((flags & CL_GET_NOINCLUDE) && unknowns)
		{
			cl_ulist_t *dieser;
			cl_ulist_t *last;

		   /* Sichern */
			for (dieser = *unknowns, last = NULL; dieser;
				 last = dieser, dieser = dieser->next)
			   /* nothing */ ;
			dieser = malloc (sizeof (cl_ulist_t));
			if (!dieser)
			{
				cl_warning (CL_ERR(conf,NULL),"out of memory\n");
				return -1;
			}
			dieser->varname = NULL;
			dieser->inhalt = strdup (conf->buf);
			if (!dieser->inhalt)
			{
				free (dieser);
				cl_warning (CL_ERR(conf,NULL),"out of memory\n");
				return -1;
			}
			dieser->next = NULL;
			if (!last)
				*unknowns = dieser;
			else
				last->next = dieser;
			return 0;
		}

		if (!strncasecmp (p, "includestanza ", 14))
		{
			long pos;
			int ret;
			cl_file_t backup;
			char *kopie;

			q = p + 14;
			while (isspace (*q))
				q++;
			if (*q=='\0')
			{
			   /* warning, not error */
				cl_warning (CL_WARN(conf,NULL),
					"includestanza used, but no stanza given");
				return 0;
			}
			kopie = strdup (q); /* because q points to a buffer which 
			                     * will be overwritten! */
			if (!kopie)
			{
				cl_warning (CL_ERR(conf,NULL),"out of memory\n");
				return -1;
			}
			pos = ftell (conf->fhandle);
			backup=*conf;
			ret = cl_getstanza(conf,tab,kopie,flags,unknowns);
			*conf=backup;
			if (ret)
				cl_warning (CL_WARN(conf,NULL),
					"includestanza used, cannot read that stanza: %s\n", kopie);
			free(kopie);
			fseek (conf->fhandle, pos, SEEK_SET);
			return 0;
		}
		if (!strncasecmp (p, "include ", 8))
		{
			int ret;
			char *r;
			char *kopie1,*kopie2;

			if (flags & CL_GET_NOINCLUDE)
				return 0;
			q = p + 8;
			while (isspace (*q))
				q++;
			if (*q=='\0')
			{
				cl_warning (CL_WARN(conf,NULL),
					"include used, but no filename given");
				return 0;
			}
			r = strpbrk (q, "\t ");
			if (r)
			{
				*r++ = '\0';
				r = cl_strtrim (r);
				q = cl_strtrim (q);
			}
			kopie1 = strdup (q); /* because q points to a buffer which 
			                     * will be overwritten! */
			if (!kopie1)
			{
				cl_warning (CL_ERR(conf,NULL),"out of memory\n");
				return -1;
			}
			if (r) {
				kopie2 = strdup (r); /* because q points to a buffer which 
									  * will be overwritten! */
				if (!kopie2)
				{
					cl_warning (CL_ERR(conf,NULL),"out of memory\n");
					free(kopie1);
					return -1;
				}
			}
			else
				kopie2=NULL;
			ret = cl_getconfstanza (kopie1, tab, kopie2);
			if (ret)
				cl_warning (CL_WARN(conf,NULL),
					"include used, but cannot read: file %s, stanza %s",
							 conf->fname, (kopie2 ? "* (all)" : kopie1));
			free(kopie1);
			free(kopie2);
			return 0;
		}
	   /*
	    * unbekanntes Statement.
	    */
		if (conf->report_unknown)
			cl_warning (CL_WARN(conf,NULL),"unknown statement '%s'", p);
		return 0;
	}

	if (*p == '$')
	{
		/* Dann diese Variable dem Environment zufuegen */
		return cl_putenv(conf,p);
	}
	if (q[-1]=='~') {
		char *res;
		int ret;
		/* var ~= value -> interpretieren */
		q[-1]=0;
		*q++ = '\0';
		p = cl_strtrim (p);			/* varname */
		q = cl_strtrim (q);			/* value */
		if (!conf->interptab) {
			res=tip_ainterpret(cl_tiptab, q);
		} else {
			res=tip_ainterpret(conf->interptab, q);
		}
		if (!res) {
			cl_warning (CL_ERR(conf,NULL),"out of memory\n");
			return -1;
		}
		ret=cl_assign (conf, tab, p, res, unknowns);
		free(res);
		return(ret);
	}
	*q++ = '\0';
	p = cl_strtrim (p);			/* varname */
	q = cl_strtrim (q);			/* value */
	return cl_assign (conf, tab, p, q, unknowns);
}

/* yessir, no tip_tilde (~user). Would be useful, indeed, but 
 * tip_tilde uses getpwent etcetera, and there is no way to put
 * that back into it's original state.
 */
static tip_t putenvtiptab[]={
    { '\0', "${literal",   0, tip_literal },
	{ '\\', NULL,        0, tip_backslash },
	{ '\0', "$[",        0, tip_conditional },
	{ '\0', "${strip",   0, tip_strip },
	{ '\0', "${force",   0, tip_force },
	{ '$',  NULL,        0, tip_dollar },
	{ '\0', NULL,        0, NULL }
};

static int
cl_putenv(cl_file_t *conf, char *p)
{
	char *gleich;
	char *erg;

	gleich=strchr(p,'=');
	if (!gleich) {
		cl_warning (CL_ERR(conf,NULL),"illegal line: %s\n",p);
		return -1;
	}
	erg=tip_ainterpret(putenvtiptab,p+1);
	if (!erg)
	{
		cl_warning (CL_ERR(conf,NULL),"cannot interpret: %s\n",p);
		return -1;
	}
	gleich=strchr(erg,'=');
	if (!gleich) {
		cl_warning (CL_ERR(conf,NULL),"illegal line: %s\n",p);
		return -1;
	}

#ifdef HAVE_PUTENV
	/* putenv fügt den String dem Environment zu, nicht etwa
	 * eine Kopie. setenv ist das deutlich schönere Interface ...
	 * aber wir haben hier nun eine Kopie.
	 */
	if (putenv (erg)) {
		cl_warning (CL_ERR3(errno,conf,NULL),"putenv");
		free(erg);
		return -1;
	}
#elif defined(HAVE_SETENV)
	if (gleich && gleich[1])
	{
		*gleich=0;
		strtrim(erg);
		if (setenv(erg,gleich+1,1)!=0) {
			cl_warning (CL_ERR3(errno,conf,NULL),"setenv");
			free(erg);
			return -1;
		}
		free(erg);
	} else {
		if (gleich)
			*gleich=0;
#ifdef HAVE_UNSETENV
		cl_strtrim(erg);
		unsetenv(erg);
#else
		cl_warning (CL_ERR(conf,NULL),
					"cannot unset environment variable");
#endif
		free(erg);
	}
#endif
	return 0;
}

static int 
env_auswertung (cl_file_t *file, cl_var_t * KonfTab, long pass)
{
	cl_var_t *Vp;
	char *Ptr;
	int Ret = 0;

   /* Env-Variablen, Postauswertung */
	for (Vp = KonfTab; Vp->varname; Vp++)
	{
		if (!Vp->secdata)
			continue;
		if (!(Vp->flags & pass))
			continue;
		Ptr = getenv ((const char *) Vp->secdata);
		if (Ptr)
		{
			if (Vp->typ == CL_DIRNAME)
			{
				char *dirname;
				size_t l;
				l=strlen(Ptr);
				dirname=malloc(l+2);
				if (!dirname) {
					cl_warning (CL_ERR(file,Vp),
						"cannot allocate %lu bytes", 
						(unsigned long) (PATH_MAX));
					return 1;
				}
				memcpy(dirname,Ptr,l+1);
				if (dirname[l - 1] != '/') {
					dirname[l]='/';
					dirname[l+1]='\0';
				}
				Ptr = dirname;
			}
			switch (Vp->typ)
			{
			case CL_STRING:
			case CL_DIRNAME:
				if ((Vp->flags & CL_MALLOCED)
					&& (char *) (Vp->adr) != (char *) NULL)
				{
					free (*(char **) Vp->adr);
					Vp->flags -= CL_MALLOCED;
				}
				if (cl_dup_to (Vp, Ptr))
				{
					Ret = 1;
				}
				if (Vp->typ==CL_DIRNAME)
					free(Ptr);
				break;
			case CL_CHAR:
				if (Ptr[1])
				{
					cl_warning (CL_WARN(file,Vp),
						"environment variable %s: desired char, got string: %s", 
					 	Vp->secdata,Ptr);
				}
				*(char *) Vp->adr = *Ptr;
				return 0;
			case CL_NUM:
				{
					strtol_error s_err;
					s_err=cl_strtol(Ptr,NULL,0,Vp->adr,"bckmw");
					if (s_err!=LONGINT_OK) {
						CL_STRTOL_WARNING(s_err,file,Vp,Ptr,Vp->varname);
						*(long*)Vp->adr = strtol(Ptr,NULL,0);
					}
				}
				break;
			case CL_NUMSHORT:
				*(short *) Vp->adr = (short) atoi (Ptr);
				{
					strtol_error s_err;
					long lo;
					s_err=cl_strtol(Ptr,NULL,0,&lo,"bckmw");
					if (s_err!=LONGINT_OK) {
						CL_STRTOL_WARNING(s_err,file,Vp,Ptr,Vp->varname);
						lo = strtol(Ptr,NULL,0);
					}
					if (lo > SHRT_MAX || lo < SHRT_MIN) {
						CL_STRTOL_WARNING(LONGINT_INT_OVERFLOW,file,Vp,Ptr,Vp->varname);
						lo=0;
					}
					*(short*)Vp->adr = (int) lo;
				}
				break;
			case CL_BOOLEAN:
				*(int *) Vp->adr = cl_convert2bool (Ptr);
				break;
			case CL_ARRAY:
			case CL_ENUM:
			case CL_BITFIELD:
			case CL_ALIAS:		/* keep gcc happy */
			case CL_DUMMY:
			case CL_LIST:
				break;
			}
		}
	}
	return Ret;
}
