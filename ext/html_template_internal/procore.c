#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tmplpro.h"
#include "procore.h"
#include "prostate.h"
#include "provalue.h"
#include "tagstack.h"
#include "pbuffer.h"
#include "parse_expr.h"
#include "pparam.h"
#include "proscope.h"
#include "proscope.inc"
#include "pstrutils.inc"
#include "pmiscdef.h" /*for snprintf */
/* for mmap_load_file & mmap_unload_file */
#include "loadfile.inc"

#define HTML_TEMPLATE_NO_TAG     -1
#define HTML_TEMPLATE_BAD_TAG     0
#define HTML_TEMPLATE_FIRST_TAG_USED 1
#define HTML_TEMPLATE_TAG_VAR     1
#define HTML_TEMPLATE_TAG_INCLUDE 2
#define HTML_TEMPLATE_TAG_LOOP    3
#define HTML_TEMPLATE_TAG_IF      4
#define HTML_TEMPLATE_TAG_ELSE    5
#define HTML_TEMPLATE_TAG_UNLESS  6
#define HTML_TEMPLATE_TAG_ELSIF   7
#define HTML_TEMPLATE_LAST_TAG_USED  7

static 
const char* const tagname[]={
    "Bad or unsupported tag", /* 0 */
    "var", "include", "loop", "if", "else", "unless", "elsif"
};

static 
const char* const TAGNAME[]={
    "Bad or unsupported tag", /* 0 */
    "VAR", "INCLUDE", "LOOP", "IF", "ELSE", "UNLESS", "ELSIF"
};

/* max offset to ensure we are not out of file when try <!--/  */
#define TAG_WIDTH_OFFSET 4

static int debuglevel=0;

#include "prostate.inc"

static 
const char* const innerloopname[]={
  "", "first__", "last__", "inner__", "odd__", "counter__"
};

static 
const char* const INNERLOOPNAME[]={
  "", "FIRST__", "LAST__", "INNER__", "ODD__", "COUNTER__"
};

#define HTML_TEMPLATE_INNER_LOOP_VAR_FIRST   1
#define HTML_TEMPLATE_INNER_LOOP_VAR_LAST    2
#define HTML_TEMPLATE_INNER_LOOP_VAR_INNER   3
#define HTML_TEMPLATE_INNER_LOOP_VAR_ODD     4
#define HTML_TEMPLATE_INNER_LOOP_VAR_COUNTER 5

#define HTML_TEMPLATE_FIRST_INNER_LOOP 1
#define HTML_TEMPLATE_LAST_INNER_LOOP  5


static 
int 
try_inner_loop_variable (PSTRING name)
{ 
  int i;
  const char* cur_pos;
  const char* pattern;
  const char* PATTERN;
  for (i=HTML_TEMPLATE_FIRST_INNER_LOOP; i<=HTML_TEMPLATE_LAST_INNER_LOOP; i++) {
    cur_pos=name.begin;
    pattern=innerloopname[i];
    PATTERN=INNERLOOPNAME[i];
    while (*pattern && cur_pos<name.endnext) {
      if (*pattern == *cur_pos || *PATTERN == *cur_pos) {
	pattern++;
	PATTERN++;
	cur_pos++;
      } else {
	break;
      }
    }
    if (cur_pos==name.endnext) {
      return i;
    }
  }
  return 0;
}

static 
PSTRING 
get_loop_context_vars_value (struct tmplpro_param *param, PSTRING name) {
  static char* FalseString="0";
  static char* TrueString ="1";
  static char buffer[20]; /* for snprintf %d */
  int loop;
  PSTRING retval={NULL,NULL};
  struct ProScopeEntry* currentScope = getCurrentScope(&param->var_scope_stack);
  if (isScopeLoop(currentScope)
      && name.endnext-name.begin>4
      && '_'==*(name.begin)
      && '_'==*(name.begin+1)
      ) { 
    /* we can meet loop variables here -- try it first */
    /* length of its name >4 */
    /* __first__ __last__ __inner__ __odd__ __counter__ */
    PSTRING shiftedname; /* (PSTRING) {name.begin+2,name.endnext} */
    shiftedname.begin=name.begin+2;
    shiftedname.endnext=name.endnext;
    switch (try_inner_loop_variable(shiftedname)) {
    case 0:  break;
    case HTML_TEMPLATE_INNER_LOOP_VAR_FIRST: 
      if (currentScope->loop==0) {  /* first__ */
	retval.begin=TrueString; retval.endnext=TrueString+1;
      } else {
	retval.begin=FalseString; retval.endnext=FalseString+1;
      }; break;
    case HTML_TEMPLATE_INNER_LOOP_VAR_LAST: 
      if (currentScope->loop==(currentScope->loop_count-1)) {
	retval.begin=TrueString; retval.endnext=TrueString+1;
      } else {
	retval.begin=FalseString; retval.endnext=FalseString+1;
      }; break;
    case HTML_TEMPLATE_INNER_LOOP_VAR_ODD: 
      if ((currentScope->loop%2)==0) {
	retval.begin=TrueString; retval.endnext=TrueString+1;
      } else {
	retval.begin=FalseString; retval.endnext=FalseString+1;
      }; break;
    case HTML_TEMPLATE_INNER_LOOP_VAR_INNER: 
      if (currentScope->loop>0 && 
	  (currentScope->loop_count<0 /* loop_count < 0 if number of loops is unknown/undefined */
	   || currentScope->loop < (currentScope->loop_count-1))) {
	retval.begin=TrueString; retval.endnext=TrueString+1;
      } else {
	retval.begin=FalseString; retval.endnext=FalseString+1;
      }; break;
    case HTML_TEMPLATE_INNER_LOOP_VAR_COUNTER: 
      loop=currentScope->loop+1;
      snprintf(buffer,sizeof(buffer),"%d",loop);
      retval.begin=buffer; retval.endnext=buffer+strlen(buffer);
      break;
    }
  }
  return retval;
}

