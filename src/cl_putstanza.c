/*
    cl_putstanza.c - save a stanza to a file
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
		Uwe_Ohse@me.maus.de
		uwe@tirka.gun.de
	snail-mail:
	    Uwe Ohse, Drosselstrasse 2, 47055 Duisburg, Germany

$Id: cl_putstanza.c,v 1.2 1998/08/15 09:43:51 uwe Exp $
*/

#include "ownstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h> /* unlink */
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include "conflib.h"
#include "intern.h"

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

/*
 * we try in that order:
 * /some/where/file.cnf.bak
 * /some/where/file.cnf~
 * /some/where/file.bak
 * /some/where/file~
 */
static FILE *
cl_open_backup(cl_file_t *c, char *backupfname, const char *mode)
{
	FILE *f;
	strcpy(backupfname,c->fname);
	strcat(backupfname,".bak");
	f=fopen(backupfname,mode);
	if (!f)
	{
		strcpy(backupfname,c->fname);
		strcat(backupfname,"~");
		f=fopen(backupfname,mode);
	}
	if (!f)
	{
		/* name too long? -- using errno is not good here. We need to
		 * try again anyway. */
		char *slash;
		char *dot;
		strcpy(backupfname,c->fname);
		slash=strrchr(backupfname,'/');
		if (!slash)
			dot=strchr(backupfname,'.');
		else
			dot=strchr(slash,'.');
		if (dot)
		{
			strcpy(dot,".bak");
			f=fopen(backupfname,mode);
			if (!f)
			{
				strcpy(dot,"~");
				f=fopen(backupfname,mode);
			}
		}
	}
	return f;
}

