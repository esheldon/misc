#if !defined (_read_cat_h)
#define _read_cat_h

#include "LensSource.h"
#include "sigmaCritInv.h"

//#define SOURCE_HEADER_LINES 20
//#define LENS_HEADER_LINES 16

lens_struct *read_lcat(char *file, int &nlens);
source_struct *read_scat(char *file, int &nsource);
source_pixel_struct *read_scat_pixel(char *file, int32 &npix);
PAR_STRUCT read_par(char *file);
int32 *read_rev(char *file, int32 &nrev, int32 &minid, int32 &maxid);

SCINV_STRUCT2D* read_scinv2d(char* file);

int nrows(FILE *fptr);

#endif /* _read_cat_h */
