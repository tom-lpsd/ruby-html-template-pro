#ifndef _PMISCDEF_H
#define _PMISCDEF_H	1

/* snprintf MS VC++ support;
 * thanks to Viacheslav Sheveliov <slavash@aha.ru>
 */
#ifdef _MSC_VER
#  define snprintf _snprintf
#endif

/* printf ptrdiff_t format modifier */
#if __STDC_VERSION__ >= 199901L
# define TO_PTRDIFF_T(X) (X)
# define MOD_TD "%td"
#elif defined _MSC_VER
# define TO_PTRDIFF_T(X) (X)
# define MOD_TD "%Id"
#else
# define TO_PTRDIFF_T(X) ((long) (X))
# define MOD_TD "%ld"
#endif

#if ! HAVE_STRDUP && ! defined strdup
# if HAVE__STRDUP
#  define strdup _strdup
# else
#  define strdup(str) strcpy(malloc(strlen(str) + 1), str)
# endif
#endif

#define COMPILE_TIME_ASSERT(x) \
void __cta_proto__(int __cta_foo__[(x) ? 1 : -1])

#endif /* pmiscdef.h */

/*
 * Local Variables:
 * mode: c
 * End:
 */
