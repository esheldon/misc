import std.stdio;
import std.math;
import stack;

struct Match {
    long index;
    double cos_dist;

    this(long index, double cos_dist) {
        this.index = index;
        this.cos_dist=cos_dist;
    }
    int opCmp(ref const Match m) const {
        return m.cos_dist > this.cos_dist;
    }
}
unittest 
{
    Match[] matches;

    matches.length = 5;
    matches[0].cos_dist = cos(0.05);
    matches[1].cos_dist = cos(0.01);
    matches[2].cos_dist = cos(0.0);
    matches[3].cos_dist = cos(0.07);
    matches[4].cos_dist = cos(0.04);

    writeln("match: before sorting");
    foreach (m; matches) {
        writefln("  %.16g",m.cos_dist);
    }

    matches.sort;

    writeln("match: after sorting");
    foreach (m; matches) {
        writefln("  %.16g",m.cos_dist);
    }


    // see if we can sort a subset
    matches[0].cos_dist = cos(0.05);
    matches[1].cos_dist = cos(0.01);
    matches[2].cos_dist = cos(0.0);
    matches[3].cos_dist = cos(0.07);
    matches[4].cos_dist = cos(0.04);


    matches[2..$].sort;
    writeln("match: after subset sorting 2..$");
    foreach (m; matches) {
        writefln("  %.16g",m.cos_dist);
    }

}
unittest 
{
    Stack!(Match) matches;

    // can do this better by building a special stack
    matches.push(Match(0, cos(0.05)));
    matches.push(Match(1, cos(0.01)));
    matches.push(Match(2, cos(0.00)));
    matches.push(Match(3, cos(0.07)));
    matches.push(Match(4, cos(0.04)));

    matches.resize(3);
    matches.sort();

    writeln("match: Sorting a stack of em, resized 3");
    foreach (m; matches) {
        writefln("  %s %.16g",m.index,m.cos_dist);
    }
}
