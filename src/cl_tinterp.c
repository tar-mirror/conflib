/*
    tinterp.c - textinterpretations
    Copyright (C) 1993, 1994, 1995 Uwe Ohse,  1991, 1992 Wolfram Roesler

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

$Id: cl_tinterp.c,v 1.2 1998/08/15 09:43:51 uwe Exp $
$Date: 1998/08/15 09:43:51 $
$Name:  $
$Log: cl_tinterp.c,v $
Revision 1.2  1998/08/15 09:43:51  uwe
*** empty log message ***

Revision 1.1.1.1  1998/07/09 21:57:52  uwe
import

Revision 1.2  1997/11/02 18:41:00  uwe
Einen Bug bereinigt und einige Warnungen beseitigt

Revision 1.1.1.2  1997/11/02 17:11:39  uwe
import nachträglich

Revision 1.1.1.1  1996/06/15 08:55:27  uwe
Imported sources

Revision 1.1.1.1  1996/02/18 22:45:23  uwe
Import Sourcen

Revision 1.5  1995/11/29 14:53:38  uwe
Kleinere Optimierungen
Klammerschachtelungscode nicht mehr so leicht zu verwirren

Revision 1.4  1995/05/08 18:35:13  quark
Makroroutinen ausgegliedert

Revision 1.3  1995/05/08 12:34:03  quark
tip_ainterpret: neu
tip_defmacro heißt nun tip_defmacro_intern
   kann nun auch den Wert eines Makros aendern
tip_macro liefert einen um eins zu hohen Rückgabewert zurück
tip_defmacro: neue Funktion (nicht zu verwechseln mit der, die
   nun tip_defmacro_intern heißt), die aus der Interpretationsroutine
   aufgerufen werden kann.
tip_undefmacro: neue Funktion
tip_literal: neue Funktion
tip_force: neue Funktion

Revision 1.2  1995/05/07 21:17:01  quark
$-RCS-Identifier ergänzt.

*/

#include "ownstd.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_PWD_H
#  include <pwd.h>
#endif

#include "tinterp.h"

const char *tip_errtext;
#ifdef HAVE_PWD_H
static int pw_used;
#endif

#define PARANOIA_STRING(fn_name,start,this_magic) \
do { \
	if (*this_magic!=*start  || strncmp(start,this_magic,strlen(this_magic))) \
	{ \
		tip_errtext = fn_name " called without proper magic " this_magic ; \
		return TIP_ERROR; \
	} \
} while (0)

#define PARANOIA_CHAR(fn_name,start,this_magic) \
do { \
	if (*start!=*this_magic) \
	{ \
		tip_errtext = fn_name " called without proper magic " this_magic ; \
		return TIP_ERROR; \
	} \
} while (0)

#define LENCHECK(l) if (l>=maxlen) { tip_errtext="buffer shortage"; return TIP_ERROR; }
#define ADD_CHAR(c) { *to=c; to[1]='\0'; return 1; } 

char *
tip_getpart(const char *start, const char *endchars)
{
	char in_quote='\0';
	int in_cond=0;
	int in_fn=0;
	int eckig=0;
	int geschweift=0;
	char anti='\0';
	int anti_zahl=0;
	if (endchars[1]=='\0')
	{
		if (*endchars=='}')
			anti='{';
		else if (*endchars==')')
			anti='(';
		else if (*endchars==']')
			anti='[';
	}
	while (*start != '\0')
	{
		if (in_quote != '\0')
		{
			if (*start==in_quote)
				in_quote='\0';
		}
		else if (*start=='\"' || *start=='\'')
			in_quote=*start;
		else if (*start=='\\')
		{
			if (start[1]!='\0')
				start++;
		}
		else if (*start=='$' && start[1]=='[')
		{
			in_cond++;
			start++;
		}
		else if (in_cond)
		{
			if (*start=='[')
				eckig++;
			else if (*start==']' && eckig>0)
				eckig--;
			else if (*start==']')
				in_cond--;
		}
		else if (*start=='$' && start[1]=='{')
		{
			in_fn++;
			start++;
		}
		else if (in_fn)
		{
			if (*start=='{')
				geschweift++;
			else if (*start=='}' && geschweift>0)
				geschweift--;
			else if (*start=='}')
				in_fn--;
		}
		else if (anti && *start==anti)
			anti_zahl++;
		else if (anti_zahl>0 && *start==*endchars)
			anti_zahl--;
		else if (strchr(endchars,*start)) {
			/* disqualify const. c doesn't provide a way to
			 * do that clean. */
			return (char *) start; 
		}
		start++;
	}
	return NULL;
}

