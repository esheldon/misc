#ifndef _GMIX_HEADER_GUARD
#define _GMIX_HEADER_GUARD

#include <stdio.h>
#include "gauss2.h"

#define GMIX_BAD_MODEL 0x1
#define GMIX_ZERO_GAUSS 0x2
#define GMIX_WRONG_NPARS 0x4
#define GMIX_MISMATCH_SIZE 0x8


struct gmix {
    size_t size;
    struct gauss2* data;
};

/*
struct gmix {
    size_t size;
    struct gauss2* data;

    // these only make sense for same-center gaussians
    // e.g. the psf
    double total_irr;
    double total_irc;
    double total_icc;
    double psum;
};

*/

enum gmix_model {
    GMIX_FULL=0,
    GMIX_COELLIP=1,
    GMIX_TURB=2,
    GMIX_EXP=3,
    GMIX_DEV=4,
    GMIX_BD=5
};

enum gmix_model gmix_string2model(const char *model_name, long *flags);

long gmix_get_simple_ngauss(enum gmix_model model, long *flags);
long gmix_get_coellip_ngauss(long npars, long *flags);
long gmix_get_full_ngauss(long npars, long *flags);

struct gmix *gmix_new(size_t n, long *flags);

struct gmix* gmix_new_empty_simple(enum gmix_model model, long *flags);
struct gmix* gmix_new_empty_coellip(long npars, long *flags);
struct gmix* gmix_new_empty_full(long npars, long *flags);

struct gmix* gmix_new_model(enum gmix_model model, double *pars, long npars, long *flags);
struct gmix *gmix_new_coellip(double *pars, long npars, long *flags);


void gmix_fill_model(struct gmix *self,
                     enum gmix_model model,
                     double *pars,
                     long npars,
                     long *flags);

void gmix_fill_full(struct gmix *self, double *pars, long npars, long *flags);
void gmix_fill_coellip(struct gmix *self, double *pars, long npars, long *flags);
void gmix_fill_exp6(struct gmix *self, double *pars, long npars, long *flags);
void gmix_fill_dev10(struct gmix *self, double *pars, long npars, long *flags);
void gmix_fill_bd(struct gmix *self, double *pars, long npars, long *flags);
void gmix_fill_turb3(struct gmix *self, double *pars, long npars, long *flags);


struct gmix *gmix_free(struct gmix *self);
void gmix_set_dets(struct gmix *self);

// make sure pointer not null and det>0 for all gauss
long gmix_verify(const struct gmix *self);


void gmix_copy(const struct gmix *self, struct gmix* dest, long *flags);
struct gmix *gmix_new_copy(const struct gmix *self, long *flags);
void gmix_print(const struct gmix *self, FILE* fptr);

// calculate the weighted sum of the moments
//  sum_gi( p*(irr + icc )
double gmix_wmomsum(const struct gmix* gmix);

void gmix_get_cen(const struct gmix *gmix, double *row, double *col);
// set the overall centroid.  Note individual gaussians can have
// a different center
void gmix_set_cen(struct gmix *gmix, double row, double col);

double gmix_get_T(const struct gmix *self);
double gmix_get_psum(const struct gmix *gmix);
// set the overall sum(p)
void gmix_set_psum(struct gmix *gmix, double psum);

// 0 returned if a zero determinant is found somewhere, else 1
//int gmix_wmean_center(const struct gmix* gmix, struct vec2* mu_new);

//void gmix_wmean_covar(const struct gmix* gmix, struct mtx2 *cov);

// convolution results in an nobj*npsf total gaussians 
struct gmix *gmix_convolve(const struct gmix *obj_gmix, 
                           const struct gmix *psf_gmix,
                           long *flags);

void gmix_convolve_fill(struct gmix *self, 
                        const struct gmix *obj_gmix, 
                        const struct gmix *psf_gmix,
                        long *flags);

// totals in size not so meaningful if not cocentric
void gmix_get_totals(const struct gmix *self,
                     double *row, double *col,
                     double *irr, double *irc, double *icc,
                     double *counts);




