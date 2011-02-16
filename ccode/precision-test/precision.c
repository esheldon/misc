/*
 *
 * Conclusions from this:
 *      for 4-byte float
 *          %.7g is the appropriate format
 *
 *      for 8-byte float
 *          %.16g or %.15e
 *
 *          The tradeoff is readability.  the g format will write very
 *          large numbers like 3.141592653589793e+12 as 3141592653589.793.
 *          while this saves space it is not readable.
 *
 *      I'm leaning toward the %.7g for floats and the %.15e for doubles
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>

int main(int argc, char** argv) {

    float fpi=3.141592653589793238462643383279502884197;
    double dpi=3.141592653589793238462643383279502884197;

    char pistr[] = "3.141592653589793238462643383279502884197";
    printf("actual pi:     %s\n", pistr);
    printf("float pi %%.15g %.15g\n", fpi);
    printf("float pi %%.7g  %.7g\n", fpi);
    printf("float pi %%.6e  %.6e\n", fpi);
    printf("float pi %%g    %g\n", fpi);
    printf("         %%.7g  %.7g\n", fpi*1.e6);
    printf("         %%.7g  %.7g\n", fpi*1.e8);
    printf("         %%.7g  %.7g\n", fpi*1.e12);
    printf("         %%.7g  %.7g\n", 21.3496753213);
    printf("         %%.6e  %.6e\n", 21.3496753213);
    printf("\n\n");

    printf("actual pi:      %s\n", pistr);
    printf("double pi %%.20g %.20g\n", dpi);
    printf("double pi %%.16g %.16g\n", dpi);
    printf("double pi %%.15e %.15e\n", dpi);
    printf("double pi %%g    %g\n", dpi);
    printf("double t  %%.16g %.16g\n", dpi*1.e12);
    printf("double t  %%.15e %.15e\n", dpi*1.e12);
    printf("double t  %%.15e %.15e\n", -dpi*1.e12);
    printf("double t  %%.16g %.16g\n", dpi*1.e24);


    return 0;

    /*
    printf("float fpi: %f\n", fpi);
    printf("float exp fpi: %e\n", fpi);
    printf("float g fpi: %e\n", fpi);
    printf("float .8g fpi: %.8g\n", fpi);
    printf("float .8g fpi: %.8g\n", fpi*100.0);
    printf("float .8g fpi: %.8g\n", fpi*10000.0);
    printf("float .8g fpi: %.8g\n", fpi*1000000.0);
    printf("float .8g fpi: %.8g\n", fpi*1.e9);

    printf("double dpi: %lf\n", dpi);
    printf("double exp dpi: %.15le\n", dpi);

    printf("double t: %.15le\n", t);
    printf("double g t: %g\n", t);
    */
}
