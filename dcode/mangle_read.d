import std.stdio;
import std.string;
import std.conv;
import Point;
import Cap;
import Polygon;
import MangleMask;

int main(string[] args)
{
    string fname = args[1];

    auto mask = new MangleMask(fname,1);
    return 0;
}
