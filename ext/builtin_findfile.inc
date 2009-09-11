/* 
 * File: builtin_findfile.c
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Tue Jul 14 22:47:11 2009
 */

#include <ctype.h> /* for isalpha */

/* TODO: support CYGWIN
 * see
 * http://sourceware.org/autobook/autobook/autobook_249.html
 */

#if defined (_WIN32) || defined (WIN32)
# define IS_FILE_SEP(X) ((X=='/') || (X=='\\'))
#else
# define IS_FILE_SEP(X) (X=='/')
#endif

static int _ff_exists(const char* path) {
  FILE *file_p;
  file_p = fopen(path, "r");
  if (file_p) {
      fclose(file_p);
      //tmpl_log(NULL,TMPL_LOG_ERROR,"_ff_exists: found [%s]\n",path);
      return 1;
    }
  //tmpl_log(NULL,TMPL_LOG_ERROR,"_ff_exists: not found [%s]\n",path);
  return 0;
}

/* lame dirname implementation */
static PSTRING _ff_dirname(const char* path) {
  PSTRING retval={(char*)path,(char*)path};
  char c=0;
  if (path!=NULL) retval.endnext += strlen(path);
  else return retval;
  while (retval.endnext > retval.begin && (c=*(--retval.endnext)) && ! IS_FILE_SEP(c));
  return retval;
}
/*
Windows Relative Paths

For functions that manipulate files, the file names can be relative to the current directory. A file name is relative to the current directory if it does not begin with one of the following:

    * A UNC name of any format.
    * A disk designator with a backslash, for example "C:\".
    * A backslash, for example, "\directory").
 */

/* remember about \\?\ and \\?\UNC\ prefixes on WIN platform.
 * see "File Names, Paths, and Namespaces"
 * http://msdn.microsoft.com/en-us/library/aa365247(VS.85).aspx
 */
int _ff_is_absolute(const char * filename) {
  unsigned char c0 = *filename;
#if defined (_WIN32) || defined (WIN32)
  unsigned char c1;
#endif
  if ('\0' == c0) return 0;
  /* \\?\ and \\?\UNC\ prefixes are included too */
  if (IS_FILE_SEP(c0)) return 1;
#if defined (_WIN32) || defined (WIN32)
  c1 = *(++filename);
  if ('\0' == c1) return 0;
  if (isalpha(c0) && ':'==c1 && '\\'==*filename) return 1;
#endif
  return 0;
}

#if defined (_WIN32) || defined (WIN32)
int _ff_is_win_fully_qualified_path(const char * filename) {
  unsigned char c0 = *filename;
  unsigned char c1;
  unsigned char c2;
  if ('\0' == c0) return 0;
  c1 = *(++filename);
  if ('\0' == c1) return 0;
  c2 = *(++filename);
  /* \\?\ and \\?\UNC\ prefixes are included too */
  if (isalpha(c0) && ':'==c1 && '\\'==c2) return 1;
  if ('\\'==c0 && '\\'==c1 && '?'==c2 && '\\'==*filename) return 1;
  return 0;
}
#endif

#define _ff_canonical_path(X) (X)

static PSTRING _shift_back_pstring_at(PSTRING buf, char* pos, long shift) {
  if (pos >= buf.begin && (pos+shift) <=buf.endnext) {
    buf.endnext -= shift;
    while (pos<buf.endnext) {
      *pos=*(pos+shift);
      pos++;
    }
  }
  *buf.endnext='\0';
  return buf;
}

