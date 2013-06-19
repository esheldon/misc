#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
int get_npars(const char *fit_type, int ngauss)
{
    if (0==strcmp(fit_type,"coellip")) {
        npars=(ngauss-4)/2;
    } else {
        fprintf(stderr,"only coellip for now\n");
        exit(EXIT_FAILURE);
    }
    return npars;
}
*/

int main(int argc, char **argv)
{

    int human=0;
    if (argc > 1) {
        human=1;
    }
    char fit_type[32]={0};
    char fit_type_psf[32]={0};

    if (2 != fscanf(stdin,"%s %s", fit_type, fit_type_psf)) {
        fprintf(stderr,"Could not read fit types\n");
        exit(EXIT_FAILURE);
    }

    size_t n=0;
    double e1=0, e1sum=0;
    double e2=0, e2sum=0;
    double e1_ivar_sum=0, e2_ivar_sum=0;
    double esq=0, esqsum=0;
    double arate=0, aratesum=0;
    double arate_psf=0;//, aratesum_psf=0;
    double R=0;

    int obj_row=0, obj_col=0;

    int npars=0, npars_psf=0; 

    double tmp=0;
    int nread=0, nskip=0;
    while (1) {
        if (2 != fscanf(stdin,"%d %d", &obj_row, &obj_col)) {
            break;
        }
        n+=1;

        nread = fscanf(stdin,"%d", &npars);
        nread +=fscanf(stdin,"%lf", &arate);
        aratesum+=arate;

        for (size_t i=0; i<npars; i++) {
            nread += fscanf(stdin,"%lf", &tmp);
            if (i==2) {
                e1sum+=tmp;
                esqsum += tmp*tmp;
            } else if (i==3) {
                e2sum+=tmp;
                esqsum += tmp*tmp;
            }
        }

        for (size_t i=0; i<npars;i++) {
            for (size_t j=0; j<npars;j++) {
                nread+=fscanf(stdin,"%lf",&tmp);
                if (i==2 && j==2) {
                    e1_ivar_sum += 1/tmp;
                } else if (i==3 && j==3) {
                    e2_ivar_sum += 1/tmp;
                }
            }
        }

        if (nread != npars*npars + npars + 2) {
            fprintf(stderr,"Could not read entire stats row\n");
            exit(EXIT_FAILURE);
        }

        // skip all the psf
        nread = fscanf(stdin,"%d", &npars_psf);
        nread +=fscanf(stdin,"%lf", &arate_psf);

        nskip=npars_psf + npars_psf*npars_psf;
        for (size_t i=0; i<nskip; i++) {
            nread += fscanf(stdin,"%lf", &tmp);
        }

        if (nread != npars_psf*npars_psf + npars_psf + 2) {
            fprintf(stderr,"Could not read entire psf stats\n");
            exit(EXIT_FAILURE);
        }

    }

    arate = aratesum/n;
    e1 = e1sum/n;
    e2 = e2sum/n;
    esq = esqsum/n;

    R = 1-.5*esq;

    double g1 = .5*e1/R;
    double g2 = .5*e2/R;
    double g1err = .5*sqrt(1./e1_ivar_sum)/R;
    double g2err = .5*sqrt(1./e2_ivar_sum)/R;

    if (human) {
        printf("nobj:  %lu\n", n);
        printf("arate: %.16g\n", arate);
        printf("R:     %.16g\n", R);
        printf("g1:    %.16g +/- %.16g\n", g1,g1err);
        printf("g2:    %.16g +/- %.16g\n", g2,g2err);
    } else {
        printf("%lu %.16g %.16g %.16g %.16g %.16g %.16g\n",
               n, arate, R, g1, g1err, g2, g2err);
    }
}

