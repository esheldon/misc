#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "shape.h"

// commenting until we put error checking in place
/*
struct shape *shape_new(void)
{
    struct shape *self=calloc(1,sizeof(struct shape));
    if (!self) {
        fprintf(stderr,"error: could not allocate struct "
                       "shape: %s: %d\n",
                       __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }
    return self;
}

struct shape *shape_new_e(double e1, double e2)
{
    struct shape *self=shape_new();
    shape_set_e(self,e1,e2);
    return self;
}
struct shape *shape_new_g(double g1, double g2)
{
    struct shape *self=shape_new();
    shape_set_g(self,g1,g2);
    return self;
}
struct shape *shape_free(struct shape *self)
{
    free(self);
    return NULL;
}
*/

void shape_show(const struct shape *self, FILE *fptr)
{
    fprintf(fptr,"e1:   %12.9f e2:   %12.9f\n", self->e1, self->e2);
    fprintf(fptr,"g1:   %12.9f g2:   %12.9f\n", self->g1, self->g2);
    fprintf(fptr,"eta1: %12.9f eta2: %12.9f\n", self->eta1, self->eta2);
}

void shape_write_e(const struct shape *self, FILE *fptr)
{
    fprintf(fptr,"%.16g %.16g\n", self->e1, self->e2);
}
void shape_write_g(const struct shape *self, FILE *fptr)
{
    fprintf(fptr,"%.16g %.16g\n", self->g1, self->g2);
}
void shape_write_eta(const struct shape *self, FILE *fptr)
{
    fprintf(fptr,"%.16g %.16g\n", self->eta1, self->eta2);
}


void shape_read_e(struct shape *self, FILE *fptr)
{
    double e1=0,e2=0;
    int nread=fscanf(fptr,"%lf %lf", &e1, &e2);
    if (nread != 2) {
        fprintf(stderr,"expected to read 2 doubles, read %d\n",
                nread);
        exit(EXIT_FAILURE);
    }
    shape_set_e(self, e1, e2);
}
void shape_read_g(struct shape *self, FILE *fptr)
{
    double g1=0,g2=0;
    int nread=fscanf(fptr,"%lf %lf\n", &g1, &g2);
    if (nread != 2) {
        fprintf(stderr,"expected to read 2 doubles, read %d\n",
                nread);
        exit(EXIT_FAILURE);
    }
    shape_set_g(self, g1, g2);
}
void shape_read_eta(struct shape *self, FILE *fptr)
{
    double eta1=0,eta2=0;
    int nread=fscanf(fptr,"%lf %lf\n", &eta1, &eta2);
    if (nread != 2) {
        fprintf(stderr,"expected to read 2 doubles, read %d\n",
                nread);
        exit(EXIT_FAILURE);
    }
    shape_set_eta(self, eta1, eta2);
}





static void setbad(struct shape *self)
{
    self->e1=-9999.e9;
    self->e2=-9999.e9;
    self->g1=-9999.e9;
    self->g2=-9999.e9;
    self->eta1=-9999.e9;
    self->eta2=-9999.e9;
}
int shape_set_e(struct shape *self, double e1, double e2)
{

    self->e1=e1;
    self->e2=e2;
    double e=sqrt(e1*e1 + e2*e2);

    if (e==0) {
        self->g1=0;
        self->g2=0;
        self->eta1=0;
        self->eta2=0;
        return 1;
    } 

    if (e >= 1) {
        setbad(self);
        return 0;
    }

    double eta = atanh(e);
    double g = tanh(0.5*eta);


    if (g >= 1) {
        setbad(self);
        return 0;
    }

    double cos2theta = e1/e;
    double sin2theta = e2/e;

    self->g1=g*cos2theta;
    self->g2=g*sin2theta;
    self->eta1=eta*cos2theta;
    self->eta2=eta*sin2theta;

    return 1;
}
int shape_set_g(struct shape *self, double g1, double g2)
{

    self->g1=g1;
    self->g2=g2;
    double g=sqrt(g1*g1 + g2*g2);
    if (g==0) {
        self->e1=0;
        self->e2=0;
        self->eta1=0;
        self->eta2=0;
        return 1;
    } 

    if (g >= 1) {
        setbad(self);
        return 0;
    }

    double eta = 2*atanh(g);
    double e = tanh(eta);

    if (e >= 1) {
        setbad(self);
        return 0;
    }

    double cos2theta = g1/g;
    double sin2theta = g2/g;

    self->e1=e*cos2theta;
    self->e2=e*sin2theta;
    self->eta1=eta*cos2theta;
    self->eta2=eta*sin2theta;

    return 1;
}

// eta is well behaved, so we should not see an error returned
int shape_set_eta(struct shape *self, double eta1, double eta2)
{
    
    self->eta1=eta1;
    self->eta2=eta2;

    double eta=sqrt(eta1*eta1 + eta2*eta2);

    if (eta==0) {
        self->e1=0;
        self->e2=0;
        self->g1=0;
        self->g2=0;
        return 1;
    }

    double e = tanh(eta);
    if (e >= 1.0) {
        setbad(self);
        return 0;
    }

    double g = tanh(0.5*eta);
    if (g >= 1.0) {
        setbad(self);
        return 0;
    }

    double cos2theta = eta1/eta;
    double sin2theta = eta2/eta;

    self->e1=e*cos2theta;
    self->e2=e*sin2theta;

    self->g1=g*cos2theta;
    self->g2=g*sin2theta;

    return 1;
}

double shape_get_theta(const struct shape *self)
{
    return 0.5*atan2( self->g2, self->g1 );
}

