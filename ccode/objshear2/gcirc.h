#ifndef _GCIRC_H
#define _GCIRC_H
void gcirc(double lon1, double lat1, 
           double lon2, double lat2, 
           double& dis, double& theta);

void gcirc_eq(double ra1, double dec1, 
           double ra2, double dec2, 
           double& dis, double& theta);

void gcirc_survey(
        double lam1, double eta1, 
        double lam2, double eta2,
        double& dis,  double& theta);
#endif
