import std.stdio;
import std.math;

import point;

/*
void print_point(Point p, string name) {
    writefln("%s.ndim = %d", name, p.ndim());
    writefln("%s.x.length = %d", name, p.x.length);
    writefln("%s.z = %f", name, p.z);
    for (int i=0; i<p.ndim(); i++) {
        writefln("  %s.x[%d] = %f", name, i, p.x[i]);
    }
}
*/
/*
void print_spoint(sPoint p, string name) {
    writefln("%s.ndim = %d", name, p.ndim);
    writefln("%s.x.length = %d", name, p.x.length);
    writefln("%s.z = %f", name, p.z);
    for (int i=0; i<p.ndim; i++) {
        writefln("  %s.x[%d] = %f", name, i, p.x[i]);
    }
}
*/



int main (char[][] args) 
{
    const int ndim=2;


    HCube[] hvec;
    hvec.length = 3;

    double[] low;
    double[] high;

    low.length = ndim;
    high.length = ndim;
    low[] = [0.0,0.0];
    high[] = [1.0,1.0];

    hvec[1] = new HCube(low,high);

    double[] p_inside = [0.5,0.5];
    double[] p_outside = [0.5,1.5];

    if (hvec[1].contains(p_inside)) {
        writefln("inside point correctly identified");
    } else {
        writefln("inside point not correctly identified");
    }
    if (hvec[1].contains(p_outside)) {
        writefln("outside point not correctly identified");
    } else {
        writefln("outside point correctly identified");
    }

    return 0;
}
