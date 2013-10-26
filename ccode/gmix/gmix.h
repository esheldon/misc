#ifndef _GMIX_HEADER_GUARD
#define _GMIX_HEADER_GUARD

#include <stdio.h>
#include "gauss2.h"
#include "shape.h"

#define GMIX_BAD_MODEL 0x1
#define GMIX_ZERO_GAUSS 0x2
#define GMIX_WRONG_NPARS 0x4
#define GMIX_MISMATCH_SIZE 0x8

enum gmix_model {
    GMIX_FULL,
    GMIX_COELLIP,
    GMIX_TURB,
    GMIX_EXP,
    GMIX_DEV,
    GMIX_BD,

    // these models have shear in the last 2 elements
    GMIX_EXP_SHEAR,
    GMIX_DEV_SHEAR
};

struct gmix {
    size_t size;
    struct gauss2 *data;
};

struct gmix_list {
    size_t size;
    struct gmix **data;
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


struct gmix_pars {
    enum gmix_model model;

    enum shape_system shape_system;

    size_t size;
    double *data;

    // not used by GMIX_FULL
    struct shape shape;

    // used when exploring shear
    struct shape shear;
};

struct gmix_pars *gmix_pars_new(enum gmix_model model,
                                const double *pars,
                                size_t npars,
                                enum shape_system system,
                                long *flags);

void gmix_pars_fill(struct gmix_pars *self,
                    const double *pars,
                    size_t npars,
                    enum shape_system system,
                    long *flags);

struct gmix_pars *gmix_pars_free(struct gmix_pars *self);
void gmix_pars_print(const struct gmix_pars *self, FILE *stream);

enum gmix_model gmix_string2model(const char *model_name, long *flags);
long gmix_get_simple_npars(enum gmix_model model, long *flags);

long gmix_get_simple_ngauss(enum gmix_model model, long *flags);
long gmix_get_coellip_ngauss(long npars, long *flags);
long gmix_get_full_ngauss(long npars, long *flags);

struct gmix *gmix_new(size_t n, long *flags);
void gmix_resize(struct gmix *self, size_t size);

struct gmix* gmix_new_empty_simple(enum gmix_model model, long *flags);
struct gmix* gmix_new_empty_coellip(long npars, long *flags);
struct gmix* gmix_new_empty_full(long npars, long *flags);

struct gmix* gmix_new_model(const struct gmix_pars *pars, long *flags);
struct gmix* gmix_new_model_from_array(enum gmix_model model,
                                       const double *pars,
                                       long npars,
                                       enum shape_system system,
                                       long *flags);

//struct gmix *gmix_new_coellip(const gmix_pars *pars, long *flags);


void gmix_fill_model(struct gmix *self, const struct gmix_pars *pars, long *flags);

/*
void gmix_fill_full(struct gmix *self, const double *pars, long npars, long *flags);
void gmix_fill_coellip(struct gmix *self, const double *pars, long npars, long *flags);
void gmix_fill_exp6(struct gmix *self, const double *pars, long npars, long *flags);
void gmix_fill_dev10(struct gmix *self, const double *pars, long npars, long *flags);
void gmix_fill_bd(struct gmix *self, const double *pars, long npars, long *flags);
void gmix_fill_turb3(struct gmix *self, const double *pars, long npars, long *flags);
*/


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




struct gmix_list *gmix_list_new(size_t size, size_t ngauss, long *flags);
struct gmix_list *gmix_list_free(struct gmix_list *self);


#define GMIX_EVAL(gmix, rowval, colval) ({                     \
    double _val=0.0;                                           \
    struct gauss2 *_gauss=(gmix)->data;                         \
    for (int _i=0; _i<(gmix)->size; _i++) {                    \
        _val += GAUSS2_EVAL(_gauss, (rowval), (colval));        \
        _gauss++;                                              \
    }                                                          \
    _val;                                                      \
})

#define GMIX_EVAL_SLOW(gmix, rowval, colval) ({                \
    double _val=0.0;                                           \
    struct gauss2 *_gauss=(gmix)->data;                        \
    for (int _i=0; _i<(gmix)->size; _i++) {                    \
        _val += GAUSS2_EVAL_SLOW(_gauss, (rowval), (colval));  \
        _gauss++;                                              \
    }                                                          \
    _val;                                                      \
})




#endif