static
void init_tmpl_var_case_buffers (struct tmplpro_param *param) {
  param->lowercase_varname.begin = NULL;
  param->lowercase_varname.endnext = NULL;
  param->uppercase_varname.begin = NULL;
  param->uppercase_varname.endnext = NULL;
}

static 
ABSTRACT_VALUE* get_abstract_value (struct tmplpro_param *param, int scope_level, PSTRING name) {
  ABSTRACT_VALUE* retval = NULL;
  ABSTRACT_MAP* param_HV = getScope(&param->var_scope_stack, scope_level)->param_HV;
  ABSTRACT_DATASTATE* data_state = param->ext_data_state;
  get_ABSTRACT_VALUE_functype getval_func = param->GetAbstractValFuncPtr;
  int tmpl_var_case = param->tmpl_var_case;
  if ((tmpl_var_case & ASK_NAME_MASK) == ASK_NAME_DEFAULT
      || tmpl_var_case & ASK_NAME_AS_IS) {
    retval = getval_func(data_state, param_HV, name);
    if (retval != NULL) return retval;
  }
  if (tmpl_var_case & ASK_NAME_LOWERCASE) {
    if (param->lowercase_varname.begin == NULL) {
      param->lowercase_varname=lowercase_pstring(&param->lowercase_varname_buffer, name);
    }
    retval = getval_func(data_state, param_HV, param->lowercase_varname);
    if (retval != NULL) return retval;
  }
  if (tmpl_var_case & ASK_NAME_UPPERCASE) {
    if (param->uppercase_varname.begin == NULL) {
      param->uppercase_varname=uppercase_pstring(&param->uppercase_varname_buffer, name);
    }
    retval = getval_func(data_state, param_HV, param->uppercase_varname);
    if (retval != NULL) return retval;
  }
  return retval;
}

static
ABSTRACT_VALUE* walk_through_nested_loops (struct tmplpro_param *param, PSTRING name) {
  int CurLevel;
  ABSTRACT_VALUE* valptr;
  init_tmpl_var_case_buffers (param);
  /* Shigeki Morimoto path_like_variable_scope extension */
  if (param->path_like_variable_scope) {
    if(*(name.begin) == '/' || strncmp(name.begin, "../", 3) == 0){
      PSTRING tmp_name;
      int GoalHash;
      if(*(name.begin) == '/'){
	tmp_name.begin   = name.begin+1; // skip '/'
	tmp_name.endnext = name.endnext;
	GoalHash = 0;
      }else{
	tmp_name.begin   = name.begin;
	tmp_name.endnext = name.endnext;
	GoalHash = curScopeLevel(&param->var_scope_stack);
	while(strncmp(tmp_name.begin, "../", 3) == 0){
	  tmp_name.begin   = tmp_name.begin + 3; // skip '../'
	  GoalHash --;
	}
      }
      return get_abstract_value(param, GoalHash, tmp_name);
    }
  }
  /* end Shigeki Morimoto path_like_variable_scope extension */

  CurLevel = curScopeLevel(&param->var_scope_stack);
  valptr = get_abstract_value(param, CurLevel, name);
  if (valptr) return valptr;
  /* optional strict scoping; does it have much sence?
    if ((STRICT_SCOPING==param->global_vars)) return NULL;
  */

  /* loop-bounded scoping; */
  if (0==param->global_vars) {
    while (isScopeMap(getScope(&param->var_scope_stack,CurLevel)) && --CurLevel>=0) {
      valptr = get_abstract_value(param, CurLevel, name);
      if (valptr!=NULL) return valptr;
    }
    return NULL;
  }

  while (--CurLevel>=0) {
    valptr = get_abstract_value(param, CurLevel, name);
    if (valptr!=NULL) return valptr;
  }
  return NULL;
}

static 
int 
is_string(struct tmplpro_state *state, const char* pattern,const char* PATTERN)
{
  const char* cur_pos=state->cur_pos;
  while (*pattern && cur_pos<state->next_to_end) {
    if (*pattern == *cur_pos || *PATTERN == *cur_pos) {
      pattern++;
      PATTERN++;
      cur_pos++;
    } else {
      return 0;
    }
  }
  if (cur_pos>=state->next_to_end) return 0;
  state->cur_pos=cur_pos;
  return 1;
}


static 
INLINE 
void 
jump_over_space(struct tmplpro_state *state)
{
  while (isspace(*(state->cur_pos)) && state->cur_pos<state->next_to_end) {state->cur_pos++;};
}

static 
INLINE
void 
jump_to_char(struct tmplpro_state *state, char c)
{
  while (c!=*(state->cur_pos) && state->cur_pos<state->next_to_end) {state->cur_pos++;};
}

TMPLPRO_LOCAL
PSTRING _get_variable_value (struct tmplpro_param *param, PSTRING name) {
  PSTRING varvalue ={NULL, NULL};
  ABSTRACT_VALUE* abstrval;
  if (param->loop_context_vars) {
    varvalue=get_loop_context_vars_value(param, name);
  }
  if (varvalue.begin==NULL) {
    abstrval=walk_through_nested_loops(param, name);
    if (abstrval!=NULL) varvalue=(param->AbstractVal2pstringFuncPtr)(param->ext_data_state, abstrval);
  }
  if (debuglevel>=TMPL_LOG_DEBUG2) {
    if (name.begin!=NULL) {
      tmpl_log(TMPL_LOG_DEBUG2,"_get_variable_value: name = %.*s ",(int)(name.endnext-name.begin),name.begin);
    } else {
      tmpl_log(TMPL_LOG_DEBUG2,"_get_variable_value: name = NULL ");
    }
    if (varvalue.begin!=NULL) {
      tmpl_log(TMPL_LOG_DEBUG2,"value = %.*s\n",(int)(varvalue.endnext-varvalue.begin),varvalue.begin);
    } else {
      tmpl_log(TMPL_LOG_DEBUG2,"value = UNDEF\n");
    }
  }
  return varvalue;
}

