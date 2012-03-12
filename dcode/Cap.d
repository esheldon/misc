import std.stdio;
import std.c.stdio;
import std.string;
import Typedef;
import Point;

struct Cap {
    ftype x;
    ftype y;
    ftype z;
    ftype cm;

    this(File* f) {
        this.read(f);
    }
    void read(File* f) {
        // So much faster to use fscanf!  Factor of ~4-5!
        // but we have to use introspection here and unsafe formats
        //auto nread = f.readf("%s %s %s %s\n", &x, &y, &z, &cm);

        int nread;

        // introspection: better than ifdef
        if (is(typeof(x)==real)) {
            nread = fscanf(f.getFP(),"%Lf %Lf %Lf %Lf\n", &x, &y, &z, &cm);
        } else {
            nread = fscanf(f.getFP(),"%lf %lf %lf %lf\n", &x, &y, &z, &cm);
        }
        if (nread != 4) {
            throw new Exception("Could not read cap");
        }
    }
    this(ftype x, ftype y, ftype z, ftype cm) {
        this.x=x;
        this.y=y;
        this.z=z;
        this.cm=cm;
    }

    bool contains(in Point p) {
        bool cont;
        ftype cdot = 1.0 - x*p.x - y*p.y - z*p.z;
        if (cm < 0.0)
            cont = cdot > (-cm);
        else
            cont = cdot < cm;
        return cont;
    }

    string opCast() {
        string repr;
        if (is(typeof(x)==real)) {
            repr = format("%.19g %.19g %.19g %.19g",x,y,z,cm);
        } else {
            repr = format("%.16g %.16g %.16g %.16g",x,y,z,cm);
        }
        return repr;
    }
}

