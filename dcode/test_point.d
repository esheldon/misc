import std.stdio;
import std.math;
import Point;

int main (char[][] args) 
{
    Point p1 = new Point(210., -45.);
    Point p2 = new Point(75., 37.2);

    real ra1 = 180./PI*atan2(p1.y, p1.x);
    real dec1 = 180./PI*asin(p1.z);
    real ra2 = 180./PI*atan2(p2.y, p2.x);
    real dec2 = 180./PI*asin(p2.z);
    while (ra1 < 0.) {
        ra1 += 360.;
    }
    while (ra1 > 360.) {
        ra1 -= 360.;
    }
    while (ra2 < 0.) {
        ra2 += 360.;
    }
    while (ra2 > 360.) {
        ra2 -= 360.;
    }


    writeln("p1 dot p2: ",p1.dot(p2));
    writefln("ra1      : %.16g dec1      : %.16g",p1.ra,p1.dec);
    writefln("ra1 infer: %.16g dec1 infer: %.16g",ra1,dec1);
    writefln("ra2      : %.16g dec2      : %.16g",p2.ra,p2.dec);
    writefln("ra2 infer: %.16g dec2 infer: %.16g",ra2,dec2);
 
    return 0;
}
