#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "randn.h"
#include "dist.h"

enum dist dist_string2dist(const char *dist_name, long *flags)
{
    enum dist type=0;
    if (0==strcmp(dist_name,"DIST_GAUSS")) {
        type=DIST_GAUSS;
    } else if (0==strcmp(dist_name,"DIST_GMIX3_ETA")) {
        type=DIST_GMIX3_ETA;
    } else if (0==strcmp(dist_name,"DIST_LOGNORMAL")) {
        type=DIST_LOGNORMAL;
    } else if (0==strcmp(dist_name,"DIST_G_BA")) {
        type=DIST_G_BA;
    } else {
        *flags |= DIST_BAD_DIST;
    }
    return type;
}

long dist_get_npars(enum dist dist_type, long *flags)
{
    long npars=-1;
    switch (dist_type) {
        case DIST_GAUSS:
            npars=2;
            break;
        case DIST_LOGNORMAL:
            npars=2;
            break;
        case DIST_G_BA:
            npars=1;
            break;
        case DIST_GMIX3_ETA:
            npars=6;
            break;

        default: 
            fprintf(stderr,"Bad 1d dist type %u: %s: %d\n",
                    dist_type, __FILE__,__LINE__);
            *flags |= DIST_BAD_DIST;
            break;
    }
    return npars;
}
/*
struct dist1d *dist1d_new(enum dist dist_type, VEC(double) pars, long *flags)
{
    size_t nbytes=0;

    struct dist1d *self=calloc(1, sizeof(struct dist1d));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_gmix3_eta: %s: %d\n",
                       __FILE__,__LINE__);
        exit(1);
    }

    self->dist_type=dist_type;

    switch (dist_type) {
        case DIST_GAUSS:
            if (VEC_SIZE(pars) != 2) {
                *flags |= DIST_WRONG_NPARS;
            }
            self->data = dist_gauss_new(VEC_GET(pars,0), VEC_GET(pars,1));
            break;
        case DIST_LOGNORMAL:
            if (VEC_SIZE(pars) != 2) {
                *flags |= DIST_WRONG_NPARS;
            }
            self->data = dist_lognorm_new(VEC_GET(pars,0), VEC_GET(pars,1));
            break;
        default: 
            fprintf(stderr,"Bad 1d dist type %u: %s: %d\n",
                    dist_type, __FILE__,__LINE__);
            *flags |= DIST_BAD_DIST;
            break;
    }

_dist1d_new_bail:
    if (*flags != 0) {
        fprintf(stderr,"Bad number of pars %lu for dist type %u: %s: %d\n",
                VEC_SIZE(pars), dist_type, __FILE__,__LINE__);
        self=dist1d_free(self);
    }
    return self;
}

struct dist2d *dist2d_new(enum dist dist_type, VEC(double) pars, long *flags)
{
    size_t nbytes=0;

    struct dist2d *self=calloc(1, sizeof(struct dist2d));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_gmix3_eta: %s: %d\n",
                       __FILE__,__LINE__);
        exit(1);
    }

    self->dist_type=dist_type;

    switch (dist_type) {
        case DIST_G_BA:
            if (VEC_SIZE(pars) != 1) {
                *flags |= DIST_WRONG_NPARS;
            }
            self->data = dist_g_ba_new(VEC_GET(pars,0));
            break;
        case DIST_GMIX3_ETA:
            if (VEC_SIZE(pars) != 6) {
                *flags |= DIST_WRONG_NPARS;
            }
            self->data = dist_gmix3_eta_new(VEC_GET(pars,0), // ivar1
                                            VEC_GET(pars,1), // p1
                                            VEC_GET(pars,2), // ivar2
                                            VEC_GET(pars,3), // p2
                                            VEC_GET(pars,4), // ivar3
                                            VEC_GET(pars,5)); // p3
            break;
        default: 
            fprintf(stderr,"Bad 2d dist type %u: %s: %d\n",
                    dist_type, __FILE__,__LINE__);
            *flags |= DIST_BAD_DIST;
            break;
    }

_dist2d_new_bail:
    if (*flags != 0) {
        fprintf(stderr,"Bad number of pars %lu for dist type %u: %s: %d\n",
                VEC_SIZE(pars), dist_type, __FILE__,__LINE__);
        self=dist2d_free(self);
    }
    return self;
}



struct dist1d *dist1d_free(struct dist1d *self)
{
    if (self) {
        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}
struct dist2d *dist2d_free(struct dist2d *self)
{
    if (self) {
        free(self->data);
        free(self);
        self=NULL;
    }
    return self;
}


double dist1d_lnprob(struct dist1d *self, double x)
{

}

*/



