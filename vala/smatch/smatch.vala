/* vim: set ft=vala : */
using Gee;
using Math;

void print_matches(size_t index, 
                   ArrayList<Match> matches,
                   long maxmatch,
                   bool print_dist)
{
    if (matches.size > 0) {
        if (maxmatch > 0) {
            if (maxmatch < matches.size) {
                // not keeping all, sort and keep the closest matches
                matches.sort();
            }
        } else {
            maxmatch=matches.size;
        }
        Match match;
        for (long i=0; i<maxmatch; i++) {
            match = matches[(int)i];
            stdout.printf("%lu %ld", index, match.index);
            if (print_dist) {
                stdout.printf(" %.16g", 1.0-match.cos_dist);
            }
            stdout.printf("\n");
        }
        /*
        foreach (var match in matches) {
            stdout.printf("%lu %ld", index, match.index);
            if (print_dist) {
                stdout.printf(" %.16g", acos(match.cos_dist)*R2D);
            }
            stdout.printf("\n");
        }
        */
    }

}
void main() {
    bool print_dist=true;
    long nside=4096;
    double rad_arcsec=20.0;
    //long maxmatch=1;
    long maxmatch=-1;

    //string fname="/home/esheldon/tmp/rand-radec-10000.dat";
    string fname="/home/esheldon/tmp/scat-05-007-radec-only.dat";
    var cat = new Cat(fname,nside,rad_arcsec);

    //cat.print_counts();
    var matches = new ArrayList<Match>();
    //var pt = new HPoint.from_radec(200.0,0.0);
    var pt = new HPoint();

    size_t index=0;
    double ra=0,dec=0;
    while (2==stdin.scanf("%lf %lf", &ra, &dec)) {
        //stderr.printf("%.16g %.16g\n", ra, dec);
        pt.set_radec(ra,dec);
        cat.match(pt, matches);
        print_matches(index, matches, maxmatch, print_dist);
        /*
        matches.sort(match_compare);
        foreach (var m in matches) {
            stdout.printf("%ld %.16g\n", m.index, acos(m.cos_dist));
        }
        */
        index++;
    }
}