static PSTRING _filepath_remove_multiple_slashes(PSTRING buf) {
  char* pos = buf.begin;
#if defined (_WIN32) || defined (WIN32)
  /* due to \\?\ and \\?\UNC\ prefixes on WIN platform.
   * see "File Names, Paths, and Namespaces"
   * http://msdn.microsoft.com/en-us/library/aa365247(VS.85).aspx
   * we skip first 2 bytes of path
   */
  if (((buf.endnext-pos)>1) && ('\\'==*pos && '\\'==*(pos+1))) pos += 2;
#endif
  while (pos<buf.endnext-1) {
    if (IS_FILE_SEP(*pos) && IS_FILE_SEP(*(pos+1))) buf=_shift_back_pstring_at(buf, pos, 1);
    else pos++;
  }
  return buf;
}

static const char* _ff_canonical_path_from_buf(PSTRING buf) {
  char* pos;
  char* prev_slash_next;
  char* slash_begin;
  /*  /./ <-- shift -2 */
  pos = buf.begin;
  while (pos<buf.endnext-2) {
    if (IS_FILE_SEP(*pos) && ('.'==*(pos+1)) && IS_FILE_SEP(*(pos+2))) buf=_shift_back_pstring_at(buf, pos, 2);
    pos++;
  }

  /*  // <-- shift -1 */
  buf=_filepath_remove_multiple_slashes(buf);

  /*   /.* /../ shift (scan from prevslash to slash back) */
  pos = buf.begin;
  slash_begin = buf.begin;
#if defined (_WIN32) || defined (WIN32)
  /* check for C: */
  if (((buf.endnext-pos)>1) && isalpha(*pos) && ':'==*(pos+1)) {
    pos += 2;
    slash_begin += 2;
  }
#endif
  prev_slash_next = slash_begin;
  while (pos<buf.endnext-3) {
    /*printf("debug: %s pos=%c[%ld] fsn=%c[%ld]\n",buf.begin,*pos, pos-buf.begin, *prev_slash_next,prev_slash_next-buf.begin);*/
    if (IS_FILE_SEP(*pos)) {
      if (('.'==*(pos+1)) && ('.'==*(pos+2)) && IS_FILE_SEP(*(pos+3))) {
	/*printf("debug: do shift pos=%ld fsn=%ld shift=%ld\n", pos-buf.begin, prev_slash_next-buf.begin, pos-prev_slash_next+4);*/
	if (pos == prev_slash_next && prev_slash_next==slash_begin) {
	  /* begining of the string ("/../") -> leave one slash */
	  buf=_shift_back_pstring_at(buf, prev_slash_next, pos-prev_slash_next+3);
	  pos=prev_slash_next-1;/* 1 to compensate pos++ */
	} else {
	  buf=_shift_back_pstring_at(buf, prev_slash_next, pos-prev_slash_next+4);
	  pos=prev_slash_next-2;/* 2 to compensate / and pos++ */
	  /* 2 to step back slashnext char and 'slash' char, if any  */
	  if (prev_slash_next>slash_begin) prev_slash_next--;
	  if (prev_slash_next>slash_begin) prev_slash_next--;
	}
	/* old prev_slash_next now current, so we need to recalculate it  */
	/* first find a 'slash' char */
	while (!IS_FILE_SEP(*prev_slash_next) && prev_slash_next>=slash_begin) prev_slash_next--;
	if (prev_slash_next>slash_begin) prev_slash_next++;/* step next to slash */
      } else {
	prev_slash_next=pos+1;
      }
    }
    pos++;
  }

  /*  // <-- shift -1 */
  buf=_filepath_remove_multiple_slashes(buf);

  /* offset 0: if ./ shift -2 */
  if ((buf.endnext-buf.begin)<2) return buf.begin;
  pos = buf.begin;
  if (('.'==*pos) && IS_FILE_SEP(*(pos+1))) buf=_shift_back_pstring_at(buf, pos, 2);
  return buf.begin;
}

