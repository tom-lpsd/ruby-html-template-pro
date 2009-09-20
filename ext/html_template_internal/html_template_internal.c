#include "ruby.h"
#include "tmplpro.h"

static int debuglevel = 0;

static void write_chars_to_file (ABSTRACT_WRITER* OutputFile, const char* begin, const char* endnext) {
    rb_io_write((VALUE)OutputFile, rb_str_new(begin, endnext - begin));
}

static void write_chars_to_string (ABSTRACT_WRITER* OutputString, const char* begin, const char* endnext) {
    rb_str_cat((VALUE)OutputString, begin, endnext - begin);
}

static ABSTRACT_VALUE* get_ABSTRACT_VALUE_impl (ABSTRACT_MAP* hashPtr, PSTRING name) {
    VALUE key = ID2SYM(rb_intern2(name.begin, name.endnext - name.begin));
    VALUE val = rb_hash_aref((VALUE)hashPtr, key);
    if (NIL_P(val)) {
        key = rb_str_new(name.begin, name.endnext - name.begin);
        val = rb_hash_aref((VALUE)hashPtr, key);
    }
    return (ABSTRACT_VALUE*)val;
}

static
PSTRING ABSTRACT_VALUE2PSTRING_impl (ABSTRACT_VALUE* valptr) {
    VALUE val = (VALUE)valptr;
    PSTRING retval = {NULL,NULL};

    if (valptr == NULL || NIL_P(valptr)) return retval;

    if (TYPE(val) != T_STRING) {
        ID to_s = rb_intern("to_s");
        val = rb_funcall(val, to_s, 0);
    }
    retval.begin = RSTRING_PTR(val);
    retval.endnext = retval.begin + RSTRING_LEN(val);
    return retval;
}

static
int is_ABSTRACT_VALUE_true_impl (ABSTRACT_VALUE* valptr) {
    if (NIL_P(valptr) || TYPE(valptr) == T_FALSE) return 0;
    return 1;
}

static 
ABSTRACT_ARRAY* ABSTRACT_VALUE2ABSTRACT_ARRAY_impl (ABSTRACT_VALUE* abstrvalptr) {
    if (TYPE(abstrvalptr) != T_ARRAY) return 0;
    return (ABSTRACT_ARRAY*)abstrvalptr;
}

static 
int get_ABSTRACT_ARRAY_length_impl (ABSTRACT_ARRAY* loops) {
    if (TYPE(loops) != T_ARRAY) return 0;
    return RARRAY_LEN((VALUE)loops);
}

static 
ABSTRACT_MAP* get_ABSTRACT_MAP_impl (ABSTRACT_ARRAY* loops_ary, int loop) {
    return (ABSTRACT_MAP *)rb_ary_entry((VALUE)loops_ary, loop);
}

typedef void (*set_int_option_functype) (struct tmplpro_param*, int);

static 
void set_integer_from_hash(VALUE option_hash, char* key, struct tmplpro_param* param, set_int_option_functype setfunc) {
    ID    option_key = ID2SYM(rb_intern(key));
    VALUE option_val = rb_hash_aref(option_hash, option_key);
    setfunc(param, NUM2INT(option_val));
}

static 
void set_boolean_from_hash(VALUE option_hash, char* key, struct tmplpro_param* param, set_int_option_functype setfunc) {
    ID    option_key = ID2SYM(rb_intern(key));
    VALUE option_val = rb_hash_aref(option_hash, option_key);
    int val = NIL_P(option_val) || TYPE(option_val) == T_FALSE ? 0 : 1;
    setfunc(param, val);
}

static 
PSTRING get_string_from_value(VALUE self, char* key) {
  ID key_id = rb_intern(key);
  VALUE strval = rb_ivar_get(self, key_id);
  PSTRING retval={NULL,NULL};
  if (NIL_P(strval)) return retval;
  retval.begin = StringValuePtr(strval);
  retval.endnext = retval.begin + RSTRING_LEN(strval);
  return retval;
}

