/* -*- c -*- 
 * File: pparam.h
 * Author: Igor Vlasenko <vlasenko@imath.kiev.ua>
 * Created: Thu Jul  9 20:05:37 2009
 */

#ifndef _PPARAM_H
#define _PPARAM_H	1

#include "proscope.h"
#include "pbuffer.h"
#include "exprval.h" /* TODO: remove together with buffer */

typedef int flag;

struct tmplpro_param {
  int global_vars;
  int max_includes;
  int debug;
  flag no_includes;
  flag case_sensitive;
  flag loop_context_vars;
  flag strict;
  /* filters --- indicates whether to use 
   * external file loader hook specified as LoadFileFuncPtr. 
   * Set it to 1 if you want to preprocess file with filters
   * before they'll be processed by exec_tmpl */
  flag filters;
  int default_escape; /* one of HTML_TEMPLATE_OPT_ESCAPE_* */
  const char* filename; /* template file */
  PSTRING scalarref; /* memory area */
  flag path_like_variable_scope;
  flag search_path_on_include;
  char** path;
  /* still unsupported  */
  flag die_on_bad_params;
  /* flag vanguard_compatibility_mode; */

  /* hooks to perl or other container */
  /* HTML::Template callback hooks */
  writer_functype WriterFuncPtr;
  get_ABSTRACT_VALUE_functype GetAbstractValFuncPtr;
  ABSTRACT_VALUE2PSTRING_functype AbstractVal2pstringFuncPtr;
  ABSTRACT_VALUE2ABSTRACT_ARRAY_functype AbstractVal2abstractArrayFuncPtr;
  get_ABSTRACT_ARRAY_length_functype GetAbstractArrayLengthFuncPtr;
  get_ABSTRACT_MAP_functype GetAbstractMapFuncPtr;
  /* user-supplied --- optional; we use it for full emulation of perl quirks */
  is_ABSTRACT_VALUE_true_functype IsAbstractValTrueFuncPtr;
  find_file_functype FindFileFuncPtr;
  load_file_functype LoadFileFuncPtr;
unload_file_functype UnloadFileFuncPtr;
  select_loop_scope_functype SelectLoopScopeFuncPtr;
  end_loop_functype EndLoopFuncPtr;
  /* external state references to be supplied to callbacks */
  ABSTRACT_MAP* root_param_map;
  ABSTRACT_WRITER* ext_writer_state;
  ABSTRACT_FILTER* ext_filter_state;
  ABSTRACT_FINDFILE* ext_findfile_state;
  /* HTML::Template::Expr hooks */
  ABSTRACT_CALLER* ext_calluserfunc_state;
  init_expr_arglist_functype InitExprArglistFuncPtr;
  free_expr_arglist_functype FreeExprArglistFuncPtr;
  /**
     important note: 
     PushExprArglistFuncPtr should always copy the supplied pstring arg
     as it could point to a temporary location.
   */
  push_expr_arglist_functype PushExprArglistFuncPtr;
  call_expr_userfnc_functype CallExprUserfncFuncPtr;
  is_expr_userfnc_functype   IsExprUserfncFuncPtr;
  ABSTRACT_FUNCMAP*  expr_func_map;
  /* private */
  int cur_includes; /* internal counter of include depth */
  const char* masterpath; /* file that has included this file, or NULL */
  /* moved from state; are passed to include */
  /* variable scope (nested loops) */
  struct scope_stack var_scope_stack;

  /* private buffer of builtin_findfile */
  pbuffer builtin_findfile_buffer;
  /* private buffers of walk_through_nested_loops */
  pbuffer lowercase_varname_buffer;
};

#endif /* _PPARAM_H */

/*
 *  Local Variables:
 *  mode: c
 *  End:
 */
