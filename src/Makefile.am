libconf_la_SOURCES= cl_alist.c cl_assign.c cl_clearconf.c cl_closeconf.c \
	cl_find_var.c cl_free_unk.c cl_gcstanza.c cl_getconf.c cl_getstanza.c \
	cl_getvers.c cl_in_array.c cl_in_warray.c cl_lib.c cl_macro.c \
	cl_openconf.c cl_put_one.c cl_putstanza.c cl_setvar.c \
	cl_tinterp.c cl_unalias.c \
	cl_warning.c cl_writeconf.c \
	strcasecmp.c strdup.c strncasecmp.c strtol.c wildmat.c cl_strtol.c
lib_LTLIBRARIES = libconf.la
libconf_la_LDFLAGS = -version-info @SOVERSION@ -rpath $(libdir)

noinst_HEADERS=intern.h ownstd.h cl_strtol.h
noinst_PROGRAMS=libtest
libtest_LDADD=./libconf.la
libtest_DEPENDENCIES=./libconf.la
include_HEADERS=conflib.h tinterp.h
EXTRA_DIST=.cvsignore

