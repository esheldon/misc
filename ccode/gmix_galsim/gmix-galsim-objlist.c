#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc < 5) {
        fprintf(stderr,
                "gmix-galsim-objlist nrow ncol nobj_row nobj_col\n"
                "  nrow,ncol are the image dimensions\n"
                "  nobj_row, nobj_col are the number of objects \n"
                "    in each dimension\n"
                "  output to stdout is \n"
                "    objrow objcol rowcen colcen rowmin rowmax colmin colmax\n"
                "  the center is just a rough guess\n");
        exit(EXIT_FAILURE);
    }

    int nrows=atoi(argv[1]);
    int ncols=atoi(argv[2]);
    int nobj_row=atoi(argv[3]);
    int nobj_col=atoi(argv[4]);

    int nrows_per=nrows/nobj_row;
    int ncols_per=ncols/nobj_col;

    for (int orow=0; orow<nobj_row; orow++) {

        int rowmin=orow*nrows_per;
        int rowmax=(orow+1)*nrows_per-1;

        double rowcen=(rowmax+rowmin)/2.;

        for (int ocol=0; ocol<nobj_col; ocol++) {

            int colmin=ocol*ncols_per;
            int colmax=(ocol+1)*ncols_per-1;
            double colcen=(colmax+colmin)/2.;

            printf("%d %d %lf %lf %d %d %d %d\n",
                   orow,ocol,rowcen,colcen,rowmin,rowmax,colmin,colmax);

        }
    }
}
