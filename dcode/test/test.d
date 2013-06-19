import std.stdio;
import std.math;
import point;
import hpoint;

class CatPoint : Point {
    // we inherit x,y,z
    long index;
    double cos_radius;

    this(double ra, double dec, long index, double cos_radius) {
        this.index=index;
        this.cos_radius = cos_radius;
        super(ra,dec);
    }
}

int main() {

    auto hp = new HPoint(210.0, 35.0);
    auto cp = new CatPoint(57.3, -8.5,35,0.2);

    writeln("hp.phi (member): ",hp.phi);
    writeln("cp.phi (property): ",cp.phi);
    writefln("hp: (%s,%s,%s)",hp.x,hp.y,hp.z);
    writefln("cp: (%s,%s,%s)",cp.x,cp.y,cp.z);
    auto dotp = hp.dot(cp);
    
    writeln("dotp: ",dotp);
    return 0;
}
