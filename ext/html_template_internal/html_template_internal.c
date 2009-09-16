#include "ruby.h"
#include "tmplpro.h"
#define MAX_KEY_LENGTH 256

static ABSTRACT_VALUE* get_ABSTRACT_VALUE_impl (ABSTRACT_MAP* hashPtr, PSTRING name) {
    char tmp = *name.endnext;
    *name.endnext = '\0';
    VALUE key = ID2SYM(rb_intern(name.begin));
    *name.endnext = tmp;
    VALUE val = rb_hash_aref((VALUE)hashPtr, key);
    if (NIL_P(val)) {
        key = rb_str_new(name.begin, name.endnext - name.begin);
        val = rb_hash_aref((VALUE)hashPtr, key);
    }
    return val;
}

static
PSTRING ABSTRACT_VALUE2PSTRING_impl (ABSTRACT_VALUE* valptr) {
    PSTRING retval = {NULL,NULL};

    if (valptr == NULL) return retval;

    if (TYPE(valptr) != T_STRING) {
        ID to_s = rb_intern("to_s");
        valptr = rb_funcall(valptr, to_s, 0);
    }

    retval.begin = StringValuePtr(valptr);
    retval.endnext = retval.begin + RSTRING_LEN(valptr);
    return retval;
}

static 
ABSTRACT_ARRAY* ABSTRACT_VALUE2ABSTRACT_ARRAY_impl (ABSTRACT_VALUE* abstrvalptr) {
    if (TYPE(abstrvalptr) != T_ARRAY) return 0;
    return (ABSTRACT_ARRAY*)abstrvalptr;
}

static 
ABSTRACT_MAP* get_ABSTRACT_MAP_impl (ABSTRACT_ARRAY* loops_ary, int loop) {
    return (ABSTRACT_MAP *)rb_ary_entry(loops_ary, loop);
}

static struct tmplpro_param*
process_tmplpro_options(VALUE self)
{
    struct tmplpro_param* param = tmplpro_param_init();
    tmplpro_set_option_GetAbstractValFuncPtr(param, &get_ABSTRACT_VALUE_impl);
    tmplpro_set_option_AbstractVal2pstringFuncPtr(param, &ABSTRACT_VALUE2PSTRING_impl);
    tmplpro_set_option_AbstractVal2abstractArrayFuncPtr(param, &ABSTRACT_VALUE2ABSTRACT_ARRAY_impl);
    tmplpro_set_option_GetAbstractMapFuncPtr(param, &get_ABSTRACT_MAP_impl);
    tmplpro_set_option_filename(param, "foo.tmpl");
    ID params_id = rb_intern("@params");
    VALUE map = rb_ivar_get(self, params_id);
    tmplpro_set_option_root_param_map(param, map);
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
   tmplpro_exec_tmpl(proparam);
   release_tmplpro_options(proparam);
}

VALUE m_html;
VALUE m_template;
VALUE m_internal;

void Init_html_template_internal(void)
{
    m_html = rb_define_module("HTML");
    m_template = rb_define_module_under(m_html, "Template");
    m_internal = rb_define_module_under(m_template, "Internal");
    rb_define_module_function(m_internal, "exec_tmpl", &exec_tmpl, 1);
}
