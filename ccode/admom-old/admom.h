#define AM_MAXIT 100
#define AM_XINTERP 0.0
#define AM_XINTERP2 0.0
#define AM_TOL1 0.001
#define AM_TOL2 0.01
#define AM_DETTOL 1.e-6

/* Flags: these correspond to what used to be in PHOTO */
#define AMFLAG_FAINT 0x200000
#define AMFLAG_SHIFT 0x400000
#define AMFLAG_MAXIT 0x800000


/*
 * This runs adaptive moments on a list of centers 
 */
void
admom(float *data, 
	  int nx, 
	  int ny, 
	  int nobj,
	  float *bkgd, 
	  float *sigsky, 
	  float *shiftmax, 
	  float *wguess, 
	  float *xcen, 
	  float *ycen, 
	  float *Ixx, 
	  float *Iyy, 
	  float *Ixy, 
	  float *momerr, 
	  float *rho4, 
	  int *whyflag);



/* run adaptive moments on a single center in the input image */
void
admom1(float *data, 
		int nx, 
		int ny, 
		float bkgd, 
		float sigsky, 
		float petroRad, 
		float wguess, 
		float xcenin, 
		float ycenin, 
		float *Ixx, 
		float *Iyy, 
		float *Ixy, 
		float *momerr, 
		float *rho4, 
		int *whyflag);

/* perform a single iteration for adaptive moments */
int
calcmom(float xcen, 
		float ycen,		/* centre of object */
		int *ix1, 
		int *ix2, 
		int *iy1, 
		int *iy2, /* bounding box to consider */
		float bkgd,			/* data's background level */
		int interpflag,			/* interpolate within pixels? */
		float w11, 
		float w22, 
		float w12,	/* weights */
		float detw,
		float *w1,
		float *w2, 
		float *ww12,
		double *sumx, 
		double *sumy,	/* desired */
		double *sumxx, 
		double *sumyy,	/* desired */
		double *sumxy, 
		double *sum,	/*       sums */
		double *sum1, 
		double *sum2,
		float *data, 
		int ncol, 
		int nrow);		/* the data */

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
		float *data, int ncol, int nrow);		/* the data */