/*
 * $[bedingung?what-if-true:what-if-false]
 * $[bedingung?what-if-true]
 */
size_t 
tip_conditional(tip_t *tab, const char *akt, 
	const char **next, char *to, size_t maxlen)
{
	char *true_part=NULL;
	char *false_part=NULL;
	char *bedingung=NULL;
	char *ende=NULL;
	char *duplicate=NULL;
	char *tmpbuf1=NULL;
	char *tmpbuf2=NULL;
	char *result=NULL;
	char *gleich=NULL;
	size_t resultlen=0; /* keep gcc quiet */
 	size_t lentmp1=0,lentmp2=0;
	int ok=0;

	*to='\0';

	PARANOIA_STRING("tip_conditional",akt,"$[");

	duplicate=strdup(akt); /* definitiv genug */
	if (!duplicate)
		goto abbruch;

	bedingung=duplicate+2;

 	/* 
 	 * $[bedingung?what-if-true:what-if-false] 
 	 *                          ^false_part
 	 *             ^true_part
 	 *   ^ bedingung
 	 */
 	true_part=tip_getpart(bedingung,"?");
 	if (!true_part)
 		goto abbruch;

 	*true_part++='\0'; /* Fragezeichen weg */

 	false_part=tip_getpart(true_part,":]");
 	if (*false_part==':')
 	{
 		*false_part++='\0'; /* : weg */
	 	ende=tip_getpart(false_part,"]");
		if (!ende)
			goto abbruch;
 	}
 	else if (!false_part)
 		goto abbruch;
 	else
 	{
 		ende=false_part;
 	}
	*ende='\0';
 	/* 
 	 * Zustand nun: 
 	 * bedingung zeigt auf einen String "$a == %b" z.B.
 	 * true_part zeigt auf den JA-Teil
 	 * false_part zeigt auf den NEIN-Teil oder ist NULL
 	 * ende zeigt auf ]
 	 */
 	/*
 	 * Bedingung kann sein:
 	 * irgendwas == irgendwasanderes
 	 * irgendwas != irgendwasanderes
 	 * irgendwas
 	 */
 	gleich=tip_getpart(bedingung,"=!");
 	if (!gleich)
 	{
 		tmpbuf1=malloc(1024);
 		if (!tmpbuf1)
 			goto abbruch;
 		lentmp1=tip_interpret(tab,bedingung,tmpbuf1,1024);
 		if (lentmp1==TIP_ERROR)
 			goto abbruch;
 		if (lentmp1!=1 && lentmp1!=TIP_NOTDONE) /* 1 wegen \0 */
 			result=true_part;
 		else
 			result=false_part;
 	}
 	else
 	{
 		int nicht=0;
 		int equal;

 		if (gleich[1]!='=') /* nur != und == */
 			goto abbruch;

 		tmpbuf1=malloc(1024);
 		if (!tmpbuf1)
 			goto abbruch;

 		tmpbuf2=malloc(1024);
 		if (!tmpbuf2)
 			goto abbruch;

		if (*gleich=='!')
 			nicht=1;
 		*gleich='\0';
 		gleich+=2; /* zweites Zeichen überspringen */
 		/* Zustand nun: 
 		 * bedingung zeigt auf ersten Teil der Abfrage, 
 		 * gleich auf den zweiten. Nicht ist 1 wenn Abfrage auf Ungleich 
 		 */
 		lentmp1=tip_interpret(tab,bedingung,tmpbuf1,1024);
 		if (lentmp1==TIP_ERROR)
 			goto abbruch;
 		lentmp2=tip_interpret(tab,gleich,tmpbuf2,1024);
 		if (lentmp2==TIP_ERROR)
 			goto abbruch;
 		equal=!nicht;
 		if (lentmp1!=lentmp2 || strcmp(tmpbuf1,tmpbuf2))
 			equal=!equal;
 		if (equal)
 			result=true_part;
 		else
 			result=false_part;
 	}


 	if (!result)
 		goto abbruch;

	resultlen=tip_interpret(tab, result, to, maxlen);
	if (resultlen!=TIP_ERROR)
		ok=1;

	*next=ende-duplicate+akt+1;
abbruch:
	if (tmpbuf1)
		free(tmpbuf1);
	if (tmpbuf2)
		free(tmpbuf2);
 	if (duplicate)
		free(duplicate);
	if (ok)
		return resultlen-1; /* Die +1 kommt hinterher von tip_interpret */
	else
		return TIP_ERROR;
}

