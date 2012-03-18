import std.stdio;
import std.c.stdio;
import std.math;
import Typedef;

class Point {
    // must always initialize in classes, but not structs
    ftype x=0;
    ftype y=0;
    ftype z=0;
    ftype ra=0;
    ftype dec=0;

    this() {}
    this(ftype ra, ftype dec) {
        this.set(ra,dec);
    }
    this(ftype x, ftype y, ftype z) {
        this.set(x,y,z);
    }

    void set(ftype ra, ftype dec) {
        ftype cdec;

        this.ra = ra;
        this.dec = dec;

        ra *= PI/180.;
        dec *= PI/180.;

        cdec = cos(dec);
        x = cdec*cos(ra);
        y = cdec*sin(ra);
        z = sin(dec);
    }
    void set(ftype x, in ftype y, in ftype z) {
        this.x=x;
        this.y=y;
        this.z=z;

        this.ra = 180./PI*atan2(y, x);
        this.dec = 180./PI*asin(z);

        while (this.ra < 0.) {
            this.ra += 360.;
        }
        while (this.ra > 360.) {
            this.ra -= 360.;
        }
    }
    ftype dot(in Point p) {
        return this.x*p.x + this.y*p.y + this.z*p.z;
    }

    void read_radec(File* f) {
        // introspection: better than ifdef
        int nread;
        if (is(typeof(ra)==real)) {
            nread = fscanf(f.getFP(),"%Lf %Lf\n", &ra, &dec);
        } else {
            nread = fscanf(f.getFP(),"%lf %lf\n", &ra, &dec);
        }
        if (nread != 2) {
            throw new Exception("Could not read ra,dec");
        }
        this.set(ra,dec);
    }
    long read_radec_nothrow(File* f) {
        // introspection: better than ifdef
        int nread;
        if (is(typeof(ra)==real)) {
            nread = fscanf(f.getFP(),"%Lf %Lf\n", &ra, &dec);
        } else {
            nread = fscanf(f.getFP(),"%lf %lf\n", &ra, &dec);
        }

        this.set(ra,dec);
        return nread;
    }

    // note written *without* newline
    void write_radec(File* f) {
        if (is(typeof(ra)==real)) {
            fprintf(f.getFP(),"%.19Lg %.19Lg", ra, dec);
        } else {
            fprintf(f.getFP(),"%.16g %.16g", ra, dec);
        }
    }

}


