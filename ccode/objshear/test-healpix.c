#include <stdio.h>
#include <stdlib.h>
#include "healpix.h"
#include "defs.h"

int main(int argc, char** argv) {
    int64 nside=4096;

    struct healpix* hpix = hpix_new(nside);

    printf("nside: %ld\n", nside);
    printf("  npix: %ld\n", hpix->npix);
    printf("  area: %le\n", hpix->area*R2D*R2D);

    double ra1=175.0;
    double dec1=27.2;
    double vector[3];
    hpix_eq2vec(ra1, dec1, vector);
    printf("convert %15.8lf %15.8lf to vector (%15.8lf %15.8lf %15.8lf)\n", 
            ra1, dec1, vector[0], vector[1], vector[2]);

    double z[4]={-0.75, -0.2, 0.2, 0.75};
    for (int i=0; i<4; i++) {
        int64 ringnum = hpix_ring_num(hpix, z[i]);
        printf("ring num at z=%15.8lf: %ld\n", z[i], ringnum);
    }

    double ra[10]={0.,   40.,   80.,  120.,  160.,  200.,  240.,  280.,  320.,  360.};
    double dec[11] = {-85., -65., -45., -25., -5., 0., 5., 25., 45., 65., 85.};
    printf("testing eq2pix\n");
    for (int ira=0; ira<10; ira++) {
        for (int idec=0; idec<11; idec++) {
            int64 ipix = hpix_eq2pix(hpix, ra[ira], dec[idec]);
            printf("%15.8lf %15.8lf %ld\n", ra[ira], dec[idec], ipix);
        }
    }

    hpix_delete(hpix);

    int64 npface = nside*nside;
    printf("ncap from c++: %ld\n", (npface-nside)<<1);
    printf("ncap guess from mine: %ld\n", 2*nside*(nside-1) );
}
