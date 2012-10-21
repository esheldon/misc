#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gmix.h"

struct gmix* gmix_new(size_t ngauss)
{
    struct gmix*self=NULL;
    if (ngauss == 0) {
        fprintf(stderr,"number of gaussians must be > 0\n");
        return NULL;
    }

    self = calloc(1, sizeof(struct gmix));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct gmix\n");
        exit(EXIT_FAILURE);
    }

    self->size=ngauss;

    self->data = calloc(self->size, sizeof(struct gauss));
    if (self->data==NULL) {
        fprintf(stderr,"could not allocate %lu gaussian structs\n",ngauss);
        free(self);
        exit(EXIT_FAILURE);
    }

    return self;

}

struct gmix *gmix_free(struct gmix *self)
{
    if (self) {
        free(self->data);
        self->data=NULL;
        free(self);
        self=NULL;
    }
    return self;
}

void gmix_set_dets(struct gmix *self)
{
    struct gauss *gptr = self->data;
    size_t i=0;
    for (i=0; i<self->size; i++) {
        gptr->det = gptr->irr*gptr->icc - gptr->irc*gptr->irc;
        gptr++;
    }
}

int gmix_verify(const struct gmix *self)
{
    size_t i=0;
    struct gauss *gauss=NULL;

    if (!self) {
        fprintf(stderr,"gmix is not initialized\n");
        return 0;
    }

    gauss=self->data;
    for (i=0; i<self->size; i++) {
        if (gauss->det <= 0) {
            //fprintf(stderr,"found det: %.16g\n", gauss->det);
            return 0;
        }
        gauss++;
    }
    return 1;
}

struct gmix *gmix_new_copy(struct gmix *self)
{
    struct gmix *dest=gmix_new(self->size);
    gmix_copy(self, dest);
    return dest;
}


int gmix_copy(struct gmix *self, struct gmix* dest)
{
    if (dest->size != self->size) {
        fprintf(stderr,"gmix are not same size\n");
        return 0;
    }
    memcpy(dest->data, self->data, self->size*sizeof(struct gauss));
    return 1;
}

void gmix_print(struct gmix *self, FILE* fptr)
{
    struct gauss *gptr = self->data;
    size_t i=0;
    for (i=0; i<self->size; i++) {
        fprintf(fptr,
             "%lu p: %9.6lf row: %9.6lf col: %9.6lf " 
             "irr: %9.6lf irc: %9.6lf icc: %9.6lf\n",
             i, gptr->p, gptr->row, gptr->col,
             gptr->irr,gptr->irc, gptr->icc);
        gptr++;
    }
}

double gmix_wmomsum(struct gmix* gmix)
{
    double wmom=0;
    struct gauss* gauss=gmix->data;
    size_t i=0;
    for (i=0; i<gmix->size; i++) {
        wmom += gauss->p*(gauss->irr + gauss->icc);
        gauss++;
    }
    return wmom;
}

void gmix_set_total_moms(struct gmix *self)
{
    size_t i=0;
    double p=0, psum=0;
    struct gauss *gauss=NULL;

    self->total_irr=0;
    self->total_irc=0;
    self->total_icc=0;

    gauss = self->data;
    for (i=0; i<self->size; i++) {
        p = gauss->p;
        psum += p;

        self->total_irr += p*gauss->irr;
        self->total_irc += p*gauss->irc;
        self->total_icc += p*gauss->icc;
        gauss++;
    }

    self->total_irr /= psum;
    self->total_irc /= psum;
    self->total_icc /= psum;
}


/* convolution results in an nobj*npsf total gaussians */
struct gmix *gmix_convolve(const struct gmix *obj_gmix, 
                           const struct gmix *psf_gmix)
{
    struct gauss *psf=NULL, *obj=NULL, *comb=NULL;

    int ntot=0, iobj=0, ipsf=0;
    double irr=0, irc=0, icc=0, psum=0;

    ntot = obj_gmix->size*psf_gmix->size;
    struct gmix *gmix = gmix_new(ntot);

    for (ipsf=0; ipsf<psf_gmix->size; ipsf++) {
        psf = &psf_gmix->data[ipsf];
        psum += psf->p;
    }

    obj = obj_gmix->data;
    comb = gmix->data;
    for (iobj=0; iobj<obj_gmix->size; iobj++) {

        psf = psf_gmix->data;
        for (ipsf=0; ipsf<psf_gmix->size; ipsf++) {

            irr = obj->irr + psf->irr;
            irc = obj->irc + psf->irc;
            icc = obj->icc + psf->icc;

            gauss_set(comb,
                      obj->p*psf->p/psum,
                      obj->row, obj->col, 
                      irr, irc, icc);

            psf++;
            comb++;
        }

        obj++;
    }

    return gmix;
}



// pars are full gmix of size 6*ngauss
struct gmix *gmix_from_pars(double *pars, int size)
{
    int ngauss=0;
    struct gauss *gauss=NULL;

    int i=0, beg=0;

    if ( (size % 6) != 0) {
        return NULL;
    }
    ngauss = size/6;

    struct gmix *gmix = gmix_new(ngauss);


    for (i=0; i<ngauss; i++) {
        beg = i*6;

        gauss = &gmix->data[i];

        gauss_set(gauss,
                  pars[beg+0],
                  pars[beg+1],
                  pars[beg+2],
                  pars[beg+3],
                  pars[beg+4],
                  pars[beg+5]);
    }

