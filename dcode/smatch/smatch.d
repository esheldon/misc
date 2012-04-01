import std.stdio;
import std.getopt;
import std.math;
static import std.c.stdio;
import cat;
import stack;
import hpoint;
import match;
import point;

void usage() {
    writeln(
"usage:
    cat file1 | smatch [options] file2 > result

Use smaller list as file2 and stream the larger. Each line of output is 
    index1 index2 
If there are dups in one list, send that on stdin.

options:

    --rad {val}
    -r{val}  search radius in arcsec. If not sent, must be the third column in
             file2, in which case it can be different for each point.

    --nside {val}
    -n{val}  nside nside parameter for healpix, power of two, default 4096.

    --maxmatch {val}
    -m{val}  maximum number of matches.  Default is 1.  maxmatch=0 means return
             all matches

    --printdist
    -d       print out the distance in degrees in the third column

    --verbose
    -v       print out info and progress in stderr

    --help
    -h       print this message and exit
"   );

}
int procargs(string[] args, 
               long* nside, long* maxmatch, 
               double* rad_arcsec, bool* print_dist, bool* verbose,
               string* cat_file)
{
    int status=1;
    bool help=false;
    getopt(args, 
           "help|h", &help,
           "nside|n", nside, 
           "maxmatch|m", maxmatch,
           "rad|r", rad_arcsec,
           "printdist|d",print_dist,
           "verbose|v",verbose);

    if (help || args.length < 2) {
        usage();
        status=0;
    } else {
        if (*verbose) {
            if (*rad_arcsec > 0)
                stderr.writefln("radius:      %0.2g arcsec", *rad_arcsec);
            stderr.writefln("nside:       %s", *nside);
            stderr.writefln("maxmatch:    %s", *maxmatch);
            stderr.writefln("print dist?: %s",*print_dist); 
            stderr.writefln("file:        %s", args[1]);
        }
    }
    *cat_file=args[1];
    return status;
}

void print_matches(size_t index, 
                   Stack!(Match)* matches, 
                   long maxmatch, 
                   int print_dist) {

    if (matches.length > 0) {
        if (maxmatch > 0) {
            if (maxmatch < matches.length) {
                // not keeping all, sort and keep the closest matches
                matches.sort();
                matches.resize(maxmatch);
            }
        }
        foreach (match; *matches) {
            std.c.stdio.printf("%lu %ld", index, match.index);
            if (print_dist) {
                std.c.stdio.printf(" %.16g", acos(match.cos_dist)*R2D);
            }
            std.c.stdio.printf("\n");
        }
    }
}

int main(string[] args)
{
    long nside=4096, maxmatch=1;
    double rad_arcsec=-1;
    bool print_dist=false, verbose=false;
    string cat_file;

    if (!procargs(args, &nside, &maxmatch, &rad_arcsec, 
                  &print_dist, &verbose, &cat_file) ) { 
        return 1;
    }

    auto cat = new Cat(cat_file,nside,rad_arcsec);

    size_t index=0;
    Stack!(Match) matches;
    auto point = new HPoint();
    double ra,dec;

    while (2 == std.c.stdio.scanf("%lf %lf", &ra, &dec)) {
        point.set(ra,dec);
        cat.match(point, &matches);
        print_matches(index, &matches, maxmatch, print_dist);
        index++;
    }
    return 0;
}