static 
void 
tag_handler_var (struct tmplpro_state *state, PSTRING name, PSTRING defvalue, int escapeopt)
{
  PSTRING varvalue ={NULL, NULL};
  /*
  if (debuglevel>=TMPL_LOG_DEBUG2) {
    log_state(state,TMPL_LOG_DEBUG2,"Entered tag_handler_var\n");
    }*/
  if (! state->is_visible) return;
  if (state->is_expr) {
    varvalue=parse_expr(name, state);
  } else {
    varvalue=_get_variable_value(state->param, name);
  }
  if (debuglevel>=TMPL_LOG_DEBUG) {
    if (name.begin!=NULL) {
      log_state(state,TMPL_LOG_DEBUG,"variable name = %.*s ",(int)(name.endnext-name.begin),name.begin);
    } else {
      log_state(state,TMPL_LOG_DEBUG,"variable name = NULL ");
    }
    if (varvalue.begin!=NULL) {
      tmpl_log(TMPL_LOG_DEBUG,"value = %.*s\n",(int)(varvalue.endnext-varvalue.begin),varvalue.begin);
    } else {
      tmpl_log(TMPL_LOG_DEBUG,"value = UNDEF\n");
    }
  }
  if (varvalue.begin==NULL) {
    if (defvalue.begin!=defvalue.endnext) {
      varvalue=defvalue;
    } else return;
  }
  if (escapeopt!=HTML_TEMPLATE_OPT_ESCAPE_NO) {
    varvalue=escape_pstring(&(state->str_buffer), varvalue, escapeopt);
  }
  (state->param->WriterFuncPtr)(state->param->ext_writer_state,varvalue.begin,varvalue.endnext);
}

static 
void 
tag_handler_include (struct tmplpro_state *state, PSTRING name, PSTRING defvalue)
{
  struct tmplpro_param* param;
  char* filename;
  const char* masterpath;
  int x;
  PSTRING varvalue=name;
  if (! state->is_visible) return;
  param=state->param;
  if (param->no_includes) {
    log_state(state,TMPL_LOG_ERROR, "HTML::Template::Pro : Illegal attempt to use TMPL_INCLUDE in template file : (no_includes => 1)\n");
    return;
  }
  if (param->max_includes && param->max_includes < param->cur_includes) return;
  param->cur_includes++;

  if (state->is_expr) {
    varvalue=parse_expr(name, state);
  } else 
    if (varvalue.begin==varvalue.endnext && defvalue.begin!=defvalue.endnext)
      varvalue=defvalue;
  /* pstrdup */
  filename =(char*) malloc(varvalue.endnext-varvalue.begin+1);
  for (x=0;x<varvalue.endnext-varvalue.begin;x++) {
    *(filename+x)=*(varvalue.begin+x);
  }
  *(filename+(varvalue.endnext-varvalue.begin))=0;
  /* end pstrdup */
  masterpath=param->masterpath; /* saving current file name */
  tmplpro_exec_tmpl_filename (param,filename);
  free (filename);
  param->masterpath=masterpath;
  param->cur_includes--; 
  return;
}

static 
int 
is_var_true(struct tmplpro_state *state, PSTRING name) 
{
  register int ifval=-1; /*not yet defined*/
  if (state->is_expr) {
    ifval=is_pstring_true(parse_expr(name, state));
  } else
    if (state->param->loop_context_vars) {
      PSTRING loop_var=get_loop_context_vars_value(state->param, name);
      if (loop_var.begin!=NULL) {
	ifval=is_pstring_true(loop_var);
      }
    }
  if (ifval==-1) {
    is_ABSTRACT_VALUE_true_functype userSuppliedIsTrueFunc;
    ABSTRACT_VALUE* abstrval=walk_through_nested_loops(state->param, name);
    if (abstrval==NULL) return 0;
    userSuppliedIsTrueFunc = state->param->IsAbstractValTrueFuncPtr;
    if (userSuppliedIsTrueFunc!=NULL) {
      ifval=(userSuppliedIsTrueFunc)(state->param->ext_data_state, abstrval);
    } else {
      ifval=is_pstring_true((state->param->AbstractVal2pstringFuncPtr)(state->param->ext_data_state, abstrval));
    }
  }
  return ifval;
}

static 
void 
tag_handler_if (struct tmplpro_state *state, PSTRING name)
{
  struct tagstack_entry iftag;
  iftag.tag=HTML_TEMPLATE_TAG_IF;
  iftag.vcontext=state->is_visible;
  iftag.position=state->cur_pos; /* unused */
  /* state->is_visible && means that we do not evaluate variable in shadow */
  if (state->is_visible && is_var_true(state,name)) {
    iftag.value=1;
    /* state->is_visible is not touched */
  } else {
    iftag.value=0;
    state->is_visible=0;
  }
  tagstack_push(&(state->tag_stack), iftag);
  if (debuglevel>=TMPL_LOG_DEBUG2) log_state(state,TMPL_LOG_DEBUG2,"tag_handler_if:visible context =%d value=%d ",iftag.vcontext,iftag.value);
}

static 
void 
tag_handler_unless (struct tmplpro_state *state, PSTRING name)
{
  struct tagstack_entry iftag;
  iftag.tag=HTML_TEMPLATE_TAG_UNLESS;
  iftag.vcontext=state->is_visible;
  iftag.position=state->cur_pos; /* unused */
  /* state->is_visible && means that we do not evaluate variable in shadow */
  if (state->is_visible && !is_var_true(state,name)) {
    iftag.value=1;
    /* state->is_visible is not touched */
  } else {
    iftag.value=0;
    state->is_visible=0;
  }
  tagstack_push(&(state->tag_stack), iftag);
  if (debuglevel>=TMPL_LOG_DEBUG2) log_state(state,TMPL_LOG_DEBUG2,"tag_handler_unless:visible context =%d value=%d ",iftag.vcontext,iftag.value);
}

