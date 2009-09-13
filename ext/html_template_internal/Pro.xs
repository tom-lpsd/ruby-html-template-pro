#define PERLIO_NOT_STDIO 0    /* For co-existence with stdio only */

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include <string.h>
#include <stdio.h>

#include "ppport.h"

#ifdef USE_SYSTEM_HTP_HEADER
#include <tmplpro.h>
#else
#include "tmplpro.h"
#endif

typedef PerlIO *        OutputStream;

struct perl_callback_state {
  SV* perl_obj_self_ptr;
  AV* filtered_tmpl_array;
  AV* pool_for_perl_vars;
};

static 
int debuglevel=0;

/* endnext points on next character to end of interval as in c++ */
static void write_chars_to_file (ABSTRACT_WRITER* OutputFile, const char* begin, const char* endnext) {
  register const char* i;
  for (i=begin; i<endnext; i++) {
    PerlIO_putc((PerlIO*)OutputFile,*i);
  }
}

/* endnext points on next to end character of the interval */
static void write_chars_to_string (ABSTRACT_WRITER* OutputString, const char* begin, const char* endnext) {
  sv_catpvn((SV*)OutputString, begin, endnext-begin);
}

static
ABSTRACT_VALUE* get_ABSTRACT_VALUE_impl (ABSTRACT_MAP* ptr_HV, PSTRING name) {
  return hv_fetch((HV*) ptr_HV,name.begin, name.endnext-name.begin, 0);
}

static
PSTRING ABSTRACT_VALUE2PSTRING_impl (ABSTRACT_VALUE* valptr) {
  STRLEN len=0;
  PSTRING retval={NULL,NULL};
  SV* SVval;
  if (valptr==NULL) return retval;
  SVval = *((SV**) valptr);
  if (SvOK(SVval)) {
    retval.begin=SvPV(SVval, len);
    retval.endnext=retval.begin+len;
  }
  return retval;
}

static
int is_ABSTRACT_VALUE_true_impl (ABSTRACT_VALUE* valptr) {
  SV* SVval;
  if (valptr==NULL) return 0;
  SVval = *((SV**) valptr);
  if (SvROK(SVval)) {
    /* arrayptr : in HTML::Template, true if len(array)>0 */
    if ((SvTYPE(SvRV(SVval)) == SVt_PVAV)
	&& (av_len((AV *)SvRV(SVval))<0)) {
      return 0;
    } else return 1;
  }
  if(SvTRUE(SVval)) return 1;
  return 0;
}

static 
ABSTRACT_ARRAY* ABSTRACT_VALUE2ABSTRACT_ARRAY_impl (ABSTRACT_VALUE* abstrvalptr) {
  SV** valptr = (SV**) abstrvalptr;
  if ((!SvROK(*valptr)) || (SvTYPE(SvRV(*valptr)) != SVt_PVAV)) return 0;
  return (ABSTRACT_ARRAY*) SvRV(*valptr);
}

static 
int get_ABSTRACT_ARRAY_length_impl (ABSTRACT_ARRAY* loops_AV) {
  return av_len((AV *)loops_AV)+1;
}

static 
ABSTRACT_MAP* get_ABSTRACT_MAP_impl (ABSTRACT_ARRAY* loops_AV, int loop) {
  SV** arrayvalptr;
  arrayvalptr=av_fetch((AV*)loops_AV, loop, 0);
  if ((arrayvalptr==NULL) || (!SvROK(*arrayvalptr)) || (SvTYPE(SvRV(*arrayvalptr)) != SVt_PVHV)) {
    return NULL;
  } else {
    return (ABSTRACT_MAP *)SvRV(*arrayvalptr);
  }
}

static 
const char* get_filepath (ABSTRACT_FINDFILE* callback_state, const char* filename, const char* prevfilename) {
  dSP ;
  int count ;
  STRLEN len;
  char* filepath;
  SV* perlprevfile;
  SV* PerlSelfHTMLTemplatePro = ((struct perl_callback_state*)callback_state)->perl_obj_self_ptr;
  SV* perlretval = sv_2mortal(newSVpv(filename,0));
  if (prevfilename) {
    perlprevfile=sv_2mortal(newSVpv(prevfilename,0));
  } else {
    perlprevfile=sv_2mortal(newSV(0));
  }
  ENTER ;
  SAVETMPS;
  PUSHMARK(SP) ;
  XPUSHs((SV*)PerlSelfHTMLTemplatePro);
  XPUSHs(perlretval);
  XPUSHs(perlprevfile);
  PUTBACK ;
  count = call_pv("_get_filepath", G_SCALAR);
  SPAGAIN ;
  if (count != 1) croak("Big troublen") ;
  perlretval=POPs;
  /* any memory leaks??? */  
  if (SvOK(perlretval)) {
    filepath = SvPV(perlretval, len);
    av_push(((struct perl_callback_state*)callback_state)->pool_for_perl_vars,perlretval);
    SvREFCNT_inc(perlretval);
  } else {
    filepath = NULL;
  }
  PUTBACK ;
  FREETMPS ;
  LEAVE ;
  return filepath;
}

