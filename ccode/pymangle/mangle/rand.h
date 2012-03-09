#ifndef _MANGLE_RAND_H
#define _MANGLE_RAND_H

void seed_random(void);

void genrand_theta_phi_allsky(double* theta, double* phi);

void genrand_theta_phi(double cthmin, double cthmax, 
                       double phimin, double phimax,
                       double* theta, double* phi);
#endif
