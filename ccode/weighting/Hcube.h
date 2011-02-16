#ifndef _HCUBE_H
#define _HCUBE_H

#include "Point.h"
#include <stdio.h>


//A k-dim hypercube 
template<int dim> struct Hcube
{
	
	Point<dim> low,  high;   //The two corners with lowest and highest values
	Hcube(){}
	Hcube(Point<dim> &ilo, Point<dim> &ihi): low(ilo), high(ihi){}

    double dist(const Point<dim> &b) const {
        double sum = 0;

        for(int i=0; i < dim; i++) {
            double hlow = low.x[i];
            double hhigh = high.x[i];
            double pdata = b.x[i];

            if(b.x[i] < low.x[i])
                sum += (low.x[i] - b.x[i])*(low.x[i] - b.x[i]);
            if(b.x[i] > high.x[i])
                sum += (b.x[i] - high.x[i])*(b.x[i] - high.x[i]);
        }
        return sqrt(sum);
    }

    bool contains(const Point<dim>& pt) const {
        if (dist(pt) == 0) {
            return true;
        } else {
            return false;
        }
    }

};

#endif
