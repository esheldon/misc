#include "binner.h"

binner::binner() {} // default constructor does nothing
binner::binner(par_struct &par) // constructor
{

    nlum    = par.nlum;
    loglmin = par.loglmin;
    loglmax = par.loglmax;
    lmin    = par.lmin;
    lmax    = par.lmax;

    loglstep = (loglmax - loglmin)/nlum;

    nkgmr    = par.nkgmr;
    kgmrmin  = par.kgmrmin;
    kgmrmax  = par.kgmrmax;
    kgmrstep = (kgmrmax - kgmrmin)/nkgmr;

}


int binner::kgmr_bin(float kgmr)
{
    // Need the >= so the index will be in bounds
    if ( kgmr < kgmrmin || kgmr >= kgmrmax )
        return(-1);

    int bin = (int) ( (kgmr - kgmrmin)/kgmrstep );

    if (bin >= nkgmr) 
    {
        //cout << "Out of bounds kgmr bin: " << bin;
        return(-1);
    }
    return(bin);
}

// Convert lum (10^10 solar) to the loglum bin
int binner::logl_bin(float lumsolar)
{
    // Need the >= so the index will be in bounds
    if (lumsolar < lmin || lumsolar >= lmax)
        return(-1);

    float loglum = 10.0 + log10(lumsolar);

    int bin = (int) (  (loglum - loglmin)/loglstep );

    if (bin >= nlum) 
    {
        //cout << "Out of bounds lum bin: " << bin;
        return(-1);
    }
    return(bin);

}