static 
PSTRING load_file (ABSTRACT_FILTER* callback_state, const char* filepath) {
  dSP ;
  int count ;
  STRLEN len;
  PSTRING tmpl;
  SV* templateptr;
  SV* perlretval = sv_2mortal(newSVpv(filepath,0));
  ENTER ;
  SAVETMPS;
  PUSHMARK(SP) ;
  XPUSHs(((struct perl_callback_state*)callback_state)->perl_obj_self_ptr);
  XPUSHs(perlretval);
  PUTBACK ;
  count = call_pv("_load_template", G_SCALAR);
  SPAGAIN ;
  if (count != 1) croak("Big troublen") ;
  templateptr=POPs;
  /* any memory leaks??? */  
  if (SvOK(templateptr) && SvROK(templateptr)) {
    tmpl.begin = SvPV(SvRV(templateptr), len);
    tmpl.endnext=tmpl.begin+len;
    av_push(((struct perl_callback_state*)callback_state)->filtered_tmpl_array,templateptr);
    SvREFCNT_inc(SvRV(templateptr));
  } else {
    croak("Big trouble! _load_template internal fatal error\n") ;
  }
  PUTBACK ;
  FREETMPS ;
  LEAVE ;
  return tmpl;
}

static
int unload_file(ABSTRACT_FILTER* callback_state, PSTRING memarea) {
  /*
   * scalar is already mortal so we don't need dereference it
   * SvREFCNT_dec(SvRV(av_pop(((struct perl_callback_state*)callback_state)->filtered_tmpl_array)));
   */
  av_pop(((struct perl_callback_state*)callback_state)->filtered_tmpl_array); 
  return 0;
}

static 
ABSTRACT_USERFUNC* is_expr_userfnc (ABSTRACT_FUNCMAP* FuncHash, PSTRING name) {
  SV** hashvalptr=hv_fetch((HV *) FuncHash, name.begin, name.endnext-name.begin, 0);
  return hashvalptr;
}

static 
void free_expr_arglist(ABSTRACT_ARGLIST* arglist)
{
  if (NULL!=arglist) {
    av_undef((AV*) arglist);
    SvREFCNT_dec(arglist);
  }
}

static 
ABSTRACT_ARGLIST* init_expr_arglist(ABSTRACT_CALLER* none)
{
  return newAV();
}

static 
void push_expr_arglist(ABSTRACT_ARGLIST* arglist, ABSTRACT_EXPRVAL* exprval)
{
  SV* val=NULL;
  int exprval_type=tmplpro_get_expr_type(exprval);
  PSTRING parg;
  switch (exprval_type) {
  case EXPR_TYPE_NULL: val=newSV(0);break;
  case EXPR_TYPE_INT:  val=newSViv(tmplpro_get_expr_as_int64(exprval));break;
  case EXPR_TYPE_DBL:  val=newSVnv(tmplpro_get_expr_as_double(exprval));break;
  case EXPR_TYPE_PSTR: parg=tmplpro_get_expr_as_pstring(exprval);
                 val=newSVpvn(parg.begin, parg.endnext-parg.begin);break;
  default: die ("Perl wrapper: FATAL INTERNAL ERROR:Unsupported type %d in exprval", exprval_type);
  }
  av_push ((AV*) arglist, val);
}

