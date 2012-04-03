/* vim: set ft=vala : */
using Gee;

/*
struct Color {
    public double red;
    public double green;
    public double blue;
}
*/

void main() {
    /*
    stdout.printf("hello world\n");

    var list = new ArrayList<long> ();

    list.add(35);
    list.add(77);
    foreach (long i in list) {
        stdout.printf("val: %ld\n", i);
    }

    var mc = new MyClass();

    stdout.printf("mc.x: %d\n", mc.x);

    var mcarr = new ArrayList<MyClass>();

    for (int64 i=0; i<1000; i++) {
        mcarr.add(new MyClass());
    }


    var acolor=Color();
    acolor.red = 0.3;

    var c = new Color[200];

    c[35].red = 0.5;

    stdout.printf("c[35].red: %.16g\n", c[35].red);
    stdout.printf("cos(c[35].red): %.16g\n", GLib.Math.cos(c[35].red));
    */

    test_pixelof();
    test_ring_num();
    test_intersect();
}
void test_ring_num() 
{
    stdout.printf("hpix: doing ring_num unit test\n");

    var hpix = new Healpix(4096);

    warn_if_fail(hpix.ring_num(-0.75) == 12837);
    warn_if_fail(hpix.ring_num(-0.2) == 9421);
    warn_if_fail(hpix.ring_num(0.2) == 6963);
    warn_if_fail(hpix.ring_num(0.75) == 3547);
}
void test_pixelof() 
{
    stdout.printf("hpix: doing pixelof unit test\n");

    var hpix = new Healpix(4096);

    warn_if_fail(hpix.pixelof_eq(0.00000000,-90.00000000)== 201326588);
    warn_if_fail(hpix.pixelof_eq(0.00000000,-85.00000000)== 200942028);
    warn_if_fail(hpix.pixelof_eq(0.00000000,-65.00000000)== 191887080);
    warn_if_fail(hpix.pixelof_eq(0.00000000,-45.00000000)== 171827712);
    warn_if_fail(hpix.pixelof_eq(0.00000000,-25.00000000)== 143204352);
    warn_if_fail(hpix.pixelof_eq(0.00000000,-5.00000000)== 109420544);
    warn_if_fail(hpix.pixelof_eq(0.00000000,0.00000000)== 100638720);
    warn_if_fail(hpix.pixelof_eq(0.00000000,5.00000000)== 91889664);
    warn_if_fail(hpix.pixelof_eq(0.00000000,25.00000000)== 58105856);
    warn_if_fail(hpix.pixelof_eq(0.00000000,45.00000000)== 29483520);
    warn_if_fail(hpix.pixelof_eq(0.00000000,65.00000000)== 9430824);
    warn_if_fail(hpix.pixelof_eq(0.00000000,85.00000000)== 382812);
    warn_if_fail(hpix.pixelof_eq(0.00000000,90.00000000)== 0);
    warn_if_fail(hpix.pixelof_eq(40.00000000,-90.00000000)== 201326588);
    warn_if_fail(hpix.pixelof_eq(40.00000000,-85.00000000)== 200942222);
    warn_if_fail(hpix.pixelof_eq(40.00000000,-65.00000000)== 191888045);
    warn_if_fail(hpix.pixelof_eq(40.00000000,-45.00000000)== 171829418);
    warn_if_fail(hpix.pixelof_eq(40.00000000,-25.00000000)== 143189788);
    warn_if_fail(hpix.pixelof_eq(40.00000000,-5.00000000)== 109438748);
    warn_if_fail(hpix.pixelof_eq(40.00000000,0.00000000)== 100656924);
    warn_if_fail(hpix.pixelof_eq(40.00000000,5.00000000)== 91875100);
    warn_if_fail(hpix.pixelof_eq(40.00000000,25.00000000)== 58124060);
    warn_if_fail(hpix.pixelof_eq(40.00000000,45.00000000)== 29485226);
    warn_if_fail(hpix.pixelof_eq(40.00000000,65.00000000)== 9431789);
    warn_if_fail(hpix.pixelof_eq(40.00000000,85.00000000)== 383006);
    warn_if_fail(hpix.pixelof_eq(40.00000000,90.00000000)== 0);
    warn_if_fail(hpix.pixelof_eq(80.00000000,-90.00000000)== 201326588);
    warn_if_fail(hpix.pixelof_eq(80.00000000,-85.00000000)== 200942417);
    warn_if_fail(hpix.pixelof_eq(80.00000000,-65.00000000)== 191889010);
    warn_if_fail(hpix.pixelof_eq(80.00000000,-45.00000000)== 171846484);
    warn_if_fail(hpix.pixelof_eq(80.00000000,-25.00000000)== 143207993);
    warn_if_fail(hpix.pixelof_eq(80.00000000,-5.00000000)== 109424185);
    warn_if_fail(hpix.pixelof_eq(80.00000000,0.00000000)== 100658744);
    warn_if_fail(hpix.pixelof_eq(80.00000000,5.00000000)== 91893305);
    warn_if_fail(hpix.pixelof_eq(80.00000000,25.00000000)== 58109497);
    warn_if_fail(hpix.pixelof_eq(80.00000000,45.00000000)== 29471576);
    warn_if_fail(hpix.pixelof_eq(80.00000000,65.00000000)== 9432754);
    warn_if_fail(hpix.pixelof_eq(80.00000000,85.00000000)== 383201);
    warn_if_fail(hpix.pixelof_eq(80.00000000,90.00000000)== 0);
    warn_if_fail(hpix.pixelof_eq(120.00000000,-90.00000000)== 201326589);
    warn_if_fail(hpix.pixelof_eq(120.00000000,-85.00000000)== 200944362);
    warn_if_fail(hpix.pixelof_eq(120.00000000,-65.00000000)== 191898662);
    warn_if_fail(hpix.pixelof_eq(120.00000000,-45.00000000)== 171848190);
    warn_if_fail(hpix.pixelof_eq(120.00000000,-25.00000000)== 143193429);
    warn_if_fail(hpix.pixelof_eq(120.00000000,-5.00000000)== 109442389);
    warn_if_fail(hpix.pixelof_eq(120.00000000,0.00000000)== 100660565);
    warn_if_fail(hpix.pixelof_eq(120.00000000,5.00000000)== 91878741);
    warn_if_fail(hpix.pixelof_eq(120.00000000,25.00000000)== 58127701);
    warn_if_fail(hpix.pixelof_eq(120.00000000,45.00000000)== 29473282);
    warn_if_fail(hpix.pixelof_eq(120.00000000,65.00000000)== 9425034);
    warn_if_fail(hpix.pixelof_eq(120.00000000,85.00000000)== 381646);
    warn_if_fail(hpix.pixelof_eq(120.00000000,90.00000000)== 1);
    warn_if_fail(hpix.pixelof_eq(160.00000000,-90.00000000)== 201326589);
    warn_if_fail(hpix.pixelof_eq(160.00000000,-85.00000000)== 200942806);
    warn_if_fail(hpix.pixelof_eq(160.00000000,-65.00000000)== 191899627);
    warn_if_fail(hpix.pixelof_eq(160.00000000,-45.00000000)== 171834538);
    warn_if_fail(hpix.pixelof_eq(160.00000000,-25.00000000)== 143211634);
    warn_if_fail(hpix.pixelof_eq(160.00000000,-5.00000000)== 109427826);
    warn_if_fail(hpix.pixelof_eq(160.00000000,0.00000000)== 100662385);
    warn_if_fail(hpix.pixelof_eq(160.00000000,5.00000000)== 91896946);
    warn_if_fail(hpix.pixelof_eq(160.00000000,25.00000000)== 58113138);
    warn_if_fail(hpix.pixelof_eq(160.00000000,45.00000000)== 29490346);
    warn_if_fail(hpix.pixelof_eq(160.00000000,65.00000000)== 9425999);
    warn_if_fail(hpix.pixelof_eq(160.00000000,85.00000000)== 383590);
    warn_if_fail(hpix.pixelof_eq(160.00000000,90.00000000)== 1);
    warn_if_fail(hpix.pixelof_eq(200.00000000,-90.00000000)== 201326590);
    warn_if_fail(hpix.pixelof_eq(200.00000000,-85.00000000)== 200943001);
    warn_if_fail(hpix.pixelof_eq(200.00000000,-65.00000000)== 191900592);
    warn_if_fail(hpix.pixelof_eq(200.00000000,-45.00000000)== 171836245);
    warn_if_fail(hpix.pixelof_eq(200.00000000,-25.00000000)== 143213454);
    warn_if_fail(hpix.pixelof_eq(200.00000000,-5.00000000)== 109429646);
    warn_if_fail(hpix.pixelof_eq(200.00000000,0.00000000)== 100664206);
    warn_if_fail(hpix.pixelof_eq(200.00000000,5.00000000)== 91898766);
    warn_if_fail(hpix.pixelof_eq(200.00000000,25.00000000)== 58114958);
    warn_if_fail(hpix.pixelof_eq(200.00000000,45.00000000)== 29492053);
    warn_if_fail(hpix.pixelof_eq(200.00000000,65.00000000)== 9426964);
    warn_if_fail(hpix.pixelof_eq(200.00000000,85.00000000)== 383785);
    warn_if_fail(hpix.pixelof_eq(200.00000000,90.00000000)== 2);
    warn_if_fail(hpix.pixelof_eq(240.00000000,-90.00000000)== 201326590);
    warn_if_fail(hpix.pixelof_eq(240.00000000,-85.00000000)== 200944945);
    warn_if_fail(hpix.pixelof_eq(240.00000000,-65.00000000)== 191901557);
    warn_if_fail(hpix.pixelof_eq(240.00000000,-45.00000000)== 171853309);
    warn_if_fail(hpix.pixelof_eq(240.00000000,-25.00000000)== 143198890);
    warn_if_fail(hpix.pixelof_eq(240.00000000,-5.00000000)== 109447850);
    warn_if_fail(hpix.pixelof_eq(240.00000000,0.00000000)== 100666026);
    warn_if_fail(hpix.pixelof_eq(240.00000000,5.00000000)== 91884202);
    warn_if_fail(hpix.pixelof_eq(240.00000000,25.00000000)== 58133162);
    warn_if_fail(hpix.pixelof_eq(240.00000000,45.00000000)== 29478401);
    warn_if_fail(hpix.pixelof_eq(240.00000000,65.00000000)== 9427929);
    warn_if_fail(hpix.pixelof_eq(240.00000000,85.00000000)== 382229);
    warn_if_fail(hpix.pixelof_eq(240.00000000,90.00000000)== 2);
    warn_if_fail(hpix.pixelof_eq(280.00000000,-90.00000000)== 201326591);
    warn_if_fail(hpix.pixelof_eq(280.00000000,-85.00000000)== 200943390);
    warn_if_fail(hpix.pixelof_eq(280.00000000,-65.00000000)== 191893837);
    warn_if_fail(hpix.pixelof_eq(280.00000000,-45.00000000)== 171855015);
    warn_if_fail(hpix.pixelof_eq(280.00000000,-25.00000000)== 143217095);
    warn_if_fail(hpix.pixelof_eq(280.00000000,-5.00000000)== 109433287);
    warn_if_fail(hpix.pixelof_eq(280.00000000,0.00000000)== 100667847);
    warn_if_fail(hpix.pixelof_eq(280.00000000,5.00000000)== 91902407);
    warn_if_fail(hpix.pixelof_eq(280.00000000,25.00000000)== 58118599);
    warn_if_fail(hpix.pixelof_eq(280.00000000,45.00000000)== 29480107);
    warn_if_fail(hpix.pixelof_eq(280.00000000,65.00000000)== 9437581);
    warn_if_fail(hpix.pixelof_eq(280.00000000,85.00000000)== 384174);
    warn_if_fail(hpix.pixelof_eq(280.00000000,90.00000000)== 3);
    warn_if_fail(hpix.pixelof_eq(320.00000000,-90.00000000)== 201326591);
    warn_if_fail(hpix.pixelof_eq(320.00000000,-85.00000000)== 200943585);
    warn_if_fail(hpix.pixelof_eq(320.00000000,-65.00000000)== 191894802);
    warn_if_fail(hpix.pixelof_eq(320.00000000,-45.00000000)== 171841365);
    warn_if_fail(hpix.pixelof_eq(320.00000000,-25.00000000)== 143202531);
    warn_if_fail(hpix.pixelof_eq(320.00000000,-5.00000000)== 109451491);
    warn_if_fail(hpix.pixelof_eq(320.00000000,0.00000000)== 100669667);
    warn_if_fail(hpix.pixelof_eq(320.00000000,5.00000000)== 91887843);
    warn_if_fail(hpix.pixelof_eq(320.00000000,25.00000000)== 58136803);
    warn_if_fail(hpix.pixelof_eq(320.00000000,45.00000000)== 29497173);
    warn_if_fail(hpix.pixelof_eq(320.00000000,65.00000000)== 9438546);
    warn_if_fail(hpix.pixelof_eq(320.00000000,85.00000000)== 384369);
    warn_if_fail(hpix.pixelof_eq(320.00000000,90.00000000)== 3);
    warn_if_fail(hpix.pixelof_eq(360.00000000,-90.00000000)== 201326588);
    warn_if_fail(hpix.pixelof_eq(360.00000000,-85.00000000)== 200942028);
    warn_if_fail(hpix.pixelof_eq(360.00000000,-65.00000000)== 191887080);
    warn_if_fail(hpix.pixelof_eq(360.00000000,-45.00000000)== 171827712);
    warn_if_fail(hpix.pixelof_eq(360.00000000,-25.00000000)== 143204352);
    warn_if_fail(hpix.pixelof_eq(360.00000000,-5.00000000)== 109420544);

    warn_if_fail(hpix.pixelof_eq(360.00000000,0.00000000)== 100638720);

    warn_if_fail(hpix.pixelof_eq(360.00000000,5.00000000)== 91889664);
    warn_if_fail(hpix.pixelof_eq(360.00000000,25.00000000)== 58105856);
    warn_if_fail(hpix.pixelof_eq(360.00000000,45.00000000)== 29483520);
    warn_if_fail(hpix.pixelof_eq(360.00000000,65.00000000)== 9430824);
    warn_if_fail(hpix.pixelof_eq(360.00000000,85.00000000)== 382812);
    warn_if_fail(hpix.pixelof_eq(360.00000000,90.00000000)== 0);

}
void test_intersect()
{
    var pixlist = new ArrayList<long>();

    var hpix = new Healpix(4096);

    double rad_arcmin=40.0/60.0;
    double rad = rad_arcmin/60.0*D2R;

    stdout.printf("hpix: doing intersect unit test\n");
    warn_if_fail(hpix.disc_intersect_eq(0.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,-85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,-45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,-5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(0.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,-85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,-45.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,-5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,45.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(40.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,-85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,-45.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,-5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,45.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(80.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,-85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,-45.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,-5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,45.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(120.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,-85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,-65.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,-45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,-5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,65.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(160.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,-85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,-65.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,-45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,-5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,65.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(200.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,-85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,-45.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,-5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,45.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(240.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,-85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,-45.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,-5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,45.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(280.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,-85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,-45.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,-5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,5.000000,rad,pixlist)==7);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,45.000000,rad,pixlist)==10);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,85.000000,rad,pixlist)==9);
    warn_if_fail(hpix.disc_intersect_eq(320.000000,90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,-90.000000,rad,pixlist)==12);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,-85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,-65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,-45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,-25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,-5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,0.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,5.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,25.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,45.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,65.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,85.000000,rad,pixlist)==8);
    warn_if_fail(hpix.disc_intersect_eq(360.000000,90.000000,rad,pixlist)==12);

}
