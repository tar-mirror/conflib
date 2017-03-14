/* conflib.h : still no description */
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
/* @(#) conflib.h - wr 13.9.91, uo 18.8.93 */
#ifndef __conflib_h
#define __conflib_h


#ifndef __CL_ATTRIB_CONST
#ifdef __GNUC__
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define __CL_ATTRIB_CONST  __attribute__((const))
# define __CL_ATTRIB_UNUSED  __attribute__((unused))
#endif
#endif
#endif

#ifndef __CL_ATTRIB_CONST
#define __CL_ATTRIB_CONST
#endif
#ifndef __CL_ATTRIB_UNUSED
#define __CL_ATTRIB_UNUSED
#endif

#define CL_MALLOCED     0x0001
#define CL_POSTENV      0x0002
#define CL_PREENV       0x0004
	/* internal use only */
#define CL_VAR_NOT_DONE 0x0008
	/* may be saved - this means the value is not a default value 
	 * == read from a file or set from the user
	 * cl_putstanza won't save any variable without this flag set, *except*
	 * unknowns, which are saved anyway 
	 */
#define CL_MAY_SAVE         0x0010
#define CL_RANGE_REGEX      0x0020 /* default! */
#define CL_RANGE_WILDCARD   0x0040
#define CL_WARNING          0x0080

/* describes an enumeration */
typedef struct 
{
	const char *magic;
	long value;
} cl_enum_t;

/* Holds an element of a CL_LIST type variable */
typedef struct cl_list_struct
{
	char *inhalt;
	struct cl_list_struct *next;
} cl_list_t;

/* used to hold unknown variables if CL_GET_UNKNOWN is used */
typedef struct cl_ulist_struct
{
	char *varname;
	char *inhalt;
	struct cl_ulist_struct *next;
} cl_ulist_t;

enum cl_var_typ {	
		CL_STRING,	 /* simple string */
		CL_NUM,      /* long numeric - type long */
		CL_NUMSHORT, /* short numeric - type short */
		CL_BOOLEAN,  /* boolean - type int */
		CL_ARRAY,    /* Array of simple strings */
		CL_ENUM,     /* Enumeration - result type long */
		CL_ALIAS,    /* Ein Alias */
		CL_DUMMY,    /* /dev/null */
		CL_LIST,     /* Eine Liste - result cl_list_t * */
		CL_CHAR,     /* Eine einzelnes Zeichen */
		CL_DIRNAME,  /* Ein Verzeichnisname, der mit / abgeschlossen wird */
		CL_BITFIELD  /* handled much like an enumeration, but unsigned long result */
};

typedef struct
{
	const char *varname; /* name of variable */
	const void *secdata; /* secondary data */
	enum cl_var_typ typ; 
	void *adr;      /* result */
	unsigned long flags;
	const char *rangeexp;
} cl_var_t;

typedef struct cl_stanza_list_t
{
	long pos;
	long lineno;
	char *stanzaname;
	struct cl_stanza_list_t *next;
} cl_stanza_list_t;

typedef struct cl_file_t
{
	FILE *fhandle;
	char *buf;
	size_t buflen;
	int in_comment;
	int in_stanza;
	int report_unknown;
	char *fname;          /* dynamisch alloziert, nicht const char * */
	char *stanza;         /* dynamisch alloziert */
	long  line_nr;
	cl_stanza_list_t *stanzas;
	cl_stanza_list_t *last_ele;
	short in_scanmode;
	short dont_scan;
	short did_backup;     /* if we really write to the file */
	char mode; /* 'r'ead, read + 'w'rite, read 'm'mapped */
	void *interptab; /* "tip_t *", but i don't want to force everybody to include that */
} cl_file_t;

#define CL_MAXARRAY 256

	/* close a configuration file */
void cl_closeconf(cl_file_t *handle);

	/* open a configuration file */
cl_file_t * cl_openconf(const char *fname, const char *mode);

    /* keine include- und include-stanza-Statements ausführen */
#define CL_GET_NOINCLUDE 1
int cl_getstanza(cl_file_t *handle, cl_var_t *tab, const char *stanza,
				 long flags, cl_ulist_t **unknowns);

	/* mit den drei obigen Funktionen realisiert */
int cl_getconf(const char *fname,cl_var_t *);
int cl_getconfstanza(const char *fname,cl_var_t *KonfTab,const char *stanza);

int cl_isinarray (const char **, const char *, int insensitive);
int cl_isinwarray (const char **, const char *);
int cl_getversion (void) __CL_ATTRIB_CONST;
const char * cl_getversionstr (void) __CL_ATTRIB_CONST;
void cl_clearconf (cl_var_t * tab);
cl_var_t *cl_find_confvar(cl_var_t *tab, const char *name);
void cl_free_unknowns(cl_ulist_t *);


void cl_writeconf (FILE * Fp, const cl_var_t * K);
int cl_putstanza(cl_file_t *handle, cl_var_t *tab, const char *stanza,
				  long flags, cl_ulist_t *unknowns);

char ** cl_build_stanza_array(const char *pattern, cl_file_t *conf);
void cl_delete_stanza_array(char **array);

	/* this function is called if a rangecheck fails. It may print
	 * an error message.
	 */
extern void (*cl_rangecheck_hook) 
	(cl_file_t *, cl_var_t *, const char *, const char *);
extern void (*cl_warning_hook) 
	(int, cl_file_t *, cl_var_t *, const char *sourcefile, long sourceline,
		const char *message);
extern void (*cl_error_hook) 
	(int, cl_file_t *, cl_var_t *, const char *sourcefile, long sourceline,
		const char *errormessage);

#endif

