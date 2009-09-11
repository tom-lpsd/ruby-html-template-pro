#include "ruby.h"
#include "tmplpro.h"

VALUE cpro;

void Init_pro(void)
{
    cpro = rb_define_class("Pro", rb_cObject);
}
