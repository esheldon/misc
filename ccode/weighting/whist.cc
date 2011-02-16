#include "whist.h"
void whist(
        double overall_xmin,      // min x value
        double overall_xmax,      // max x value
        vector<double>& xvals,    // x values from training set neighbors
        vector<double>& weights,  // weights from training set neighbors
        vector<double>& xmin,     // x values of histogram, left side of bin
        vector<double>& xmax,     // x values of histogram, right side of bin
        vector<double>& hist)     // y values of histogram to fill in
{
    int npoints = xvals.size();
    int ndiv=hist.size();

    xmin.resize(ndiv);
    xmax.resize(ndiv);
    hist.resize(ndiv);

    double dx = (overall_xmax - overall_xmin)/ndiv;

    for (int i = 0; i < ndiv; ++i) {
        xmin[i] = overall_xmin + i*dx;
        xmax[i] = xmin[i] + dx;
        hist[i] = 0.0;
    }

    double wsum=0.;
    for (int j = 0; j < ndiv; ++j) {

        for (int i = 0; i < npoints; ++i) {                               
            if ((xvals[i] >= xmin[j]) && (xvals[i] < xmax[j])){
                wsum    += weights[i];
                hist[j] += weights[i];
            }
        }

    }

    // normalize
    if (wsum > 0) {
        for (int i=0; i<ndiv; i++) {
            hist[i] /= wsum;
        }
    }
}