void shape_rotate(struct shape *self, double theta_radians)
{
    double twotheta = 2*theta_radians;

    double cos2angle = cos(twotheta);
    double sin2angle = sin(twotheta);
    double e1rot =  self->e1*cos2angle + self->e2*sin2angle;
    double e2rot = -self->e1*sin2angle + self->e2*cos2angle;

    shape_set_e(self, e1rot, e2rot);
}

/*
struct shape *shape_add(struct shape *self, const struct shape *shear)
{
    struct shape *new=shape_new_e(self->e1,self->e2);
    if (!shape_add_inplace(new, shear)) {
        new=shape_free(new);
        return NULL;
    }
    return new;
}
*/
int shape_add_inplace(struct shape *self, const struct shape *shear)
{

    if (shear->e1 == 0 && shear->e2 == 0) {
        return 1;
    }

    double oneplusedot_inv  = 1.0 + self->e1*shear->e1 + self->e2*shear->e2;

    if (oneplusedot_inv == 0) {
        shape_set_e(self, 0, 0);
        return 1;
    }
    oneplusedot_inv = 1.0/oneplusedot_inv;

    double se1sq = shear->e1*shear->e1 + shear->e2*shear->e2;

    double fac = (1. - sqrt(1.-se1sq))/se1sq;

    double e1 = (self->e1 + shear->e1 + shear->e2*fac*(self->e2*shear->e1 - self->e1*shear->e2));
    double e2 = (self->e2 + shear->e2 + shear->e1*fac*(self->e1*shear->e2 - self->e2*shear->e1));

    e1 *= oneplusedot_inv;
    e2 *= oneplusedot_inv;

    if (!shape_set_e(self,e1,e2)) {
        return 0;
    }
    return 1;
}

// jacobian of the transformation
//        |des/deo|_{shear}
// for pqr you will evaluate at -shear

double shape_detas_by_detao_jacob(const struct shape *shape, const struct shape *shear, 
                                  long *flags)
{
    double h=1.e-3;
    double h2inv = 1./(2*h);
    long res=0;

    struct shape shape_plus={0}, shape_minus={0};
    //struct shape shape_offset={0};

    // derivatives by eta1
    shape_plus = *shape;
    shape_minus = *shape;

    res += shape_set_eta(&shape_plus,  shape->eta1 + h, shape->eta2);
    res += shape_set_eta(&shape_minus, shape->eta1 - h, shape->eta2);

    res += shape_add_inplace(&shape_plus, shear);
    res += shape_add_inplace(&shape_minus, shear);

    double eta1s_by_eta1o = (shape_plus.eta1 - shape_minus.eta1)*h2inv;
    double eta2s_by_eta1o = (shape_plus.eta2 - shape_minus.eta2)*h2inv;

    // derivatives by eta2

    res += shape_set_eta(&shape_plus,  shape->eta1, shape->eta2 + h);
    res += shape_set_eta(&shape_minus, shape->eta1, shape->eta2 - h);

    res += shape_add_inplace(&shape_plus, shear);
    res += shape_add_inplace(&shape_minus, shear);

    double eta1s_by_eta2o = (shape_plus.eta1 - shape_minus.eta1)*h2inv;
    double eta2s_by_eta2o = (shape_plus.eta2 - shape_minus.eta2)*h2inv;

    double jacob = eta1s_by_eta1o*eta2s_by_eta2o - eta1s_by_eta2o*eta2s_by_eta1o;

    if (res != 8) {
        *flags |= SHAPE_RANGE_ERROR;
    }
    return jacob;

}

double shape_dgs_by_dgo_jacob(const struct shape *shape, const struct shape *shear)
{
    double ssq = shear->g1*shear->g1 + shear->g2*shear->g2;
    double num = (ssq - 1)*(ssq - 1);
    double denom=(1 
                  + 2*shape->g1*shear->g1
                  + 2*shape->g2*shear->g2
                  + shape->g1*shape->g1*ssq
                  + shape->g2*shape->g2*ssq);
    denom *= denom;

    return num/denom;
}

double shape_dgs_by_dgo_jacob_num(const struct shape *shape, const struct shape *shear, long *flags)
{
    double h=1.e-3;
    double h2inv = 1./(2*h);
    long res=0;

    struct shape shape_plus={0}, shape_minus={0};

    // derivatives by g1
    shape_plus = *shape;
    shape_minus = *shape;

    res += shape_set_g(&shape_plus,  shape->g1 + h, shape->g2);
    res += shape_set_g(&shape_minus, shape->g1 - h, shape->g2);

    res += shape_add_inplace(&shape_plus, shear);
    res += shape_add_inplace(&shape_minus, shear);

    double g1s_by_g1o = (shape_plus.g1 - shape_minus.g1)*h2inv;
    double g2s_by_g1o = (shape_plus.g2 - shape_minus.g2)*h2inv;

    res += shape_set_g(&shape_plus,  shape->g1, shape->g2 + h);
    res += shape_set_g(&shape_minus, shape->g1, shape->g2 - h);

    res += shape_add_inplace(&shape_plus, shear);
    res += shape_add_inplace(&shape_minus, shear);

    double g1s_by_g2o = (shape_plus.g1 - shape_minus.g1)*h2inv;
    double g2s_by_g2o = (shape_plus.g2 - shape_minus.g2)*h2inv;

    /*
    printf("g1s_by_g1o: %g\n", g1s_by_g1o);
    printf("g2s_by_g2o: %g\n", g2s_by_g2o);
    printf("g1s_by_g2o: %g\n", g1s_by_g2o);
    printf("g2s_by_g1o: %g\n", g2s_by_g1o);
    */

    double jacob = g1s_by_g1o*g2s_by_g2o - g1s_by_g2o*g2s_by_g1o;

    if (res != 8) {
        *flags |= SHAPE_RANGE_ERROR;
    }
    return jacob;

}
