#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "prob.h"
#include "gmix_image.h"
#include "gmix_mcmc_config.h"
#include "gmix.h"
#include "shape.h"

enum prob_type prob_string2type(const char *type_name, long *flags)
{
    enum prob_type type=0;
    if (0==strcmp(type_name,"PROB_NOSPLIT_ETA")) {
        type=PROB_NOSPLIT_ETA;
    } else if (0==strcmp(type_name,"PROB_BA13")) {
        type=PROB_BA13;
    } else {
        *flags |= PROB_BAD_TYPE;
    }
    return type;
}


// generic likelihood calculator
void prob_calc_simple_likelihood(struct gmix *obj0,
                                 struct gmix *obj,
                                 enum gmix_model model,
                                 const struct obs_list *obs_list,
                                 const double *pars,
                                 long npars,
                                 double *s2n_numer,
                                 double *s2n_denom,
                                 double *loglike,
                                 long *flags)
{

    long i=0;
    double t_loglike=0, t_s2n_numer=0, t_s2n_denom=0;
    struct obs *obs=NULL;

    *loglike=0;
    *s2n_numer=0;
    *s2n_denom=0;

    *flags=0;

    gmix_fill_model(obj0,model,pars,npars,flags);
    //jacobian_print(&obs_list->data[0].jacob, stderr);

    //fprintf(stderr,"psf\n");
    //gmix_print(obs_list->data[0].psf_gmix,stderr);

    //fprintf(stderr,"gmix0\n");
    //gmix_print(obj0,stderr);
    // g out of range is not a fatal error in the likelihood
    if (*flags) {
        goto _prob_calc_simple_likelihood_generic_bail;
    }

    for (i=0; i<obs_list->size; i++) {
        obs=&obs_list->data[i];

        gmix_convolve_fill(obj, obj0, obs->psf_gmix, flags);
        fprintf(stderr,"obj0\n");
        gmix_print(obj0,stderr);
        fprintf(stderr,"obj\n");
        gmix_print(obj,stderr);
        fprintf(stderr,"nrows: %lu ncols: %lu\n",
                IM_NROWS(obs->image), IM_NCOLS(obs->image));
        if (*flags) {
            goto _prob_calc_simple_likelihood_generic_bail;
        }

        //fprintf(stderr,"gmix convolved\n");
        //gmix_print(obj,stderr);
        // the only failure is actually redundant with above
        *flags |= gmix_image_loglike_wt_jacob(obs->image, 
                                              obs->weight,
                                              &obs->jacob,
                                              obj,
                                              &t_s2n_numer,
                                              &t_s2n_denom,
                                              &t_loglike);
        if (*flags) {
            goto _prob_calc_simple_likelihood_generic_bail;
        }

        (*s2n_numer) += t_s2n_numer;
        (*s2n_denom) += t_s2n_denom;
        (*loglike)   += t_loglike;
    }

_prob_calc_simple_likelihood_generic_bail:
    if (*flags) {
        *loglike = PROB_LOG_LOWVAL;
        *s2n_numer=0;
        *s2n_denom=0;
    }
}


/*

   BA13

*/


struct prob_data_simple_ba *prob_data_simple_ba_new(enum gmix_model model,
                                                    long psf_ngauss,

                                                    const struct dist_gauss *cen1_prior,
                                                    const struct dist_gauss *cen2_prior,

                                                    const struct dist_g_ba *shape_prior,

                                                    const struct dist_lognorm *T_prior,
                                                    const struct dist_lognorm *counts_prior,
                                                    long *flags)


{

    struct prob_data_simple_ba *self=calloc(1, sizeof(struct prob_data_simple_ba));
    if (!self) {
        fprintf(stderr,"could not allocate struct prob_data_simple_ba\n");
        exit(EXIT_FAILURE);
    }

    self->type = PROB_BA13;
    self->model=model;

    self->obj0 = gmix_new_empty_simple(model, flags);
    if (*flags) {
        goto _prob_data_simple_ba_new_bail;
    }

    long ngauss0 = self->obj0->size;
    long ngauss_tot = ngauss0*psf_ngauss;

    self->obj = gmix_new(ngauss_tot, flags);
    if (*flags) {
        goto _prob_data_simple_ba_new_bail;
    }

    self->cen1_prior = (*cen1_prior);
    self->cen2_prior = (*cen2_prior);
    self->shape_prior = (*shape_prior);
    self->T_prior = (*T_prior);
    self->counts_prior = (*counts_prior);

_prob_data_simple_ba_new_bail:
    if (*flags) {
        self=prob_data_simple_ba_free(self);
    }
    return self;
}

struct prob_data_simple_ba *prob_data_simple_ba_free(struct prob_data_simple_ba *self)
{
    if (self) {
        self->obj0=gmix_free(self->obj0);
        self->obj=gmix_free(self->obj);

        free(self);
    }
    return NULL;
}

void prob_simple_ba_calc_priors(struct prob_data_simple_ba *self,
                                const double *pars, long npars,
                                double *lnprob,
                                long *flags)
{
    (*flags) = 0;
    (*lnprob) = 0;

    (*lnprob) += dist_gauss_lnprob(&self->cen1_prior,pars[0]);
    (*lnprob) += dist_gauss_lnprob(&self->cen2_prior,pars[1]);

    (*lnprob) += dist_g_ba_lnprob(&self->shape_prior,pars[2],pars[3]);

    (*lnprob) += dist_lognorm_lnprob(&self->T_prior,pars[4]);
    (*lnprob) += dist_lognorm_lnprob(&self->counts_prior,pars[5]);
}