static 
INLINE
int
test_stack (int tag)
{
  //  return (tagstack_notempty(&(state->tag_stack)) && (tagstack_top(&(state->tag_stack))->tag==tag));
  return 1;
}

static 
void 
tag_stack_debug  (struct tmplpro_state *state, int stack_tag_type)
{
  if (stack_tag_type) {
    if (tagstack_notempty(&(state->tag_stack))) {
      struct tagstack_entry* iftag=tagstack_top(&(state->tag_stack));
      if (iftag->tag!=stack_tag_type) {
	log_state(state,TMPL_LOG_ERROR, "ERROR: tag mismatch with %s\n",TAGNAME[iftag->tag]);
      }
    } else {
      log_state(state,TMPL_LOG_ERROR, "ERROR: opening tag %s not found\n",TAGNAME[stack_tag_type]);
    }
  }
}

static 
void 
tag_handler_closeif (struct tmplpro_state *state)
{
  struct tagstack_entry iftag;
  if (! test_stack(HTML_TEMPLATE_TAG_IF)) {
    tag_stack_debug(state,HTML_TEMPLATE_TAG_IF);
    return;
  }
  iftag=tagstack_pop(&(state->tag_stack));
  if (0==state->is_visible) state->last_processed_pos=state->cur_pos;
  state->is_visible=iftag.vcontext;
}

static 
void 
tag_handler_closeunless (struct tmplpro_state *state)
{
  struct tagstack_entry iftag;
  if (! test_stack(HTML_TEMPLATE_TAG_UNLESS)) {
    tag_stack_debug(state,HTML_TEMPLATE_TAG_UNLESS);
    return;
  }
  iftag=tagstack_pop(&(state->tag_stack));
  if (0==state->is_visible) state->last_processed_pos=state->cur_pos;
  state->is_visible=iftag.vcontext;
}

static 
void 
tag_handler_else (struct tmplpro_state *state)
{
  struct tagstack_entry* iftag;
  if (! test_stack(HTML_TEMPLATE_TAG_IF) && 
      ! test_stack(HTML_TEMPLATE_TAG_UNLESS)) {
    tag_stack_debug(state,HTML_TEMPLATE_TAG_ELSE);
    return;
  }
  iftag=tagstack_top(&(state->tag_stack));
  if (0==state->is_visible) state->last_processed_pos=state->cur_pos;
  if (iftag->value) {
    state->is_visible=0;
  } else if (1==iftag->vcontext) {
    state->is_visible=1;
  }
  if (debuglevel>=TMPL_LOG_DEBUG2) log_state(state,TMPL_LOG_DEBUG2,"else:(pos " MOD_TD ") visible:context =%d, set to %d ",
			     TO_PTRDIFF_T(iftag->position - state->top),iftag->vcontext,state->is_visible);
}

static 
void 
tag_handler_elsif (struct tmplpro_state *state, PSTRING name)
{
  struct tagstack_entry *iftag;
  if (! test_stack(HTML_TEMPLATE_TAG_IF) && 
      ! test_stack(HTML_TEMPLATE_TAG_UNLESS)) {
    tag_stack_debug(state,HTML_TEMPLATE_TAG_ELSIF);
    return;
  }
  iftag=tagstack_top(&(state->tag_stack));
  if (0==state->is_visible) state->last_processed_pos=state->cur_pos;
  if (iftag->value) {
    state->is_visible=0;
  } else if (1==iftag->vcontext) {
    /* test only if vcontext==true; if the whole tag if..endif itself is invisible, skip the is_var_true test */
    /*TODO: it is reasonable to skip is_var_true test in if/unless too */
    if (is_var_true(state,name)) {
      iftag->value=1;
      state->is_visible=1;
    } else {
      iftag->value=0;
      state->is_visible=0;
    }
  }
  if (debuglevel>=TMPL_LOG_DEBUG2) log_state(state,TMPL_LOG_DEBUG2,"elsif:(pos " MOD_TD ") visible:context =%d, set to %d ",
			     TO_PTRDIFF_T(iftag->position - state->top), iftag->vcontext, state->is_visible);
}

static 
int 
next_loop (struct tmplpro_state* state) {
#ifdef DEBUG
  log_state(state,TMPL_LOG_DEBUG2,"next_loop:before NextLoopFuncPtr\n");
#endif
  struct ProScopeEntry* currentScope = getCurrentScope(&state->param->var_scope_stack);
  if (!isScopeLoop(currentScope)) {
    log_state(state,TMPL_LOG_ERROR, "next_loop:at scope level %d: internal error - loop is null\n", curScopeLevel(&state->param->var_scope_stack));
    return 0;
  }
  if (++currentScope->loop < currentScope->loop_count || currentScope->loop_count< 0) {
    ABSTRACT_MAP* arrayvalptr=(state->param->GetAbstractMapFuncPtr)(state->param->ext_data_state, currentScope->loops_AV,currentScope->loop);
    if ((arrayvalptr!=NULL)) {
      currentScope->param_HV=arrayvalptr;
      return 1;
    } else {
      /* either undefined loop ended normally or defined loop ended ubnormally */
      if (currentScope->loop_count>0) log_state(state,TMPL_LOG_ERROR, "PARAM:LOOP:next_loop(%d): callback returned null scope\n", currentScope->loop);
    }
  }
  if (state->param->ExitLoopScopeFuncPtr) state->param->ExitLoopScopeFuncPtr(state->param->ext_data_state, currentScope->loops_AV);
  popScope(&state->param->var_scope_stack);
  return 0;
}

static 
int init_loop (struct tmplpro_state *state, PSTRING name) {
  int loop_count;
  ABSTRACT_ARRAY* loopptr=(ABSTRACT_ARRAY*) walk_through_nested_loops(state->param,name);
  if (loopptr==NULL) {
    return 0;
  } else {
    /* set loop array */
    loopptr = (*state->param->AbstractVal2abstractArrayFuncPtr)(state->param->ext_data_state, loopptr);
    if (loopptr == NULL)
      {
	log_state(state,TMPL_LOG_ERROR, "PARAM:LOOP:loop argument:loop was expected but not found.\n");
	return 0;
      }
    loop_count = (*state->param->GetAbstractArrayLengthFuncPtr)(state->param->ext_data_state, loopptr);
    /* 0 is an empty array; <0 is an undefined array (iterated until next_loop==NULL */
    if (0==loop_count) return 0;
    pushScopeLoop(&state->param->var_scope_stack, loop_count, loopptr);
    return 1;
  }
}

