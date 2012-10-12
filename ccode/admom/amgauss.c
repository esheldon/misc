#include <stdio.h>
#include <stdlib.h>
#include "amgauss.h"

// use this to keep the structure internally consistent
int amgauss_set(struct amgauss *self,
                double row,
                double col,
                double irr,
                double irc,
                double icc)
{
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
}


