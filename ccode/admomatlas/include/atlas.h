#include "dervish.h"

REGION * atls(char *infile, int color, int row);

static void set_background(REGION *reg, int bkgd);


int
calcmom(float xcen, float ycen,		/* centre of object */
	int *ix1, int *ix2, int *iy1, int *iy2, /* bounding box to consider */
	float bkgd,			/* data's background level */
	int interpflag,			/* interpolate within pixels? */
	float w11, float w22, float w12,	/* weights */
	float detw,
	float *w1,float *w2, float *ww12,
	double *sumx, double *sumy,	/* desired */
	double *sumxx, double *sumyy,	/* desired */
	double *sumxy, double *sum,	/*       sums */
	double *sum1, double *sum2,
	REGION *data);		/* the data */

void
calcerr(float xcen, float ycen,		/* centre of object */
	int ix1, int ix2, int iy1, int iy2, /* bounding box to consider */
	float bkgd,			/* data's background level */
	int interpflag,			/* interpolate within pixels? */
	float w1, float w2, float ww12,	/* weights */
	float sumxx, float sumyy, float sumxy, /* quadratic sums */
	double sum1, double sum2,
	float *errxx, float *erryy, float *errxy, /* errors in sums */
	double *sums4,			/* ?? */
	double *s1, double *s2,
	REGION *data);		/* the data */


void
calc_adaptive_moments(REGION *data, float bkgd, float sigsky, 
		      float petroRad, float xcenin, float ycenin, 
		      float *Mcc, float *Mrr, float *Mcr, 
		      float *M1err, float *rho4, int *whyflag);

int
calcmom_float(float xcen, float ycen,		/* centre of object */
	      int *ix1, int *ix2, int *iy1, int *iy2, /* bounding box to consider */
	      float bkgd,			/* data's background level */
	      int interpflag,			/* interpolate within pixels? */
	      float w11, float w22, float w12,	/* weights */
	      float detw,
	      float *w1,float *w2, float *ww12,
	      double *sumx, double *sumy,	/* desired */
	      double *sumxx, double *sumyy,	/* desired */
	      double *sumxy, double *sum,	/*       sums */
	      double *sum1, double *sum2,
	      float *data, int ncol, int nrow);		/* the data */

void
calcerr_float(float xcen, float ycen,		/* centre of object */
	      int ix1, int ix2, int iy1, int iy2, /* bounding box to consider */
	      float bkgd,			/* data's background level */
	      int interpflag,			/* interpolate within pixels? */
	      float w1, float w2, float ww12,	/* weights */
	      float sumxx, float sumyy, float sumxy, /* quadratic sums */
	      double sum1, double sum2,
	      float *errxx, float *erryy, float *errxy, /* errors in sums */
	      double *sums4,			/* ?? */
	      double *s1, double *s2,
	      float *data, int ncol, int nrow);		/* the data */


void
calc_adaptive_moments_float(float *data, int ncol, int nrow, 
			    float bkgd, float sigsky, 
			    float petroRad, float wguess, 
			    float xcenin, float ycenin, 
			    float *Mcc, float *Mrr, float *Mcr, 
			    float *M1err, float *rho4, int *whyflag);