static 
void 
tag_handler_loop (struct tmplpro_state *state, PSTRING name)
{
  struct tagstack_entry iftag;
  iftag.tag=HTML_TEMPLATE_TAG_LOOP;
  iftag.vcontext=state->is_visible;
  iftag.value=0;
  iftag.position=state->cur_pos; /* loop start - to restore in </tmpl_loop> */
#ifdef DEBUG
  log_state(state,TMPL_LOG_DEBUG2,"tag_handler_loop:before InitLoopFuncPtr\n");
#endif
  if (state->is_visible && init_loop(state,name) && next_loop(state)) {
    iftag.value=1; /* the loop is non - empty */
  } else {
    /* empty loop is equal to <if false> ... </if> */
    state->is_visible=0;
  }
#ifdef DEBUG
  log_state(state,TMPL_LOG_DEBUG2,"tag_handler_loop:after InitLoopFuncPtr\n");
#endif
  tagstack_push(&(state->tag_stack), iftag);
}

static 
void 
tag_handler_closeloop (struct tmplpro_state *state)
{
  struct tagstack_entry* iftag_ptr;
  if (! test_stack(HTML_TEMPLATE_TAG_LOOP)) {
    tag_stack_debug(state,HTML_TEMPLATE_TAG_LOOP);
    return;
  }
  iftag_ptr=tagstack_top(&(state->tag_stack));
  if (iftag_ptr->value==1 && next_loop(state)) {
    /* continue loop */
    state->cur_pos=iftag_ptr->position;
    state->last_processed_pos=iftag_ptr->position;
    return;
  } else {
    /* finish loop */
    struct tagstack_entry iftag;
    iftag=tagstack_pop(&(state->tag_stack));
    state->is_visible=iftag.vcontext;
    state->last_processed_pos=state->cur_pos;
  }
}

static 
void 
tag_handler_unknown (struct tmplpro_state *state)
{
  log_state(state,TMPL_LOG_ERROR,"tag_handler_unknown: unknown tag\n");
}

static 
PSTRING 
read_tag_parameter_value (struct tmplpro_state *state)
{
  PSTRING modifier_value;
  char cur_char;
  char quote_char=0;
  register const char* cur_pos;
  register const char* next_to_end=state->next_to_end;
  jump_over_space(state);
  cur_pos=state->cur_pos;
  cur_char=*(cur_pos);
  if (('"'==cur_char) || ('\''==cur_char)) {
    quote_char=*(cur_pos);
    cur_pos++;
  }
  modifier_value.begin=cur_pos;
  cur_char=*(cur_pos);
  if (quote_char) {
    while (quote_char!=cur_char 
#ifdef COMPAT_ON_BROKEN_QUOTE
/* compatibility mode; HTML::Template doesn't allow '>' inside quotes */
	   && ('>' != quote_char)
#endif
	   && cur_pos<next_to_end) {
      cur_pos++;
      cur_char=*(cur_pos);
    }
  } else {
    while ('>'!=cur_char && ! isspace(cur_char) && cur_pos<next_to_end) {
      cur_pos++;
      cur_char=*(cur_pos);
    }
  }
  if (cur_pos>=next_to_end) {
    log_state(state,TMPL_LOG_ERROR,"quote char %c at pos " MOD_TD " is not terminated\n",
	     quote_char,TO_PTRDIFF_T(state->cur_pos-state->top));
    modifier_value.endnext=modifier_value.begin;
    jump_over_space(state);
    return modifier_value;
  }
  modifier_value.endnext=cur_pos;
  if (quote_char) {
    if (quote_char==*cur_pos) {
      cur_pos++;
    } else {
      log_state(state,TMPL_LOG_ERROR,"found %c instead of end quote %c at pos " MOD_TD "\n",
	       *cur_pos,quote_char,TO_PTRDIFF_T(cur_pos - state->top));
    }
  }
  state->cur_pos=cur_pos;
  /* log_state(state,TMPL_LOG_DEBUG2," at pos " MOD_TD "",TO_PTRDIFF_T(state->cur_pos-state->top)); */
  jump_over_space(state);
  return modifier_value;
}

static 
int 
try_tag_parameter (struct tmplpro_state *state,const char *modifier,const char *MODIFIER)
{
  const char* initial_pos=state->cur_pos;
  jump_over_space(state);
  if (is_string(state, modifier, MODIFIER)) {
    jump_over_space(state);
    if ('='==*(state->cur_pos)) {
      state->cur_pos++;
      jump_over_space(state);
      return 1;
    }
  }
  state->cur_pos=initial_pos;
  return 0;
}

static 
void 
try_tmpl_var_options (struct tmplpro_state *state, PSTRING* OptEscape, PSTRING* OptDefault)
{
  static const char* escapeopt="escape";
  static const char* ESCAPEOPT="ESCAPE";
  static const char* defaultopt="default";
  static const char* DEFAULTOPT="DEFAULT";
  if (try_tag_parameter(state, escapeopt, ESCAPEOPT)) {
    *OptEscape=read_tag_parameter_value(state);
    log_state(state,TMPL_LOG_DEBUG, "found option ESCAPE at pos " MOD_TD "\n",TO_PTRDIFF_T(state->cur_pos-state->top));
  }
  if (try_tag_parameter(state, defaultopt, DEFAULTOPT)) {
    *OptDefault=read_tag_parameter_value(state);
    log_state(state,TMPL_LOG_DEBUG, "found option DEFAULT at pos " MOD_TD "\n",TO_PTRDIFF_T(state->cur_pos-state->top));
  }
}


