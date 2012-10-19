#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gauss.h"

// use this to keep the structure internally consistent
int gauss_set(struct gauss *self,
              double p,
              double row,
              double col,
              double irr,
              double irc,
              double icc)
{
    int retval=1;

    self->p=p;
    self->row=row;
    self->col=col;

    self->irr=irr;
    self->irc=irc;
    self->icc=icc;

    self->det = irr*icc - irc*irc;

    self->drr = irr/self->det;
    self->drc = irc/self->det;
    self->dcc = icc/self->det;

    self->e1 = (icc-irr)/(icc+irr);
    self->e2 = 2.*irc/(icc+irr);

    if (self->det <= 0) {
        retval=0;
    } else {
        self->norm = 1./(M_TWO_PI*sqrt(self->det));
    }
    return retval;
}

void gauss_print(const struct gauss *self, FILE *stream)
{
    fprintf(stream,"  p:   %.16g\n", self->p);
    fprintf(stream,"  row: %.16g\n", self->row);
    fprintf(stream,"  col: %.16g\n", self->col);
    fprintf(stream,"  irr: %.16g\n", self->irr);
    fprintf(stream,"  irc: %.16g\n", self->irc);
    fprintf(stream,"  icc: %.16g\n", self->icc);
    fprintf(stream,"  e1:  %.16g\n", self->e1);
    fprintf(stream,"  e2:  %.16g\n", self->e2);
}


double gauss_lnprob0(const struct gauss *self, double row, double col)
{
    double u=row-self->row;
    double v=col-self->col;
    double chi2=self->dcc*u*u + self->drr*v*v - 2.0*self->drc*u*v;

    return -.5*chi2;
}
