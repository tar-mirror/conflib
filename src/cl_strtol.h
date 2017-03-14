#ifndef _xstrtol_h_
#define _xstrtol_h_ 1

#if STRING_TO_UNSIGNED
# define __xstrtol xstrtoul
# define __strtol strtoul
# define __unsigned unsigned
# define __ZLONG_MAX ULONG_MAX
#else
# define __xstrtol xstrtol
# define __strtol strtol
# define __unsigned /* empty */
# define __ZLONG_MAX LONG_MAX
#endif

#undef __P
#if defined (__STDC__) && __STDC__
#define	__P(x) x
#else
#define	__P(x) ()
#endif

enum strtol_error
  {
    LONGINT_OK, LONGINT_INVALID, LONGINT_INVALID_SUFFIX_CHAR, LONGINT_OVERFLOW, LONGINT_INT_OVERFLOW
  };
typedef enum strtol_error strtol_error;

strtol_error
  cl_strtol __P ((const char *s, char **ptr, int base,
		  __unsigned long int *val, const char *valid_suffixes));

#define CL_STRTOL_WARNING(err,file,var,str,argument_type_string) \
	do { switch((err)) { \
	     case LONGINT_OK: break; \
	     case LONGINT_INVALID: \
	     	cl_warning(CL_WARN(file,var),"invalid %s `%s'", \
	     	         (argument_type_string), (str)); \
	     	break; \
	     case LONGINT_INVALID_SUFFIX_CHAR: \
	     	cl_warning(CL_WARN(file,var),"invalid character following %s `%s'", \
	     	         (argument_type_string), (str)); \
	     	break; \
	     case LONGINT_OVERFLOW: \
	     	cl_warning(CL_WARN(file,var),"%s `%s' larger than maximum long int", \
	     	         (argument_type_string), (str)); \
	     	break; \
	     case LONGINT_INT_OVERFLOW: \
	     	cl_warning(CL_WARN(file,var),"%s `%s' larger than maximum int", \
	     	         (argument_type_string), (str)); \
	     	break; \
	     } \
	 } while (0)

#endif /* _xstrtol_h_ */
