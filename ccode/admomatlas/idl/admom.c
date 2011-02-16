/********************************************************************************

 NAME:
    admom.c

 PURPOSE:
    Take in an image and run calc_adaptive_moments on it. This is designed
    to be a .so, linked from IDL, so argv[0] holds a variable

 CALLING SEQUENCE:
    

*********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "dervish.h"
#include "phFits.h"
#include "phConsts.h"
#include "export.h"
#include "atlas.h"

/* Function Prototypes */

int
admom(int argc, void *argv[]) {

  int nrow, ncol, *pnrow, *pncol;
  float row, col, *prow, *pcol;
  int ic, ir, *pnobj, nobj;
  int *whyflag,twhyflag=0,dummy;
  float *image, *ixx, *iyy, *ixy, *momerr, *rho4;
  float *ixxerr, *iyyerr, *ixyerr, *icov;
  float Mcc=0., Mrr=0., Mcr=0.,  M1err=0.,  Mrho4=0., val;
  float *sky, *skysig, petrorad;
  REGION *reg;

  /* copy into local variables */
  /* image stats */
  image = (float *) argv[0];
  pncol = (int *) argv[1];      /* image size */
  pnrow = (int *) argv[2];
  pcol = (float *) argv[3];       /* positions of objects */
  prow = (float *) argv[4];
  pnobj = (int *) argv[5];
  sky = (float *) argv[6];
  skysig = (float *) argv[7];

  
  /* returned variables */
  ixx = (float *) argv[8];
  iyy = (float *) argv[9];
  ixy = (float *) argv[10];
  momerr = (float *) argv[11];
  rho4 = (float *) argv[12];
  whyflag = (int *) argv[13];
  
  /* convert to local variables for convenience */
  nobj = *(&pnobj[0]);
  nrow = *(&pnrow[0]);
  ncol = *(&pncol[0]);

  /* create a region */
  reg = shRegNew("image",
		 nrow, ncol, 
		 TYPE_U16);
  
  /* copy from image to region */
  for (ir=0; ir<nrow;++ir)
    for (ic=0;ic<ncol;++ic) {
      reg->rows[ir][ic] = (U16) ( *(&image[0] + ir*ncol + ic)  );
    }
  
  /* default values for size*/
  petrorad = 2.25;

  /* calculate moments for each object in image*/
  for (ic=0;ic<nobj;++ic) {
    row = *(&prow[ic]);
    col = *(&pcol[ic]);
    
    setzero(&Mcc,&Mrr,&Mcr, 
	    &M1err,
	    &Mrho4,&twhyflag);
    calc_adaptive_moments(reg, *sky, *skysig, petrorad, 
			  col, row,
			  &Mcc, &Mrr, &Mcr, 
			  &M1err, &Mrho4, &twhyflag);

    if(twhyflag != 0) setzero(&Mcc,&Mrr,&Mcr,
			      &M1err,
			      &Mrho4,&dummy);
    
    /* set return values */
    *(&ixx[ic]) = Mcc;
    *(&iyy[ic]) = Mrr;
    *(&ixy[ic]) = Mcr;
    *(&momerr[ic]) = M1err;
    *(&rho4[ic]) = Mrho4;
    *(&whyflag[ic]) = twhyflag;
  }
  
  /* free the memory */
  shRegDel(reg);

  return 0;

}
