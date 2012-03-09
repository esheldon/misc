import std.stdio;

int main (char[][] args) 
{
    double x=0;
    double[] a;
    ptrdiff_t i=0;

    a.length = 100000000;

    for (i=0; i<a.length; i++) {
        a[i] = i*2 + 5;
    }
    foreach (val; a) {
        x += val;
    }
    writefln("sum: %.16g", x);

    return 0;
}
