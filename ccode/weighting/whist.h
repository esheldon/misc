#ifndef _WHIST_H
#define _WHIST_H

#include <vector>
using namespace std;

void whist(
        double rho_min,        // min x value
        double rho_max,        // max x value
        vector<double>& xvals, // x values from training set neighbors
        vector<double>& wei,   // weights from training set neighbors
        vector<double>& xmin,  // x values of histogram, left side of bin
        vector<double>& xmax,  // x values of histogram, right side of bin
        vector<double>& hy);   // y values of histogram to fill in

#endif