static 
void 
process_tmpl_tag(struct tmplpro_state *state)
{
  const char* tag_start=state->tag_start;
  int is_tag_closed=state->is_tag_closed;

  int tag_type=HTML_TEMPLATE_BAD_TAG;
  static const char* nameopt="name";
  static const char* NAMEOPT="NAME";
  static const char* expropt="expr";
  static const char* EXPROPT="EXPR";

  PSTRING OptName  = {NULL,NULL};
  PSTRING OptDefault={NULL,NULL};
  PSTRING OptEscape= {NULL,NULL};

  int i;
  for (i=HTML_TEMPLATE_FIRST_TAG_USED; i<=HTML_TEMPLATE_LAST_TAG_USED; i++) {
    if (is_string(state, tagname[i], TAGNAME[i])) {
      tag_type=i;
      state->tag=tag_type;
      if (debuglevel) {
	if (is_tag_closed) {
	  tmpl_log(TMPL_LOG_DEBUG, "found </TMPL_%s> at pos " MOD_TD "\n",TAGNAME[i], TO_PTRDIFF_T(state->cur_pos-state->top));
	} else {
	  tmpl_log(TMPL_LOG_DEBUG, "found <TMPL_%s> at pos " MOD_TD "\n",TAGNAME[i], TO_PTRDIFF_T(state->cur_pos-state->top));
	}
      }
      break;
    }
  }
  if (HTML_TEMPLATE_BAD_TAG==tag_type) {
    log_state(state,TMPL_LOG_ERROR, "found bad tag at pos " MOD_TD "\n", TO_PTRDIFF_T(state->cur_pos-state->top));
    /* TODO: flush its data ---  */
    state->cur_pos++;
    return;
  }

  if (is_tag_closed && (
			tag_type==HTML_TEMPLATE_TAG_ELSE
			|| tag_type==HTML_TEMPLATE_TAG_INCLUDE
			|| tag_type==HTML_TEMPLATE_TAG_VAR
)) {
    log_state(state,TMPL_LOG_ERROR, "incorrect tag </TMPL_%s> at pos " MOD_TD "\n",
	     TAGNAME[tag_type], TO_PTRDIFF_T(state->cur_pos-state->top));
  }

  if (is_tag_closed || tag_type==HTML_TEMPLATE_TAG_ELSE) {
    /* tag has no parameter */
    
    /* requested compatibility mode 
       to try reading NAME inside </closing tags NAME="  ">
       (useful for comments?) */
#ifdef COMPAT_ALLOW_NAME_IN_CLOSING_TAG
    try_tag_parameter(state, nameopt, NAMEOPT);
    read_tag_parameter_value(state);
#endif
  } else {
    /* reading parameter */
    state->is_expr=0;
    if (tag_type==HTML_TEMPLATE_TAG_VAR
	 || tag_type==HTML_TEMPLATE_TAG_INCLUDE
	) {
      try_tmpl_var_options(state,&OptEscape,&OptDefault);
    }

    if (try_tag_parameter(state, expropt, EXPROPT)) {
      state->is_expr=1;
    } else {
      try_tag_parameter(state, nameopt, NAMEOPT);
    }
    OptName=read_tag_parameter_value(state);

    if (tag_type==HTML_TEMPLATE_TAG_VAR
	|| tag_type==HTML_TEMPLATE_TAG_INCLUDE
	) {
      try_tmpl_var_options(state,&OptEscape,&OptDefault);
    }
  }

  if (state->is_tag_commented) {
    /* try read comment end */
    /* jump_over_space(state); it should be already done :( */
    jump_over_space(state);
    if (state->cur_pos<state->next_to_end-2 && '-'==*(state->cur_pos) && '-'==*(state->cur_pos+1)) {
      state->cur_pos+=2;
    }
  }
  /* template tags could also be decorated as xml <tmpl_TAG /> */
  if (!state->is_tag_closed && '/'==*(state->cur_pos)) state->cur_pos++;

  if ('>'==*(state->cur_pos)) {
    state->cur_pos++;
  } else {
    log_state(state,TMPL_LOG_ERROR,"end tag:found %c instead of > at pos " MOD_TD "\n",
	     *state->cur_pos, TO_PTRDIFF_T(state->cur_pos-state->top));
  }
  /* flush run chars (if in SHOW mode) */
  if (state->is_visible) {
    (state->param->WriterFuncPtr)(state->param->ext_writer_state,state->last_processed_pos,tag_start);
    state->last_processed_pos=state->cur_pos;
  }
  /* TODO: call tag_specific handler by array of handlers 
     var_tag_handler(..) */
  if (is_tag_closed) {
    switch (tag_type) {
    case HTML_TEMPLATE_TAG_IF:	tag_handler_closeif(state);break;
    case HTML_TEMPLATE_TAG_UNLESS:	tag_handler_closeunless(state);break;
    case HTML_TEMPLATE_TAG_LOOP:	tag_handler_closeloop(state);break;
    default:	tag_handler_unknown(state);break;
    
    }
  } else {
    /* int escape = HTML_TEMPLATE_OPT_ESCAPE_NO; */
    int escape = state->param->default_escape;
    switch (tag_type) {
    case HTML_TEMPLATE_TAG_VAR:	
      if (OptEscape.begin!=OptEscape.endnext) {
	switch (*OptEscape.begin) {
	case '1': case 'H': case 'h': 	/* HTML*/
	  escape = HTML_TEMPLATE_OPT_ESCAPE_HTML;
	  break;
	case 'U': case 'u': 		/* URL */
	  escape = HTML_TEMPLATE_OPT_ESCAPE_URL;
	  break;
	case 'J': case 'j':		/* JS  */
	  escape = HTML_TEMPLATE_OPT_ESCAPE_JS;
	  break;
	case '0': case 'N': case 'n': /* 0 or NONE */
	  escape = HTML_TEMPLATE_OPT_ESCAPE_NO;
	  break;
	default:
	  log_state(state,TMPL_LOG_ERROR, " unsupported value of ESCAPE=%.*s\n",(int)(OptEscape.endnext-OptEscape.begin),OptEscape.begin);
	}
      }
      tag_handler_var(state,OptName,OptDefault,escape);
      break;
    case HTML_TEMPLATE_TAG_IF:	tag_handler_if(state,OptName);	break;
    case HTML_TEMPLATE_TAG_UNLESS:	tag_handler_unless(state,OptName);break;
    case HTML_TEMPLATE_TAG_ELSE:	tag_handler_else(state);	break;
    case HTML_TEMPLATE_TAG_ELSIF:	tag_handler_elsif(state,OptName);break;
    case HTML_TEMPLATE_TAG_LOOP:	tag_handler_loop(state,OptName);break;
    case HTML_TEMPLATE_TAG_INCLUDE:	tag_handler_include(state,OptName,OptDefault);break;
    default:	tag_handler_unknown(state);break;
    }
  }

}

