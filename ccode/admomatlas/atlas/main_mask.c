/*
 * Read an fpM file produced by photo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dervish.h"
#include "phFits.h"
#include "phConsts.h"
/*
 * some symbols to prevent dervish.o from being loaded from libatlas.a
 * if we are also linking against real dervish
 */

int verbose = 0;

static void usage(void);

int
main(ac,av)
int ac;
char *av[];
{
   FITS *fits;				/* the table in question */
   char *infile, *outfile;		/* input and output filenames */
   int hdu;				/* desired table in file */
   MASK *mask;				/* mask to write */
   OBJMASK *om;				/* objmask read from file */
   int row;				/* desired row */

   while(ac > 1 && (av[1][0] == '-' || av[1][0] == '+')) {
      switch (av[1][1]) {
       case '?':
       case 'h':
	 usage();
	 exit(0);
	 break;
       case 'i':
	 fprintf(stderr,"SDSS read_mask. Id: %s\n", phPhotoVersion());
	 exit(0);
	 break;
       case 'v':
	 verbose++;
	 break;
       default:
	 shError("Unknown option %s\n",av[1]);
	 break;
      }
      ac--;
      av++;
   }
   if(ac <= 3) {
      shError("You must specify an input file, a table, and an output file\n");
      exit(1);
   }
   infile = av[1]; hdu = atoi(av[2]); outfile = av[3];
/*
 * dummy calls to pull .o files out of the real libdervish.a if we are
 * linking against it
 */
   (void)shTypeGetFromName("RHL");
/*
 * open file
 */
   if((fits = open_fits_table(infile, hdu)) == NULL) {
      exit(1);
   }
/*
 * Create MASK
 */
   mask = shMaskNew("from file", fits->maskrows, fits->maskcols);
   shMaskClear(mask);
/*
 * read OBJMASKs and set bits in mask
 */
   if(verbose) {
      printf("reading %4d OBJMASKs from HDU %2d into a %dx%d MASK\n",
	     fits->naxis2, hdu, mask->nrow, mask->ncol);
   }
   for(row = 1; row <= fits->naxis2; row++) {
      if((om = read_objmask(fits, row)) == NULL) {
	 exit(1);
      }

      phMaskSetFromObjmask(om, mask, 1);

      phObjmaskDel(om);
   }
/*
 * and write it to a file
 */
   write_fits_file(outfile, mask, 8);
   shMaskDel(mask);

   phFitsDel(fits);
   
   return(0);
}

/*****************************************************************************/

static void
usage(void)
{
   char **line;

   static char *msg[] = {
      "Usage: read_mask [options] input-file hdu output-file",
      "Your options are:",
      "       -?      This message",
      "       -h      This message",
      "       -i      Print an ID string and exit",
      "       -v      Turn up verbosity (repeat flag for more chatter)",
      NULL,
   };

   for(line = msg;*line != NULL;line++) {
      fprintf(stderr,"%s\n",*line);
   }
}
