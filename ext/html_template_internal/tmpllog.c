/* -*- c -*- 
 * File: log.c
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Thu Sep  1 17:18:16 2005
 *
 * $Id$
 */

/* based on FFmpeg av_log API */

#include "pstring.h"
#include "tmpllog.h"
#include "prostate.h"
#include <stdio.h>
#include <string.h>

static int tmpl_log_level = TMPL_LOG_ERROR;

static void 
tmpl_log_default_callback(void* ptr, int level, const char* fmt, va_list vl)
{
    static int print_prefix=1;
    struct tmplpro_state* state= ptr ? (struct tmplpro_state*)ptr : NULL;
    if(level>tmpl_log_level) return;
    /*#undef fprintf*/
    if(print_prefix && state) {
      _tmpl_log_state(state,level);
    }
    /*#define fprintf please_use_tmpl_log*/
        
    print_prefix= strstr(fmt, "\n") != NULL;
        
    vfprintf(stderr, fmt, vl);
}

static void (*tmpl_log_callback)(void*, int, const char*, va_list) = tmpl_log_default_callback;

TMPLPRO_LOCAL
void 
tmpl_log(void* state, int level, const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    tmpl_vlog(state, level, fmt, vl);
    va_end(vl);
}

TMPLPRO_LOCAL
void 
tmpl_vlog(void* state, int level, const char *fmt, va_list vl)
{
    tmpl_log_callback(state, level, fmt, vl);
}

TMPLPRO_LOCAL
int 
tmpl_log_get_level(void)
{
    return tmpl_log_level;
}

TMPLPRO_LOCAL
void 
tmpl_log_set_level(int level)
{
    tmpl_log_level = level;
}

TMPLPRO_LOCAL
void 
tmpl_log_set_callback(void (*callback)(void*, int, const char*, va_list))
{
    tmpl_log_callback = callback;
}
