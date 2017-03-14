AUTOMAKE_OPTIONS=dejagnu

DISTFILES=Makefile.am
CLEANFILES=conflib.log conflib.sum site.bak testrun.log testrun.sum tmp.cnf tmpinclude.cnf core core.*
DISTCLEANFILES=site.exp
noinst_PROGRAMS=conflibtest
conflibtest_LDADD=../src/libconf.la
conflibtest_DEPENDENCIES=../src/libconf.la
INCLUDES=-I$(top_srcdir)/src


dist-hook:
	mkdir $(distdir)/config
	mkdir $(distdir)/lib
	mkdir $(distdir)/conflib
	cp -p $(srcdir)/config/*.exp $(distdir)/config
	cp -p $(srcdir)/lib/*.exp $(distdir)/lib
	cp -p $(srcdir)/conflib/*.exp $(distdir)/conflib
	cp -p $(srcdir)/*.cnf $(distdir)/
	cp -p $(srcdir)/*.out $(distdir)/
	cp -p $(srcdir)/conflibtest.c $(distdir)/