/*
struct gmix *gmix_new(size_t n);
struct gmix *gmix_free(struct gmix *self);
void gmix_set_dets(struct gmix *self);

// make sure pointer not null and det>0 for all gauss
int gmix_verify(const struct gmix *self);

// get sum(p_i*T_i)/sum(p_i);
// this only works for co-centric gaussians
double gmix_get_T(const struct gmix *self);

// same as get_psum
double gmix_get_counts(const struct gmix *self);
double gmix_get_psum(const struct gmix *self);
void gmix_set_psum(struct gmix *self, double psum);

void gmix_get_cen(const struct gmix *self, double *row, double *col);
void gmix_set_cen(struct gmix *self, double row, double col);

//    total flux 
//    average moments
//    average position
//    total flux

void gmix_get_totals(const struct gmix *self,
                     double *row, double *col,
                     double *irr, double *irc, double *icc,
                     double *counts);

// only makes sense for same center, e.g. psf
//void gmix_set_total_moms(struct gmix *self);


struct gmix *gmix_new_copy(const struct gmix *self);
int gmix_copy(const struct gmix *self, struct gmix* dest);

void gmix_print(const struct gmix *self, FILE* fptr);

// calculate the weighted sum of the moments
//  sum_gi( p*(irr + icc )
double gmix_wmomsum(struct gmix* gmix);

// 0 returned if a zero determinant is found somewhere, else 1
//int gmix_wmean_center(const struct gmix* gmix, struct vec2* mu_new);

//void gmix_wmean_covar(const struct gmix* gmix, struct mtx2 *cov);

// convolution results in an nobj*npsf total gaussians 
//   NULL is returned on error.   
struct gmix *gmix_convolve(const struct gmix *obj_gmix, 
                           const struct gmix *psf_gmix);


//   fill in the gmix with the convolved gmix 
//   0 is returned on error, 1 otherwise   

int gmix_fill_convolve(struct gmix *self,
                       const struct gmix *obj_gmix, 
                       const struct gmix *psf_gmix);

// full parameters list
//   [pi,rowi,coli,irri,irci,icci,...]

struct gmix *gmix_from_pars(double *pars, int size);
//struct gmix *gmix_from_pars(gsl_vector *pars);

// coellip list
//   [row,col,e1,e2,T1,T2,T3,...,p1,p2,p3..]

struct gmix *gmix_make_coellip(const double *pars, int npars);
int gmix_fill_coellip(struct gmix *gmix, 
                      const double *pars, 
                      int npars);


// coellip Tfrac list
//   [row,col,e1,e2,Tmax,f2,f3,...,p1,p2,p3..]

struct gmix *gmix_make_coellip_Tfrac(double *pars, int size);


//   Generate new a gmix from the inputs pars assuming an appoximate
//   3-gaussian representation of an exponential profile. See
//   gmix_fill_exp for more info. 

struct gmix *gmix_make_exp6(const double *pars, int size);
*/

/* 
   Generate a gmix from the inputs pars assuming an appoximate
   3-gaussian representation of an exponential disk. It's only
   a good approx when convolved with a substantial psf.

   One component is nearly a delta function

   pars should be [row,col,e1,e2,T,p]

   T = sum(pi*Ti)/sum(pi)

   The p and F values are chosen to make this so
*/
//int gmix_fill_exp6(struct gmix *self, const double *pars, int size);

/* 
   Generate new a gmix from the inputs pars assuming an appoximate
   3-gaussian representation of a devauc profile. See
   gmix_fill_dev for more info. 
*/
//struct gmix *gmix_make_dev10(const double *pars, int size);

/*
   fill a gmix from the inputs pars assuming an appoximate
   3-gaussian representation of a devauc profile. It's only
   a good approx when convolved with a substantial psf.

   One component is nearly a delta function

   pars should be [row,col,e1,e2,T,p]

   T = sum(pi*Ti)/sum(pi)

   The p and F values are chosen to make this so
*/
//int gmix_fill_dev10(struct gmix *self,const double *pars, int size);

/* similar to above but for a turbulent psf */

//struct gmix *gmix_make_turb3(const double *pars, int size);
//int gmix_fill_turb3(struct gmix *self,const double *pars, int size);


#define GMIX_EVAL(gmix, rowval, colval) ({                     \
    double _val=0.0;                                           \
    struct gauss2 *_gauss=(gmix)->data;                         \
    for (int _i=0; _i<(gmix)->size; _i++) {                    \
        _val += GAUSS2_EVAL(_gauss, (rowval), (colval));        \
        _gauss++;                                              \
    }                                                          \
    _val;                                                      \
})



#endif
