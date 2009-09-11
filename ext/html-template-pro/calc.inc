/* -*- c -*- 
 * File: calc.c
 * $Id$
 */

#include <stdlib.h>
#include <string.h>

static
const symrec_const *
getsym (const symrec_const symrec_array[], PSTRING sym_name)
{
  const symrec_const* ptr;
  for (ptr=symrec_array; ptr->name!=NULL; ptr++) 
    if (strncmp (ptr->name,sym_name.begin,sym_name.endnext-sym_name.begin) == 0)
      return ptr;
  return 0;
}

#define ABS(X) (((X)<0)? (-X) : (X))

static 
struct exprval builtin_int (struct expr_parser* exprobj, struct exprval e) {
  expr_to_int1(exprobj, &e);
  return e;
}

static 
struct exprval builtin_abs (struct expr_parser* exprobj, struct exprval e) {
  expr_to_int_or_dbl1(exprobj, &e);
  if (e.type==EXPR_TYPE_DBL) e.val.dblval = ABS(e.val.dblval);
  else if (e.type==EXPR_TYPE_INT) e.val.intval = ABS(e.val.intval);
  return e;
}

static 
struct exprval builtin_defined (struct expr_parser* exprobj, struct exprval e) {
  struct exprval retval = { EXPR_TYPE_INT };
  if (e.type==EXPR_TYPE_NULL || 
      (e.type==EXPR_TYPE_PSTR && e.val.strval.begin == NULL)) retval.val.intval = 0;
  else retval.val.intval = 1;
  return retval;
}

static 
struct exprval builtin_length (struct expr_parser* exprobj, struct exprval e) {
  struct exprval retval = { EXPR_TYPE_INT };
  expr_to_str1(exprobj->state, &e);
  retval.val.intval = (EXPR_int64) (e.val.strval.endnext - e.val.strval.begin);
  return retval;
}
