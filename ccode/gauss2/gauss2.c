#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gauss2.h"

// use this to keep the structure internally consistent
void gauss2_set(struct gauss2 *self,
                double p,
                double row,
                double col,
                double irr,
                double irc,
                double icc,
                long *flags)
{

    self->det = irr*icc - irc*irc;

    if (self->det <= 0) {
        *flags |= GAUSS2_ERROR_NEGATIVE_DET; 
    }

    self->p   = p;
    self->row = row;
    self->col = col;
    self->irr = irr;
    self->irc = irc;
    self->icc = icc;

    self->drr = self->irr/self->det;
    self->drc = self->irc/self->det;
    self->dcc = self->icc/self->det;
    self->norm = 1./(M_TWO_PI*sqrt(self->det));

    self->pnorm = p*self->norm;
}

void gauss2_print(const struct gauss2 *self, FILE *stream)
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


double gauss2_lnprob0(const struct gauss2 *self, double row, double col)
{
    double u=row-self->row;
    double v=col-self->col;
    double chi2=self->dcc*u*u + self->drr*v*v - 2.0*self->drc*u*v;

    return -.5*chi2;
}