static PSTRING _ff_add_pstr_to_buffer(PSTRING buf, PSTRING pstr) {
  PSTRING ret = buf;
  const char* s;
  //tmpl_log(NULL,TMPL_LOG_ERROR,"_ff_add_pstr_to_buffer: called as [%p,%p]+[%p,%p]\n",buf.begin,buf.endnext, pstr.begin,pstr.endnext);
  for (s=pstr.begin;s<pstr.endnext;s++) {*(ret.endnext++)=*s;}
  //tmpl_log(NULL,TMPL_LOG_ERROR,"_ff_add_pstr_to_buffer: ret = [%p,%p]\n",ret.begin,ret.endnext);
  return ret;
}
static PSTRING _ff_add_str_to_buffer(PSTRING buf, const char* str) {
  PSTRING ret = buf;
  const char* s=str;
  //tmpl_log(NULL,TMPL_LOG_ERROR,"_ff_add_str_to_buffer: called as [%p,%p]+[%s]\n",buf.begin,buf.endnext, str);
  while ('\0'!=*s) {*(ret.endnext++)=*s++;}
  //tmpl_log(NULL,TMPL_LOG_ERROR,"_ff_add_str_to_buffer: ret = [%p,%p]\n",ret.begin,ret.endnext);
  return ret;
}
static PSTRING _ff_add_sep_to_buffer(PSTRING buf) {
  PSTRING ret = buf;
  if (ret.endnext>ret.begin && IS_FILE_SEP(*(ret.endnext-1))) return ret;
  *(ret.endnext++)='/';
  return ret;
}
static PSTRING _ff_add_0_to_buffer(PSTRING buf) {
  PSTRING ret = buf;
  *(ret.endnext++)='\0';
  return ret;
}
static const char* _find_file (struct tmplpro_param* param, const char* filename, PSTRING extra_dir) {
  // TODO: finish it
  char* HTML_TEMPLATE_ROOT = getenv("HTML_TEMPLATE_ROOT");
  size_t HTML_TEMPLATE_ROOT_length=0;
  size_t buffsize=0;
  char** pathlist=param->path;
  PSTRING pbuf_begin, filepath;

  /* first check for a full path */
  if (_ff_is_absolute(filename) && _ff_exists(filename)) return _ff_canonical_path(filename);

#if defined (_WIN32) || defined (WIN32)
  /* no sense of prefixing C:\ or \\?\ */
  if (_ff_is_win_fully_qualified_path(filename)) return NULL;
#endif

  if (HTML_TEMPLATE_ROOT!=NULL) HTML_TEMPLATE_ROOT_length=strlen(HTML_TEMPLATE_ROOT);
  if (pathlist!=NULL) {
    while (NULL!=*pathlist) {
      size_t pathentrylen=strlen(*pathlist);
      if (buffsize<pathentrylen) buffsize=pathentrylen;
      pathlist++;
    }
  }
  /* bufsize is max possible length path of path considered
   * min is max_len(foreach pathlist)+HTML_TEMPLATE_ROOT_length+strlen(filename)+len(extra_dir)+1) 
   * but we malloc an extra space to avoid  frequent reallocing
   */
  buffsize+=HTML_TEMPLATE_ROOT_length+strlen(filename)+(extra_dir.endnext-extra_dir.begin)+4+1; /* 4 - for slashes */
  if (0==pbuffer_size(&param->builtin_findfile_buffer)) {
    pbuffer_init_as(&param->builtin_findfile_buffer, 3*buffsize);
  } else {
    pbuffer_resize(&param->builtin_findfile_buffer, buffsize);
  }
  pbuf_begin.begin=pbuffer_string(&param->builtin_findfile_buffer);
  pbuf_begin.endnext=pbuf_begin.begin;

  /* try the extra_path if one was specified */
  if (extra_dir.begin!=NULL) {
    filepath=_ff_add_pstr_to_buffer(pbuf_begin,extra_dir);
    filepath=_ff_add_sep_to_buffer(filepath);
    filepath=_ff_add_str_to_buffer(filepath,filename);
    filepath=_ff_add_0_to_buffer(filepath);
    if (_ff_exists(filepath.begin)) return _ff_canonical_path_from_buf(filepath);
  }

  /* try pre-prending HTML_Template_Root */
  if (HTML_TEMPLATE_ROOT!=NULL) {
    filepath=_ff_add_str_to_buffer(pbuf_begin,HTML_TEMPLATE_ROOT);
    filepath=_ff_add_sep_to_buffer(filepath);
    filepath=_ff_add_str_to_buffer(filepath,filename);
    filepath=_ff_add_0_to_buffer(filepath);
    if (_ff_exists(filepath.begin)) return _ff_canonical_path_from_buf(filepath);
  }

  /* try "path" option list.. */
  pathlist=param->path;
  if (pathlist!=NULL) {
    while (NULL!=*pathlist) {
      //tmpl_log(NULL,TMPL_LOG_ERROR,"try 'path' option list..: looking in [%s]\n",*pathlist);
      filepath=_ff_add_str_to_buffer(pbuf_begin,*pathlist);
      filepath=_ff_add_sep_to_buffer(filepath);
      filepath=_ff_add_str_to_buffer(filepath,filename);
      filepath=_ff_add_0_to_buffer(filepath);
      if (_ff_exists(filepath.begin)) return _ff_canonical_path_from_buf(filepath);
      pathlist++;
    }
  }

  /* try even a relative path from the current directory...*/
  if (_ff_exists(filename)) return _ff_canonical_path(filename);

  /* try "path" option list with HTML_TEMPLATE_ROOT prepended... */
  if (HTML_TEMPLATE_ROOT!=NULL) {
    pathlist=param->path;
    if (pathlist!=NULL) {
      while (NULL!=*pathlist) {
	filepath=_ff_add_str_to_buffer(pbuf_begin,HTML_TEMPLATE_ROOT);
	filepath=_ff_add_sep_to_buffer(filepath);
	filepath=_ff_add_str_to_buffer(filepath,*pathlist);
	filepath=_ff_add_sep_to_buffer(filepath);
	filepath=_ff_add_str_to_buffer(filepath,filename);
	filepath=_ff_add_0_to_buffer(filepath);
	if (_ff_exists(filepath.begin)) return _ff_canonical_path_from_buf(filepath);
	pathlist++;
      }
    }
  }

  return NULL;
} 

