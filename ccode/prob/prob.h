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
    PROB_BA13,
    PROB_NOSPLIT_ETA,
    PROB_BA13_SHEAR  // full shear exploration
};

#define PROB_BAD_TYPE 0x1

#define PROB_LOG_LOWVAL -9.999e9

// we can always cast to this type to extract
// the model
struct prob_data_base {
    enum prob_type type;
};


// cast to (prob_data_base *) and get the type field, shared
// by all prob structures in the first element
#define PROB_GET_TYPE(self) ( (prob_data_base *) (self) )->type

// BA13 g prior
// log normal priors on T and flux
// gaussian prior on center
struct prob_data_simple_ba {
    enum prob_type type;

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

// the distributions are value types and get copied
struct prob_data_simple_ba *
prob_data_simple_ba_new(enum gmix_model model,
                        long psf_ngauss,

                        const struct dist_gauss *cen1_prior,
                        const struct dist_gauss *cen2_prior,

                        const struct dist_g_ba *shape_prior,

                        const struct dist_lognorm *T_prior,
                        const struct dist_lognorm *counts_prior,
                        long *flags);
 
struct prob_data_simple_ba *prob_data_simple_ba_free(struct prob_data_simple_ba *self);
                                                 
void prob_simple_ba_print(struct prob_data_simple_ba *self, FILE *stream);

void prob_simple_ba_calc_priors(struct prob_data_simple_ba *self,
                                const struct gmix_pars *pars,
                                double *lnprob,
                                long *flags);

// calculate the lnprob for the input pars
// also running s/n values
void prob_simple_ba_calc(struct prob_data_simple_ba *self,
                         const struct obs_list *obs_list,
                         const struct gmix_pars *pars,
                         double *s2n_numer, double *s2n_denom,
                         double *lnprob,
                         long *flags);


// ba13 with exploration of shear in mcmc
struct prob_data_simple_ba *
prob_data_simple_ba_new_with_shear(enum gmix_model model,
                                   long psf_ngauss,

                                   const struct dist_gauss *cen1_prior,
                                   const struct dist_gauss *cen2_prior,

                                   const struct dist_g_ba *shape_prior,

                                   const struct dist_lognorm *T_prior,
                                   const struct dist_lognorm *counts_prior,
                                   long *flags);

void prob_simple_ba_calc_priors_with_shear(struct prob_data_simple_ba *self,
                                           const struct gmix_pars *pars,
                                           double *lnprob,
                                           long *flags);


void prob_simple_ba_calc_with_shear(struct prob_data_simple_ba *self,
                                    const struct obs_list *obs_list,
                                    const struct gmix_pars *pars,
                                    double *s2n_numer, double *s2n_denom,
                                    double *lnprob, long *flags);


// using gaussian mixture in eta space
struct prob_data_simple_gmix3_eta {
    enum prob_type type;

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

// the distributions are value types and get copied
struct prob_data_simple_gmix3_eta *
prob_data_simple_gmix3_eta_new(enum gmix_model model,
                               long psf_ngauss,

                               const struct dist_gauss *cen1_prior,
                               const struct dist_gauss *cen2_prior,

                               const struct dist_gmix3_eta *shape_prior,

                               const struct dist_lognorm *T_prior,
                               const struct dist_lognorm *counts_prior,
                               long *flags);

struct prob_data_simple_gmix3_eta *prob_data_simple_gmix3_eta_free(struct prob_data_simple_gmix3_eta *self);
                                                 
double prob_simple_gmix3_eta_calc_priors(struct prob_data_simple_gmix3_eta *self,
                                         const struct gmix_pars *pars,
                                         long *flags);

// calculate the lnprob for the input pars
// also running s/n values
void prob_simple_gmix3_eta_calc(struct prob_data_simple_gmix3_eta *self,
                                const struct obs_list *obs_list,
                                const struct gmix_pars *pars,
                                double *s2n_numer,
                                double *s2n_denom,
                                double *lnprob,
                                long *flags);








// generic likelihood calculator
void prob_calc_simple_likelihood(struct gmix *obj0,
                                 struct gmix *obj,
                                 const struct obs_list *obs_list,
                                 const struct gmix_pars *pars,
                                 double *s2n_numer,
                                 double *s2n_denom,
                                 double *loglike,
                                 long *flags);

void prob_simple_gmix3_eta_print(struct prob_data_simple_gmix3_eta *self, FILE *stream);

#endif
