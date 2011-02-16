/*
 * Read an atlas image table produced by photo
 * Modified for linking, returns reg rather than writing out 17-DEC-2000 E.S.S.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dervish.h"
#include "phFits.h"
#include "phConsts.h"
#include "export.h"
#include "atlas.h"
/*
 * some symbols to prevent dervish.o from being loaded from libatlas.a
 * if we are also linking against real dervish
 */

int verbose = 0;

REGION * atls(char *infile, int color, int row)
{
   ATLAS_IMAGE *ai;			/* the desired atlas image */
   int bkgd = SOFT_BIAS;		/* desired background level */
   FITS *fits;				/* the table in question */
   /*U16 row0,col0;*/
   int row0, col0;			/* origin of image in region */
   REGION *reg;				/* region to write */

/*
 * dummy calls to pull .o files out of the real libdervish.a if we are
 * linking against it
 */
   (void)shTypeGetFromName("RHL");
/*
 * open file
 */

   if((fits = open_fits_table(infile, 1)) == NULL) {
      return(NULL);
   }

/*
 * read atlas image
 */
   if((ai = read_atlas_image(fits,row)) == NULL) {
      return(NULL);
   }

   if(ai->id < 0) {			/* no atlas image for this object */
      shError("Object %d has no atlas image", row);
      return(NULL);
   }
   shAssert(ai->master_mask != NULL);

   if(color < 0 || color >= ai->ncolor) {
      shError("Invalid color; please choose a number in 0..%d", ai->ncolor-1);
      return(NULL);
   }
/*
 * convert it to a region
 */
   reg = shRegNew("atlas image",
		  ai->master_mask->rmax - ai->master_mask->rmin + 1,
		  ai->master_mask->cmax - ai->master_mask->cmin + 1, TYPE_U16);

   set_background(reg, bkgd);
   row0 = ai->master_mask->rmin + ai->drow[color];
   col0 = ai->master_mask->cmin + ai->dcol[color];
   phRegionSetFromAtlasImage(ai, color, reg, row0, col0);

   reg->row0 = row0;
   reg->col0 = col0;

/*
 * and write it to a file
 */
   /*   write_fits_file(outfile, reg, 16);*/

   /*   shRegDel(reg);*/
   phFitsDel(fits);
   phAtlasImageDel(ai,1);

   return(reg);
}

static void
set_background(REGION *reg,
	       int bkgd)
{
   U16 *row0;
   int i;

   row0 = reg->rows[0];
   for(i = 0; i < reg->ncol; i++) {
      row0[i] = bkgd;
   }
   for(i = 1; i < reg->nrow; i++) {
      memcpy(reg->rows[i], row0, reg->ncol*sizeof(U16));
   }
}
