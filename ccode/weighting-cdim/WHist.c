#include <stdlib.h>
#include <assert.h>
#include "WHist.h"


struct WHist* WHistAlloc(int nbin, double minval, double maxval) {
    struct WHist* wh = malloc(sizeof(struct WHist));

    assert(wh != NULL);

    wh->nbin = nbin;
    wh->min = minval;
    wh->max = maxval;
    wh->binsize = (maxval-minval)/nbin;

    wh->binmin = malloc(nbin*sizeof(double));
    assert(wh->binmin != NULL);

    wh->binmax = malloc(nbin*sizeof(double));
    assert(wh->binmax != NULL);

    wh->whist  = malloc(nbin*sizeof(double));
    assert(wh->whist != NULL);


    for (int i = 0; i < nbin; ++i) {
        wh->binmin[i] = minval + i*wh->binsize;
        wh->binmax[i] = wh->binmin[i] + wh->binsize;
        wh->whist[i] = 0.0;
    }


    return wh;
}

void WHistClear(struct WHist* wh) {
    for (int i=0; i<wh->nbin; i++) {
        wh->whist[i] = 0.0;
    }
}


void WHistCalc(double* xvals, double* weights, int npts, struct WHist* wh) {

    WHistClear(wh);
    double wsum=0.;

    double x,w;
    for (int i=0; i<npts; i++) {
        x = xvals[i];
        for (int j = 0; j < wh->nbin; ++j) {
            if ( (x >= wh->binmin[j]) && (x < wh->binmax[j]) ) {
                w = weights[i];
                wsum += w;
                wh->whist[j] += w;
                break;
            }
        }
    }

    /*
    for (int j = 0; j < wh->nbin; ++j) {

        for (int i = 0; i < npts; ++i) {                               
            if ((xvals[i] >= wh->binmin[j]) && (xvals[i] < wh->binmax[j])){
                wsum    += weights[i];
                wh->whist[j] += weights[i];
            }
        }
    }
    */

    // normalize
    if (wsum > 0) {
        for (int i=0; i<wh->nbin; i++) {
            wh->whist[i] /= wsum;
        }
    }

}
