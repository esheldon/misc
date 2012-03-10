// this one only works when polygons are snapped and balkanized
import std.stdio;
import std.string;
import std.conv;
import Point;
import Cap;
import Polygon;
import MangleMask;
import Typedef;

int main(string[] args)
{
    if (args.length < 2) {
        stderr.writeln("usage: mangle_polyid mask_file");
        stderr.writeln("    send ra dec on stdin");
        stderr.writeln("    ra dec polyid goes to stdout");
        return 1;
    }
    string mask_url = args[1];

    int verbosity=1;
    if (args.length > 2) {
        verbosity = args[2].to!int();
    }
    
    auto mask = new MangleMask(mask_url,verbosity);

    auto p = new Point();
    ftype ra=0, dec=0;
    long poly_id;

    while (2 == p.read_radec_nothrow(&stdin)) {
        p.write_radec(&stdout);

        poly_id = mask.polyid(p);
        stdout.writef(" %s\n", poly_id);
    }
    return 0;
}