static 
void call_expr_userfnc (ABSTRACT_CALLER* callback_state, ABSTRACT_ARGLIST* arglist, ABSTRACT_USERFUNC* hashvalptr, ABSTRACT_EXPRVAL* exprval) {
  dSP ;
  char* empty="";
  char* strval;
  SV ** arrval;
  SV * svretval;
  I32 i;
  I32 numretval;
  I32 arrlen=av_len((AV *) arglist);
  PSTRING retvalpstr = { empty, empty };
  retvalpstr.begin=empty;
  retvalpstr.endnext=empty;
  if (hashvalptr==NULL) {
    die ("FATAL INTERNAL ERROR:Call_EXPR:function called but not exists");
    tmplpro_set_expr_as_pstring(exprval,retvalpstr);
    return;
  } else if (! SvROK(*((SV**) hashvalptr)) || (SvTYPE(SvRV(*((SV**) hashvalptr))) != SVt_PVCV)) {
    die ("FATAL INTERNAL ERROR:Call_EXPR:not a function reference");
    tmplpro_set_expr_as_pstring(exprval,retvalpstr);
    return;
  }
  
  ENTER ;
  SAVETMPS ;
  
  PUSHMARK(SP) ;
  for (i=0;i<=arrlen;i++) {
    arrval=av_fetch((AV *) arglist,i,0);
    if (arrval) XPUSHs(*arrval);
    else warn("INTERNAL: call: strange arrval");
  }
  PUTBACK ;
  numretval=call_sv(*((SV**) hashvalptr), G_SCALAR);
  SPAGAIN ;
  if (numretval) {
    svretval=POPs;
    if (SvOK(svretval)) {
      if (SvIOK(svretval)) {
	tmplpro_set_expr_as_int64(exprval,SvIV(svretval));
      } else if (SvNOK(svretval)) {
	tmplpro_set_expr_as_double(exprval,SvNV(svretval));
      } else {
	STRLEN len=0;
	strval =SvPV(svretval, len);
	/* hack !!! */
	av_push(((struct perl_callback_state*)callback_state)->pool_for_perl_vars,svretval);
	SvREFCNT_inc(svretval);
	retvalpstr.begin=strval;
	retvalpstr.endnext=strval +len;
	tmplpro_set_expr_as_pstring(exprval,retvalpstr);
      }
    } else {
      if (debuglevel) warn ("user defined function returned undef\n");
    }
  } else {
    if (debuglevel) warn ("user defined function returned nothing\n");
  }

  FREETMPS ;
  LEAVE ;

  return;
}

typedef void (*set_int_option_functype) (struct tmplpro_param*, int);

static 
void set_integer_from_hash(HV* TheHash, char* key, struct tmplpro_param* param, set_int_option_functype setfunc) {
  SV** hashvalptr=hv_fetch(TheHash, key, strlen(key), 0);
  if (hashvalptr==NULL) return;
  setfunc(param,SvIV(*hashvalptr));
}

static 
int get_integer_from_hash(HV* TheHash, char* key) {
  SV** hashvalptr=hv_fetch(TheHash, key, strlen(key), 0);
  if (hashvalptr==NULL) return 0;
  return SvIV(*hashvalptr);
}

static 
PSTRING get_string_from_hash(HV* TheHash, char* key) {
  SV** hashvalptr=hv_fetch(TheHash, key, strlen(key), 0);
  STRLEN len=0;
  char * begin;
  PSTRING retval={NULL,NULL};
  if (hashvalptr==NULL) return retval;
  if (SvROK(*hashvalptr)) {
    /* if (SvTYPE(SvRV(*hashvalptr))!=SVt_PV) return (PSTRING) {NULL,NULL}; */
    begin=SvPV(SvRV(*hashvalptr),len);
  } else {
    if (! SvPOK(*hashvalptr)) return retval;
    begin=SvPV(*hashvalptr,len);
  }
  retval.begin=begin;
  retval.endnext=begin+len;
  return retval;
}


static 
char** get_array_of_strings_from_hash(HV* TheHash, char* key, struct perl_callback_state* callback_state) {
  SV** valptr=hv_fetch(TheHash, key, strlen(key), 0);
  int amax;
  char** path=NULL;
  AV* pathAV;
  int i =0;
  char** j;
  SV* store;
  if (valptr!=NULL && SvROK(*valptr) && (SvTYPE(SvRV(*valptr)) == SVt_PVAV) ) {
    pathAV=(AV *)SvRV(*valptr);
    amax=av_len(pathAV);
    if (amax<0) {
      return NULL;
    } else {
      store = newSV(sizeof(char*)*(amax+2));
      path = (char**) SvGROW(store, sizeof(char*)*(amax+2));
      av_push(((struct perl_callback_state*)callback_state)->pool_for_perl_vars,store);
      SvREFCNT_inc(store);
      //path=(char**) malloc(sizeof(char*)*(amax+2));
      j=path;
      for (i=0; i<=amax;i++) {
	valptr = av_fetch(pathAV,i,0);
	if (valptr!=NULL) {
	  *j=SvPV_nolen(*valptr);
	  j++;
	}
	*j=NULL;
      }
    }
  } else {
    warn ("get_array_of_strings:option %s not found :(\n", key);
  }
  return path;
}

static 
struct tmplpro_param* process_tmplpro_options (struct perl_callback_state* callback_state) {
  HV* SelfHash;
  SV** hashvalptr;
  const char* tmpstring;
  SV* PerlSelfPtr=callback_state->perl_obj_self_ptr;
  int default_escape=HTML_TEMPLATE_OPT_ESCAPE_NO;

