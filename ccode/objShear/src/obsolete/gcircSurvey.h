#if !defined (_gcircSurvey_h)
#define _gcircSurvey_h

#include <stdlib.h>
#include <math.h>

int
gcircSurvey(double lam1, double eta1, 
	    double lam2, double eta2,
	    double& dis,  double& theta);
int
gcircSurvey2(double lam1, double eta1, 
	     double lam2, double eta2,
	     double& dis,  double& theta);

#endif /* _gcircSurvey_h */
