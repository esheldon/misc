import std.stdio;
import std.string;
import std.conv;
import Point;
import Cap;
import Typedef;

struct Polygon {
    long poly_id;
    ftype weight;
    ftype area;
    long pixel_id;

    long ncaps;
    Cap[] caps;

    // only for classes
    //this() { }

    this(File* file, char[][] header) {
        this.read(file,header);
    }
    this(long poly_id, ftype weight, ftype area, long pixel_id) {

        this.poly_id=poly_id;
        this.weight=weight;
        this.area=area;
        this.pixel_id=pixel_id;

    }

    // take a file object and the header tokens
    // file must be placed after the header
    void read(File* file, char[][] header) {
        this.process_header(header);
        this.read_caps(file);
    }
    void process_header(char[][] header) {
        if (header.length < 9) {
            throw new Exception(format(
                "Expected at least 9 header tokens, got %s",header.length));
        }
        if (header[0] != "polygon") {
            throw new Exception(format(
              "Expected polygon as first keyword, got %s", header[0]));
        }
        if (header[2] != "(" || header[$-1] != "str):") {
            throw new Exception(format(
              "Malformed polygon header: %s", header.join(" ")));

        }
        this.poly_id = header[1].to!long();

        for (long i=3; i < header.length-1; i++) {
            if (header[i+1] == "caps,") {
                this.ncaps = header[i].to!long();
            } else if (header[i+1] == "weight,") {
                this.weight = header[i].to!ftype();
            } else if (header[i+1] == "pixel,") {
                this.pixel_id = header[i].to!long();
            } else if (header[i+1] == "str):") {
                this.area = header[i].to!ftype();
            }
        }

        if (this.ncaps < 1) {
            throw new Exception(format("got ncaps < 1: %s",this.ncaps));
        }
    }
    void read_caps(File* file) {
        this.caps.length = this.ncaps;
        for (long i=0; i<this.caps.length; i++) {
            this.caps[i].read(file);
        }
    }

    bool contains(in Point p) {
        bool inpoly=true;
        foreach (cap; this.caps) {
            inpoly = inpoly && cap.contains(p);
            if (!inpoly) { break; }
        }
        return inpoly;
    }

    string opCast() {
        string rep;
        rep = format(
          "polyid: %s ncaps: %s pixel: %s weight: %0.16g area: %0.16g",
          this.poly_id,this.ncaps,this.pixel_id,this.weight,this.area);
        return rep;
    }

    void print_caps() {
        for (long i=0; i<this.caps.length; i++) {
            stderr.writeln(cast(string)this.caps[i]);
        }
    }

}
