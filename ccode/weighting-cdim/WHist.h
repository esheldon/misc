#ifndef _WHIST_H
#define _WHIST_H

struct WHist {
    int nbin;
    double min;
    double max;
    double binsize;

    double* binmin;
    double* binmax;
    double* whist;
};

struct WHist* WHistAlloc(int nbin, double minval, double maxval);
void WHistCalc(double* xvals, double* weights, int npts, struct WHist* wh);
void WHistClear(struct WHist* wh);

#endif