static 
void 
process_state (struct tmplpro_state * state)
{
  static const char* metatag="tmpl_";
  static const char* METATAG="TMPL_";
  flag is_tag_closed;
  flag is_tag_commented;
  const char* last_safe_pos=state->next_to_end-TAG_WIDTH_OFFSET;
  if (debuglevel) tmpl_log(TMPL_LOG_DEBUG,"process_state:initiated at scope stack depth = %d\n", 
			   curScopeLevel(&state->param->var_scope_stack));
  tagstack_init(&(state->tag_stack));
  pbuffer_init(&(state->str_buffer));
  /* magic; 256 > 50 (50 is min.required for double to string conversion */
  pbuffer_init_as(&(state->expr_left_pbuffer), 256); 
  pbuffer_init_as(&(state->expr_right_pbuffer), 256);

  while (state->cur_pos < last_safe_pos) {
    register const char* cur_pos=state->cur_pos;
    while (cur_pos < last_safe_pos && '<'!=*(cur_pos++)) {};
    if (cur_pos >= last_safe_pos) break;
    state->tag_start=cur_pos-1;
    is_tag_closed=0;
    is_tag_commented=0;
    state->cur_pos=cur_pos;
    if (('!'==*(cur_pos)) && ('-'==*(cur_pos+1)) && ('-'==*(cur_pos+2))) {
      state->cur_pos+=3;
      jump_over_space(state);
      is_tag_commented=1;
    }
    if ('/'==*(state->cur_pos)) {
      state->cur_pos++;
      is_tag_closed=1;
    }
    if (is_string(state,metatag,METATAG)) {
      state->is_tag_commented=is_tag_commented;
      state->is_tag_closed=is_tag_closed;
      process_tmpl_tag(state);
    }
  }
  (state->param->WriterFuncPtr)(state->param->ext_writer_state,state->last_processed_pos,state->next_to_end);

  pbuffer_free(&(state->expr_right_pbuffer));
  pbuffer_free(&(state->expr_left_pbuffer));
  pbuffer_free(&(state->str_buffer));
  tagstack_free(&(state->tag_stack));
  if (debuglevel) tmpl_log(TMPL_LOG_DEBUG,"process_state:finished\n");
}

static 
void 
init_state (struct tmplpro_state *state, struct tmplpro_param *param)
{
  debuglevel=param->debug;
  tmpl_log_set_level(debuglevel);
  /* initializing state */
  state->param=param;
  state->last_processed_pos=state->top;
  state->cur_pos=state->top;
  state->tag=HTML_TEMPLATE_NO_TAG;
  state->is_visible=1;
}

static
int 
tmplpro_exec_tmpl_filename (struct tmplpro_param *param, const char* filename)
{
  struct tmplpro_state state;
  int mmapstatus;
  PSTRING memarea;
  int retval = 0;
  /* 
   * param->masterpath is path to upper level template 
   * (or NULL in toplevel) which called <include filename>.
   * we use it to calculate filepath for filename.
   * Then filename becames upper level template for its <include>.
   */
  const char* filepath=(param->FindFileFuncPtr)(param->ext_findfile_state,filename, param->masterpath);
  if (NULL==filepath) return ERR_PRO_FILE_NOT_FOUND;
  /* filepath should be alive for every nested template */
  filepath = strdup(filepath);

  param->masterpath=filepath;
  if (param->filters) memarea=(param->LoadFileFuncPtr)(param->ext_filter_state,filepath);
  else memarea=mmap_load_file(filepath);
  if (memarea.begin == NULL) {
    retval = ERR_PRO_CANT_OPEN_FILE;
    goto cleanup_filepath;
  }
  state.top =memarea.begin;
  state.next_to_end=memarea.endnext;
  if (memarea.begin < memarea.endnext) {
    /* to avoid crash with empty file */
    init_state(&state,param);
    log_state(&state,TMPL_LOG_DEBUG, "exec_tmpl: loading %s\n",filename);
    process_state(&state);
  }
  /* destroying */
  if (param->filters) mmapstatus=(param->UnloadFileFuncPtr)(param->ext_filter_state,memarea);
  else mmapstatus=mmap_unload_file(memarea);
 cleanup_filepath:
  if (filepath!=NULL) free((void*) filepath);
  return retval;
}

static
int 
tmplpro_exec_tmpl_scalarref (struct tmplpro_param *param, PSTRING memarea)
{
  struct tmplpro_state state;
  param->masterpath=NULL; /* no upper file */
  state.top = memarea.begin;
  state.next_to_end=memarea.endnext;
  if (memarea.begin == memarea.endnext) return 0;
  init_state(&state,param);
  process_state(&state);
  return 0;
}

#include "builtin_findfile.inc"
#include "callback_stubs.inc"

