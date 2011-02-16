#include "gcirc.h"
using namespace std;

// Return the great circle distance between to ra/dec pairs. 
double gcirc(double ra1, double dec1, double ra2, double dec2)
{

    double sindec1, cosdec1, sindec2, cosdec2, radiff, cosradiff, cosdis, dis;

    sindec1 = sin(dec1*D2R);
    cosdec1 = cos(dec1*D2R);

    sindec2 = sin(dec2*D2R);
    cosdec2 = cos(dec2*D2R);

    radiff = (ra1-ra2)*D2R;
    cosradiff = cos(radiff);

    cosdis = sindec1*sindec2 + cosdec1*cosdec2*cosradiff;

    if (cosdis < -1.0) 
        cosdis = -1.0;
    else if (cosdis >  1.0) 
        cosdis =  1.0;

    dis = acos(cosdis);

    return(dis);

}
