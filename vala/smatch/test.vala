/* vim: set ft=vala : */
using Gee;

void main() {

    long nside=4096;
    double ra = 215.2;
    double dec = -13.3;
    long index = 25;
    double cos_radius=0.9;
    var cp = new CatPoint(ra,dec,index,cos_radius);
    stdout.printf("x: %lf y: %lf z: %lf\n", cp.x, cp.y, cp.z);

    var cat = new Cat("/home/esheldon/tmp/rand-radec.dat",nside,2.0);
}


