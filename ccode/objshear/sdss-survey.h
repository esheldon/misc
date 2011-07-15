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

double posangle_survey(double sinlam1, double coslam1,
                       double sineta1, double coseta1,
                       double sinlam2, double coslam2,
                       double sineta2, double coseta2);

int survey_quad(double theta);

#endif
