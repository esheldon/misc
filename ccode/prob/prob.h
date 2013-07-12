/*
   C only handling of posterior, including priors
*/
#ifndef _PROB_HEADER_GUARD
#define _PROB_HEADER_GUARD

#include "image.h"
#include "jacobian.h"
#include "gmix.h"
#include "dist.h"
#include "obs.h"

enum prob_type {
    PROB_BA13=1,
    PROB_NOSPLIT_ETA=2
};

#define PROB_BAD_TYPE 0x1

#define PROB_LOG_LOWVAL -9.999e9

// BA13 g prior
// log normal priors on T and flux
// gaussian prior on center
struct prob_data_simple_ba {
    const struct obs_list *obs_list;

    long psf_ngauss;

    enum gmix_model model;
    struct gmix *obj0;
    struct gmix *obj;

    // priors

    // currently cen prior is always gaussian in both directions
    struct dist_gauss cen1_prior;
    struct dist_gauss cen2_prior;

    struct dist_g_ba shape_prior;

    struct dist_lognorm T_prior;
    struct dist_lognorm counts_prior;
};

enum prob_type prob_string2type(const char *type_name, long *flags);

struct prob_data_simple_ba *prob_data_simple_ba_new(enum gmix_model model,
                                                    long psf_ngauss,
                                                    const struct obs_list *obs_list,

                                                    const struct dist_gauss *cen1_prior,
                                                    const struct dist_gauss *cen2_prior,

                                                    const struct dist_g_ba *shape_prior,

                                                    const struct dist_lognorm *T_prior,
                                                    const struct dist_lognorm *counts_prior,
                                                    long *flags);
 
struct prob_data_simple_ba *prob_data_simple_ba_free(struct prob_data_simple_ba *self);
                                                 
void prob_simple_ba_calc_priors(struct prob_data_simple_ba *self,
                                double *pars, long npars,
                                double *lnprob,
                                long *flags);

// calculate the lnprob for the input pars
// also running s/n values
void prob_simple_ba_calc(struct prob_data_simple_ba *self,
                         double *pars, long npars,
                         double *s2n_numer, double *s2n_denom,
                         double *lnprob,
                         long *flags);


// using gaussian mixture in eta space
struct prob_data_simple_gmix3_eta {
    const struct obs_list *obs_list;

    long psf_ngauss;

    enum gmix_model model;
    struct gmix *obj0;
    struct gmix *obj;

    // priors

    // currently cen prior is always gaussian in both directions
    struct dist_gauss cen1_prior;
    struct dist_gauss cen2_prior;

    struct dist_gmix3_eta shape_prior;

    struct dist_lognorm T_prior;
    struct dist_lognorm counts_prior;
};


struct prob_data_simple_gmix3_eta *prob_data_simple_gmix3_eta_new(
        enum gmix_model model,
        long psf_ngauss,
        const struct obs_list *obs_list,

        const struct dist_gauss *cen1_prior,
        const struct dist_gauss *cen2_prior,

        const struct dist_gmix3_eta *shape_prior,

        const struct dist_lognorm *T_prior,
        const struct dist_lognorm *counts_prior,
        long *flags);

struct prob_data_simple_gmix3_eta *prob_data_simple_gmix3_eta_free(struct prob_data_simple_gmix3_eta *self);
                                                 
void prob_simple_gmix3_eta_calc_priors(struct prob_data_simple_gmix3_eta *self,
                                       double *pars, long npars,
                                       double *lnprob,
                                       long *flags);

// calculate the lnprob for the input pars
// also running s/n values
void prob_simple_gmix3_eta_calc(struct prob_data_simple_gmix3_eta *self,
                                double *pars, long npars,
                                double *s2n_numer, double *s2n_denom,
                                double *lnprob,
                                long *flags);








// generic likelihood calculator
void prob_calc_simple_likelihood(struct gmix *obj0,
                                 struct gmix *obj,
                                 enum gmix_model model,
                                 const struct obs_list *obs_list,
                                 double *pars,
                                 long npars,
                                 double *s2n_numer,
                                 double *s2n_denom,
                                 double *loglike,
                                 long *flags);

#endif
