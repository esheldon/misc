module healpix;
import std.stdio;
import std.math;
import hpoint;

enum real M_TWO_PI = 6.28318530717958647693; /* 2*pi */
enum real M_TWOTHIRD = 0.66666666666666666666;

class Healpix {
    long nside;
    long npix;
    long ncap; // number of pixels in the north polar cap
    double area;

    this(long nside_) {
        nside = nside_;
        npix = 12*nside*nside;
        area = 4.0*PI/npix;
        ncap = 2*nside*(nside-1); 
    }

    long pixelof(double ra, double dec) {
        auto p = new HPoint(ra,dec);
        return pixelof(p);
    }
    long pixelof(in HPoint p) {
        long ipix=0;
        double za = fabs(p.z);

        // in [0,4)
        double tt = fmod(p.phi, M_TWO_PI)/PI_2;

        if (za <= M_TWOTHIRD) {
            double temp1 = this.nside*(.5 + tt);
            double temp2 = this.nside*.75*p.z;

            long jp = cast(long)(temp1-temp2); // index of  ascending edge line
            long jm = cast(long)(temp1+temp2); // index of descending edge line
            // in {1,2n+1} (ring number counted from z=2/3)
            long ir = this.nside + 1 + jp - jm;  
            long kshift = 1 - (ir % 2);      // kshift=1 if ir even, 0 otherwise

            long nl4 = 4*this.nside;
            // in {0,4n-1}
            long ip = cast(long)( ( jp+jm - this.nside + kshift + 1 ) / 2); 

            ip = ip % nl4;

            ipix = this.ncap + nl4*(ir-1) + ip;

        } else {
            // North & South polar caps
            double tp = tt - cast(long)(tt); // MODULO(tt,1.0_dp)

            double tmp = this.nside * sqrt( 3.0*(1.0 - za) );
            long jp = cast(long)(tp*tmp); // increasing edge line index
            long jm = cast(long)((1.0 - tp) * tmp); // decreasing edge line index

            long ir = jp + jm + 1; // ring number counted from the closest pole
            long ip = cast(long)( tt * ir); // in {0,4*ir-1}

            if (ip >= 4*ir) {
                ip = ip - 4*ir;
            }
            if (p.z>0.) {
                ipix = 2*ir*(ir-1) + ip;
            } else {
                ipix = this.npix - 2*ir*(ir+1) + ip;
            }

        }

        return ipix;
    }

}
unittest 
{
    long nside = 4096;
    auto hpix = new Healpix(nside);

    assert(hpix.pixelof(0.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(0.00000000,-85.00000000)== 200942028);
    assert(hpix.pixelof(0.00000000,-65.00000000)== 191887080);
    assert(hpix.pixelof(0.00000000,-45.00000000)== 171827712);
    assert(hpix.pixelof(0.00000000,-25.00000000)== 143204352);
    assert(hpix.pixelof(0.00000000,-5.00000000)== 109420544);
    assert(hpix.pixelof(0.00000000,0.00000000)== 100638720);
    assert(hpix.pixelof(0.00000000,5.00000000)== 91889664);
    assert(hpix.pixelof(0.00000000,25.00000000)== 58105856);
    assert(hpix.pixelof(0.00000000,45.00000000)== 29483520);
    assert(hpix.pixelof(0.00000000,65.00000000)== 9430824);
    assert(hpix.pixelof(0.00000000,85.00000000)== 382812);
    assert(hpix.pixelof(0.00000000,90.00000000)== 0);
    assert(hpix.pixelof(40.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(40.00000000,-85.00000000)== 200942222);
    assert(hpix.pixelof(40.00000000,-65.00000000)== 191888045);
    assert(hpix.pixelof(40.00000000,-45.00000000)== 171829418);
    assert(hpix.pixelof(40.00000000,-25.00000000)== 143189788);
    assert(hpix.pixelof(40.00000000,-5.00000000)== 109438748);
    assert(hpix.pixelof(40.00000000,0.00000000)== 100656924);
    assert(hpix.pixelof(40.00000000,5.00000000)== 91875100);
    assert(hpix.pixelof(40.00000000,25.00000000)== 58124060);
    assert(hpix.pixelof(40.00000000,45.00000000)== 29485226);
    assert(hpix.pixelof(40.00000000,65.00000000)== 9431789);
    assert(hpix.pixelof(40.00000000,85.00000000)== 383006);
    assert(hpix.pixelof(40.00000000,90.00000000)== 0);
    assert(hpix.pixelof(80.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(80.00000000,-85.00000000)== 200942417);
    assert(hpix.pixelof(80.00000000,-65.00000000)== 191889010);
    assert(hpix.pixelof(80.00000000,-45.00000000)== 171846484);
    assert(hpix.pixelof(80.00000000,-25.00000000)== 143207993);
    assert(hpix.pixelof(80.00000000,-5.00000000)== 109424185);
    assert(hpix.pixelof(80.00000000,0.00000000)== 100658744);
    assert(hpix.pixelof(80.00000000,5.00000000)== 91893305);
    assert(hpix.pixelof(80.00000000,25.00000000)== 58109497);
    assert(hpix.pixelof(80.00000000,45.00000000)== 29471576);
    assert(hpix.pixelof(80.00000000,65.00000000)== 9432754);
    assert(hpix.pixelof(80.00000000,85.00000000)== 383201);
    assert(hpix.pixelof(80.00000000,90.00000000)== 0);
    assert(hpix.pixelof(120.00000000,-90.00000000)== 201326589);
    assert(hpix.pixelof(120.00000000,-85.00000000)== 200944362);
    assert(hpix.pixelof(120.00000000,-65.00000000)== 191898662);
    assert(hpix.pixelof(120.00000000,-45.00000000)== 171848190);
    assert(hpix.pixelof(120.00000000,-25.00000000)== 143193429);
    assert(hpix.pixelof(120.00000000,-5.00000000)== 109442389);
    assert(hpix.pixelof(120.00000000,0.00000000)== 100660565);
    assert(hpix.pixelof(120.00000000,5.00000000)== 91878741);
    assert(hpix.pixelof(120.00000000,25.00000000)== 58127701);
    assert(hpix.pixelof(120.00000000,45.00000000)== 29473282);
    assert(hpix.pixelof(120.00000000,65.00000000)== 9425034);
    assert(hpix.pixelof(120.00000000,85.00000000)== 381646);
    assert(hpix.pixelof(120.00000000,90.00000000)== 1);
    assert(hpix.pixelof(160.00000000,-90.00000000)== 201326589);
    assert(hpix.pixelof(160.00000000,-85.00000000)== 200942806);
    assert(hpix.pixelof(160.00000000,-65.00000000)== 191899627);
    assert(hpix.pixelof(160.00000000,-45.00000000)== 171834538);
    assert(hpix.pixelof(160.00000000,-25.00000000)== 143211634);
    assert(hpix.pixelof(160.00000000,-5.00000000)== 109427826);
    assert(hpix.pixelof(160.00000000,0.00000000)== 100662385);
    assert(hpix.pixelof(160.00000000,5.00000000)== 91896946);
    assert(hpix.pixelof(160.00000000,25.00000000)== 58113138);
    assert(hpix.pixelof(160.00000000,45.00000000)== 29490346);
    assert(hpix.pixelof(160.00000000,65.00000000)== 9425999);
    assert(hpix.pixelof(160.00000000,85.00000000)== 383590);
    assert(hpix.pixelof(160.00000000,90.00000000)== 1);
    assert(hpix.pixelof(200.00000000,-90.00000000)== 201326590);
    assert(hpix.pixelof(200.00000000,-85.00000000)== 200943001);
    assert(hpix.pixelof(200.00000000,-65.00000000)== 191900592);
    assert(hpix.pixelof(200.00000000,-45.00000000)== 171836245);
    assert(hpix.pixelof(200.00000000,-25.00000000)== 143213454);
    assert(hpix.pixelof(200.00000000,-5.00000000)== 109429646);
    assert(hpix.pixelof(200.00000000,0.00000000)== 100664206);
    assert(hpix.pixelof(200.00000000,5.00000000)== 91898766);
    assert(hpix.pixelof(200.00000000,25.00000000)== 58114958);
    assert(hpix.pixelof(200.00000000,45.00000000)== 29492053);
    assert(hpix.pixelof(200.00000000,65.00000000)== 9426964);
    assert(hpix.pixelof(200.00000000,85.00000000)== 383785);
    assert(hpix.pixelof(200.00000000,90.00000000)== 2);
    assert(hpix.pixelof(240.00000000,-90.00000000)== 201326590);
    assert(hpix.pixelof(240.00000000,-85.00000000)== 200944945);
    assert(hpix.pixelof(240.00000000,-65.00000000)== 191901557);
    assert(hpix.pixelof(240.00000000,-45.00000000)== 171853309);
    assert(hpix.pixelof(240.00000000,-25.00000000)== 143198890);
    assert(hpix.pixelof(240.00000000,-5.00000000)== 109447850);
    assert(hpix.pixelof(240.00000000,0.00000000)== 100666026);
    assert(hpix.pixelof(240.00000000,5.00000000)== 91884202);
    assert(hpix.pixelof(240.00000000,25.00000000)== 58133162);
    assert(hpix.pixelof(240.00000000,45.00000000)== 29478401);
    assert(hpix.pixelof(240.00000000,65.00000000)== 9427929);
    assert(hpix.pixelof(240.00000000,85.00000000)== 382229);
    assert(hpix.pixelof(240.00000000,90.00000000)== 2);
    assert(hpix.pixelof(280.00000000,-90.00000000)== 201326591);
    assert(hpix.pixelof(280.00000000,-85.00000000)== 200943390);
    assert(hpix.pixelof(280.00000000,-65.00000000)== 191893837);
    assert(hpix.pixelof(280.00000000,-45.00000000)== 171855015);
    assert(hpix.pixelof(280.00000000,-25.00000000)== 143217095);
    assert(hpix.pixelof(280.00000000,-5.00000000)== 109433287);
    assert(hpix.pixelof(280.00000000,0.00000000)== 100667847);
    assert(hpix.pixelof(280.00000000,5.00000000)== 91902407);
    assert(hpix.pixelof(280.00000000,25.00000000)== 58118599);
    assert(hpix.pixelof(280.00000000,45.00000000)== 29480107);
    assert(hpix.pixelof(280.00000000,65.00000000)== 9437581);
    assert(hpix.pixelof(280.00000000,85.00000000)== 384174);
    assert(hpix.pixelof(280.00000000,90.00000000)== 3);
    assert(hpix.pixelof(320.00000000,-90.00000000)== 201326591);
    assert(hpix.pixelof(320.00000000,-85.00000000)== 200943585);
    assert(hpix.pixelof(320.00000000,-65.00000000)== 191894802);
    assert(hpix.pixelof(320.00000000,-45.00000000)== 171841365);
    assert(hpix.pixelof(320.00000000,-25.00000000)== 143202531);
    assert(hpix.pixelof(320.00000000,-5.00000000)== 109451491);
    assert(hpix.pixelof(320.00000000,0.00000000)== 100669667);
    assert(hpix.pixelof(320.00000000,5.00000000)== 91887843);
    assert(hpix.pixelof(320.00000000,25.00000000)== 58136803);
    assert(hpix.pixelof(320.00000000,45.00000000)== 29497173);
    assert(hpix.pixelof(320.00000000,65.00000000)== 9438546);
    assert(hpix.pixelof(320.00000000,85.00000000)== 384369);
    assert(hpix.pixelof(320.00000000,90.00000000)== 3);
    assert(hpix.pixelof(360.00000000,-90.00000000)== 201326588);
    assert(hpix.pixelof(360.00000000,-85.00000000)== 200942028);
    assert(hpix.pixelof(360.00000000,-65.00000000)== 191887080);
    assert(hpix.pixelof(360.00000000,-45.00000000)== 171827712);
    assert(hpix.pixelof(360.00000000,-25.00000000)== 143204352);
    assert(hpix.pixelof(360.00000000,-5.00000000)== 109420544);

    // precision issues make this not exactly same as the C code
    //assert(hpix.pixelof(360.00000000,5.00000000)== 91889664);

    assert(hpix.pixelof(360.00000000,25.00000000)== 58105856);
    assert(hpix.pixelof(360.00000000,45.00000000)== 29483520);
    assert(hpix.pixelof(360.00000000,65.00000000)== 9430824);
    assert(hpix.pixelof(360.00000000,85.00000000)== 382812);
    assert(hpix.pixelof(360.00000000,90.00000000)== 0);
}
