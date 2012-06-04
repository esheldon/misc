#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gvec.h"
#include "defs.h"
#include "matrix.h"

struct gvec* gvec_new(size_t ngauss)
{
    struct gvec*self=NULL;
    if (ngauss == 0) {
        wlog("number of gaussians must be > 0\n");
        return NULL;
    }

    self = calloc(1, sizeof(struct gvec));
    if (self==NULL) {
        wlog("could not allocate struct gvec\n");
        return NULL;
    }

    self->size=ngauss;

    self->data = calloc(self->size, sizeof(struct gauss));
    if (self->data==NULL) {
        wlog("could not allocate %lu gaussian structs\n",ngauss);
        free(self);
        return NULL;
    }

    return self;

}

struct gvec *gvec_free(struct gvec *self)
{
    if (self) {
        free(self->data);
        self->data=NULL;
        free(self);
        self=NULL;
    }
    return self;
}

void gauss_set(struct gauss* self, 
               double p, 
               double row, 
               double col,
               double irr,
               double irc,
               double icc)
{
    self->p   = p;
    self->row = row;
    self->col = col;
    self->irr = irr;
    self->irc = irc;
    self->icc = icc;
    self->det = irr*icc - irc*irc;
}
void gvec_set_dets(struct gvec *self)
{
    struct gauss *gptr = self->data;
    for (size_t i=0; i<self->size; i++) {
        gptr->det = gptr->irr*gptr->icc - gptr->irc*gptr->irc;
        gptr++;
    }
}
int gvec_copy(struct gvec *self, struct gvec* dest)
{
    if (dest->size != self->size) {
        wlog("gvec are not same size\n");
        return 0;
    }
    memcpy(dest->data, self->data, self->size*sizeof(struct gauss));
    return 1;
}

void gvec_print(struct gvec *self, FILE* fptr)
{
    struct gauss *gptr = self->data;
    for (size_t i=0; i<self->size; i++) {
        fprintf(fptr,
             "%lu p: %9.6lf row: %9.6lf col: %9.6lf " 
             "irr: %9.6lf irc: %9.6lf icc: %9.6lf\n",
             i, gptr->p, gptr->row, gptr->col,
             gptr->irr,gptr->irc, gptr->icc);
        gptr++;
    }
}

double gvec_wmomsum(struct gvec* gvec)
{
    double wmom=0;
    struct gauss* gauss=gvec->data;
    for (size_t i=0; i<gvec->size; i++) {
        wmom += gauss->p*(gauss->irr + gauss->icc);
        gauss++;
    }
    return wmom;
}

/*
 
 Find the weighted average center and set all centers
 equal to this value.

 for j gaussians

     munew = sum_j( C_j^-1 p_j mu_j )/sum( C_j^-1 p_j )

 where the mus are mean vectors and the C are the covarance
 matrices.

 The following would be a lot simple if we use vec2 and mtx2
 types in the gaussian!  Maybe some other day.

 */
int gvec_wmean_center(const struct gvec* gvec, struct vec2* mu_new)
{
    int status=1;
    struct vec2 mu_Cinvp, mu_Cinvpsum;
    struct mtx2 Cinvpsum, Cinvpsum_inv, C, Cinvp;

    memset(&Cinvpsum,0,sizeof(struct mtx2));
    memset(&mu_Cinvpsum,0,sizeof(struct vec2));

    const struct gauss* gauss = gvec->data;
    for (size_t i=0; i<gvec->size; i++) {
        // p*C^-1
        mtx2_set(&C, gauss->irr, gauss->irc, gauss->icc);
        if (!mtx2_invert(&C, &Cinvp)) {
            wlog("gvec_fix_centers: zero determinant found in C");
            status=0;
            goto _gvec_wmean_center_bail;
        }
        mtx2_sprodi(&Cinvp, gauss->p);

        // running sum of p*C^-1
        mtx2_sumi(&Cinvpsum, &Cinvp);

        // set the center as a vec2
        vec2_set(&mu_Cinvp, gauss->row, gauss->col);
        // p*C^-1 * mu in place on mu
        mtx2_vec2prodi(&Cinvp, &mu_Cinvp);

        // running sum of p*C^-1 * mu
        vec2_sumi(&mu_Cinvpsum, &mu_Cinvp);
        gauss++;
    }

    if (!mtx2_invert(&Cinvpsum, &Cinvpsum_inv)) {
        wlog("gvec_fix_centers: zero determinant found in Cinvpsum");
        status=0;
        goto _gvec_wmean_center_bail;
    }

    mtx2_vec2prod(&Cinvpsum_inv, &mu_Cinvpsum, mu_new);

_gvec_wmean_center_bail:
    return status;
}
