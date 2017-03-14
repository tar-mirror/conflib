dnl
dnl prüft ob fcntl(F_SETLK) broken ist.
dnl
AC_DEFUN(UO_FUNC_BROKEN_F_SETLK, [
	AC_MSG_CHECKING([whether fcntl(F_SETLK) is broken])
	AC_CACHE_VAL(uo_cv_setlk_broken,
	AC_TRY_RUN([
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined(F_WRLCK) && defined(F_RDLCK) && defined(F_SETLK)
#define DO_LOCK
#endif

#ifndef DO_LOCK
#error 1
#endif

int main(void)
{
	int fd;
	struct flock lock;
	fd=open("conftest.tmp23",O_RDWR|O_CREAT|O_TRUNC,
		(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) );
	if (fd<0) {
		fprintf(stderr,"open: %s\n",strerror(errno));
		exit(1);
	}
	lock.l_type = F_RDLCK;
	lock.l_start = 0;
	lock.l_len = 1;
	lock.l_whence = SEEK_SET;
	if (fcntl (fd, F_SETLK, &lock)) {
		fprintf(stderr,"fcntl(F_SETLK): %s\n",strerror(errno));
		unlink("conftest.tmp23");
		exit(1);
	}
	unlink("conftest.tmp23");
	exit(0);
}
	],uo_cv_setlk_broken=no,uo_cv_setlk_broken=yes,
	AC_MSG_WARN([considering fcntl(F_SETLK) as broken])
    uo_cv_setlk_broken=yes ))dnl end of try_run and cache_val
	AC_MSG_RESULT($uo_cv_setlk_broken)
	if test $uo_cv_setlk_broken = yes; then
		AC_DEFINE(HAVE_BROKEN_F_SETLK)
		AC_SUBST(HAVE_BROKEN_F_SETLK)
	fi
]) dnl Ende von UO_BROKEN_F_SETLK
