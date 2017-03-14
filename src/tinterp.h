/* tinterp.h : still no description */
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
             Drosselstraße 2
             47055 Duisburg
             Germany
    Contact2: godot@uni-paderborn.de, Frank Baumgart @ PB3 (mausnet)
       (might be the wrong person except for offline)

    Contact3: MausNet Newsgroups Quark & Q-Sysops.
*/
#ifndef TINTERP_H
#define TINTERP_H

/* 
 * @(#) tinterp.h -- uo, 23.5.94 
 * Prototypen und Definitionen für die Textinterpretationsroutinen
 */

#define TIP_NOTDONE 	((size_t) -1)
#define TIP_ERROR		((size_t) -2)

extern const char *tip_errtext;

struct tip_t;

typedef size_t (*tip_interpret_fn_t) (struct tip_t *tab, const char *this, 
	const char **next, char *to, size_t maxlen);

typedef struct tip_t
{
	char magic;
	const char *magicstring;
	size_t magiclen;            /* mit 0 zu initialisieren! Optimierung */
	tip_interpret_fn_t interp;
} tip_t;

	/* "einfache" Interpretationen */
size_t tip_dollar(tip_t *, const char *, const char **, char *, size_t);
size_t tip_tilde(tip_t *, const char *, const char **, char *, size_t);
size_t tip_backslash(tip_t *, const char *, const char **, char *, size_t);
	/* Funktionen */
size_t tip_conditional(tip_t *, const char *, const char **, char *, size_t);
size_t tip_strip(tip_t *, const char *, const char **, char *, size_t);
size_t tip_macro(tip_t *, const char *, const char **, char *, size_t);
size_t tip_defmacro(tip_t *, const char *, const char **, char *, size_t);
size_t tip_undefmacro(tip_t *, const char *, const char **, char *, size_t);
size_t tip_literal(tip_t *, const char *, const char **, char *, size_t);
size_t tip_force(tip_t *, const char *, const char **, char *, size_t);

	/* Hilfsfunktion */
char *tip_getpart(const char *start, const char *endchars);

size_t tip_interpret(tip_t *tab, const char *format, char *buffer, size_t buflen);
char * tip_ainterpret(tip_t *tab, const char *format);
int tip_defmacro_intern(const char *name, const char *value);

int tip_init(void);
int tip_deinit(void);

/*

$Id: tinterp.h,v 1.1.1.1 1998/07/09 21:57:52 uwe Exp $
$Date: 1998/07/09 21:57:52 $
$Name:  $
$Log: tinterp.h,v $
Revision 1.1.1.1  1998/07/09 21:57:52  uwe
import

Revision 1.1.1.2  1997/11/02 17:11:39  uwe
import nachträglich

Revision 1.1.1.1  1996/06/15 08:55:27  uwe
Imported sources

Revision 1.1.1.1  1996/02/18 22:45:23  uwe
Import Sourcen

Revision 1.4  1995/05/08 18:37:35  quark
nur aufgeräumt, keine Änderungen

Revision 1.3  1995/05/08 12:37:22  quark
tip_defmacro: neu
tip_undefmacro: neu
tip_literal: neu
tip_force: neu
tip_defmacro (alt) heißt nun tip_defmacro_intern
tip_ainterpret: neu

Revision 1.2: RCS-Ids ergänzt.
*/

#endif
