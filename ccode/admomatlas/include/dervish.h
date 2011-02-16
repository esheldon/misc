#if !defined(PHDERVISH_H)		/* not DERVISH_H -- this is a fake */
#define PHDERVISH_H

const char *phPhotoVersion(void);
/*
 * Try to include the real dervish.h; if we succeed it'll define DERVISH_H
 * and we'll know not to provide our own fake version.
 */
#include <dervish.h>

#if !defined(DERVISH_H)			/* we didn't find the real one */
/*
 * functions usually provided by dervish
 */
typedef int RET_CODE;
typedef int TYPE;
#define UNKNOWN 1

TYPE shTypeGetFromName(const char *type);
/*
 * error reporting
 */
void shError(char *fmt, ...);
void shErrStackPush(char *fmt, ...);
void shFatal(char *fmt, ...);
/*
 * memory
 */
void *shMalloc(size_t n);
void *shRealloc(void *ptr, size_t n);
void shFree(void *ptr);
int p_shMemRefCntrGet(void *ptr);
void p_shMemRefCntrDecr(void *ptr);
/*
 * assertions
 */
#define NDEBUG
#include <assert.h>
#define shAssert assert
/*
 * REGIONs
 */
#define TYPE_U16 8			/* must agree with dervish's region.h*/
typedef unsigned short U16;

typedef struct {
   int nrow, ncol;			/* size of mask */
   unsigned char **rows;		/* data in mask */
   int row0, col0;			/* origin of mask in larger mask */
} MASK;

typedef struct {			/* must agree with dervish up to col0*/
   char *name;				/* a unique identifier */
   int nrow;				/* number of rows in image */
   int ncol;				/* number of columns in image */
   int type;				/* pixel data type */
   U16 **rows;				/* pointer to pointers to rows */
   void **dummy[7];
   MASK *mask;				/* associated bad pixel mask */
   int row0,col0;			/* location of LLH corner of child */
} REGION;

REGION *shRegNew(const char *, int nrow, int ncol, int type);
void shRegDel(REGION *reg);

MASK *shMaskNew(const char *name, int nrow, int ncol);
void shMaskDel(MASK *mask);
void shMaskClear(MASK *mask);

/*
 * Chains
 */
#define AFTER 0
#define BEFORE 1
#define TAIL 1

typedef struct chain_elem {
   struct chain_elem *pNext;
   void *pElement;
} CHAIN_ELEM;

typedef struct chain {
   int nElements;
   CHAIN_ELEM *pFirst;
   TYPE type;
} CHAIN;

CHAIN *shChainNew(char *type);
void shChainDel(CHAIN *ch);
void shChainElementAddByPos(CHAIN *ch, void *el, char *type, int w, int);
void *shChainElementGetByPos(const CHAIN *ch, int el);
void *shChainElementRemByPos(const CHAIN *ch, int el);

#endif
#endif
