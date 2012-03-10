import std.stdio;
import std.string;
import std.conv;
import Point;
import Cap;
import Polygon;
import MangleMask;

int main(string[] args)
{
    if (args.length < 2) {
        stderr.writeln("usage: mangle_read mask_file");
        stderr.writeln("    for testing reading masks");
        return 1;
    }
    string fname = args[1];

    int verbosity=1;
    if (args.length > 2) {
        verbosity = args[2].to!int();
    }
    auto mask = new MangleMask(fname,verbosity);
    return 0;
}
