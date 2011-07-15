#ifndef _SDSS_SURVEY_H
#define _SDSS_SURVEY_H

double lon_bound(double lon, double minval, double maxval);
void eq2sdss(double ra, double dec, double* lam, double* eta);
void eq2sdss_sincos(double ra, double dec, 
                    double* sinlam, double* coslam, 
                    double* sineta, double* coseta);

              
void gcirc_survey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double* dis,  double* theta);

int gcirc_theta_quad(double theta);

#endif
