#ifndef _POINT_H
#define _POINT_H

#include <cmath>

using namespace std;

//A point in magnitude space
template<int dim> struct Point
{
	
	double x[dim];
	double z;
	Point operator = (Point y)
	{
		for(int q = 0; q < dim; q++)
				x[q] = y.x[q];
			z = y.z;
			return *this;
	}
	Point(double y[dim])
	{
		for(int i = 0; i < dim; i++)
			x[i] = y[i];
		z=0;
	}
	Point()
	{
		z=0;
		for(int i = 0; i < dim; i++)
			x[i] = 0;
	}
	Point(double y[],double zin)
	{
		z=zin;
		for(int i=0;i < dim;i++)
			x[i] = y[i];
	}
	bool operator == (Point y) const
	{
		
		for(int i = 0; i < dim; i++)
			if(x[i] != y.x[i])
				return false;
		if(z != y.z)
			return false;
		return true;
	}

    double dist(const Point<dim>& pt) const {
        double sum = 0;
        for(int i = 0; i < dim; i++)
            sum+= (x[i]-pt.x[i])*(x[i]-pt.x[i]);
        return sqrt(sum);

    }
    int ndim() {
        return dim;
    }
};



#endif