/*
    $ABC    the value of env var ABC
    $(ABC)  dito
	$$      a single $
*/
/* ARGSUSED */
size_t 
tip_dollar(tip_t *tab, const char *akt, const char **next, char *to, 
	size_t maxlen)
{
	size_t l;
	int bracket;
	const char *end;
	const char *start;

	PARANOIA_CHAR("tip_dollar",akt,"$");

	if (akt[1]=='$') /* $$ -> $ */
	{
		LENCHECK(1);
		*next=akt+2;
		ADD_CHAR('$');
	}

	if (akt[1]=='(')
	{
		start=akt+2;
		bracket=1;
	}
	else
	{
		start=akt+1;
		bracket=0;
	}
	end=start;
	while (*end!='\0' 
		&& ( (bracket==0 && (isalnum(*end) || *end=='_')) 
			|| (bracket==1 && *end!=')') 
		))
		end++;
	if (end!=start)
	{
		char *var;
		char varname[256];
		strncpy(varname,start,end-start);
		varname[end-start]='\0';

		*next=end+bracket; /* Schon mal setzen */

		var=getenv(varname);
		if (var)
		{
			l=strlen(var);				
			LENCHECK(l);
			strcpy(to,var);
			return l;
		}
		else if (!strcmp(varname,"CONFLIBVERSION"))
		{
			l=strlen(VERSION);				
			LENCHECK(l);
			strcpy(to,VERSION);
			return l;
		}
		else
			return 0; /* Durch nichts ersetzt */
	}
	else
	{
		if (*end=='\0')
			*next=end;
		else
			*next=end+1;
		return 0; /* durch nichts ersetzt */
	}
}

#ifdef HAVE_PWD_H
/*
 * ~name
 * ~~
 * ~(name)
 */
/* ARGSUSED */
size_t 
tip_tilde(tip_t *tab, const char *akt, const char **next, char *to, 
	size_t maxlen)
{
	size_t l;
	const char *end=akt+1;
	const char *start;
	struct passwd *pw;
	int bracket=0;

	PARANOIA_CHAR("tip_tilde",akt,"~");

	if (akt[1]=='~') /* ~~ -> ~ */
	{
		LENCHECK(1);
		*next=akt+2;
		ADD_CHAR('~'); /* returns 1 */
	}

	pw_used++;
	if (*end=='(')
	{
		bracket++;
		end++;
	}
	start=end;
	while (isalnum(*end) || *end=='_' 
			|| (bracket && *end!=')') )
		end++;
	if (start==end)
	{
		/* ~/ oder ~; oder ~ */
		pw=getpwuid(getuid());
		*next=akt+1;
	}
	else
	{
		/* ~name/ */
		char un[255];
		strncpy(un,start,end-start);
		un[end-start]='\0';
		pw=getpwnam(un);
		*next=end+bracket;
	}
	if (pw)
	{
		l=strlen(pw->pw_dir);
		LENCHECK(l);
		strcpy(to,pw->pw_dir);
		return l;

	}
	else
	{
		l=end-start+1+bracket*2;
		LENCHECK(l);
		strncpy(to,start-1-bracket,end-start+1+bracket*2);
		to[l]='\0';
		return l;
	}
}
#endif

/*
 * \was-auch-immer
 */
/* ARGSUSED */
size_t 
tip_backslash(tip_t *tab, const char *akt, const char **next, char *to, 
	size_t maxlen)
{
	char nz;

	PARANOIA_CHAR("tip_backslash",akt,"\\");

	switch(akt[1])
	{
		case '0': /* oktal oder hex (0x..) */
			if (akt[2]=='x' || akt[2]=='X')
				nz=(unsigned char) strtol(akt+1, (char **) next,16);
			else
				nz=(unsigned char) strtol(akt+1, (char **) next,8);
			LENCHECK(1);
			ADD_CHAR(nz); /* returns 1 */
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			nz=(unsigned char) strtol(akt+1, (char **) next,10);
			LENCHECK(1);
			ADD_CHAR(nz); /* returns 1 */ 
			break;										
		case 'a':	/* bell */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\007'); /* \a doesn't work with pre ANSI C */
			break;
		case 'b':   /* backspace */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\b');
			break;
		case 'f':   /* formfeed */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\f');
			break;
		case 'n':   /* newline */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\n');
			break;
		case 'r':   /* carrige return */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\r');
			break;
		case 't':   /* hor. tab. */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\t');
			break;
		case 'v':   /* vert tab. */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR('\v');
			break;
		case '\\':
		case '~':
		case '$':
		default: /* experimental */
			LENCHECK(1);
			*next=akt+2;
			ADD_CHAR(akt[1]);
			break;
	} /* switch */
	/* NOTREACHED */
	return TIP_NOTDONE;
}
#undef ADD_CHAR
#undef LENCHECK

