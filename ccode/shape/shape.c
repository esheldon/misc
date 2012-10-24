#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shape.h"

struct shape *shape_new_e1e2(double e1, double e2)
{
    struct shape *self=calloc(1,sizeof(struct shape));
    if (!self) {
        fprintf(stderr,"error: could not allocate struct "
                       "shape: %s: %d\n",
                       __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }

    shape_set_e1e2(self,e1,e2);
    return self;
}
struct shape *shape_new_g1g2(double g1, double g2)
{
    struct shape *self=calloc(1,sizeof(struct shape));
    if (!self) {
        fprintf(stderr,"error: could not allocate struct "
                       "shape: %s: %d\n",
                       __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }

    shape_set_g1g2(self,g1,g2);
    return self;
}
struct shape *shape_free(struct shape *self)
{
    free(self);
    return NULL;
}

void shape_show(struct shape *self, FILE *fptr)
{
    fprintf(fptr,"e1: %12.9f e2: %12.9f\n", self->e1, self->e2);
    fprintf(fptr,"g1: %12.9f g2: %12.9f\n", self->g1, self->g2);
}
void shape_write(struct shape *self, FILE *fptr)
{
    fprintf(fptr,"%.16g %.16g\n", self->e1, self->e2);
}


static double e2g(double e)
{
    return tanh(.5*atanh(e));
}
static double g2e(double g)
{
    return tanh(2.*atanh(g));
}

static void setbad(struct shape *self)
{
    self->e1=-9999;
    self->e2=-9999;
    self->g1=-9999;
    self->g2=-9999;
}
int shape_set_e1e2(struct shape *self, double e1, double e2)
{

    double etot=sqrt(e1*e1 + e2*e2);
    if (etot==0) {
        self->g1=0;
        self->g2=0;
        return 1;
    } 

    if (etot >= 1) {
        fprintf(stderr,"error: e must be < 1, found "
                "%.16g. %s: %d\n",etot,__FILE__,__LINE__);
        setbad(self);
        return 0;
    }

    double gtot=e2g(etot);
    if (gtot >= 1) {
        fprintf(stderr,"error: g must be < 1, found "
                "%.16g. %s: %d\n",gtot,__FILE__,__LINE__);
        setbad(self);
        return 0;
    }

    double fac=gtot/etot;

    self->e1=e1;
    self->e2=e2;
    self->g1=e1*fac;
    self->g2=e2*fac;

    return 1;
}
int shape_set_g1g2(struct shape *self, double g1, double g2)
{

    double gtot=sqrt(g1*g1 + g2*g2);
    if (gtot==0) {
        self->e1=0;
        self->e2=0;
        return 1;
    } 

    if (gtot >= 1) {
        fprintf(stderr,"error: g must be < 1, found "
                "%.16g. %s: %d\n",gtot,__FILE__,__LINE__);
        setbad(self);
        return 0;
    }

    double etot=g2e(gtot);
    if (etot >= 1) {
        fprintf(stderr,"error: g must be < 1, found "
                "%.16g. %s: %d\n",etot,__FILE__,__LINE__);
        setbad(self);
        return 0;
    }

    double fac=etot/gtot;

    self->g1=g1;
    self->g2=g2;
    self->e1=g1*fac;
    self->e2=g2*fac;

    return 1;
}


struct shape *shape_add(struct shape *self, struct shape *shear)
{
    struct shape *new=shape_new_e1e2(self->e1,self->e2);
    if (!shape_add_inplace(new, shear)) {
        new=shape_free(new);
        return NULL;
    }
    
    return new;
}
int shape_add_inplace(struct shape *self, struct shape *shear)
{

    if (shear->e1 == 0 && shear->e2 == 0) {
        return 1;
    }

    double oneplusedot = 1.0 + self->e1*shear->e1 + self->e2*shear->e2;

    if (oneplusedot == 0) {
        shape_set_e1e2(self, 0, 0);
        return 1;
    }

    double se1sq = shear->e1*shear->e1 + shear->e2*shear->e2;

    double fac = (1. - sqrt(1.-se1sq))/se1sq;

    double e1 = (self->e1 + shear->e1 + shear->e2*fac*(self->e2*shear->e1 - self->e1*shear->e2));
    double e2 = (self->e2 + shear->e2 + shear->e1*fac*(self->e1*shear->e2 - self->e2*shear->e1));

    e1 /= oneplusedot;
    e2 /= oneplusedot;

    if (!shape_set_e1e2(self,e1,e2)) {
        return 0;
    }
    return 1;
}