  /* main arguments */
  PSTRING filename;
  PSTRING scalarref;

  /* internal initialization */
  struct tmplpro_param* param=tmplpro_param_init();

  /*   setting initial hooks */
  tmplpro_set_option_WriterFuncPtr(param, &write_chars_to_string);
  tmplpro_set_option_GetAbstractValFuncPtr(param, &get_ABSTRACT_VALUE_impl);
  tmplpro_set_option_AbstractVal2pstringFuncPtr(param, &ABSTRACT_VALUE2PSTRING_impl);
  tmplpro_set_option_AbstractVal2abstractArrayFuncPtr(param, &ABSTRACT_VALUE2ABSTRACT_ARRAY_impl);
  tmplpro_set_option_GetAbstractArrayLengthFuncPtr(param, &get_ABSTRACT_ARRAY_length_impl);
  tmplpro_set_option_IsAbstractValTrueFuncPtr(param, &is_ABSTRACT_VALUE_true_impl);
  tmplpro_set_option_GetAbstractMapFuncPtr(param, &get_ABSTRACT_MAP_impl);
  tmplpro_set_option_LoadFileFuncPtr(param, &load_file);
  tmplpro_set_option_UnloadFileFuncPtr(param, &unload_file);

  /*   setting initial Expr hooks */
  tmplpro_set_option_InitExprArglistFuncPtr(param, &init_expr_arglist);
  tmplpro_set_option_FreeExprArglistFuncPtr(param, &free_expr_arglist);
  tmplpro_set_option_PushExprArglistFuncPtr(param, &push_expr_arglist);
  tmplpro_set_option_CallExprUserfncFuncPtr(param, &call_expr_userfnc);
  tmplpro_set_option_IsExprUserfncFuncPtr(param, &is_expr_userfnc);
  /* end setting initial hooks */

  /*   setting perl globals */
  tmplpro_set_option_ext_findfile_state(param,callback_state);
  tmplpro_set_option_ext_filter_state(param,callback_state);
  tmplpro_set_option_ext_calluserfunc_state(param,callback_state);
  /*  end setting perl globals */

  if ((!SvROK(PerlSelfPtr)) || (SvTYPE(SvRV(PerlSelfPtr)) != SVt_PVHV))
    {
      die("FATAL:SELF:hash pointer was expected but not found");
    }
  SelfHash=(HV *)SvRV(PerlSelfPtr);

  /* checking main arguments */
  filename=get_string_from_hash(SelfHash,"filename");
  scalarref=get_string_from_hash(SelfHash,"scalarref");
  tmplpro_set_option_filename(param, filename.begin);
  tmplpro_set_option_scalarref(param, scalarref);
  if (filename.begin==NULL && scalarref.begin==NULL) {
    die ("bad arguments: expected filename or scalarref");
  }
  
  /* setting expr_func */
  hashvalptr=hv_fetch(SelfHash, "expr_func", 9, 0); /* 9=strlen("expr_func") */
  if (!hashvalptr || !SvROK(*hashvalptr) || (SvTYPE(SvRV(*hashvalptr)) != SVt_PVHV))
    die("FATAL:output:EXPR user functions not found");
  tmplpro_set_option_expr_func_map(param, (HV *) SvRV(*hashvalptr));
  /* end setting expr_func */

  /* setting param_map */
  hashvalptr=hv_fetch(SelfHash, "param_map", 9, 0); /* 9=strlen("param_map") */
  if (!hashvalptr || !SvROK(*hashvalptr) || (SvTYPE(SvRV(*hashvalptr)) != SVt_PVHV))
    die("FATAL:output:param_map not found");

  tmplpro_set_option_root_param_map(param,  (HV *)SvRV(*hashvalptr));

  /* end setting param_map */

  /* setting filter */
  hashvalptr=hv_fetch(SelfHash, "filter", 6, 0); /* 6=strlen("filter") */
  if (!hashvalptr || !SvROK(*hashvalptr) || (SvTYPE(SvRV(*hashvalptr)) != SVt_PVAV))
    die("FATAL:output:filter not found");
  if (av_len((AV*)SvRV(*hashvalptr))>=0) tmplpro_set_option_filters(param, 1);
  /* end setting param_map */