size_t
tip_interpret(tip_t *tab, const char *format, char *buffer, size_t buflen)
{
	tip_t *akt;
	char *to;
	size_t done;
	size_t restlen;

	to=buffer;
	restlen=buflen;

	while (format && *format)
	{
		akt=tab;
		done=TIP_NOTDONE;
		while (akt->magic!='\0' || akt->magicstring)
		{
			int passt=0;
			if (akt->magicstring && !akt->magiclen)
					akt->magiclen=strlen(akt->magicstring);
			if (akt->magicstring 
					&& *akt->magicstring == *format
					&& 0==strncmp(akt->magicstring,format,akt->magiclen))
				passt=1;
			else if (akt->magic && akt->magic==*format)
				passt=1;
			if (passt)
			{
				const char *next=NULL;
				done=akt->interp(tab, format,&next,to,restlen);
				if (done==TIP_ERROR)
					return TIP_ERROR;
				if (done!=TIP_NOTDONE)
				{
					format=next;
					to+=done;
					restlen-=done;
					break;
				}
				/* Pech, hat wohl nicht gepasst */
			}
			akt++;
		}
		if (done==TIP_NOTDONE)
		{
			/* nichts hat darauf gepasst, also ein Zeichen anhaengen */
			if (1<restlen)
			{
				*to++=*format;
				*to='\0';
				restlen--;
				format++;
			}
			else
			{
				tip_errtext="buffer shortage"; 
				return TIP_ERROR; 
			}
		}
	} /* while (*format) */
	*to='\0';
	return buflen-restlen+1; /* +1 wg. \0 */
}

char *
tip_ainterpret(tip_t *tab, const char *format)
{
	char *to;
	char *buffer;
	size_t buffersize;

	buffersize=1024;
	buffer=malloc(buffersize);
	if (!buffer)
		return NULL;

	to=buffer;

	while (format && *format)
	{
		size_t restlen;
		size_t done;
		tip_t *akt;

		akt=tab;

		done=TIP_NOTDONE;

		restlen=buffersize - (to-buffer) ;
		if (restlen < 1024)
		{
			char *neubuffer;
			neubuffer=realloc(buffer,buffersize*2);
			if (neubuffer)
			{
				to=neubuffer+(to-buffer);
				restlen+=buffersize;
				buffersize*=2;
				buffer=neubuffer;
			}
		}
		while (akt->magic!='\0' || akt->magicstring)
		{
			int passt=0;
			if (akt->magicstring && !akt->magiclen)
					akt->magiclen=strlen(akt->magicstring);
			if (akt->magicstring 
					&& *akt->magicstring == *format
					&& !strncmp(akt->magicstring,format,akt->magiclen))
				passt=1;
			else if (akt->magic && akt->magic==*format)
				passt=1;
			if (passt)
			{
				const char *next=NULL;
				done=akt->interp(tab, format,&next,to,restlen);
				if (done==TIP_ERROR)
				{
					/* das *könnte* out of memory sein */
					char *neubuffer;
					neubuffer=realloc(buffer,buffersize*2+4096);
					if (neubuffer)
					{
						to=neubuffer+(to-buffer);
						buffersize=buffersize*2+4096;
						buffer=neubuffer;
					}
					else
					{
						free(buffer);
						return NULL;
					}
					done=akt->interp(tab, format,&next,to,restlen);
					if (done==TIP_ERROR)
					{
						free(buffer);
						return NULL;
					}
				}
				if (done!=TIP_NOTDONE)
				{
					format=next;
					to+=done;
					restlen-=done;
					break;
				}
				/* Pech, hat wohl nicht gepasst */
			}
			akt++;
		}
		if (done==TIP_NOTDONE)
		{
			/* nichts hat darauf gepasst, also ein Zeichen anhaengen */
			if (1<restlen)
			{
				*to++=*format;
				*to='\0';
				restlen--;
				format++;
			}
			else
			{
				tip_errtext="buffer shortage"; 
				free(buffer);
				return NULL; 
			}
		}
	} /* while (*format) */
	*to='\0';
	to=strdup(buffer);
	free(buffer);
	return to;
}

int
tip_init(void)
{
	return 0;
}

