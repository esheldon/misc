#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

union fmath_di {
    double d;
    uint64_t i;
};

static inline uint64_t fmath_mask64(int x)
{
    return (1ULL << x) - 1;
}
static inline unsigned int fmath_mask(int x)
{
    return (1U << x) - 1;
}



int main(int argc, char **argv)
{

    size_t table_size = 11;

    int sbit=table_size;
    int s =  1UL << sbit;
    int adj = (1UL << (sbit + 10)) - (1UL << sbit);
    //double a = s/log(2.0);

    double a = s/log(2.0);
    double ra = 1.0/a;

    uint64_t sbit_masked = fmath_mask(sbit);

    union fmath_di di;
    printf("#ifndef _FMATH_DTBL_GUARD\n");
    printf("#define _FMATH_DTBL_GUARD\n");
    printf("\n");
    printf("static const size_t sbit = %d;\n", sbit);
    printf("static const uint64_t sbit_masked = %lu;\n", sbit_masked);
    //printf("static const size_t s = %lu;\n", s);
    printf("static const size_t adj = %d;\n", adj);
    printf("static const double a = %.16g;\n", a);
    printf("static const double ra = %.16g;\n", ra);
    printf("static const uint64_t b = 3ULL << 51;\n");
    printf("static const double C1=1.0;\n"
           "static const double C2=0.16666666685227835064;\n"
           "static const double C3=3.0000000027955394;\n");
    printf("\n");
    printf("static const uint64_t dtbl[%d] = {\n", s);
    for (int i=0; i<s; i++) {
        // (i/s)^2
        di.d = pow(2.0, i * (1.0 / s));
        //di.d = i*(1.0/s);
        //di.d = di.d*di.d;

        uint64_t tblval = di.i & fmath_mask64(52);

        printf("%lu", tblval);
        if (i < (s-1)) {
            printf(",");
        }
        if (((i+1) % 10) == 0) {
            printf("\n");
        }

    }
    printf("};\n\n");
    printf("#endif\n");


    return 0;
}
