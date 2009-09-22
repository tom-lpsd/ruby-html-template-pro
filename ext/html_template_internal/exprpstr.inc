/* -*- c -*- 
 * File: pstring.h
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Fri Jul  1 20:11:51 2005
 *
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include "pstring.h"
#include "tmpllog.h"
#include "exprval.h"
#include "pmiscdef.h" /*for snprintf */

static
PSTRING 
double_to_pstring (double number, char buffer[], size_t bufsize) {
  size_t len=0;
  size_t tmplen=0;
  PSTRING retval;
  snprintf(buffer,bufsize,"%f",number);
  len=strlen(buffer);
  tmplen=len;
  /* removing trailing 0 as 2.00000... */
  while (buffer[tmplen-1]=='0' && tmplen-->0); 
  if (buffer[tmplen-1]=='.') {
    tmplen--;
    len=tmplen;
  }
  retval.begin=buffer;
  retval.endnext=buffer+len;
  return retval;
}

static
PSTRING 
int_to_pstring (EXPR_int64 number, char buffer[], size_t bufsize) {
  size_t len=0;
  PSTRING retval;
  snprintf(buffer, bufsize,"%" EXPR_PRId64 , number);
  len=strlen(buffer);
  retval.begin=buffer;
  retval.endnext=buffer+len;
  return retval;
}

static
int 
pstring_ge(PSTRING a, PSTRING b) {
  const char* in_a=a.begin;
  const char* in_b=b.begin;
  while (in_a<a.endnext && in_b < b.endnext && *in_a++==*in_b++);
  if ((in_a==a.endnext && in_b==b.endnext) || *(--in_a) >= *(--in_b) ) return 1; else return 0;
}

static
int 
pstring_le(PSTRING a, PSTRING b) {
  const char* in_a=a.begin;
  const char* in_b=b.begin;
  while (in_a<a.endnext && in_b < b.endnext && *in_a++==*in_b++);
  if ((in_a==a.endnext && in_b==b.endnext) || *(--in_a) <= *(--in_b) ) return 1; else return 0;
}

static
int 
pstring_ne(PSTRING a, PSTRING b) {
  const char* in_a=a.begin;
  const char* in_b=b.begin;
  while (in_a<a.endnext && in_b < b.endnext && *in_a++==*in_b++);
  if (in_a==a.endnext && in_b==b.endnext && *(--in_a) == *(--in_b)) return 0; else return 1;
}

static
int 
pstring_eq(PSTRING a, PSTRING b) {
  const char* in_a=a.begin;
  const char* in_b=b.begin;
  while (in_a<a.endnext && in_b < b.endnext && *in_a++==*in_b++);
  if (in_a==a.endnext && in_b==b.endnext && *(--in_a) == *(--in_b)) return 1; else return 0;
}

static
int 
pstring_gt(PSTRING a, PSTRING b) {
  const char* in_a=a.begin;
  const char* in_b=b.begin;
  while (in_a<a.endnext && in_b < b.endnext && *in_a++==*in_b++);
  if ((in_b==b.endnext && in_a!=a.endnext)
      || (*(--in_a) > *(--in_b)) ) return 1; else return 0;
}

static
int 
pstring_lt(PSTRING a, PSTRING b) {
  const char* in_a=a.begin;
  const char* in_b=b.begin;
  while (in_a<a.endnext && in_b < b.endnext && *in_a++==*in_b++);
  if ((in_b!=b.endnext && in_a==a.endnext)
      ||  *(--in_a) < *(--in_b) ) return 1; else return 0;
}

static
int 
re_notlike(PSTRING a, PSTRING b) {
  return ! re_like(a,b);
}

#ifndef HAVE_PCRE
static
int 
re_like(PSTRING a, PSTRING b) {
    tmpl_log(TMPL_LOG_ERROR," (sorry, Stanislav Yadykin regexp extension is disabled at compile time) \n");
  return 0;
}
#else
#include <pcre.h>
static
int 
re_like(PSTRING a, PSTRING b) {
  pcre* re;
  int ovector[30];
  int rc, erroffset;
  const char* error;
  const char* subject=a.begin;
  int subject_length=(int)(a.endnext-a.begin);
  char* pattern=(char*)malloc(b.endnext-b.begin);
  strncpy(pattern, b.begin, (b.endnext-b.begin));
  *(pattern+(b.endnext-b.begin))=0;
  re = pcre_compile(pattern, 0, &error, &erroffset, NULL); /* default character set */
  free(pattern);
  if (re==NULL) {
    tmpl_log(TMPL_LOG_ERROR, "PCRE compilation failed at offset %d: %s\n",
      erroffset, error);
    return 0;
  }
  rc=pcre_exec(re, NULL, subject, subject_length, 0, 0, ovector, 30);
  return (rc<0)?0:1;
}
#endif