int 
tip_deinit(void)
{
#ifdef HAVE_PWD_H
	if (pw_used)
	{
		endpwent();
		pw_used=0;
	}
#endif
	return 0;
}


/*
 * ${strip  any text  } -> "any text"
 */
size_t 
tip_strip(tip_t *tab, const char *akt, 
	const char **next, char *to, size_t maxlen)
{
	char *duplicate=NULL;
	char *tmpbuf=NULL;
	const char *ende=NULL;
	const char *start=NULL;
	char *p;
 	size_t lentmp=0;
	int ok=0;
#define MAGIC "${strip "

	PARANOIA_STRING("tip_strip",akt, MAGIC );

	*to='\0';
	start=akt+sizeof(MAGIC)-1;
#undef MAGIC

	ende=tip_getpart(start,"}");
	if (!ende)
		goto abbruch;

	duplicate=malloc(ende-start+1);
	if (!duplicate)
		goto abbruch;
	(void) memcpy(duplicate,start,ende-start);
	duplicate[ende-start]='\0';

	tmpbuf=malloc(1024);
 	if (!tmpbuf)
 		goto abbruch;
 	lentmp=tip_interpret(tab,duplicate,tmpbuf,1024);
 	if (lentmp==TIP_ERROR)
 		goto abbruch;

 	p=tmpbuf+lentmp-2; /* tip_interprert gibt 1 mehr zurück wg 0 */
 	while (isspace(*p))
 		p--;
 	p[1]='\0';
 	p=tmpbuf;
 	while (isspace(*p))
 		p++;
 	lentmp=strlen(p);
 	if (lentmp+1>maxlen)
 		goto abbruch;
 	strcpy(to,p);
 	ok=1;

	*next=ende+1;
abbruch:
	if (tmpbuf)
		free(tmpbuf);
 	if (duplicate)
		free(duplicate);
	if (ok)
		return lentmp; /* Die +1 kommt hinterher von tip_interpret */
	else
		return TIP_ERROR;
}

/*
 * ${literal  % } -> " % "
 */
/* ARGSUSED */
size_t 
tip_literal(tip_t *tab, const char *akt, 
	const char **next, char *to, size_t maxlen)
{
	const char *ende=NULL;
	const char *start=NULL;
#define MAGIC "${literal "

	PARANOIA_STRING("tip_literal",akt, MAGIC );

	*to='\0';
	start=akt+sizeof(MAGIC)-1;
#undef MAGIC

	ende=tip_getpart(start,"}");
	if (!ende)
		return TIP_ERROR;

	if ((size_t) (ende-start+1) > maxlen)
		return TIP_ERROR;

	strncpy(to,start,ende-start);
	to[ende-start]='\0';

	*next=ende+1;
	return ende-start; /* Die +1 kommt hinterher von tip_interpret */
}

/*
 * ${force \\\\} -> \\ -> \
 * doppelte Interpretation
 */
size_t 
tip_force(tip_t *tab, const char *akt, 
	const char **next, char *to, size_t maxlen)
{
	const char *ende=NULL;
	const char *start=NULL;

	char *kopie;
	char *buf;
	size_t len;
#define MAGIC "${force "

	PARANOIA_STRING("tip_force",akt, MAGIC );

	*to='\0';
	start=akt+sizeof(MAGIC)-1;
#undef MAGIC

	ende=tip_getpart(start,"}");
	if (!ende)
		return TIP_ERROR;

	kopie=malloc(ende-start+1);
	if (!kopie)
		return TIP_ERROR;

	strncpy(kopie,start,ende-start);
	kopie[ende-start]='\0';

	if ((size_t) (ende-start+1) > maxlen)
		return TIP_ERROR;

	strncpy(to,start,ende-start);
	to[ende-start]='\0';

	buf=malloc(1024);
	if (!buf)
	{
		free(kopie);
		return TIP_ERROR;
	}
	len=tip_interpret(tab,kopie,buf,1024);
	if (len==TIP_ERROR)
	{
		free(buf);
		buf=malloc(10240);
		if (!buf)
		{
			free(kopie);
			return TIP_ERROR;
		}
		len=tip_interpret(tab,kopie,buf,10240);
		if (len==TIP_ERROR)
		{
			free(buf);
			free(kopie);
			return TIP_ERROR;
		}
	}
	free(kopie);
	/* Ok, erste Interpretation durchgeführt. */
	len=tip_interpret(tab,buf,to,maxlen);
	free(buf);
	if (len==TIP_ERROR)
		return TIP_ERROR;
	*next=ende+1;
	return len-1;
}
