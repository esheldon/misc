#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mangle.h"

#define D2R  0.017453292519943295
#define R2D  57.295779513082323

using namespace std;

int	main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: polyid <polygon-file> < infile > outfile " <<endl;
        exit(1);
    }
    long pid=0;
    double ra=0, dec=0;
    string mask_file(argv[1]);

    Mangle::MaskClass mm(mask_file);
    cerr << "# Read " << mm.npolygons() << " polygons from " << mask_file << endl;

    while (2 == scanf("%lf %lf", &ra, &dec)) {
        //pid = mm.polyid(ra*D2R, (90.0-dec)*D2R);
        pid = mm.polyid((90.0-dec)*D2R, ra*D2R);
        printf("%0.16g %0.16g %ld\n", ra, dec, pid);
    }
}
