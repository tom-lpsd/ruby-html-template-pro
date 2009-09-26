#include "ruby.h"
#include "tmplpro.h"

static int debuglevel = 0;

struct ruby_callback_state {
    char **pathlist;
};

static struct ruby_callback_state *new_ruby_callback_state() {
    struct ruby_callback_state *state = (struct ruby_callback_state*)malloc(sizeof(struct ruby_callback_state));
    state->pathlist = NULL;
    return state;
}

static void free_ruby_callback_state(struct ruby_callback_state *state) {
    free(state->pathlist);
    free(state);
}

static void write_chars_to_file (ABSTRACT_WRITER* OutputFile, const char* begin, const char* endnext) {
    rb_io_write((VALUE)OutputFile, rb_str_new(begin, endnext - begin));
}

static void write_chars_to_string (ABSTRACT_WRITER* OutputString, const char* begin, const char* endnext) {
    rb_str_cat((VALUE)OutputString, begin, endnext - begin);
}

static ABSTRACT_VALUE* get_ABSTRACT_VALUE_impl (ABSTRACT_DATASTATE *none, ABSTRACT_MAP* hashPtr, PSTRING name) {
    VALUE key = ID2SYM(rb_intern2(name.begin, name.endnext - name.begin));
    VALUE val = rb_hash_aref((VALUE)hashPtr, key);
    if (NIL_P(val)) {
        key = rb_str_new(name.begin, name.endnext - name.begin);
        val = rb_hash_aref((VALUE)hashPtr, key);
    }
    return NIL_P(val) ? NULL : (ABSTRACT_VALUE*)val;
}

static
PSTRING ABSTRACT_VALUE2PSTRING_impl (ABSTRACT_DATASTATE *none, ABSTRACT_VALUE* valptr) {
    VALUE val = (VALUE)valptr;
    PSTRING retval = {NULL,NULL};

    if (valptr == NULL) return retval;

    if (rb_obj_is_instance_of(val, rb_cProc)) {
        val = rb_proc_call(val, rb_ary_new());
    }

    if (TYPE(val) != T_STRING) {
        ID to_s = rb_intern("to_s");
        val = rb_funcall(val, to_s, 0);
    }
    retval.begin = RSTRING_PTR(val);
    retval.endnext = retval.begin + RSTRING_LEN(val);
    return retval;
}

static
int is_ABSTRACT_VALUE_true_impl (ABSTRACT_DATASTATE *none, ABSTRACT_VALUE* valptr) {
    VALUE val = (VALUE)valptr;

    if (NIL_P(val)) return 0;

    if (rb_obj_is_instance_of(val, rb_cProc)) {
        val = rb_proc_call(val, rb_ary_new());
    }

    if (NIL_P(val) || TYPE(val) == T_FALSE) return 0;

    return 1;
}

static 
ABSTRACT_ARRAY* ABSTRACT_VALUE2ABSTRACT_ARRAY_impl (ABSTRACT_DATASTATE *none, ABSTRACT_VALUE* abstrvalptr) {
    if (TYPE(abstrvalptr) != T_ARRAY) return 0;
    return (ABSTRACT_ARRAY*)abstrvalptr;
}

static 
int get_ABSTRACT_ARRAY_length_impl (ABSTRACT_DATASTATE *none, ABSTRACT_ARRAY* loops) {
    if (TYPE(loops) != T_ARRAY) return 0;
    return RARRAY_LEN((VALUE)loops);
}

static 
ABSTRACT_MAP* get_ABSTRACT_MAP_impl (ABSTRACT_DATASTATE *none, ABSTRACT_ARRAY* loops_ary, int loop) {
    return (ABSTRACT_MAP *)rb_ary_entry((VALUE)loops_ary, loop);
}

typedef void (*set_int_option_functype) (struct tmplpro_param*, int);

static 
void set_integer_from_hash(VALUE option_hash, char* key, struct tmplpro_param* param, set_int_option_functype setfunc) {
    VALUE option_key = ID2SYM(rb_intern(key));
    VALUE option_val = rb_hash_aref(option_hash, option_key);
    if (NIL_P(option_val)) return;
    setfunc(param, NUM2INT(option_val));
}

