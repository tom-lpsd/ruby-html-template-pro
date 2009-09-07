#include "pbuffer.h"

/* reentrant pbuffer functions */

TMPLPRO_LOCAL
size_t pbuffer_size(pbuffer* pBuffer) {
  return pBuffer->bufsize;
}
TMPLPRO_LOCAL
void pbuffer_preinit(pbuffer* pBuffer) {
  if (pBuffer->bufsize!=0) {
    pBuffer->bufsize=0;
    pBuffer->buffer=NULL;
  }
}
TMPLPRO_LOCAL
char* pbuffer_init(pbuffer* pBuffer) {
  pBuffer->bufsize=256;
  pBuffer->buffer=(char*) malloc (pBuffer->bufsize * sizeof(char));
  return pBuffer->buffer;
}
TMPLPRO_LOCAL
char* pbuffer_init_as(pbuffer* pBuffer,size_t size) {
  pBuffer->bufsize=PBUFFER_MULTIPLICATOR*size;
  pBuffer->buffer=(char*) malloc (pBuffer->bufsize * sizeof(char));
  return pBuffer->buffer;
}
TMPLPRO_LOCAL
char* pbuffer_string(pbuffer* pBuffer) {
  return pBuffer->buffer;
}
TMPLPRO_LOCAL
char* pbuffer_resize(pbuffer* pBuffer, size_t size) {
  if (pBuffer->bufsize==0) {
    pbuffer_init_as(pBuffer, size);
  } else if (pBuffer->bufsize< size) {
    pBuffer->bufsize=PBUFFER_MULTIPLICATOR*size; /* aggresive memory allocation to prevent frequent requests*/
    pBuffer->buffer=(char*) realloc (pBuffer->buffer,pBuffer->bufsize * sizeof(char));
  }
  return pBuffer->buffer;
}

TMPLPRO_LOCAL
void pbuffer_free(pbuffer* pBuffer) {
  if (pBuffer->bufsize!=0) {
    pBuffer->bufsize=0;
    free(pBuffer->buffer);
  }
}
TMPLPRO_LOCAL
void pbuffer_fill_from_pstring(pbuffer* pBuffer, PSTRING pstr) {
  size_t size = pstr.endnext - pstr.begin;
  char * from = pstr.begin;
  char * dest;
  if (pBuffer->bufsize==0) {
    pbuffer_init_as(pBuffer, size+1);
  } else if (pBuffer->bufsize<size) {
    pbuffer_resize(pBuffer, size+1);
  }
  dest = pBuffer->buffer;
  while (from<pstr.endnext) {
    *(dest++)=*(from++);
  }
  *dest='\0';
}

/*
 * Local Variables:
 * mode: c
 * End:
 */
