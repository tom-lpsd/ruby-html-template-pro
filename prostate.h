#ifndef _PROSTATE_H
#define _PROSTATE_H	1

#include "pbuffer.h"

struct tagstack {
  struct tagstack_entry* entry;
  int pos;
  int depth;
};

struct tmplpro_param;

typedef int boolval;

struct tmplpro_state {
  boolval  is_visible;
  char* top;
  char* next_to_end;
  char* last_processed_pos;
  char* cur_pos;
  struct tmplpro_param* param;
  /* current tag */
  int   tag;
  boolval  is_tag_closed;
  boolval  is_tag_commented;
  boolval  is_expr;
  char* tag_start; 

/* internal buffers */
  /* main string buffer */
  pbuffer str_buffer;
  /* tag stack */
  struct tagstack tag_stack;

  /* expr string buffers; used to unescape pstring args and for num -> string */
  pbuffer expr_left_pbuffer;
  pbuffer expr_right_pbuffer;
};

extern TMPLPRO_LOCAL void _tmpl_log_state (struct tmplpro_state *state, int level);

#endif /* prostate.h */

/* 
 * Local Variables:
 * mode: c 
 * End: 
 */