    return gmix;
}


struct gmix *gmix_from_coellip(const double *pars, int npars)
{
    size_t ngauss = (npars-4)/2;
    struct gmix *gmix = gmix_new(ngauss);
    gmix_fill_coellip(gmix, pars, npars);
    return gmix;
}
int gmix_fill_coellip(struct gmix *gmix, 
                      const double *pars, 
                      int npars)
{

    if ( ((npars-4) % 2) != 0) {
        fprintf(stderr,"gmix error: pars are wrong size for coelliptical\n");
        return 0;
    }
    int ngauss=(npars-4)/2;
    if (ngauss != gmix->size) {
        fprintf(stderr,"input pars wrong length for input gmix struct\n");
        return 0;
    }

    double row=pars[0];
    double col=pars[1];
    double e1 = pars[2];
    double e2 = pars[3];

    struct gauss *gauss=gmix->data;
    for (int i=0; i<ngauss; i++) {

        double Ti = pars[4+i];
        double pi = pars[4+ngauss+i];

        gauss_set(gauss,
                  pi,
                  row, 
                  col, 
                  (Ti/2.)*(1-e1),
                  (Ti/2.)*e2,
                  (Ti/2.)*(1+e1));
        gauss++;
    }

    return 1;
}


struct gmix *gmix_from_coellip_Tfrac(double *pars, int size)
{
    int ngauss=0;
    double row=0, col=0, e1=0, e2=0, Tmax=0, Ti=0, pi=0, Tfrac=0;
    struct gauss *gauss=NULL;

    int i=0;

    if ( ((size-4) % 2) != 0) {
        return NULL;
    }
    ngauss = (size-4)/2;

    struct gmix * gmix = gmix_new(ngauss);

    row=pars[0];
    col=pars[1];
    e1 = pars[2];
    e2 = pars[3];
    Tmax = pars[4];

    for (i=0; i<ngauss; i++) {
        gauss = &gmix->data[i];

        if (i==0) {
            Ti = Tmax;
        } else {
            Tfrac = pars[4+i];
            Ti = Tmax*Tfrac;
        }

        pi = pars[4+ngauss+i];

        gauss_set(gauss,
                  pi,
                  row, 
                  col, 
                  (Ti/2.)*(1-e1),
                  (Ti/2.)*e2,
                  (Ti/2.)*(1+e1));
    }

    return gmix;
}


/* helper function */
int _gapprox_fill(struct gmix *self,
                  const double *pars, 
                  int size,
                  const double *Fvals, 
                  const double *pvals)
{
    double row=0, col=0, e1=0, e2=0;
    double T=0, Tvals[3]={0};
    double p=0, counts[3]={0};

    struct gauss *gauss=NULL;

    int i=0;

    if (!self|| self->size != 3) {
        fprintf(stderr,"approx gmix must size 3\n");
        return 0;
    }
    if (size != 6) {
        fprintf(stderr,"approx gmix pars must have pars size 6\n");
        return 0;
    }

    row=pars[0];
    col=pars[1];
    e1=pars[2];
    e2=pars[3];
    T=pars[4];
    p=pars[5];

    Tvals[0] = Fvals[0]*T;
    Tvals[1] = Fvals[1]*T;
    Tvals[2] = Fvals[2]*T;
    counts[0] = pvals[0]*p;
    counts[1] = pvals[1]*p;
    counts[2] = pvals[2]*p;

    gauss=self->data;
    for (i=0; i<self->size; i++) {
        gauss_set(gauss,
                  counts[i], 
                  row, col, 
                  (Tvals[i]/2.)*(1-e1), 
                  (Tvals[i]/2.)*e2,
                  (Tvals[i]/2.)*(1+e1));
        gauss++;
    }
    return 1;
}


int gmix_fill_exp(struct gmix *self,
                  const double *pars,
                  int size)
{
    /* pvals are normalized */
    static const double Fvals[3] = 
        {3.947146384343532e-05, 0.5010756804049256, 1.911515572152285};
        // this is the newer version using high res
        //{0.2293900119477738,1.01629012204044,2.770958917351007};
    static const double pvals[3] = 
        {0.06031348356539361,   0.5645244398053312, 0.3751620766292753};
        // this is the newer version using high res
        //{0.3327063609401037,0.5273717628243284,0.1399218762355679};

    return _gapprox_fill(self,pars,size,Fvals,pvals);
}
int gmix_fill_dev(struct gmix *self,
                  const double *pars,
                  int size)
{
    /* seems to me more a function of size than for exp */
    static const double Fvals[3] = 
        // this is the newer version using high res
        {0.09707812795975101,1.855263916143735,12.53275155599699};
        //{0.003718633817323675, 0.9268795541243965, 9.627400726500005};
    static const double pvals[3] = 
        // this is the newer version using high res
        {0.769283048205522,0.1841443288072131,0.04657262298726506};
        //{0.659318547053916,    0.2623209100496331, 0.07836054289645095};

    return _gapprox_fill(self,pars,size,Fvals,pvals);
}
int gmix_fill_turb(struct gmix *self,
                   const double *pars,
                   int size)
{
    static const double Fvals[3] = 
        {0.5793612389470884,1.621860687127999,7.019347162356363};
    static const double pvals[3] = 
        {0.596510042804182,0.4034898268889178,1.303069003078001e-07};

    return _gapprox_fill(self,pars,size,Fvals,pvals);
}



