/********************************************************************************

 NAME:
    admom_float.c

 PURPOSE:
    Take in an image and run calc_adaptive_moments_float on it. This is 
    designed to be a .so, linked from IDL, so argv[0] holds a variable

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
admom_float(int argc, void *argv[]) {

  int nrow, ncol, *pnrow, *pncol;
  float row, col, *prow, *pcol;
  float *psky, *pskysig;
  float *pwguess;

  int ic, ir, *pnobj, nobj;
  int *whyflag,twhyflag=0,dummy;
  float *image, *ixx, *iyy, *ixy, *momerr, *rho4;
  float *ixxerr, *iyyerr, *ixyerr, *icov;
  float Mcc=0., Mrr=0., Mcr=0.,  M1err=0.,  Mrho4=0., val;
  float sky, skysig, petrorad;
  float wguess;

  /* copy pointers into local variables */
  /* image stats */
  image = (float *) argv[0];
  pncol = (int *) argv[1];      /* image size */
  pnrow = (int *) argv[2];
  pcol = (float *) argv[3];       /* positions of objects */
  prow = (float *) argv[4];
  pnobj = (int *) argv[5];
  psky = (float *) argv[6];
  pskysig = (float *) argv[7];
  pwguess = (float *) argv[8];
  
  /* returned variables */
  ixx = (float *) argv[9];
  iyy = (float *) argv[10];
  ixy = (float *) argv[11];
  momerr = (float *) argv[12];
  rho4 = (float *) argv[13];
  whyflag = (int *) argv[14];
  
  /* convert to local variables for convenience */
  nobj = *(&pnobj[0]);
  nrow = *(&pnrow[0]);
  ncol = *(&pncol[0]);
    
  /* default values for size*/
  petrorad = 2.25;

  /* calculate moments for each object in image*/
  for (ic=0;ic<nobj;++ic) {
    row = prow[ic];
    col = pcol[ic];

    sky = psky[ic];
    skysig = pskysig[ic];

    wguess = pwguess[ic];

    setzero(&Mcc,&Mrr,&Mcr, 
	    &M1err,
	    &Mrho4,&twhyflag);
    calc_adaptive_moments_float(image, ncol, nrow, 
				sky, skysig, petrorad, wguess, 
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
  /* shRegDel(reg); */

  return 0;

}