static const char* BACKCALL stub_find_file_func(ABSTRACT_FINDFILE* param,const char* filename, const char* last_visited_file) {
  const char* filepath;
  PSTRING extra_path ={NULL,NULL};

  if (filename == last_visited_file) tmpl_log(NULL,TMPL_LOG_ERROR,"built-in find_file: internal error: buffer clash for %s\n",filename);

  // look for the included file...
  if (last_visited_file!=NULL && ! ((struct tmplpro_param*) param)->search_path_on_include) {
    extra_path = _ff_dirname(last_visited_file);
  }
  filepath = _find_file((struct tmplpro_param*)param,filename,extra_path);
  if (filepath==NULL) {
    char** path=((struct tmplpro_param*)param)->path;
    tmpl_log(NULL,TMPL_LOG_ERROR,"built-in find_file: can't find file %s", filename);
    if (NULL!=last_visited_file) tmpl_log(NULL,TMPL_LOG_ERROR," (included from %s)", last_visited_file);
    if (NULL!=path) {
      tmpl_log(NULL,TMPL_LOG_ERROR," with path = [");
      while (NULL!=*path) {
	tmpl_log(NULL,TMPL_LOG_ERROR," '%s'",*path);
	path++;
      }
      tmpl_log(NULL,TMPL_LOG_ERROR," ]");
    } else {
      tmpl_log(NULL,TMPL_LOG_ERROR," with empty path list");
    }
    tmpl_log(NULL,TMPL_LOG_ERROR,"\n");
    return NULL;
  } else {
    return filepath;
  }
}

/*
 *  Local Variables:
 *  mode: c
 *  End:
 */
