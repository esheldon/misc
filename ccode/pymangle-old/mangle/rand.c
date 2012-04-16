#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "rand.h"

void
seed_random(void) {
    struct timeval tm;
    gettimeofday(&tm, NULL); 
    srand48((long) (tm.tv_sec * 1000000 + tm.tv_usec));
}

/*
 * constant in cos(theta)
 */
void
genrand_theta_phi_allsky(double* theta, double* phi)
{
    *phi = drand48()*2*M_PI;
    // this is actually cos(theta) for now
    *theta = 2*drand48()-1;
    
    if (*theta > 1) *theta=1;
    if (*theta < -1) *theta=-1;

    *theta = acos(*theta);
}

/*
 * Generate random points in a range.  Inputs are
 * min(cos(theta)), max(cos(theta)), min(phi), max(phi)
 *
 * constant in cos(theta)
 */
void
genrand_theta_phi(double cthmin, double cthmax, double phimin, double phimax,
                  double* theta, double* phi)
{

    // at first, theta is cos(theta)
    *phi = phimin + (phimax - phimin)*drand48();

    // this is actually cos(theta) for now
    *theta = cthmin + (cthmax-cthmin)*drand48();
    
    if (*theta > 1) *theta=1;
    if (*theta < -1) *theta=-1;

    *theta = acos(*theta);
}

