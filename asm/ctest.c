#include <stdlib.h>
#include <stdio.h>
int main(int argc, char** argv) {
    double ra,dec;

    while (2==scanf("%lf %lf",&ra,&dec)) {
        printf("%.16g %.16g",ra,dec);
    }
    return 0;
}
