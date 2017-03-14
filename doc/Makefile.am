info_TEXINFOS = conflib.texi
M4=@M4@
conflib_TEXINFOS = tinterp.texi cl_openconf.texi cl_closeconf.texi \
	cl_getstanza.texi cl_putstanza.texi cl_writeconf.texi \
	cl_clearconf.texi cl_free_unk.texi  \
	cl_alist.texi \
	cl_find_var.texi cl_in_array.texi cl_in_warray.texi \
	cl_unalias.texi cl_getvers.texi \
	cl_getconf.texi cl_gcstanza.texi cl_setvar.texi \
	lgpl.texi \
	t_simple.c.texi
VPATH=$(top_srcdir)/doc

# Generate Texinfo files from the C source for the test programs.
%.c.texi: $(top_srcdir)/tests/%.c
	sed -e 's,[{}],@&,g'                \
	-e 's,/\*\(@.*\)\*/,\1,g'           \
	-e 's,/\*  *,/* @r{,g' -e 's,  *\*/,} */,'  \
	-e 's/\(@[a-z][a-z]*\)@{\([^}]*\)@}/\1{\2}/'\
	$< | expand -t 2 > $@.new
	mv -f $@.new $@

%.texi: $(top_srcdir)/src/%.c macros.m4
	$(M4) -P $(srcdir)/macros.m4 $(top_srcdir)/src/$(@:.texi=.c) > $(srcdir)/$@

