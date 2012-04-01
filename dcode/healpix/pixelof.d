import std.stdio;
import std.getopt;
static import std.c.stdio;
import healpix;
import hpoint;

int procargs(string[] args, long* nside)
{
    int status=1;
    bool help=false;
    getopt(args, "help|h", &help, "nside|n", nside);

    if (help) {
        writeln("usage: cat file | pixelof [options] > outfile");
        status=0;
    }
    return status;
}

int main(string[] args) {
    long nside=4096;
    if (!procargs(args, &nside)) {
        return 1;
    }

    auto hpix = new Healpix(nside);
    auto point = new HPoint();
    long pix;
    double ra,dec;
    while (2 == std.c.stdio.scanf("%lf %lf", &ra, &dec)) {
        point.set(ra,dec);
        pix = hpix.pixelof(point);
        std.c.stdio.printf("%.16g %.16g %ld\n", ra, dec, pix);
    }
    return 0;
}
