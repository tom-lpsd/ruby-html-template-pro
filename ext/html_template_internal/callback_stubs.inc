/* 
 * File: callback_stubs.c
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Tue Jul 14 22:47:11 2009
 */

/* writer_functype stub */
/* endnext points on next character to end of interval as in c++ */
static void BACKCALL stub_write_chars_to_stdout (ABSTRACT_WRITER* none,const char* begin, const char* endnext) {
  if (endnext==begin) return;
  /* register const char* i;
  for (i=begin; i<endnext; i++) putchar(*i); */
  if (0==fwrite(begin, sizeof(char), endnext-begin, stdout)) {
    tmpl_log(NULL,TMPL_LOG_ERROR,"find_file_func stub: can't fwrite to stdout\n");
  }
}

static ABSTRACT_USERFUNC* BACKCALL stub_is_expr_userfnc_func (ABSTRACT_FUNCMAP* af, PSTRING name) {
  tmpl_log(NULL,TMPL_LOG_ERROR,"is_expr_userfnc_func stub: EXPR is not initialized properly. user func dispatcher was not supplied.");
  return NULL;
}

static PSTRING BACKCALL stub_load_file_func (ABSTRACT_FILTER* none, const char* filename) {
  tmpl_log(NULL,TMPL_LOG_ERROR,"load_file_func stub: callback function for filters was not specified.");
  return mmap_load_file(filename);
}

static int BACKCALL stub_unload_file_func (ABSTRACT_FILTER* none, PSTRING memarea) {
  tmpl_log(NULL,TMPL_LOG_ERROR,"unload_file_func stub: callback function for filters was not specified.");
  return mmap_unload_file(memarea);
}

static int BACKCALL stub_get_ABSTRACT_ARRAY_length_func (ABSTRACT_ARRAY* none) {
  return -1; /* treat all arrays as arrays of undefined length */
}


/*
 *  Local Variables:
 *  mode: c
 *  End:
 */
