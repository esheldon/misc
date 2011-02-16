#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "dervish.h"

/*****************************************************************************/
/*
 * A routine to return this version of photo's name
 */
const char *
phPhotoVersion(void)
{
   static const char version[] = "$Name:  $";

   if(strlen(version) <= 9) {
      return("NOCVS");
   } else {
      return(version);
   }
}

/*****************************************************************************/
/*
 * utilities that dervish usually provides
 */
#if !defined(DERVISH_H)			/* we haven't got the _real_ dervish */

TYPE
shTypeGetFromName(const char *name)	/* NOTUSED */
{
   return(UNKNOWN);
}

/*****************************************************************************/

void
shError(char *fmt, ...)
{
   va_list args;
   char buff[1000];
   int len;

   va_start(args,fmt);
   vsprintf(buff,fmt,args);
   va_end(args);

   if(buff[len = strlen(buff)] == '\n') {
      buff[len] = '\0';
   }
   fprintf(stderr,"Error: %s\n",buff);
   fflush(stderr);
}

/*****************************************************************************/
/*
 * This is the same as shError for current purposes
 */
void
shErrStackPush(char *fmt, ...)
{
   va_list args;
   char buff[1000];
   int len;

   va_start(args,fmt);
   vsprintf(buff,fmt,args);
   va_end(args);

   if(buff[len = strlen(buff)] == '\n') {
      buff[len] = '\0';
   }
   shErrStackPush("%s",buff);
   fprintf(stderr,"Error: %s\n",buff);
   fflush(stderr);
}

/*
 * and here's the fatal handler
 */
void
shFatal(char *fmt, ...)
{
   va_list args;

   va_start(args,fmt);
   fprintf(stderr,"Fatal error: ");
   vfprintf(stderr,fmt,args);
   fprintf(stderr,"\n");
   fflush(stderr);
   va_end(args);
   abort();
}

/*****************************************************************************/
/*
 * memory management
 */
void *
shMalloc(size_t n)
{
   void *ptr = malloc(n);

   if(ptr == NULL) {
      shFatal("failed to allocate %ld bytes", (long)n);
   }

   return(ptr);
}

void *
shRealloc(void *ptr, size_t n)
{
   ptr = realloc(ptr, n);

   if(ptr == NULL) {
      shFatal("failed to reallocate %ld bytes", (long)n);
   }

   return(ptr);
}

void
shFree(void *ptr)
{
   free(ptr);
}

int
p_shMemRefCntrGet(void *ptr)		/* NOTUSED */
{
   return(0);
}

void
p_shMemRefCntrDecr(void *ptr)		/* NOTUSED */
{
   ;
}

/*****************************************************************************/
/*
 * regions
 */
REGION *
shRegNew(const char *name,		/* NOTUSED */
	 int nrow,
	 int ncol,
	 int type)
{
   int i;
   REGION *reg = shMalloc(sizeof(REGION));
   
   shAssert(type == TYPE_U16);

   reg->type = type;
   reg->rows = shMalloc(nrow*sizeof(U16 *));
   reg->rows[0] = shMalloc(nrow*ncol*sizeof(U16));
   reg->nrow = nrow; reg->ncol = ncol;

   for(i = 1; i < nrow; i++) {
      reg->rows[i] = reg->rows[i - 1] + ncol;
   }

   return(reg);
}

void
shRegDel(REGION *reg)
{
   if(reg != NULL) {
      if(reg->rows != NULL) {
	 shFree(reg->rows[0]);
	 shFree(reg->rows);
      }
      shFree(reg);
   }
}

/*****************************************************************************/
/*
 * masks
 */
MASK *
shMaskNew(const char *name,		/* NOTUSED */
	 int nrow,
	 int ncol)
{
   int i;
   MASK *mask = shMalloc(sizeof(MASK));
   
   mask->rows = shMalloc(nrow*sizeof(unsigned char *));
   mask->rows[0] = shMalloc(nrow*ncol);
   mask->nrow = nrow; mask->ncol = ncol;
   mask->row0 = mask->col0 = 0;

   for(i = 1; i < nrow; i++) {
      mask->rows[i] = mask->rows[i - 1] + ncol;
   }

   return(mask);
}

void
shMaskDel(MASK *mask)
{
   if(mask != NULL) {
      if(mask->rows != NULL) {
	 shFree(mask->rows[0]);
	 shFree(mask->rows);
      }
      shFree(mask);
   }
}

void
shMaskClear(MASK *mask)
{
   int i;
   
   shAssert(mask != NULL && mask->rows != NULL && mask->rows[0] != NULL);
   shAssert(mask->nrow >= 1);

   for(i = 0; i < mask->ncol; i++) {
      mask->rows[0][i] = '\0';
   }
   for(i = 1; i < mask->nrow; i++) {
      memcpy(mask->rows[i], mask->rows[0], mask->ncol);
   }
}

/*****************************************************************************/
/*
 * CHAINs
 */
CHAIN *
shChainNew(char *type)			/* NOTUSED */
{
   return(NULL);
}

void
shChainDel(CHAIN *ch)			/* NOTUSED */
{
   ;
}

void
shChainElementAddByPos(CHAIN *ch,	/* NOTUSED */
		       void *el,	/* NOTUSED */
		       char *type,	/* NOTUSED */
		       int w,		/* NOTUSED */
		       int how)		/* NOTUSED */
{
   ;
}

void *
shChainElementGetByPos(const CHAIN *ch,	/* NOTUSED */
		       int el)		/* NOTUSED */
{
   return(NULL);
}

void *
shChainElementRemByPos(const CHAIN *ch,	/* NOTUSED */
		       int el)		/* NOTUSED */
{
   return(NULL);
}

#endif
