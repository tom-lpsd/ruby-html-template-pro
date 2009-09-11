/* -*- c -*- 
 * File: log.h
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Thu Sep  1 17:17:37 2005
 *
 * $Id$
 */

/* based on FFmpeg av_log API */

#include "pabidecl.h"
#include <stdarg.h>

#define TMPL_LOG_QUIET -1
#define TMPL_LOG_ERROR 0
#define TMPL_LOG_INFO 1
#define TMPL_LOG_DEBUG 2
#define TMPL_LOG_DEBUG2 3

#ifdef __GNUC__
extern TMPLPRO_LOCAL void tmpl_log(void*, int level, const char *fmt, ...) __attribute__ ((__format__ (__printf__, 3, 4)));
#else
extern TMPLPRO_LOCAL void tmpl_log(void*, int level, const char *fmt, ...);
#endif

extern TMPLPRO_LOCAL void tmpl_vlog(void*, int level, const char *fmt, va_list);
extern TMPLPRO_LOCAL  int tmpl_log_get_level(void);
extern TMPLPRO_LOCAL void tmpl_log_set_level(int);
extern TMPLPRO_LOCAL void tmpl_log_set_callback(void (*)(void*, int, const char*, va_list));

