#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../healpix.h"
#include "../stack.h"
#include "../defs.h"

int main(int argc, char** argv) {
    int64 nside=4096;

    int nra=10, ndec=13;

    struct healpix* hpix = hpix_new(nside);

    printf("nside: %ld\n", nside);
    printf("  npix: %ld\n", hpix->npix);
    printf("  area: %le\n", hpix->area*R2D*R2D);

    double z[4]={-0.75, -0.2, 0.2, 0.75};
    for (int i=0; i<4; i++) {
        int64 ringnum = hpix_ring_num(hpix, z[i]);
        printf("ring num at z=%15.8lf: %ld\n", z[i], ringnum);
    }

    double ra[]={0.,   40.,   80.,  120.,  160.,  200.,  240.,  280.,  320.,  360.};
    double dec[] = {-90., -85., -65., -45., -25., -5., 0., 5., 25., 45., 65., 85., 90.};
    printf("testing eq2pix\n");
    for (int ira=0; ira<nra; ira++) {
        for (int idec=0; idec<ndec; idec++) {
            int64 ipix = hpix_eq2pix(hpix, ra[ira], dec[idec]);
            printf("%15.8lf %15.8lf %ld\n", ra[ira], dec[idec], ipix);
        }
    }

    struct i64stack* listpix =i64stack_new(4*hpix->nside);
    double rad_arcmin=40.0/60.;
    double radius = rad_arcmin/60.*M_PI/180.;
    printf("intersect disc\n");
    printf("  listpix allocated size: %ld\n", listpix->allocated_size);
    printf("  radius (arcmin): %lf\n", rad_arcmin);

    const char fname[]="test-healpix.dat";
    FILE* fptr=fopen(fname,"w");
    for (int ira=0; ira<nra; ira++) {
        for (int idec=0; idec<ndec; idec++) {

            printf("ra: %10.6lf  dec: %10.6lf ", ra[ira], dec[idec]);

            hpix_disc_intersect(hpix, ra[ira], dec[idec], radius, listpix);

            printf("npix: %ld\n", listpix->size);

            fprintf(fptr,"%10.6lf %10.6lf %ld ", ra[ira], dec[idec], listpix->size);
            for (int i=0; i<listpix->size; i++) {
                fprintf(fptr,"%ld ", listpix->data[i]);
            }
            fprintf(fptr,"\n");
        }
    }

    printf("Closing file: %s\n", fname);
    fclose(fptr);

    i64stack_delete(listpix);
    hpix_delete(hpix);

}