void prob_simple_ba_calc(struct prob_data_simple_ba *self,
                         const struct obs_list *obs_list,
                         const double *pars, long npars,
                         double *s2n_numer, double *s2n_denom,
                         double *lnprob, long *flags)
{

    double loglike=0, priors_lnprob=0;

    *lnprob=0;

    prob_calc_simple_likelihood(self->obj0,
                                self->obj,
                                self->model,
                                obs_list,
                                pars,
                                npars,
                                s2n_numer,
                                s2n_denom,
                                &loglike,
                                flags);

    if (*flags != 0) {
        goto _prob_simple_ba_calc_bail;
    }

    (*lnprob) += loglike;

    // flags are always zero from here
    prob_simple_ba_calc_priors(self, pars, npars, &priors_lnprob, flags);
    if (*flags != 0) {
        goto _prob_simple_ba_calc_bail;
    }

    (*lnprob) += priors_lnprob;

_prob_simple_ba_calc_bail:
    if (*flags != 0) {
        (*lnprob) = PROB_LOG_LOWVAL;
        *s2n_numer=0;
        *s2n_denom=0;
    }
}

/*

   gmix3 in eta space

*/


struct prob_data_simple_gmix3_eta *
prob_data_simple_gmix3_eta_new(enum gmix_model model,
                               long psf_ngauss,

                               const struct dist_gauss *cen1_prior,
                               const struct dist_gauss *cen2_prior,

                               const struct dist_gmix3_eta *shape_prior,

                               const struct dist_lognorm *T_prior,
                               const struct dist_lognorm *counts_prior,
                               long *flags)


{

    struct prob_data_simple_gmix3_eta *self=calloc(1, sizeof(struct prob_data_simple_gmix3_eta));
    if (!self) {
        fprintf(stderr,"could not allocate struct prob_data_simple_gmix3_eta\n");
        exit(EXIT_FAILURE);
    }

    self->type = PROB_NOSPLIT_ETA;
    self->model=model;

    self->obj0 = gmix_new_empty_simple(model, flags);
    if (*flags) {
        goto _prob_data_simple_gmix3_eta_new_bail;
    }

    long ngauss0 = self->obj0->size;
    long ngauss_tot = ngauss0*psf_ngauss;

    self->obj = gmix_new(ngauss_tot, flags);
    if (*flags) {
        goto _prob_data_simple_gmix3_eta_new_bail;
    }

    self->cen1_prior = (*cen1_prior);
    self->cen2_prior = (*cen2_prior);
    self->shape_prior = (*shape_prior);
    self->T_prior = (*T_prior);
    self->counts_prior = (*counts_prior);

_prob_data_simple_gmix3_eta_new_bail:
    if (*flags) {
        self=prob_data_simple_gmix3_eta_free(self);
    }
    return self;
}

struct prob_data_simple_gmix3_eta *prob_data_simple_gmix3_eta_free(struct prob_data_simple_gmix3_eta *self)
{
    if (self) {
        self->obj0=gmix_free(self->obj0);
        self->obj=gmix_free(self->obj);

        free(self);
    }
    return NULL;
}

double prob_simple_gmix3_eta_calc_priors(struct prob_data_simple_gmix3_eta *self,
                                         const double *pars, long npars,
                                         long *flags)
{
    double lnprob = 0;

    (*flags) = 0;

    lnprob += dist_gauss_lnprob(&self->cen1_prior,pars[0]);
    lnprob += dist_gauss_lnprob(&self->cen2_prior,pars[1]);

    lnprob += dist_gmix3_eta_lnprob(&self->shape_prior,pars[2],pars[3]);

    lnprob += dist_lognorm_lnprob(&self->T_prior,pars[4]);
    lnprob += dist_lognorm_lnprob(&self->counts_prior,pars[5]);

    return lnprob;
}

void prob_simple_gmix3_eta_calc(struct prob_data_simple_gmix3_eta *self,
                                const struct obs_list *obs_list,
                                const double *pars, long npars,
                                double *s2n_numer, double *s2n_denom,
                                double *lnprob, long *flags)
{

    double loglike=0, priors_lnprob=0;
    double gpars[6];
    struct shape shape={0};

    memcpy(gpars, pars, 6*sizeof(double));

    shape_set_eta(&shape, pars[2], pars[3]);

    gpars[2] = shape.g1;
    gpars[3] = shape.g2;

    *lnprob=0;

    prob_calc_simple_likelihood(self->obj0,
                                self->obj,
                                self->model,
                                obs_list,
                                gpars,
                                npars,
                                s2n_numer,
                                s2n_denom,
                                &loglike,
                                flags);

    if (*flags != 0) {
        goto _prob_simple_gmix3_eta_calc_bail;
    }

    // not this is pars with eta, not gpars
    // flags are always zero from here
    priors_lnprob = prob_simple_gmix3_eta_calc_priors(self, pars, npars, flags);
    if (*flags != 0) {
        goto _prob_simple_gmix3_eta_calc_bail;
    }

    (*lnprob) = loglike + priors_lnprob;

    //fprintf(stderr,"loglike:       %g\n", loglike);
    //fprintf(stderr,"lnprob priors: %g\n", priors_lnprob);
_prob_simple_gmix3_eta_calc_bail:
    if (*flags != 0) {
        (*lnprob) = PROB_LOG_LOWVAL;
        *s2n_numer=0;
        *s2n_denom=0;
    }
}

void prob_simple_gmix3_eta_print(struct prob_data_simple_gmix3_eta *self, FILE *stream)
{
    dist_gauss_print(&self->cen1_prior, stream);
    dist_gauss_print(&self->cen2_prior, stream);
    dist_gmix3_eta_print(&self->shape_prior, stream);
    dist_lognorm_print(&self->T_prior,stream);
    dist_lognorm_print(&self->counts_prior,stream);
}
