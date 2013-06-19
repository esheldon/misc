/* vim: set ft=vala : */
using Gee;

void main() {
    long nside=4096;
    double rad_arcsec=2.0;
    var cat = new Cat("/home/esheldon/tmp/rand-radec.dat",nside,rad_arcsec);

    //cat.print_counts();
    double ra=200.0, dec=-8.34;
    var matches = new ArrayList<Match>();
    var pth = new HPoint();
    pth.set_radec(ra,dec);

    var pt = new Point.from_radec(ra,dec);

    stderr.printf("pt.phi: %.16g pth.phi: %.16g\n",pt.phi,pth.phi);

    cat.match(pth, matches);

    stderr.printf("found %d matches\n", matches.size);
    foreach (var m in matches) {
        stderr.printf("%ld %.16g\n", m.index, m.cos_dist);
    }
    matches.sort(match_compare);
}