void dist_gauss_fill(struct dist_gauss *self, double mean, double sigma)
{
    self->mean=mean;
    self->sigma=sigma;
    self->ivar=1./(sigma*sigma);
}
struct dist_gauss *dist_gauss_new(double mean, double sigma)
{
    struct dist_gauss *self=calloc(1, sizeof(struct dist_gauss));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_gauss\n");
        exit(1);
    }

    dist_gauss_fill(self, mean, sigma);
    return self;
}
double dist_gauss_lnprob(const struct dist_gauss *self, double x)
{
    double lnp=0.0;

    // -0.5*self->ivar*(x-self->mean)**2
    lnp = x;
    lnp -= self->mean;
    lnp *= lnp;
    lnp *= self->ivar;
    lnp *= (-0.5);
    return lnp;
}
void dist_gauss_print(const struct dist_gauss *self, FILE *stream)
{
    fprintf(stream,"guass dist\n");
    fprintf(stream,"    mean: %g\n", self->mean);
    fprintf(stream,"    sigma: %g\n", self->sigma);
}


double dist_gauss_sample(const struct dist_gauss *self)
{
    double z = randn();
    return self->mean + self->sigma*z;
}





void dist_lognorm_fill(struct dist_lognorm *self, double mean, double sigma)
{
    self->mean=mean;
    self->sigma=sigma;

    self->logmean = log(mean) - 0.5*log( 1 + sigma*sigma/(mean*mean) );
    double logvar = log(1 + sigma*sigma/(mean*mean) );
    self->logsigma = sqrt(logvar);

    self->logivar = 1./logvar;
}

struct dist_lognorm *dist_lognorm_new(double mean, double sigma)
{
    struct dist_lognorm *self=calloc(1, sizeof(struct dist_lognorm));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_lognorm\n");
        exit(1);
    }

    dist_lognorm_fill(self, mean, sigma);
    return self;
}
double dist_lognorm_lnprob(const struct dist_lognorm *self, double x)
{
    double lnp=0.0, logx=0;

    //chi2 = self.logivar*(logx-self.logmean)**2;
    //lnprob = - 0.5*chi2 - logx;

    if (x < DIST_LOG_MINARG) {
        lnp = DIST_LOG_LOWVAL;
    } else {
        logx=log(x);
        lnp = logx;

        lnp -= self->logmean;
        lnp *= lnp;
        lnp *= self->logivar;

        lnp *= (-0.5);
        lnp -= logx;
    }
    return lnp;
}
void dist_lognorm_print(const struct dist_lognorm *self, FILE *stream)
{
    fprintf(stream,"lognorm dist\n");
    fprintf(stream,"    mean: %g\n", self->mean);
    fprintf(stream,"    sigma: %g\n", self->sigma);
}


double dist_lognorm_sample(const struct dist_lognorm *self)
{
    double z = randn();
    return exp(self->logmean + self->logsigma*z);
}



void dist_g_ba_fill(struct dist_g_ba *self, double sigma)
{
    self->sigma=sigma;
    self->ivar=1./(sigma*sigma);
}


struct dist_g_ba *dist_g_ba_new(double sigma)
{
    struct dist_g_ba *self=calloc(1, sizeof(struct dist_g_ba));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_g_ba\n");
        exit(1);
    }

    dist_g_ba_fill(self, sigma);
    return self;
}

double dist_g_ba_lnprob(const struct dist_g_ba *self, const struct shape *shape)
{
    double lnp=0, gsq=0, tmp=0;

    gsq = shape->g1*shape->g1 + shape->g2*shape->g2;

    tmp = 1-gsq;
    if ( tmp < DIST_LOG_MINARG ) {
        lnp = DIST_LOG_LOWVAL;
    } else {

        //p= (1-g**2)**2*exp(-0.5 * g**2 * ivar)
        // log(p) = 2*log(1-g^2) - 0.5*g^2 * ivar

        // should do a fast math version; I suspect this
        // will be a bottleneck
        lnp = log(tmp);

        lnp *= 2;
        
        tmp = 0.5;
        tmp *= gsq;
        tmp *= self->ivar;
        lnp -= tmp;
    }
    return lnp;

}

// p= (1-g**2)**2*exp(-0.5 * g**2 * ivar)
double dist_g_ba_prob(const struct dist_g_ba *self, const struct shape *shape)
{
    double prob=0, gsq=0, chi2=0, tmp=0;

    gsq = shape->g1*shape->g1 + shape->g2*shape->g2;

    tmp = 1-gsq;
    if (tmp > 0) {
        tmp *= tmp;

        chi2 = gsq;
        chi2 *= self->ivar;

        prob = exp(-0.5*chi2);

        prob *= tmp;
    }
    return prob;

}

double dist_g_ba_pj(const struct dist_g_ba *self,
                    const struct shape *shape,
                    const struct shape *shear)
{
    struct shape mshear={0}, newshape={0};
    shape_set_g(&mshear, -shear->g1, -shear->g2);

    double j = shape_dgs_by_dgo_jacob(shape, &mshear);

    newshape = *shape;
    shape_add_inplace(&newshape, &mshear);

    double p = dist_g_ba_prob(self, &newshape);

    return p*j;
}


