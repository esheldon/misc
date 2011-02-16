
/* function prototypes */

float jtotal(float *data, int ivar, int Nmeas, int power);

float jtotalcov(float *sampval, float *jval, int ivar, int jvar, int Nsamp);

int jackknife(float *data,     /* the data to jackknife [Nmeas, Nvar] */
	      int   Nmeas,     /* Number of measurements of the variables */
	      int   Nvar,      /* Number of variables */
	      int   statistic, /* What kind of statistic are we jackknifing?*/
	      float *datamean, /* the mean values */
	      float *covariance);  /* covariance [Nvar, Nvar] */