API_IMPL 
int 
APICALL
tmplpro_exec_tmpl (struct tmplpro_param *param)
{
  int exitcode=0;
  if (param->GetAbstractValFuncPtr==NULL ||
       param->AbstractVal2pstringFuncPtr==NULL ||
       param->AbstractVal2abstractArrayFuncPtr==NULL ||
       /*param->GetAbstractArrayLengthFuncPtr==NULL ||*/
       param->GetAbstractMapFuncPtr==NULL ||
      (param->IsExprUserfncFuncPtr!=NULL &&
       (param->InitExprArglistFuncPtr==NULL ||
	param->PushExprArglistFuncPtr==NULL ||
	param->FreeExprArglistFuncPtr==NULL ||
	param->CallExprUserfncFuncPtr==NULL))
      )
    {
    tmpl_log(TMPL_LOG_ERROR,"tmplpro_exec_tmpl: a required callback is missing.");
    return ERR_PRO_INVALID_ARGUMENT;
  }
  if (param->filters &&
      (param->LoadFileFuncPtr==NULL ||
       param->UnloadFileFuncPtr==NULL)) {
    tmpl_log(TMPL_LOG_ERROR,"tmplpro_exec_tmpl: filters is set but filter callbacks are missing.");
  }
  /* set up stabs */
  if (NULL==param->WriterFuncPtr) param->WriterFuncPtr = stub_write_chars_to_stdout;
  if (NULL==param->ext_findfile_state) param->ext_findfile_state = param;
  if (NULL==param->FindFileFuncPtr) {
    param->FindFileFuncPtr = stub_find_file_func;
    param->ext_findfile_state = param;
    /*pbuffer_init(&param->builtin_findfile_buffer);*/
  }
  if (NULL==param->IsExprUserfncFuncPtr) param->IsExprUserfncFuncPtr = stub_is_expr_userfnc_func;
  if (NULL==param->LoadFileFuncPtr) param->LoadFileFuncPtr = stub_load_file_func;
  if (NULL==param->UnloadFileFuncPtr) param->UnloadFileFuncPtr = stub_unload_file_func;
  if (NULL==param->GetAbstractArrayLengthFuncPtr) param->GetAbstractArrayLengthFuncPtr = stub_get_ABSTRACT_ARRAY_length_func;

  Scope_reset(&param->var_scope_stack, param->param_map_count);
  /* reset other internals */
  param->cur_includes=0; /* internal counter of include depth */
  /*masterpath=NULL;*/

  if (param->scalarref.begin) exitcode = tmplpro_exec_tmpl_scalarref(param, param->scalarref);
  else if (param->filename) exitcode = tmplpro_exec_tmpl_filename(param, param->filename);
  else {
    tmpl_log(TMPL_LOG_ERROR,"tmplpro_exec_tmpl: neither scalarref nor filename was specified.");
    exitcode = ERR_PRO_INVALID_ARGUMENT;
  }

  return exitcode;
}

API_IMPL
MPSTRING
APICALL
tmplpro_tmpl2pstring (struct tmplpro_param *param, int *retvalptr)
{
  int exitcode;
  MPSTRING retval;
  struct builtin_writer_state state;
  writer_functype save_writer_func = param->WriterFuncPtr;
  ABSTRACT_WRITER* save_writer_state = param->ext_writer_state;
  param->WriterFuncPtr = stub_write_chars_to_pbuffer;
  param->ext_writer_state = &state;
  pbuffer_init_as(&state.buffer, 4000);
  state.size = 0;
  exitcode = tmplpro_exec_tmpl (param);
  param->WriterFuncPtr = save_writer_func;
  param->ext_writer_state = save_writer_state;
  if (retvalptr) *retvalptr=exitcode;
  retval.begin = pbuffer_string(&state.buffer);
  retval.endnext = retval.begin+state.size;
  *retval.endnext='\0';
  return retval;
}

API_IMPL 
void 
APICALL
tmplpro_clear_option_param_map(struct tmplpro_param *param)
{
  param->param_map_count=0;
  Scope_reset(&param->var_scope_stack,param->param_map_count);
}

API_IMPL 
int 
APICALL
tmplpro_push_option_param_map(struct tmplpro_param *param, ABSTRACT_MAP* map, EXPR_int64 flags)
{
  pushScopeMap(&param->var_scope_stack, map, (int) flags);
  return ++(param->param_map_count);
}

API_IMPL 
int 
APICALL
tmplpro_count_option_param_map(struct tmplpro_param *param)
{
  return param->param_map_count;
}


API_IMPL 
void 
APICALL
tmplpro_procore_init(void)
{
}

API_IMPL 
void 
APICALL
tmplpro_procore_done(void)
{
}

/* internal initialization of struct tmplpro_param */
API_IMPL 
struct tmplpro_param* 
APICALL
tmplpro_param_init(void)
{
  struct tmplpro_param* param=(struct tmplpro_param*) malloc (sizeof(struct tmplpro_param));
  if (param==NULL) return param;
  /* filling initial struct tmplpro_param with 0 */
  memset (param, 0, sizeof(struct tmplpro_param));
  /* current level of inclusion */
  /* param->cur_includes=0; */
  /* not to use external file loader */
  /* param->filters=0;
     param->default_escape=HTML_TEMPLATE_OPT_ESCAPE_NO;
     param->masterpath=NULL; *//* we are not included by something *//*
     param->expr_func_map=NULL;
     param->expr_func_arglist=NULL;
  */
  param->max_includes=256;
  Scope_init(&param->var_scope_stack);
  /* no need for them due to memset 0
  pbuffer_preinit(&param->builtin_findfile_buffer);
  pbuffer_preinit(&param->lowercase_varname_buffer);
  pbuffer_preinit(&param->uppercase_varname_buffer);
  */
  return param;
}

API_IMPL 
void
APICALL
tmplpro_param_free(struct tmplpro_param* param)
{
  pbuffer_free(&param->builtin_findfile_buffer);
  pbuffer_free(&param->lowercase_varname_buffer);
  pbuffer_free(&param->uppercase_varname_buffer);
  Scope_free(&param->var_scope_stack);
  free(param);
}

#include "tagstack.inc"

/*
 * Local Variables:
 * mode: c 
 * End: 
 */