static 
void set_boolean_from_hash(VALUE option_hash, char* key, struct tmplpro_param* param, set_int_option_functype setfunc) {
    VALUE option_key = ID2SYM(rb_intern(key));
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

static 
int get_boolean_from_hash(VALUE options, char* keystr) {
    VALUE key = ID2SYM(rb_intern(keystr));
    VALUE val = rb_hash_aref(options, key);
    return NIL_P(val) || TYPE(val) == T_FALSE ? 0 : 1;
}

static 
PSTRING load_file (ABSTRACT_FILTER* callback_state, const char* filepath) {
    PSTRING tmpl;
    VALUE self = (VALUE)callback_state;
    VALUE path = rb_str_new_cstr(filepath);
    VALUE result = rb_funcall(self, rb_intern("load_template"), 1, path);
    if (!NIL_P(result) && TYPE(result) == T_STRING) {
        tmpl.begin   = StringValuePtr(result);
        tmpl.endnext = tmpl.begin + RSTRING_LEN(result);
    }
    else {
        rb_raise(rb_eRuntimeError, "Big trouble! load_template internal fatal error\n") ;
    }
    return tmpl;
}

static
int unload_file(ABSTRACT_FILTER* callback_state, PSTRING memarea) {
  return 0;
}

static 
ABSTRACT_USERFUNC* is_expr_userfnc (ABSTRACT_FUNCMAP* FuncHash, PSTRING name) {
    VALUE key = ID2SYM(rb_intern2(name.begin, name.endnext - name.begin));
    VALUE val = rb_hash_aref((VALUE)FuncHash, key);
    if (NIL_P(val)) return NULL;
    return (ABSTRACT_USERFUNC *)val;
}

static
void free_expr_arglist(ABSTRACT_ARGLIST* arglist) {
    return;
}

static 
ABSTRACT_ARGLIST* init_expr_arglist(ABSTRACT_CALLER* callback_state)
{
    VALUE self = (VALUE)callback_state;
    VALUE retval = rb_ary_new();
    VALUE registory = rb_ivar_get(self, rb_intern("@expr_results"));
    rb_ary_push(registory, retval);
    return (ABSTRACT_ARGLIST*)retval;
}

static 
void push_expr_arglist(ABSTRACT_ARGLIST* arglist, ABSTRACT_EXPRVAL* exprval)
{
    VALUE val = Qnil;
    int exprval_type = tmplpro_get_expr_type(exprval);
    PSTRING parg;
    switch (exprval_type) {
    case EXPR_TYPE_NULL: val = Qnil; break;
    case EXPR_TYPE_INT:  val = INT2NUM(tmplpro_get_expr_as_int64(exprval)); break;
    case EXPR_TYPE_DBL:  val = rb_float_new(tmplpro_get_expr_as_double(exprval)); break;
    case EXPR_TYPE_PSTR:
        parg = tmplpro_get_expr_as_pstring(exprval);
        val = rb_str_new(parg.begin, parg.endnext - parg.begin);
        break;
    default:
        rb_raise(rb_eRuntimeError, "Ruby wrapper: FATAL INTERNAL ERROR:Unsupported type %d in exprval", exprval_type);
    }
    rb_ary_push((VALUE)arglist, val);
}

static 
void call_expr_userfnc (ABSTRACT_CALLER* callback_state, ABSTRACT_ARGLIST* arglist, ABSTRACT_USERFUNC* hashvalptr, ABSTRACT_EXPRVAL* exprval) {
    char* empty = "";
    VALUE self = (VALUE)callback_state;
    VALUE func = (VALUE)hashvalptr;
    VALUE args = (VALUE)arglist;
    PSTRING retvalpstr = { empty, empty };

    if (hashvalptr==NULL) {
        rb_raise(rb_eRuntimeError, "FATAL INTERNAL ERROR:Call_EXPR:function called but not exists");
        tmplpro_set_expr_as_pstring(exprval, retvalpstr);
        return;
    } else if (NIL_P(func) || !rb_obj_is_instance_of(func, rb_cProc)) {
        rb_raise(rb_eRuntimeError, "FATAL INTERNAL ERROR:Call_EXPR:not a Proc object");
        tmplpro_set_expr_as_pstring(exprval, retvalpstr);
        return;
    }

    VALUE retval = rb_proc_call(func, args);
    VALUE registory = rb_ivar_get(self, rb_intern("@expr_results"));
    rb_ary_push(registory, retval);

    switch (TYPE(retval)) {
    case T_FIXNUM:
	tmplpro_set_expr_as_int64(exprval, FIX2LONG(retval));
        break;
    case T_TRUE:
	tmplpro_set_expr_as_int64(exprval, 1);
        break;
    case T_FALSE:
	tmplpro_set_expr_as_int64(exprval, 0);
        break;
    case T_FLOAT:
        tmplpro_set_expr_as_double(exprval, NUM2DBL(retval));
        break;
    default:
        retvalpstr.begin = StringValuePtr(retval);
	retvalpstr.endnext = retvalpstr.begin + RSTRING_LEN(retval);
	tmplpro_set_expr_as_pstring(exprval, retvalpstr);
    }

    return;
}

static 
char** set_pathlist(VALUE self, VALUE options, char* param_name) {
    long i, len;
    struct ruby_callback_state *state;
    VALUE key = ID2SYM(rb_intern(param_name));
    VALUE paths = rb_hash_aref(options, key);
    VALUE callback_state = rb_ivar_get(self, rb_intern("@callback"));
    if (NIL_P(paths) || NIL_P(callback_state)) return NULL;
    Data_Get_Struct(callback_state, struct ruby_callback_state, state);
    if (TYPE(paths) != T_ARRAY) rb_raise(rb_eRuntimeError, "path param is not array");
    len = RARRAY_LEN(paths);
    if (len < 0) return NULL;
    state->pathlist = (char **)malloc(sizeof(char*) * (len + 1));
    for (i=0;i<len;i++) {
        VALUE elem = rb_ary_entry(paths, i);
        state->pathlist[i] = StringValueCStr(elem);
    }
    state->pathlist[len] = NULL;
    return state->pathlist;
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
    tmplpro_set_option_LoadFileFuncPtr(param, &load_file);
    tmplpro_set_option_UnloadFileFuncPtr(param, &unload_file);

    /*   setting ruby globals */
    tmplpro_set_option_ext_findfile_state(param, (ABSTRACT_FINDFILE *)self);
    tmplpro_set_option_ext_filter_state(param, (ABSTRACT_FILTER *)self);
    tmplpro_set_option_ext_calluserfunc_state(param, (ABSTRACT_CALLER *)self);
    tmplpro_set_option_ext_data_state(param, (ABSTRACT_DATASTATE *)self);
    /*  end setting ruby globals */

    /*   setting initial Expr hooks */
    tmplpro_set_option_InitExprArglistFuncPtr(param, &init_expr_arglist);
    tmplpro_set_option_FreeExprArglistFuncPtr(param, &free_expr_arglist);
    tmplpro_set_option_PushExprArglistFuncPtr(param, &push_expr_arglist);
    tmplpro_set_option_CallExprUserfncFuncPtr(param, &call_expr_userfnc);
    tmplpro_set_option_IsExprUserfncFuncPtr(param, &is_expr_userfnc);
    /* end setting initial hooks */

    if ((void*)self == NULL) {
        rb_raise(rb_eRuntimeError, "FATAL:SELF:self was expected but not found");
    }
    Check_Type(self, T_OBJECT);

    /* checking main arguments */
    PSTRING filename = get_string_from_value(self, "@filename");
    PSTRING scalarref = get_string_from_value(self, "@scalarref");
    tmplpro_set_option_filename(param, filename.begin);
    tmplpro_set_option_scalarref(param, scalarref);
    if (filename.begin==NULL && scalarref.begin==NULL) {
        rb_raise(rb_eRuntimeError, "bad arguments: expected filename or scalarref");
    }

    /* setting param_map */
    tmplpro_clear_option_param_map(param);
    VALUE map = rb_ivar_get(self, rb_intern("@params"));
    Check_Type(map, T_HASH);
    tmplpro_push_option_param_map(param, (ABSTRACT_MAP*)map, 0);
    /* end setting param_map */

    VALUE options = rb_ivar_get(self, rb_intern("@options"));
    Check_Type(options, T_HASH);

    /* setting expr_func */
    VALUE expr_key = ID2SYM(rb_intern("expr_func"));
    VALUE expr = rb_hash_aref(options, expr_key);
    if (!expr || NIL_P(expr) || TYPE(expr) != T_HASH) {
        rb_raise(rb_eRuntimeError, "FATAL:output:EXPR user functions not found");
    }
    tmplpro_set_option_expr_func_map(param, (ABSTRACT_FUNCMAP *)expr);
    /* end setting expr_func */

    /* setting filter */
    VALUE filter_key = ID2SYM(rb_intern("filter"));
    VALUE filter = rb_hash_aref(options, filter_key);
    Check_Type(filter, T_ARRAY);
    if (RARRAY_LEN(filter) >= 0) tmplpro_set_option_filters(param, 1);
    /* end setting filter */

    if (!get_boolean_from_hash(options, "case_sensitive")) {
        tmplpro_set_option_tmpl_var_case(param, ASK_NAME_LOWERCASE);
    }

    set_integer_from_hash(options,"tmpl_var_case",param,tmplpro_set_option_tmpl_var_case);
    set_integer_from_hash(options,"max_includes",param,tmplpro_set_option_max_includes);
    set_boolean_from_hash(options,"no_includes",param,tmplpro_set_option_no_includes);
    set_boolean_from_hash(options,"search_path_on_include",param,tmplpro_set_option_search_path_on_include);
    set_boolean_from_hash(options,"global_vars",param,tmplpro_set_option_global_vars);
    set_integer_from_hash(options,"debug",param,tmplpro_set_option_debug);
    debuglevel = tmplpro_get_option_debug(param);
    set_boolean_from_hash(options,"loop_context_vars",param,tmplpro_set_option_loop_context_vars);
    set_boolean_from_hash(options,"path_like_variable_scope",param,tmplpro_set_option_path_like_variable_scope);
    /* still unsupported */
    set_boolean_from_hash(options,"strict",param,tmplpro_set_option_strict);

    /* set path */
    tmplpro_set_option_path(param, set_pathlist(self, options, "path"));
    tmplpro_set_option_FindFileFuncPtr(param, NULL);

    return param;
}

static void
release_tmplpro_options(struct tmplpro_param* param)
{
    tmplpro_param_free(param);
}

VALUE mHtml;
VALUE mTemplate;
VALUE mInternal;
VALUE cHTMLTemplateInternalCallbackState;

static void set_callback_state(VALUE self)
{
    ID id = rb_intern("@callback");
    struct ruby_callback_state *state = new_ruby_callback_state();
    VALUE callback_state = Data_Wrap_Struct(cHTMLTemplateInternalCallbackState, 0,
                                            &free_ruby_callback_state, state);
    rb_ivar_set(self, id, callback_state);
}

static VALUE exec_tmpl(VALUE self, VALUE output)
{
    writer_functype writer;
    struct tmplpro_param* proparam;
    set_callback_state(self);
    proparam = process_tmplpro_options(self);

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

void Init_html_template_internal(void)
{
    mHtml = rb_define_module("HTML");
    mTemplate = rb_define_module_under(mHtml, "Template");
    mInternal = rb_define_module_under(mTemplate, "Internal");
    rb_define_module_function(mInternal, "exec_tmpl", &exec_tmpl, 1);
    cHTMLTemplateInternalCallbackState = rb_define_class_under(mInternal, "CallbackState", rb_cObject);
}
