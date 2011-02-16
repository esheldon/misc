/* 
   FUNCTION PROTOTYPES
*/


/* 
   creates random samples for bootstrapping 
*/
int bootstrap_resample(float *data, /* data to be resamples [Nmeas,Nvar]*/
		       int Nmeas,   
		       int Nvar, 
		       int statistic, /* which statistic to measure? */ 
		       int Nresamp,   /* Number of samples */
		       float *sampval); /* the values for samples */

/* finds the mean */
float bmean(float *data, int jvar, int Nresamp);

/* finds the covariance for input variables */
float bcov(float *data, 
	   int jvar, float jmean, 
	   int ivar, float imean, 
	   int Nresamp);


int bootstrap(float *data,     /* the data to bootstrap [Nmeas, Nvar] */
	      int   Nmeas,     /* Number of measurements of the variables */
	      int   Nvar,      /* Number of variables */
	      int   statistic, /* What kind of statistic? */
	      float *sampval,  /* an array to hold the resampled values 
				  [Nresamp, Nvar] */
	      int   Nresamp,   /* Number of random samples to create */
	      float *datamean, /* the mean values */
	      float *covariance);