  set_integer_from_hash(SelfHash,"max_includes",param,tmplpro_set_option_max_includes);
  set_integer_from_hash(SelfHash,"no_includes",param,tmplpro_set_option_no_includes);
  set_integer_from_hash(SelfHash,"search_path_on_include",param,tmplpro_set_option_search_path_on_include);
  set_integer_from_hash(SelfHash,"global_vars",param,tmplpro_set_option_global_vars);
  set_integer_from_hash(SelfHash,"debug",param,tmplpro_set_option_debug);
  debuglevel = tmplpro_get_option_debug(param);
  set_integer_from_hash(SelfHash,"loop_context_vars",param,tmplpro_set_option_loop_context_vars);
  set_integer_from_hash(SelfHash,"case_sensitive",param,tmplpro_set_option_case_sensitive);
  set_integer_from_hash(SelfHash,"path_like_variable_scope",param,tmplpro_set_option_path_like_variable_scope);
  /* still unsupported */
  set_integer_from_hash(SelfHash,"strict",param,tmplpro_set_option_strict);
  set_integer_from_hash(SelfHash,"die_on_bad_params",param,tmplpro_set_option_die_on_bad_params);
 
  tmpstring=get_string_from_hash(SelfHash,"default_escape").begin;
  if (tmpstring && *tmpstring) {
    switch (*tmpstring) {
    case '1': case 'H': case 'h': 	/* HTML*/
      default_escape = HTML_TEMPLATE_OPT_ESCAPE_HTML;
      break;
    case 'U': case 'u': 		/* URL */
      default_escape = HTML_TEMPLATE_OPT_ESCAPE_URL;
      break;
    case 'J': case 'j':		/* JS  */
      default_escape = HTML_TEMPLATE_OPT_ESCAPE_JS;
      break;
    case '0': case 'N': case 'n': /* 0 or NONE */
      default_escape = HTML_TEMPLATE_OPT_ESCAPE_NO;
      break;
    default:
      warn("unsupported value of default_escape=%s. Valid values are HTML, URL or JS.\n",tmpstring);
    }
    tmplpro_set_option_default_escape(param, default_escape);

  }

  if (get_integer_from_hash(SelfHash,"__use_perl_find_file")) {
    tmplpro_set_option_FindFileFuncPtr(param, &get_filepath);
  } else {
    tmplpro_set_option_path(param, get_array_of_strings_from_hash(SelfHash, "path", callback_state));
    tmplpro_set_option_FindFileFuncPtr(param, NULL);
  }
  return param;
}

static void
release_tmplpro_options(struct tmplpro_param* param, struct perl_callback_state callback_state)
{
  av_undef(callback_state.filtered_tmpl_array);
  av_undef(callback_state.pool_for_perl_vars);
  tmplpro_param_free(param);
}



MODULE = HTML::Template::Pro		PACKAGE = HTML::Template::Pro

void 
_init()
    CODE:
	tmplpro_procore_init();

void 
_done()
    CODE:
	tmplpro_procore_done();


int
exec_tmpl(self_ptr,possible_output)
	SV* self_ptr;
	OutputStream possible_output;
    CODE:
	struct perl_callback_state callback_state = {self_ptr,newAV(),newAV()};
	struct tmplpro_param* proparam=process_tmplpro_options(&callback_state);
	if (possible_output == NULL){
	  warn("bad file descriptor. Use output=stdout\n");
	  tmplpro_set_option_ext_writer_state(proparam,PerlIO_stdout());
	} else {
	  tmplpro_set_option_ext_writer_state(proparam,possible_output);
	}
	tmplpro_set_option_WriterFuncPtr(proparam,&write_chars_to_file);
	RETVAL = tmplpro_exec_tmpl(proparam);
	release_tmplpro_options(proparam,callback_state);
	if (RETVAL!=0) warn ("Pro.xs: non-zero exit code %d",RETVAL);
    OUTPUT:
	RETVAL



SV*
exec_tmpl_string(self_ptr)
	SV* self_ptr;
    CODE:
	int retstate;
	SV* OutputString;
	struct perl_callback_state callback_state = {self_ptr,newAV(),newAV()};
	/* TODO: */
	/* 1) estimate string as 2*filesize */
	/* 2) make it mortal ? auto...  */
	struct tmplpro_param* proparam=process_tmplpro_options(&callback_state);
	OutputString=newSV(256); /* 256 allocated bytes -- should be approx. filesize*/
	sv_setpvn(OutputString, "", 0);
	tmplpro_set_option_WriterFuncPtr(proparam,&write_chars_to_string);
	tmplpro_set_option_ext_writer_state(proparam,OutputString);
	retstate = tmplpro_exec_tmpl(proparam);
	release_tmplpro_options(proparam,callback_state);
	if (retstate!=0) warn ("Pro.xs: non-zero exit code %d",retstate);
	RETVAL = OutputString;
    OUTPUT:
	RETVAL

