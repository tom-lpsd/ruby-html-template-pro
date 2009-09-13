#include "ruby.h"
#include "tmplpro.h"

static ABSTRACT_VALUE* get_ABSTRACT_VALUE_impl (ABSTRACT_MAP* hashPtr, PSTRING name) {
    rb_hash_aref((VALUE)hashPtr, rb_str_new(name.begin, name.endnext - name.begin));
}

static
PSTRING ABSTRACT_VALUE2PSTRING_impl (ABSTRACT_VALUE* valptr) {
    PSTRING retval = {NULL,NULL};  
    if (valptr==NULL || NIL_P(valptr)) return retval;
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
process_tmplpro_options(void)
{
    struct tmplpro_param* param = tmplpro_param_init();
    tmplpro_set_option_GetAbstractValFuncPtr(param, &get_ABSTRACT_VALUE_impl);
    tmplpro_set_option_AbstractVal2pstringFuncPtr(param, &ABSTRACT_VALUE2PSTRING_impl);
    tmplpro_set_option_AbstractVal2abstractArrayFuncPtr(param, &ABSTRACT_VALUE2ABSTRACT_ARRAY_impl);
    tmplpro_set_option_GetAbstractMapFuncPtr(param, &get_ABSTRACT_MAP_impl);
    tmplpro_set_option_filename(param, "foo.tmpl");
    VALUE map = rb_hash_new();
    rb_hash_aset(map, rb_str_new2("foo"), rb_str_new2("bar"));
    tmplpro_set_option_root_param_map(param, map);
    return param;
}

static void
release_tmplpro_options(struct tmplpro_param* param)
{
    tmplpro_param_free(param);
}

VALUE cpro;

void Init_html_template_internal(void)
{
    cpro = rb_define_class("Pro", rb_cObject);
    struct tmplpro_param* proparam = process_tmplpro_options();
    tmplpro_exec_tmpl(proparam);
    release_tmplpro_options(proparam);
}
