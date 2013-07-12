#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "dist.h"
#include "VEC.h"

//#include "defs.h"

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



void dist_lognorm_fill(struct dist_lognorm *self, double mean, double sigma)
{
    self->mean=mean;
    self->sigma=sigma;

    self->logmean = log(mean) - 0.5*log( 1 + sigma*sigma/(mean*mean) );
    self->logivar = 1./(  log(1 + sigma*sigma/(mean*mean) ) );
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

double dist_g_ba_lnprob(const struct dist_g_ba *self, double g1, double g2)
{
    double lnp=0, gsq=0, tmp=0;

    gsq = g1*g1 + g2*g2;

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
double dist_g_ba_prob(const struct dist_g_ba *self, double g1, double g2)
{
    double prob=0, gsq=0, chi2=0, tmp=0;

    gsq = g1*g1 + g2*g2;

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


void dist_g_ba_print(const struct dist_g_ba *self, FILE *stream)
{
    fprintf(stream,"g dist BA13\n");
    fprintf(stream,"    sigma: %g\n", self->sigma);
}





void dist_gmix3_eta_fill(struct dist_gmix3_eta *self,
                         double ivar1, double p1,
                         double ivar2, double p2,
                         double ivar3, double p3)
{
    self->gauss1_ivar=ivar1;
    self->gauss1_pnorm = p1*ivar1/(2*M_PI);

    self->gauss2_ivar=ivar2;
    self->gauss2_pnorm = p2*ivar2/(2*M_PI);

    self->gauss3_ivar=ivar3;
    self->gauss3_pnorm = p3*ivar3/(2*M_PI);
}


struct dist_gmix3_eta *dist_gmix3_eta_new(double ivar1, double p1,
                                          double ivar2, double p2,
                                          double ivar3, double p3)
{
    struct dist_gmix3_eta *self=calloc(1, sizeof(struct dist_gmix3_eta));
    if (!self) {
        fprintf(stderr,"Could not allocate struct dist_gmix3_eta\n");
        exit(1);
    }

    dist_gmix3_eta_fill(self, ivar1, p1, ivar2, p2, ivar3, p3);
    return self;
}

double dist_gmix3_eta_lnprob(const struct dist_gmix3_eta *self, double eta1, double eta2)
{
    double lnp=DIST_LOG_LOWVAL, p=0;
    p = dist_gmix3_eta_prob(self, eta1, eta2);
    if (p > 0) {
        lnp = log(p);
    } else {
        fprintf(stderr,"dist_gmix3_eta_prob is <= 0: %g\n", p);
    }
    return lnp;

}

double dist_gmix3_eta_prob(const struct dist_gmix3_eta *self, double eta1, double eta2)
{
    double prob=0, eta_sq=0;

    eta_sq = eta1*eta1 + eta2*eta2;
    prob += self->gauss1_pnorm*exp(-0.5*self->gauss1_ivar*eta_sq );
    prob += self->gauss2_pnorm*exp(-0.5*self->gauss2_ivar*eta_sq );
    prob += self->gauss3_pnorm*exp(-0.5*self->gauss3_ivar*eta_sq );
 
    return prob;
}


void dist_gmix3_eta_print(const struct dist_gmix3_eta *self, FILE *stream)
{
    fprintf(stream,"eta gmix3 dist\n");
    fprintf(stream,"    ivar1:    %g\n", self->gauss1_ivar);
    fprintf(stream,"    p1*norm1: %g\n", self->gauss1_pnorm);
    fprintf(stream,"    ivar2:    %g\n", self->gauss2_ivar);
    fprintf(stream,"    p2*norm2: %g\n", self->gauss2_pnorm);
    fprintf(stream,"    ivar3:    %g\n", self->gauss3_ivar);
    fprintf(stream,"    p3*norm3: %g\n", self->gauss3_pnorm);
}


