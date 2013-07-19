#include <stdlib.h>
#include <stdio.h>
#include "jacobian.h"

struct jacobian *jacobian_new(double row0,
                              double col0,
                              double dudrow,
                              double dudcol,
                              double dvdrow,
                              double dvdcol)
{
    struct jacobian *self=NULL;

    self=calloc(1, sizeof(struct jacobian));
    if (!self) {
        fprintf(stderr,"could not allocate struct jacobian\n");
        exit(1);
    }

    jacobian_set(self,
                 row0,
                 col0,
                 dudrow,
                 dudcol,
                 dvdrow,
                 dvdcol);
    return self;
}


void jacobian_set(struct jacobian *self, 
                  double row0,
                  double col0,
                  double dudrow,
                  double dudcol,
                  double dvdrow,
                  double dvdcol)
{
    self->row0=row0;
    self->col0=col0;
    self->dudrow=dudrow;
    self->dudcol=dudcol;
    self->dvdrow=dvdrow;
    self->dvdcol=dvdcol;
}

void jacobian_set_identity(struct jacobian *self)
{
    self->row0=0;
    self->col0=0;
    self->dudrow=1;
    self->dudcol=0;
    self->dvdrow=0;
    self->dvdcol=1;
}

void jacobian_set_cen(struct jacobian *self, double row, double col)
{
    self->row0=row;
    self->col0=col;
}



void jacobian_print(const struct jacobian *self, FILE *stream)
{
    fprintf(stream,"jacobian\n");
    fprintf(stream,"    row0: %g\n", self->row0);
    fprintf(stream,"    col0: %g\n", self->col0);
    fprintf(stream,"    dudrow: %g\n", self->dudrow);
    fprintf(stream,"    dudcol: %g\n", self->dudcol);
    fprintf(stream,"    dvdrow: %g\n", self->dvdrow);
    fprintf(stream,"    dvdcol: %g\n", self->dvdcol);
}