static struct tmplpro_param*
process_tmplpro_options(VALUE self)
{
    struct tmplpro_param* param = tmplpro_param_init();

    tmplpro_set_option_WriterFuncPtr(param, &write_chars_to_string);
    tmplpro_set_option_GetAbstractValFuncPtr(param, &get_ABSTRACT_VALUE_impl);
    tmplpro_set_option_AbstractVal2pstringFuncPtr(param, &ABSTRACT_VALUE2PSTRING_impl);
    tmplpro_set_option_AbstractVal2abstractArrayFuncPtr(param, &ABSTRACT_VALUE2ABSTRACT_ARRAY_impl);
    tmplpro_set_option_GetAbstractArrayLengthFuncPtr(param, &get_ABSTRACT_ARRAY_length_impl);
    tmplpro_set_option_IsAbstractValTrueFuncPtr(param, &is_ABSTRACT_VALUE_true_impl);
    tmplpro_set_option_GetAbstractMapFuncPtr(param, &get_ABSTRACT_MAP_impl);

    /* checking main arguments */
    PSTRING filename = get_string_from_value(self, "@filename");
    PSTRING scalarref = get_string_from_value(self, "@scalarref");
    tmplpro_set_option_filename(param, filename.begin);
    tmplpro_set_option_scalarref(param, scalarref);
    if (filename.begin==NULL && scalarref.begin==NULL) {
        rb_raise(rb_eRuntimeError, "bad arguments: expected filename or scalarref");
    }

    /* setting param_map */
    VALUE map = rb_ivar_get(self, rb_intern("@params"));
    if (NIL_P(map)) {
        rb_raise(rb_eRuntimeError, "FATAL:output:@param not found");
    }
    tmplpro_set_option_root_param_map(param, (ABSTRACT_MAP*)map);
    /* end setting param_map */

    /* setting filter */
    ;
    /* end setting filter */

    VALUE options = rb_ivar_get(self, rb_intern("@options"));
    if (NIL_P(options)) {
        rb_raise(rb_eRuntimeError, "FATAL:output:@options not found");
    }
    set_integer_from_hash(options,"max_includes",param,tmplpro_set_option_max_includes);
    set_boolean_from_hash(options,"no_includes",param,tmplpro_set_option_no_includes);
    set_boolean_from_hash(options,"search_path_on_include",param,tmplpro_set_option_search_path_on_include);
    set_boolean_from_hash(options,"global_vars",param,tmplpro_set_option_global_vars);
    set_boolean_from_hash(options,"debug",param,tmplpro_set_option_debug);
    debuglevel = tmplpro_get_option_debug(param);
    set_boolean_from_hash(options,"loop_context_vars",param,tmplpro_set_option_loop_context_vars);
    set_boolean_from_hash(options,"case_sensitive",param,tmplpro_set_option_case_sensitive);
    set_boolean_from_hash(options,"path_like_variable_scope",param,tmplpro_set_option_path_like_variable_scope);
    /* still unsupported */
    set_boolean_from_hash(options,"strict",param,tmplpro_set_option_strict);
    set_boolean_from_hash(options,"die_on_bad_params",param,tmplpro_set_option_die_on_bad_params);

    return param;
}

static void
release_tmplpro_options(struct tmplpro_param* param)
{
    tmplpro_param_free(param);
}

static VALUE exec_tmpl(VALUE self, VALUE output)
{
   struct tmplpro_param* proparam = process_tmplpro_options(self);
   writer_functype writer;

   switch (TYPE(output)) {
   case T_STRING:
       writer = &write_chars_to_string;
       break;
   case T_FILE:
       writer = &write_chars_to_file;
       break;
   default:
       rb_warning("bad file descriptor. Use output=stdout\n");
       writer = &write_chars_to_file;
       output = rb_stdout;
   }

   tmplpro_set_option_WriterFuncPtr(proparam, writer);
   tmplpro_set_option_ext_writer_state(proparam, (ABSTRACT_WRITER *)output);
   int retval = tmplpro_exec_tmpl(proparam);
   release_tmplpro_options(proparam);
   return INT2FIX(retval);
}

VALUE mHtml;
VALUE mTemplate;
VALUE mInternal;

void Init_html_template_internal(void)
{
    mHtml = rb_define_module("HTML");
    mTemplate = rb_define_module_under(mHtml, "Template");
    mInternal = rb_define_module_under(mTemplate, "Internal");
    rb_define_module_function(mInternal, "exec_tmpl", &exec_tmpl, 1);
}
