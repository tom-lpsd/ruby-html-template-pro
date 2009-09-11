#define ERR_PRO_INVALID_ARGUMENT 1
#define ERR_PRO_FILE_NOT_FOUND 2
#define ERR_PRO_CANT_OPEN_FILE 3

#include "pstring.h"
#include "tmpllog.h"
#include "pabstract.h"

struct tmplpro_param;

static int tmplpro_exec_tmpl_filename (struct tmplpro_param* ProParams,const char* filename);
static int tmplpro_exec_tmpl_scalarref (struct tmplpro_param* ProParams, PSTRING memarea);

/* 
 * Local Variables:
 * mode: c 
 * End: 
 */
