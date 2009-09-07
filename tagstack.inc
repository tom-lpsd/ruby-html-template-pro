#include <stdlib.h>
#include <assert.h>

#define INIT_ENV_DEPTH 256

static 
void tagstack_init(struct tagstack* tagstack) {
  tagstack->depth=INIT_ENV_DEPTH;
  tagstack->pos=-1;
  tagstack->entry=(struct tagstack_entry*) malloc (tagstack->depth * sizeof(struct tagstack_entry));
}

static 
void tagstack_free(struct tagstack* tagstack) {
  tagstack->depth=-1;
  tagstack->pos=-1;
  free(tagstack->entry);
}

static 
int tagstack_notempty(struct tagstack* tagstack) {
  return tagstack->pos>=0;
}

/*
static 
void tagstack_entry_debug(struct tagstack_entry item) {
 tmpl_log(NULL,TMPL_LOG_DEBUG,"vcontext = %d value=%d\n",item.vcontext,item.value);
}
*/

static 
struct tagstack_entry* tagstack_top(struct tagstack* tagstack) {
  return tagstack->entry+tagstack->pos;
}

static 
struct tagstack_entry tagstack_pop(struct tagstack* tagstack) {
  if (tagstack->pos<0) {
    tmpl_log(NULL,TMPL_LOG_ERROR,"stack underflow:tags stack is empty\n");
    tagstack->pos=0;
    if (tagstack->depth<0) {
      tmpl_log(NULL,TMPL_LOG_ERROR,"FATAL:stack error:tags stack is uninitialized\n");
      tagstack_init(tagstack);
    }
  }
  return *(tagstack->entry+ tagstack->pos--);
}

static 
void tagstack_push(struct tagstack* tagstack, struct tagstack_entry item) {
  /* overflow check */
  if (++(tagstack->pos)>=tagstack->depth) {
    if (tagstack->depth<INIT_ENV_DEPTH) tagstack->depth=INIT_ENV_DEPTH;
    tagstack->depth*=2;
    tagstack->entry=(struct tagstack_entry*) realloc (tagstack->entry, tagstack->depth * sizeof(struct tagstack_entry));
  }
  *(tagstack->entry+tagstack->pos)=item;
}

/* 
 * Local Variables:
 * mode: c 
 * End: 
 */
