/********************************************************************************

 NAME:
    psfadmom.c

 PURPOSE:
    Take in a psf image and run calc_adaptive_moments on it. This is designed
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
void setzero(float *Mcc, float *Mrr, float *Mcr, float *M1err, 
	    float *Mrho4,int *whyflag);

int
psfadmom(int argc, void *argv[]) {

  int nrow, ncol, *prow, *pcol;
  int ic, ir;
  int *whyflag,twhyflag=0,dummy;
  float *psfimage, *ixx, *iyy, *ixy, *momerr, *rho4;
  float Mcc=0., Mrr=0., Mcr=0.,  M1err=0.,  Mrho4=0., val;
  float bkgd, skysig, petrorad, xcen, ycen;
  REGION *reg;

  /* copy into local variables */
  psfimage = (float *) argv[0];
  prow = (int *) argv[1];
  pcol = (int *) argv[2];
  ixx = (float *) argv[3];
  iyy = (float *) argv[4];
  ixy = (float *) argv[5];
  momerr = (float *) argv[6];
  rho4 = (float *) argv[7];
  whyflag = (int *) argv[8];

  nrow = *(&prow[0]);
  ncol = *(&pcol[0]);

  /* create a region */
  reg = shRegNew("psf",
		 nrow, ncol, 
		 TYPE_U16);

  /* copy from image to region */
  for (ir=0; ir<nrow;++ir)
    for (ic=0;ic<ncol;++ic) {
      reg->rows[ir][ic] = (U16) ( *(&psfimage[0] + ir*ncol + ic)  );
    }

  /* default values for psf*/
  bkgd = 0.0;
  skysig = 5.5;
  petrorad = 2.25;
  xcen = ( (float)ncol-1. )/2.;
  ycen = ( (float)nrow-1. )/2.;

  /* calculate moments */
  calc_adaptive_moments(reg, bkgd, skysig, petrorad, xcen, ycen,
			&Mcc, &Mrr, &Mcr, &M1err, &Mrho4, &twhyflag);

  /* free the memory */
  shRegDel(reg);

  if(twhyflag != 0) setzero(&Mcc,&Mrr,&Mcr,&M1err,&Mrho4,&dummy);

  /* set return values */
  *(&ixx[0]) = Mcc;
  *(&iyy[0]) = Mrr;
  *(&ixy[0]) = Mcr;
  *(&momerr[0]) = M1err;
  *(&rho4[0]) = Mrho4;
  *(&whyflag[0]) = twhyflag;
  return 0;

}

/* for setting values to zero */
void setzero(float *Mcc, float *Mrr, float *Mcr, float *M1err, 
	    float *Mrho4,int *whyflag)
{
  *Mcc=0.;
  *Mrr=0.;
  *Mcr=0.;
  *M1err=0.;
  *Mrho4=0.;
  *whyflag=0;
}