void dist_g_ba_print(const struct dist_g_ba *self, FILE *stream)
{
    fprintf(stream,"g dist BA13\n");
    fprintf(stream,"    sigma: %g\n", self->sigma);
}


/*
static int dist_gmix3_eta_compare(const void *d1, const void *d2)
{
    const struct dist_gmix3_eta_data *s1=d1;
    const struct dist_gmix3_eta_data *s2=d2;

    if (s1->p < s2->p) {
        return -1;
    } else if (s1->p > s2->p) {
        return 1;
    } else {
        return 0;
    }
}
*/


void dist_gmix3_eta_fill(struct dist_gmix3_eta *self,
                         double sigma1, double sigma2, double sigma3,
                         double p1, double p2, double p3)
{
    double ivar1=1.0/(sigma1*sigma1);
    double ivar2=1.0/(sigma2*sigma2);
    double ivar3=1.0/(sigma3*sigma3);

    self->size=3;
    self->data[0].sigma = sigma1;
    self->data[1].sigma = sigma2;
    self->data[2].sigma = sigma3;

    self->data[0].p = p1;
    self->data[1].p = p2;
    self->data[2].p = p3;

    self->data[0].ivar = ivar1;
    self->data[1].ivar = ivar2;
    self->data[2].ivar = ivar3;

    self->data[0].pnorm = p1*ivar1/(2*M_PI);
    self->data[1].pnorm = p2*ivar2/(2*M_PI);
    self->data[2].pnorm = p3*ivar3/(2*M_PI);

    //qsort(self->data, self->size, sizeof(struct dist_gmix3_eta_data), dist_gmix3_eta_compare);

    double psum=0.0;
    for (long i=0; i<self->size; i++) {
        psum += self->data[i].p;
    }

    for (long i=0; i<self->size; i++) {
        self->p_normalized[i] = self->data[i].p/psum;
    }

}


struct dist_gmix3_eta *dist_gmix3_eta_new(double sigma1,
                                          double sigma2,
                                          double sigma3,
                                          double p1,
                                          double p2,
                                          double p3)

{
    struct dist_gmix3_eta *self=calloc(1, sizeof(struct dist_gmix3_eta));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_gmix3_eta\n");
        exit(1);
    }

    dist_gmix3_eta_fill(self, sigma1, sigma2, sigma3, p1, p2, p3);
    return self;
}

double dist_gmix3_eta_lnprob(const struct dist_gmix3_eta *self, const struct shape *shape)
{
    double lnp=DIST_LOG_LOWVAL, p=0;
    p = dist_gmix3_eta_prob(self, shape);
    if (p > 0) {
        lnp = log(p);
    } else {
        fprintf(stderr,"dist_gmix3_eta_prob is <= 0: %g\n", p);
    }
    return lnp;

}

double dist_gmix3_eta_prob(const struct dist_gmix3_eta *self, const struct shape *shape)
{
    double prob=0, eta_sq=0;

    eta_sq = shape->eta1*shape->eta1 + shape->eta2*shape->eta2;
    prob += self->data[0].pnorm*exp(-0.5*self->data[0].ivar*eta_sq );
    prob += self->data[1].pnorm*exp(-0.5*self->data[1].ivar*eta_sq );
    prob += self->data[2].pnorm*exp(-0.5*self->data[2].ivar*eta_sq );
 
    return prob;
}


// draw a value between 0 and 1 and use it to choose a gaussian
// from which to draw shapes
// data must be sorted by p
void dist_gmix3_eta_sample(const struct dist_gmix3_eta *self, struct shape *shape)
{
    double p = drand48();
    long found=0, i=0;

    double pcum=0;
    for (i=0; i<self->size; i++) {
        pcum += self->p_normalized[i];
        if (p <= pcum) {
            found=1;
            break;
        }
    }

    if (!found) {
        i=self->size-1;
    }

    // The dimensions are independent
    double eta1 = self->data[i].sigma*randn();
    double eta2 = self->data[i].sigma*randn();

    shape_set_eta(shape, eta1, eta2);
}

double dist_gmix3_eta_pj(const struct dist_gmix3_eta *self,
                         const struct shape *shape,
                         const struct shape *shear)
{
    struct shape mshear={0}, newshape={0};
    shape_set_g(&mshear, -shear->g1, -shear->g2);

    double j = shape_detas_by_detao_jacob(shape, &mshear);

    newshape = *shape;
    shape_add_inplace(&newshape, &mshear);

    double p = dist_gmix3_eta_prob(self, &newshape);

    return p*j;
}

void dist_gmix3_eta_print(const struct dist_gmix3_eta *self, FILE *stream)
{
    fprintf(stream,"eta gmix dist\n");
    for (long i=0; i<self->size; i++) {
        fprintf(stream,"    sigma%ld:   %g\n", i+1, self->data[i].sigma);
        fprintf(stream,"    p%ld:       %g\n", i+1, self->data[i].p);
    }
}