/* CL_FUNCTION(Function,int,cl_file_t,
[[(cl_file_t *@var{handle}, cl_var_t *@var{tab}, const char *@var{stanza}, 
	long @var{flags}, cl_ulist_t *@var{unknowns})]],[[
@code{cl_putstanza} writes @var{tab} to the configuration file 
@var{handle} into the stanza @var{stanza}. It tries hard to
do the right thing (especially it preserves any variables found
in the it doesn't know about).

But there a things @code{cl_putstanza} can or will not handle:

@itemize @bullet
@item @code{include}- and @code{includestanza}-statements are
ignored (it would be hard to do that right, and i don't see a really
good reason for it).
@item Variables in @var{tab} without the @code{CL_MAY_SAVE} flag
set will not be saved (this means: if you want to save all variables
all variables need that flag). If the variable is already found
in the stanza @var{stanza} it will be preserved (this means: it's 
not enough to leave out the @code{CL_MA_SAVE} flag to delete a 
variable. In fact it's impossible to delete a variable at this
time).
@end itemize

All variables in @var{unknowns} will be saved. 

To state it clearly: At the moment there is no way to remove a
variable from the configuration file.

Returns 0 if successful, other values is unsuccessful.
]]) */
int 
cl_putstanza(cl_file_t *conf, cl_var_t *tab, const char *stanza, long flags,
	cl_ulist_t *unknowns)
{
	cl_stanza_list_t *akt_position;
	int first;
	int in_comment;
	FILE *tf;
	cl_var_t *var;
	cl_ulist_t *up;
	int unk_count;
	struct unk_struct{
		char *varname;
		char *value;
		int to_do;
	} *unk_array;
	int found_one;
	int ok;
	char indent[256]="\t"; /* default to use if we don't read any line */
	char *backupfname;

	if (conf->mode!='w')
	{
		cl_warning(CL_ERR(conf,NULL),
			"file is readonly");
		return -1;
	}

	if (strpbrk(stanza,"?*[]"))
	{
		/* das geht einfach nicht! */
		cl_warning(CL_ERR(conf,NULL),
			"trying to write to wildcard stanza %s",stanza);
		return -1;
	}

	tf=tmpfile();
	if (!tf)
	{
		cl_warning(CL_ERR(conf,NULL),"cannot open tmpfile()");
		return -1;
	}
	
	if (fseek(conf->fhandle, 0, SEEK_SET))
	{
		/* i believe this should not happen */
		cl_warning(CL_ERR3(errno,conf,NULL),"cannot seek");
		fclose(tf);
		return -1;
	}

	if (!conf->buf)
	{
		conf->buf=malloc(1024);
		if (!conf->buf)
		{
			fclose(tf);
			cl_warning(CL_ERR(conf,NULL),"out of memory");
			return -1;
		}
		conf->buflen=1024;
	}

	if (unknowns)
	{
		unk_count=1;
		for (up=unknowns;up;up=up->next)
			unk_count++;
		unk_array=malloc(sizeof(struct unk_struct) * unk_count);
		if (!unk_array)
		{
			fclose(tf);
			cl_warning(CL_ERR(conf,NULL),"out of memory");
			
			return -1;
		}
		unk_count=0;
		for (up=unknowns;up;up=up->next)
		{
			unk_array[unk_count].varname=up->varname;
			unk_array[unk_count].value=up->inhalt;
			unk_array[unk_count].to_do=1;
			unk_count++;
		}
		unk_array[unk_count].varname=0;
		unk_array[unk_count].value=0;
		unk_array[unk_count].to_do=0;
	}
	else
		unk_array=NULL;

	/* Markieren, daß diese Variable noch ausgegeben werden muß. */
	for (var=tab; var->varname!=NULL; var++)
	{
		var->flags |= CL_VAR_NOT_DONE;
	}

	conf->line_nr=0;
	conf->in_comment=0;

	conf->in_stanza=0;
	if (conf->stanzas==NULL)
	{
		int erg;
		conf->in_scanmode=1;
		erg=cl_getstanza(conf, tab, NULL, flags, NULL);
		conf->in_scanmode=0;
		if (erg!=0)
		{
			fclose(tf);
			free(unk_array);
			return -1;
		}
		conf->line_nr=0;
		conf->in_comment=0;
		conf->in_stanza=0;
	}

	if (fseek(conf->fhandle,0,SEEK_SET)!=0)
	{
		cl_warning(CL_ERR3(errno,conf,NULL),"cannot seek to start");
		fclose(tf);
		return -1;
	}

	akt_position=conf->stanzas;
	found_one=0;
	*conf->buf=0;
	for (;;)
	{
		long bytes_to_copy=0;
		while (akt_position && 0!=strcasecmp(stanza,akt_position->stanzaname))
			akt_position=akt_position->next;
		if (akt_position==NULL)
		{
			struct stat st;
			*conf->buf=0;
			if (found_one)
				break; /* no other incarnation of the stanza */
			/* else: copy rest of file first */
			if (-1==fstat(fileno(conf->fhandle),&st))
			{
				cl_warning(CL_ERR3(errno,conf,NULL),"cannot fstat");
				fclose(tf);
				return -1;
			}
			bytes_to_copy=st.st_size;
		}
		else
		{
			bytes_to_copy=akt_position->pos-ftell(conf->fhandle);
			akt_position=akt_position->next;
			found_one=1;
		}

		/* if where is something between */
		if (bytes_to_copy>0)
		{
			size_t need=bytes_to_copy;
			while (need>0)
			{
				long got;
				long out;
				if (need < conf->buflen)
					got=fread(conf->buf,1,need,conf->fhandle);
				else
					got=fread(conf->buf,1,conf->buflen,conf->fhandle);
				if (got==0)
				{
					cl_warning(CL_ERR3(errno,conf,NULL),
						"cannot read from config file");
					break;
				}
				errno=0;
				out=fwrite(conf->buf,1,got,tf);
				if (out!=got)
				{
					cl_warning(CL_ERR3(errno,conf,NULL),
						"cannot write to tmp file");
					break;
				}
				need-=got;
			}
			if (fflush(tf)==EOF)
			{
				cl_warning(CL_ERR3(errno, conf,NULL),
						   "cannot write to tmp file");
				need=1;
			}
			if (need)
			{
				fclose(tf);
				free(unk_array);
				return -1;
			}
		}

		if (!found_one)
		{
			/* we copied the start of the file. */
			*conf->buf=0;
			break;
		}

		/* Wir sind jetzt in der Datei an der Position, ab der die Stanza
		 * ist, die wir umschreiben wollen. */

		first=0; /* die nächste Zeile ist die erste, die mit der Stanza */
		in_comment=0;
		while(fgets(conf->buf,(int) conf->buflen,conf->fhandle))
		{
			char *varname;
			char *value;
			char *p;
			int done;
			conf->line_nr++;
			p=conf->buf+strlen(conf->buf)-1;
			while (*p=='\r' || *p=='\n')
			{
				*p=0;
				if (p==conf->buf)
					break;
				else	
					p--;
			}

			while (*p=='\\')
			{
				char *p2;
				int noch_frei;
				/* Zeile ist noch nicht zu Ende */
				*p--=0;
				noch_frei=(int) (conf->buflen - (p-conf->buf) -1);
				if (noch_frei<=0 || !fgets(p+1,noch_frei,conf->fhandle))
					break;
				p2=strpbrk(p,"\r\n");
				if (p2)
				{	
					*p2='\0';
					p=p2-1;
				}
				else
					p=p+strlen(p)-1;
			}

			if (first)
			{
				/* The stanza line */
				fputs(conf->buf,tf);
				fputc('\n',tf);
				first=0;
				continue;
			}

			/* Skip white space */
			p=conf->buf;
			while(isspace(*p))
				p++;
			if (!*p)
			{
				/* put out the empty lines */
				fputs(conf->buf,tf);
				fputc('\n',tf);
				continue;
			}

			/* Start or end of comment block? */
			if (p[0]=='#' && p[1]=='#' && p[2]!='#')
			{
				in_comment=!in_comment;
				fputs(conf->buf,tf);
				fputc('\n',tf);
				continue;
			}

			/* in comment block */
			if (in_comment)
			{
				fputs(conf->buf,tf);
				fputc('\n',tf);
				continue;
			}

			/* one line comment? */
			if (*p=='#')
			{
				fputs(conf->buf,tf);
				fputc('\n',tf);
				continue;
			}

			/* Start of a new stanza or at least end of this stanza? */
			if ((!isspace(*conf->buf) && !strchr(conf->buf,'=') 
				&& (p=strrchr(conf->buf,':'))!=NULL && p[1]=='\0')
				|| strcmp(conf->buf,"end")==0)
			{
				goto out_outer;
			}

			if ((size_t)(p-conf->buf+1) < sizeof(indent))
			{
				strncpy(indent,conf->buf,p-conf->buf);
				indent[p-conf->buf]=0;
			}

			/* Statement or variable assignment? */
			varname=p;
			p=strpbrk(varname,"= \t");
			if (!p)
			{
				/* neither statement nor variable assignment. Heck, what's this?
				 * preserve it! */
				fputs(conf->buf,tf);
				fputc('\n',tf);
				continue;
			}

			if (*p!='=')
			{
				/* A statement. Leave it unchanged for now - statements are a 
				 * major problem */
				fputs(conf->buf,tf);
				fputc('\n',tf);
				continue;
			}
			/* Ok, it's a variable assignment */
			value=p+1;
			*p--=0;
			while (p>= varname && isspace(*p))
				*p--=0;
			while (isspace(*value))
				value++;

			done=0;
			for (var=tab;var->varname;var++)
			{
				char c1=tolower(*varname);
				char c2=tolower(*var->varname);
				if (c1=='-')
					c1='_';
				if (c2=='-')
					c2='_';
				if (c1==c2 && cl_varnamecmp(varname+1,var->varname + 1)==0)
				{
					if (!(var->flags & CL_MAY_SAVE))
						/* may be a default value */
						continue;
					if (!(var->flags & CL_VAR_NOT_DONE))
					{
						/* die haben wir schon ausgegeben */
						done=1;
						continue;
					}

					if (var->typ==CL_ALIAS)
					{
						var=cl_unalias(conf,tab,var);
						if (!var)
							break;
					}
					fputs(indent,tf);
					cl_put_one(tf,var);
					var->flags &= ~(CL_VAR_NOT_DONE);
					done=1;
					break;
				}
			}
			if (done)
				continue;
			/* may be an unknown variable */
			if (unk_array)
			{
				for (unk_count=0;unk_array[unk_count].varname!=0;unk_count++)
				{
					char c1=tolower(*varname);
					char c2=tolower(*unk_array[unk_count].varname);
					if (unk_array[unk_count].to_do==0)
						continue;
					if (c1=='-')
						c1='_';
					if (c2=='-')
						c2='_';
					if (c1==c2 && cl_varnamecmp(varname+1,
						unk_array[unk_count].varname + 1)==0)
					{
						unk_array[unk_count].to_do=0;
						fputs(indent,tf);
						fputs(unk_array[unk_count].varname,tf);
						fputc('=',tf);
						fputs(unk_array[unk_count].value,tf);
						fputc('\n',tf);
						done=1;
						break;
					}
				}
			}
			if (done)
				continue;
			/* where's something, but we don't know it. Leave it in */
			fputs(indent,tf);
			fputs(varname,tf);
			fputc('=',tf);
			fputs(value,tf);
			fputc('\n',tf);
		}
	}
out_outer:
	if (!found_one)
	{
		/* print the stanza name */
		fprintf(tf,"%s:\n",stanza);
	}

	/* output remaining variables */
	for (var=tab;var->varname;var++)
	{
		if ((var->flags & CL_VAR_NOT_DONE) && (var->flags & CL_MAY_SAVE))
		{
			fputs(indent,tf);
			cl_put_one(tf,var);
		}
	}
	/* and the unknown variables */
	if (unk_array)
	{
		for (unk_count=0;unk_array[unk_count].varname!=0;unk_count++)
		{
			if (unk_array[unk_count].to_do==0)
				continue;
			fputs(indent,tf);
			fputs(unk_array[unk_count].varname,tf);
			fputc('=',tf);
			fputs(unk_array[unk_count].value,tf);
			fputc('\n',tf);
		}
	}
	/* do not need it any more */
	free(unk_array);

	/* print what's left in the buffer - this is the beginning of the
	 * next stanza or the end line. */
	if (*conf->buf)
		fprintf(tf,"%s\n",conf->buf);

	/* copy the remaining stanzas ... */
	ok=1;
	for (;;)
	{
		size_t got;
		got=fread(conf->buf,1,conf->buflen,conf->fhandle);
		if (got==0)
		{
			if (ferror(conf->fhandle))
			{
				cl_warning(CL_ERR3(errno,conf,NULL),
					"cannot read from config file");
				ok=0;
			}
			break;
		}
		if (fwrite(conf->buf,1,got,tf)!=got)
		{
			cl_warning(CL_ERR3(errno,conf,NULL),
					   "cannot write to tmp file");
			ok=0;
			break;
		}
	}
	if (ok && fflush(tf)==EOF)
	{
		cl_warning(CL_ERR3(errno,conf,NULL),
			"cannot write to tmp file");
		ok=0;
	}

	if (!ok)
	{
		fclose(tf);
		return -1;
	}

	backupfname=malloc(PATH_MAX);
	if (!backupfname) {
		cl_warning(CL_ERR3(errno,conf,NULL),
			"cannot malloc %lu bytes",(unsigned long) (PATH_MAX));
		fclose(tf);
		return -1;
	}
	*backupfname=0;

	/* do we need to back up the file? */
	if (!conf->did_backup)
	{
		FILE *f;
		f=cl_open_backup(conf,backupfname,"w");
		if (!f)
		{
			/* put the prefered name in the error message */
			strcpy(backupfname,conf->fname);
			strcat(backupfname,".bak");
			cl_warning(CL_ERR3(errno,conf,NULL),"cannot backup to %s", 
				backupfname);
			fclose(tf);
			free(backupfname);
			return -1;
		}
		rewind(conf->fhandle);
		ok=1;
		for (;;)
		{
			size_t got;
			got=fread(conf->buf,1,conf->buflen,conf->fhandle);
			if (got==0)
			{
				if (ferror(conf->fhandle))
				{
					cl_warning(CL_ERR3(errno,conf,NULL),
						"cannot read from config file");
					ok=0;
				}
				break;
			}
			if (fwrite(conf->buf,1,got,f)!=got)
			{
				cl_warning(CL_ERR3(errno,conf,NULL),
					"cannot write to backup file %s", 
					backupfname);
				ok=0;
				break;
			}
		}
		if (fclose(f)==EOF && ok)
		{
			cl_warning(CL_ERR3(errno,conf,NULL),
				"cannot write to backup file %s",
				backupfname);
			ok=0;
		}
		if (!ok)
		{
			fclose(tf);
			if (0!=unlink(backupfname))
			{
				/* Oh yes, *this* is paranoia */
				cl_warning(CL_ERR3(errno,conf,NULL),
					"cannot unlink failed backup %s", 
					backupfname);
			}
			free(backupfname);
			return -1;
		}

		conf->did_backup=1;
	}

	/*
	 * Ok, we have a backup of the configuration file. Now overwrite the
	 * original.
	 */
	rewind(tf);
	rewind(conf->fhandle);
	ok=1;
	while (1)
	{
		size_t got;
		got=fread(conf->buf,1,conf->buflen,tf);
		if (got==0)
		{
			if (ferror(tf))
			{
				cl_warning(CL_ERR3(errno,conf,NULL),
					"cannot read from tmp file");
				ok=0;
			}
			break;
		}
		if (fwrite(conf->buf,1,got,conf->fhandle)!=got)
		{
			cl_warning(CL_ERR3(errno,conf,NULL),
				"cannot write to config file");
			ok=0;
			break;
		}
	}
	fclose(tf); /* don't need it anymore. */

	if (fflush(conf->fhandle)==EOF)
	{
		cl_warning(CL_ERR3(errno,conf,NULL), "cannot write to config file");
		ok=0;
	}

	if (!ok)
	{
		/* We now try to play in the backup */
		FILE *f;
		if (*backupfname)
			/* we did the backup */
			f=fopen(backupfname,"r");
		else
			/* the backup was done before */
			f=cl_open_backup(conf,backupfname,"r");
		if (!f)
		{
			/* this can not happen? */
			cl_warning(CL_ERR3(errno,conf,NULL),
				"cannot open backup file %s", backupfname);
			cl_warning(CL_ERR3(errno,conf,NULL),"configuration file damaged");
			/* Cache is invalid */
			cl_invalidate_stanzalist(conf);
			free(backupfname);
			return -2;
		}
		rewind(conf->fhandle);
		ok=1;
		while (1)
		{
			size_t got;
			got=fread(conf->buf,1,conf->buflen,f);
			if (got==0)
			{
				if (ferror(f))
				{
					cl_warning(CL_ERR3(errno,conf,NULL),
						"cannot read from backup file %s");
					ok=0;
				}
				break;
			}
			if (fwrite(conf->buf,1,got,conf->fhandle)!=got)
			{
				cl_warning(CL_ERR3(errno,conf,NULL),
					"cannot write to config file");
				ok=0;
				break;
			}
		}
		if (ok && fflush(conf->fhandle)==EOF)
		{
			cl_warning(CL_ERR3(errno,conf,NULL),
				"cannot write to config file");
			ok=0;
		}
		fclose(f); /* backup file */
		free(backupfname);
		if (ok)
			return -1;
		/* Cache is invalid */
		cl_invalidate_stanzalist(conf);
		cl_warning(CL_ERR3(errno,conf,NULL),
			"configuration file damaged");
		return -2;
	}

	/* Cache is invalid */
	cl_invalidate_stanzalist(conf);

	/* Alles OK. */
	free(backupfname);
	return 0;
}
