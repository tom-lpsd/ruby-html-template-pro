/* -*- c -*- 
 * File: pstring.h
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Fri Jul  1 20:11:51 2005
 *
 * $Id$
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pbuffer.h"
#include "pstring.h"

static
PSTRING
lowercase_pstring (pbuffer* pBuffer ,PSTRING pstring) {
  size_t size=pstring.endnext-pstring.begin;
  char* buf=pbuffer_resize(pBuffer, size+1);
  char* inbuf=buf;
  char* i=pstring.begin;
  PSTRING retval;
  while (i<pstring.endnext) {
    *inbuf++=tolower(*i++);
  }
  *inbuf=0;
  retval.begin=buf;
  retval.endnext=buf+size;
  return retval;
}

/*
static
void 
lowercase_pstring_inplace (PSTRING pstring) {
  char* i=pstring.begin;
  while (i<pstring.endnext) {
    *i=tolower(*i);
    i++;
  }
}
*/

static
int
is_pstring_true (PSTRING s) {
  if (s.begin == NULL || s.begin == s.endnext) return 0;
  if (1==s.endnext-s.begin) {
    if (*(s.begin)=='0') return 0; else return 1;
  } else if (3==s.endnext-s.begin) {
    if ('0'==*(s.begin) && '.'==*(s.begin+1) && '0'==*(s.begin+2)) return 0; else return 1;
  } else return 1;
}

static 
PSTRING 
escape_pstring (pbuffer* StrBuffer, PSTRING pstring, int escapeopt) {
  char* buf=pbuffer_resize(StrBuffer, 2*(pstring.endnext-pstring.begin+1));
  char* curpos=pstring.begin;
  size_t offset=0;
  size_t buflen=pbuffer_size(StrBuffer);
  PSTRING retval;
  switch (escapeopt) {
  case HTML_TEMPLATE_OPT_ESCAPE_HTML:
    while (curpos<pstring.endnext) {
      char curchar=*curpos++;
      int bufdelta=1;
      if (offset>=buflen) {
	buf=pbuffer_resize(StrBuffer, 2*offset);
	buflen=pbuffer_size(StrBuffer);
      }
      switch (curchar) {
	/* straight from the CGI.pm bible. (HTML::Template) */
      case '&' : bufdelta=5; strncpy(buf+offset, "&amp;", bufdelta);break;
      case '"' : bufdelta=6; strncpy(buf+offset, "&quot;",bufdelta);break;
      case '>' : bufdelta=4; strncpy(buf+offset, "&gt;",  bufdelta);break;
      case '<' : bufdelta=4; strncpy(buf+offset, "&lt;",  bufdelta);break;
      case '\'': bufdelta=5; strncpy(buf+offset, "&#39;", bufdelta);break;
      default: *(buf+offset)=curchar;
      }
      offset+=bufdelta;
    }
    break;
  case HTML_TEMPLATE_OPT_ESCAPE_JS:
    while (curpos<pstring.endnext) {
      char curchar=*curpos++;
      int bufdelta=1;
      if (offset>=buflen) {
	buf=pbuffer_resize(StrBuffer, 2*offset);
	buflen=pbuffer_size(StrBuffer);
      }
      switch (curchar) {
      case '\\' : bufdelta=2; strncpy(buf+offset, "\\\\", bufdelta);break;
      case '"'  : bufdelta=2; strncpy(buf+offset, "\\\"",bufdelta);break;
      case '\'' : bufdelta=2; strncpy(buf+offset, "\\'",bufdelta);break;
      case '\n' : bufdelta=2; strncpy(buf+offset, "\\n",bufdelta);break;
      case '\r' : bufdelta=2; strncpy(buf+offset, "\\r",bufdelta);break;
      default: *(buf+offset)=curchar;
      }
      offset+=bufdelta;
    }
    break;
  case HTML_TEMPLATE_OPT_ESCAPE_URL: 
    while (curpos<pstring.endnext) {
      unsigned char curchar=*curpos++;
      int bufdelta=1;
      if (offset>=buflen) {
	buf=pbuffer_resize(StrBuffer, 2*offset);
	buflen=pbuffer_size(StrBuffer);
      }
      /* 
       * # do the translation (RFC 2396 ^uric)
       * s!([^a-zA-Z0-9_.\-])!sprintf('%%%02X', $_)
       */
      if ((curchar>='a' && curchar<='z') ||
	  (curchar>='A' && curchar<='Z') ||
	  (curchar>='0' && curchar<='9') ||
	  curchar=='_' || curchar=='.' || curchar=='\\' || curchar=='-'
	  ) 
	*(buf+offset)=curchar;
      else {
	bufdelta=3; sprintf(buf+offset,"%%%.2X",(int) curchar);
      }
      offset+=bufdelta;
    }
    break;
  default : return pstring;
  }
  retval.begin=buf;
  retval.endnext=buf+offset;
  return retval;
}
