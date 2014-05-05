#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mca.h"
#include "config.h"
#include "prob.h"
#include "gmix.h"
#include "randn.h"
#include "gmix_mcmc_config.h"
#include "gmix_mcmc.h"

// you should caste prob_data_base to your actual type
static struct prob_data_base *get_prob_ba13(const struct gmix_mcmc_config *conf, long *flags)
{

    struct prob_data_simple_ba *prob=NULL;

    struct dist_gauss cen_prior={0};

    struct dist_g_ba shape_prior={0};

    struct dist_lognorm T_prior={0};
    struct dist_lognorm counts_prior={0};

    fprintf(stderr,"loading prob ba13\n");

    if (conf->cen_prior_npars != 2
            || conf->T_prior_npars != 2
            || conf->counts_prior_npars != 2
            || conf->shape_prior_npars != 1) {

        fprintf(stderr,"error: wrong npars: %s: %d\n",
                __FILE__,__LINE__);
        *flags |= DIST_WRONG_NPARS;
        return NULL;
    }
    dist_gauss_fill(&cen_prior,
                    conf->cen_prior_pars[0],
                    conf->cen_prior_pars[1]);
    dist_lognorm_fill(&T_prior, 
                      conf->T_prior_pars[0],
                      conf->T_prior_pars[1]);
    dist_lognorm_fill(&counts_prior,
                      conf->counts_prior_pars[0],
                      conf->counts_prior_pars[1]);
    dist_g_ba_fill(&shape_prior, conf->shape_prior_pars[0]);

    // priors get copied
    if (conf->prob_type == PROB_BA13_SHEAR) {
        prob=prob_data_simple_ba_new_with_shear(conf->fitmodel,
                                                conf->psf_ngauss,

                                                &cen_prior,
                                                &cen_prior,

                                                &shape_prior,

                                                &T_prior,
                                                &counts_prior,
                                                flags);

    } else {
        prob=prob_data_simple_ba_new(conf->fitmodel,
                                     conf->psf_ngauss,

                                     &cen_prior,
                                     &cen_prior,

                                     &shape_prior,

                                     &T_prior,
                                     &counts_prior,
                                     flags);
    }
    fprintf(stderr,"prob:\n");
    prob_simple_ba_print(prob, stderr);

    return (struct prob_data_base *) prob;
}


// you should caste prob_data_base to your actual type
static struct prob_data_base *get_prob_gmix3_eta(const struct gmix_mcmc_config *conf, long *flags)
{

    struct prob_data_simple_gmix3_eta *prob=NULL;

    struct dist_gauss cen_prior={0};

    struct dist_gmix3_eta shape_prior={0};

    struct dist_lognorm T_prior={0};
    struct dist_lognorm counts_prior={0};

    fprintf(stderr,"loading prob gmix3_eta\n");

    if (conf->cen_prior_npars != 2
            || conf->T_prior_npars != 2
            || conf->counts_prior_npars != 2
            || conf->shape_prior_npars != 6) {

        fprintf(stderr,"error: wrong npars: %s: %d\n",
                __FILE__,__LINE__);
        *flags |= DIST_WRONG_NPARS;
        return NULL;
    }
    dist_gauss_fill(&cen_prior,
                    conf->cen_prior_pars[0],
                    conf->cen_prior_pars[1]);
    dist_lognorm_fill(&T_prior, 
                      conf->T_prior_pars[0],
                      conf->T_prior_pars[1]);
    dist_lognorm_fill(&counts_prior,
                      conf->counts_prior_pars[0],
                      conf->counts_prior_pars[1]);
    dist_gmix3_eta_fill(&shape_prior,
                        conf->shape_prior_pars[0],  // sigma1
                        conf->shape_prior_pars[1],  // sigma2
                        conf->shape_prior_pars[2],  // sigma3
                        conf->shape_prior_pars[3],  // p1
                        conf->shape_prior_pars[4],  // p2
                        conf->shape_prior_pars[5]); // p3


    // priors get copied
    prob=prob_data_simple_gmix3_eta_new(conf->fitmodel,
                                        conf->psf_ngauss,

                                        &cen_prior,
                                        &cen_prior,

                                        &shape_prior,

                                        &T_prior,
                                        &counts_prior,
                                        flags);

    fprintf(stderr,"prob:\n");
    prob_simple_gmix3_eta_print(prob, stderr);

    return (struct prob_data_base *) prob;
}

static struct prob_data_base *prob_new_generic(const struct gmix_mcmc_config *conf, long *flags)
{
    switch (conf->prob_type) {
        case PROB_NOSPLIT_ETA:
            return get_prob_gmix3_eta(conf, flags);
            break;

        case PROB_BA13:
            return get_prob_ba13(conf, flags);
            break;

        // note  this is the same!
        case PROB_BA13_SHEAR:
            return get_prob_ba13(conf, flags);
            break;

        default:
            fprintf(stderr, "bad prob type: %u: %s: %d, aborting\n",
                    conf->prob_type, __FILE__,__LINE__);
            exit(1);

    }
}
// can generalize this
static struct prob_data_base *prob_free_generic(struct prob_data_base *prob)
{
    if (prob->type == PROB_NOSPLIT_ETA) {
        struct prob_data_simple_gmix3_eta *ptmp
            =(struct prob_data_simple_gmix3_eta *) prob;
        prob_data_simple_gmix3_eta_free(ptmp);
    } else if (prob->type == PROB_BA13
                   || prob->type == PROB_BA13_SHEAR) {
        struct prob_data_simple_ba *ptmp
            =(struct prob_data_simple_ba *) prob;
        prob_data_simple_ba_free(ptmp);
    } else {
        fprintf(stderr, "bad prob type: %u: %s: %d, aborting\n",
                prob->type, __FILE__,__LINE__);
            exit(1);
    }

    return NULL;
}


static void create_chain_data(struct gmix_mcmc *self)
{

    self->chain_data.mca_a = self->conf.mca_a;
    self->chain_data.burnin_chain = mca_chain_new(self->conf.nwalkers,
                                                  self->conf.burnin,
                                                  self->conf.npars);
    self->chain_data.chain = mca_chain_new(self->conf.nwalkers,
                                           self->conf.nstep,
                                           self->conf.npars);
    self->chain_data.stats = mca_stats_new(self->conf.npars);

}
static void free_chain_data(struct gmix_mcmc *self)
{
    self->chain_data.burnin_chain = mca_chain_free(self->chain_data.burnin_chain);
    self->chain_data.chain = mca_chain_free(self->chain_data.chain);
    self->chain_data.stats = mca_stats_free(self->chain_data.stats);
}


struct gmix_mcmc *gmix_mcmc_new(const struct gmix_mcmc_config *conf, long *flags)
{
    struct gmix_mcmc *self=calloc(1, sizeof(struct gmix_mcmc));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct gmix_mcmc: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    // value type
    self->conf = (*conf);

    // cast to (prob_data_base *) to check the type
    // use PROB_GET_TYPE macro
    self->prob = prob_new_generic(conf, flags);
    if (*flags != 0) {
        goto _gmix_mcmc_new_bail;
    }

    create_chain_data(self);

_gmix_mcmc_new_bail:
    if (*flags != 0)  {
       self=gmix_mcmc_free(self);
    }

    return self;
}
struct gmix_mcmc *gmix_mcmc_new_from_config(const char *name, long *flags)
{

    struct gmix_mcmc *self=NULL;
    struct gmix_mcmc_config conf={0};

    *flags=gmix_mcmc_config_load(&conf, name);
    if (*flags != 0) {
        goto _gmix_mcmc_new_from_config_bail;
    }

    self = gmix_mcmc_new(&conf, flags);

_gmix_mcmc_new_from_config_bail:
    if (*flags != 0) {
        self=gmix_mcmc_free(self);
    }

    return self;
}
struct gmix_mcmc *gmix_mcmc_free(struct gmix_mcmc *self)
{
    if (self) {
        free_chain_data(self);
        self->prob=prob_free_generic((struct prob_data_base *) self->prob);

        free(self);
        self=NULL;
    }
    return self;
}

void gmix_mcmc_set_obs_list(struct gmix_mcmc *self, const struct obs_list *obs_list)
{
    self->obs_list=obs_list;
}


static void calc_pqr(struct gmix_mcmc *self,
                     const double *pars,
                     size_t npars,
                     double *P,
                     double *Q1, double *Q2,
                     double *R11, double *R12, double *R22,
                     long *flags)
{
    switch (self->prob->type) {
        case PROB_NOSPLIT_ETA:
            {
                struct prob_data_simple_gmix3_eta *prob = 
                    (struct prob_data_simple_gmix3_eta *)self->prob;

                struct gmix_pars *gmix_pars=gmix_pars_new(prob->model, pars, npars, 
                                                          SHAPE_SYSTEM_ETA, flags);
                if (*flags==0) {
                    dist_gmix3_eta_pqr(&prob->shape_prior,
                                       &gmix_pars->shape,
                                       P, Q1, Q2, R11, R12, R22,
                                       flags);
                    gmix_pars = gmix_pars_free(gmix_pars);
                }
            }
            break;
        case PROB_BA13:
            {
                struct prob_data_simple_ba *prob = 
                    (struct prob_data_simple_ba *)self->prob;

                struct gmix_pars *gmix_pars=gmix_pars_new(prob->model, pars, npars,
                                                          SHAPE_SYSTEM_G, flags);
                if (*flags==0) {
                    dist_g_ba_pqr(&prob->shape_prior,
                                  &gmix_pars->shape,
                                  P, Q1, Q2, R11, R12, R22);
                    gmix_pars = gmix_pars_free(gmix_pars);
                }
            }
            break;

        default:
            fprintf(stderr, "bad prob type: %u: %s: %d, aborting\n",
                    self->prob->type, __FILE__,__LINE__);
            exit(1);
    }
}

// "fix" pqr for the fact that the prior is already in the distribution of
// points.  Just divide by the prior.  return 1 if the numerical values are all
// ok.  note the allowed range works well for the gmix3 pars I have tried, but
// I haven't tried others.  Also they are definitely too broad for ba13, but
// that is OK since I use analytic formulas then and I don't expect crazy values

static long fix_pqr(double *P, double *Q1, double *Q2,
                    double *R11, double *R12, double *R22)
{
    long ok=0;
    if (*P > 0) {
        double Pinv=1/(*P);
        if (finite(Pinv)) {

            *P *= Pinv;
            *Q1 *= Pinv;
            *Q2 *= Pinv;
            *R11 *= Pinv;
            *R12 *= Pinv;
            *R22 *= Pinv;

            if (fabs(*Q1) < 20 && fabs(*Q2) < 20
                    && (*R11) > -100 && (*R11) < 200
                    && (*R22) > -100 && (*R22) < 200
                    && fabs(*R12) < 110) {

                ok=1;

            }
        }
    } // P >0 check

    return ok;
}
// calculate P,Q,R with fix for the fact that the prior is already
// in our chain; just divide by the prior
long gmix_mcmc_calc_pqr(struct gmix_mcmc *self)
{

    const struct mca_chain *chain = self->chain_data.chain;

    long nstep=MCA_CHAIN_NSTEPS(chain);
    size_t npars = MCA_CHAIN_NPARS(chain);

    double Psum=0, Q1sum=0, Q2sum=0, R11sum=0, R12sum=0, R22sum=0;
    double P=0, Q1=0, Q2=0, R11=0, R12=0, R22=0, Pmax=0;
    long nuse=0, flags=0;

    for (long i=0; i<nstep; i++) {
        const double *pars = MCA_CHAIN_PARS(chain, i);

        flags=0;
        calc_pqr(self,pars,npars,&P,&Q1,&Q2,&R11,&R12,&R22,&flags);

        if (flags==0) {
            if (P > Pmax) {
                Pmax=P;
            }
            // fix because prior is already in distributions
            long ok=fix_pqr(&P,&Q1,&Q2,&R11,&R12,&R22);
            if (ok) {
                nuse++;
                Psum += P;
                Q1sum += Q1;
                Q2sum += Q2;
                R11sum += R11;
                R12sum += R12;
                R22sum += R22;
            }
        } // flag check

    } // steps

    flags=0;

    self->nuse = nuse;
    if (nuse > 0) {
        self->P = Psum/nuse;
        self->Q[0] = Q1sum/nuse;
        self->Q[1] = Q2sum/nuse;
        self->R[0][0] = R11sum/nuse;
        self->R[0][1] = R12sum/nuse;
        self->R[1][1] = R22sum/nuse;

        self->R[1][0] = self->R[0][1]; 
    } else {
        fprintf(stderr,"no good P,Q,R vals; max P was %g\n", Pmax);
        flags |= GMIX_MCMC_NOPOSITIVE;
    }

    return flags;
}

long gmix_mcmc_fill_prob1(struct gmix_mcmc *self,
                          struct shear_prob1 *shear_prob1)

{
    if (self->prob->type != PROB_BA13) {
        fprintf(stderr,"only PROB_BA13 for now\n");
        exit(1);
    }

    const struct mca_chain *chain = self->chain_data.chain;

    long nstep=MCA_CHAIN_NSTEPS(chain);
    size_t npars = MCA_CHAIN_NPARS(chain);

    struct prob_data_simple_ba *prob = 
        (struct prob_data_simple_ba *)self->prob;

    double Pmax=0;
    long flags=0, nuse=0;
    for (long istep=0; istep<nstep; istep++) {
        const double *pars = MCA_CHAIN_PARS(chain, istep);
        struct gmix_pars *gmix_pars=gmix_pars_new(prob->model,
                                                  pars,
                                                  npars,
                                                  SHAPE_SYSTEM_G,
                                                  &flags);
        if (flags==0) {

            double P=dist_g_ba_prob(&prob->shape_prior, &gmix_pars->shape);

            if (P > 0) {
                if (P > Pmax) {
                    Pmax=P;
                }
                nuse++;
                for (long ishear=0; ishear< shear_prob1->nshear; ishear++) {

                    struct shape *shear = &shear_prob1->shears[ishear];
                    double Pj=dist_g_ba_pj(&prob->shape_prior,
                                           &gmix_pars->shape,
                                           shear,
                                           &flags);
                    if (flags != 0) {
                        fprintf(stderr,"range error\n");
                    }
                    double lnp = log(Pj/P);

                    shear_prob1->lnprob[ishear] += lnp;
                }
            }
        }
        gmix_pars = gmix_pars_free(gmix_pars);
    }

    flags=0;
    if (nuse == 0) {
        fprintf(stderr,"no positive P vals; max P was %g\n", Pmax);
        flags |= GMIX_MCMC_NOPOSITIVE;
    }

    return flags;
}


static void calc_dbydg(struct gmix_mcmc *self,
                       const double *pars,
                       size_t npars,
                       double *P,
                       double *g1, double *g2,
                       double *dbydg1, double *dbydg2,
                       long *flags)
{

    if (self->prob->type == PROB_BA13
            || self->prob->type == PROB_BA13_SHEAR) {
        struct prob_data_simple_ba *prob = 
            (struct prob_data_simple_ba *)self->prob;

        struct gmix_pars *gmix_pars=gmix_pars_new(prob->model,
                                                  pars,
                                                  npars,
                                                  SHAPE_SYSTEM_G,
                                                  flags);
        if (*flags==0) {
            *g1 = gmix_pars->shape.g1;
            *g2 = gmix_pars->shape.g2;

            dist_g_ba_dbyg_num(&prob->shape_prior,
                               &gmix_pars->shape,
                               P, dbydg1, dbydg2,
                               flags);

            gmix_pars = gmix_pars_free(gmix_pars);
        }
    } else {
        fprintf(stderr, "bad prob type: %u: %s: %d, aborting\n",
                self->prob->type, __FILE__,__LINE__);
        exit(1);
    }
}


long gmix_mcmc_calc_lensfit(struct gmix_mcmc *self)
{

    const struct mca_chain *chain = self->chain_data.chain;

    long nstep=MCA_CHAIN_NSTEPS(chain);
    size_t npars = MCA_CHAIN_NPARS(chain);

    double dbydg1=0, dbydg2=0, P=0, Pmax=0;
    double g1=0, g2=0, g1sum=0, g2sum=0, g1_sensum=0, g2_sensum=0; 
    double fac1_sum=0, fac2_sum=0;
    long nuse=0, flags=0;

    for (long i=0; i<nstep; i++) {
        const double *pars = MCA_CHAIN_PARS(chain, i);

        flags=0;
        calc_dbydg(self, pars, npars, &P, &g1, &g2, &dbydg1, &dbydg2, &flags);

        if (flags==0) {
            if (P > Pmax) {
                Pmax=P;
            }

            if (P > 0) {
                double fac1 = dbydg1/P;
                double fac2 = dbydg2/P;

                g1sum += g1;
                g2sum += g2;

                g1_sensum += g1*fac1;
                g2_sensum += g2*fac2;

                fac1_sum += fac1;
                fac2_sum += fac2;

                nuse++;
            } // P > 0
        } // flag check

    } // steps

    flags=0;

    self->nuse_lensfit = nuse;
    if (nuse > 0) {
        self->g[0] = g1sum/nuse;
        self->g[1] = g2sum/nuse;

        //                          < (<g>-g)*1/P*dP/dg >
        //self->gsens[0] = 1.0 - ( g1_sensum/nuse - self->g[0]*fac1_sum/nuse);
        //self->gsens[1] = 1.0 - ( g2_sensum/nuse - self->g[1]*fac2_sum/nuse);

        self->gsens[0] = 1.0 - ( self->g[0]*fac1_sum/nuse - g1_sensum/nuse);
        self->gsens[1] = 1.0 - ( self->g[1]*fac2_sum/nuse - g2_sensum/nuse);

    } else {
        fprintf(stderr,"no good lensfit P vals; max P was %g\n", Pmax);
        flags |= GMIX_MCMC_NOPOSITIVE;
    }

    return flags;
}



// note flags get "lost" here, you need good error messages
static double get_lnprob(const double *pars, size_t npars, const void *data)
{
    double lnprob=0, s2n_numer=0, s2n_denom=0;
    long flags=0;

    struct gmix_mcmc *self=(struct gmix_mcmc *)data;

    switch (self->prob->type) {
        case PROB_NOSPLIT_ETA:
            {
                struct prob_data_simple_gmix3_eta *prob = 
                    (struct prob_data_simple_gmix3_eta *)self->prob;

                struct gmix_pars *gmix_pars=
                    gmix_pars_new(prob->model, pars, npars, SHAPE_SYSTEM_ETA, &flags);
                if (flags != 0) {
                    lnprob = DIST_LOG_LOWVAL;
                } else {

                    prob_simple_gmix3_eta_calc(prob,
                                               self->obs_list,
                                               gmix_pars,
                                               &s2n_numer, &s2n_denom,
                                               &lnprob, &flags);

                    gmix_pars = gmix_pars_free(gmix_pars);
                }
            }
            break;
        case PROB_BA13:
            {
                struct prob_data_simple_ba *prob = 
                    (struct prob_data_simple_ba *)self->prob;

                struct gmix_pars *gmix_pars=
                    gmix_pars_new(prob->model, pars, npars, SHAPE_SYSTEM_G, &flags);
                if (flags != 0) {
                    lnprob = DIST_LOG_LOWVAL;
                } else {

                    prob_simple_ba_calc(prob,
                                        self->obs_list,
                                        gmix_pars,
                                        &s2n_numer, &s2n_denom,
                                        &lnprob, &flags);

                    gmix_pars = gmix_pars_free(gmix_pars);
                }
            }
            break;

        case PROB_BA13_SHEAR:
            {
                struct prob_data_simple_ba *prob = 
                    (struct prob_data_simple_ba *)self->prob;

                struct gmix_pars *gmix_pars=
                    gmix_pars_new(prob->model, pars, npars, SHAPE_SYSTEM_G, &flags);
                if (flags != 0) {
                    lnprob = DIST_LOG_LOWVAL;
                } else {

                    prob_simple_ba_calc_with_shear(prob,
                                                   self->obs_list,
                                                   gmix_pars,
                                                   &s2n_numer, &s2n_denom,
                                                   &lnprob, &flags);

                    gmix_pars = gmix_pars_free(gmix_pars);
                }
            }
            break;



        default:
            fprintf(stderr, "bad prob type: %u: %s: %d, aborting\n",
                    self->prob->type, __FILE__,__LINE__);
            exit(1);
    }

    return lnprob;
}

static
struct mca_chain *gmix_mcmc_guess_simple_ba13(const struct prob_data_simple_ba *prob,
                                              long nwalkers)
{
    size_t npars=6;

    struct mca_chain *guess=mca_chain_new(nwalkers,1,npars);
    struct shape shape={0};

    double val=0;
    for (size_t iwalk=0; iwalk<nwalkers; iwalk++) {
        val = dist_gauss_sample(&prob->cen1_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 0) = val; 
        val = dist_gauss_sample(&prob->cen2_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 1) = val; 


        dist_g_ba_sample(&prob->shape_prior, &shape);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 2) = shape.g1; 
        MCA_CHAIN_WPAR(guess, iwalk, 0, 3) = shape.g2; 

        val = dist_lognorm_sample(&prob->T_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 4) = val; 
        val = dist_lognorm_sample(&prob->counts_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 5) = val; 
    }

    return guess;
}


static
struct mca_chain *gmix_mcmc_guess_simple_ba13_with_shear(const struct prob_data_simple_ba *prob,
                                                         long nwalkers)
{
    size_t npars=8;

    struct mca_chain *guess=mca_chain_new(nwalkers,1,npars);
    struct shape shape={0};

    double val=0;
    for (size_t iwalk=0; iwalk<nwalkers; iwalk++) {
        val = dist_gauss_sample(&prob->cen1_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 0) = val; 
        val = dist_gauss_sample(&prob->cen2_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 1) = val; 


        dist_g_ba_sample(&prob->shape_prior, &shape);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 2) = shape.g1; 
        MCA_CHAIN_WPAR(guess, iwalk, 0, 3) = shape.g2; 

        val = dist_lognorm_sample(&prob->T_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 4) = val; 
        val = dist_lognorm_sample(&prob->counts_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 5) = val; 

        MCA_CHAIN_WPAR(guess, iwalk, 0, 6) = 0.1*srandu(); 
        MCA_CHAIN_WPAR(guess, iwalk, 0, 7) = 0.1*srandu(); 

    }

    return guess;
}


static struct mca_chain *gmix_mcmc_guess_simple_gmix3_eta(const struct prob_data_simple_gmix3_eta *prob,
                                                   long nwalkers)
{
    size_t npars=6;

    struct mca_chain *guess=mca_chain_new(nwalkers,1,npars);
    struct shape shape={0};

    double val=0;
    for (size_t iwalk=0; iwalk<nwalkers; iwalk++) {
        val = dist_gauss_sample(&prob->cen1_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 0) = val; 
        val = dist_gauss_sample(&prob->cen2_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 1) = val; 

        dist_gmix3_eta_sample(&prob->shape_prior, &shape);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 2) = shape.eta1; 
        MCA_CHAIN_WPAR(guess, iwalk, 0, 3) = shape.eta2; 

        val = dist_lognorm_sample(&prob->T_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 4) = val; 
        val = dist_lognorm_sample(&prob->counts_prior);
        MCA_CHAIN_WPAR(guess, iwalk, 0, 5) = val; 
    }

    return guess;
}


struct mca_chain *gmix_mcmc_get_guess_prior(struct gmix_mcmc *self)
{
    long nwalkers=MCA_CHAIN_NWALKERS(self->chain_data.chain);
    switch (self->prob->type) {
        case PROB_NOSPLIT_ETA:
            {
                const struct prob_data_simple_gmix3_eta *prob=
                    (struct prob_data_simple_gmix3_eta *) self->prob;
                return gmix_mcmc_guess_simple_gmix3_eta(prob, nwalkers);
            }

        case PROB_BA13:
            {
                const struct prob_data_simple_ba *prob=
                    (struct prob_data_simple_ba *) self->prob;
                return gmix_mcmc_guess_simple_ba13(prob, nwalkers);
            }

        case PROB_BA13_SHEAR:
            {
                const struct prob_data_simple_ba *prob=
                    (struct prob_data_simple_ba *) self->prob;
                return gmix_mcmc_guess_simple_ba13_with_shear(prob, nwalkers);
            }

        default:
            fprintf(stderr, "bad prob type: %u: %s: %d, aborting\n",
                    self->prob->type, __FILE__,__LINE__);
            exit(1);
    }
}

void gmix_mcmc_run_draw_prior(struct gmix_mcmc *self)
{
    struct mca_chain *guess=gmix_mcmc_get_guess_prior(self);
    gmix_mcmc_run(self, guess);
    guess=mca_chain_free(guess);
}

void gmix_mcmc_run(struct gmix_mcmc *self, struct mca_chain *guess)
{
    if (!self->obs_list) {
        fprintf(stderr,"gmix_mcmc->obs_list is not set! aborting: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    mca_run(self->chain_data.burnin_chain,
            self->chain_data.mca_a,
            guess,
            &get_lnprob,
            self);

    mca_run(self->chain_data.chain,
            self->chain_data.mca_a,
            self->chain_data.burnin_chain,
            &get_lnprob,
            self);

}


// we will also need one for multi-band processing
/*
void gmix_mcmc_run_uniform(struct gmix_mcmc *self,
                           double row, double col,
                           double T, double counts,
                           long *flags)
{
    if (!self->obs_list) {
        fprintf(stderr,"gmix_mcmc->obs_list is not set!: %s: %d\n",
                __FILE__,__LINE__);
        *flags |= GMIX_MCMC_INIT;
        return;
    }

    // need to generalize this
    long nwalkers=MCA_CHAIN_NWALKERS(self->chain_data.chain);
    struct mca_chain *guess=gmix_mcmc_guess_simple(row, col,
                                                   T, counts,
                                                   nwalkers);

    mca_run(self->chain_data.burnin_chain,
            self->chain_data.mca_a,
            guess,
            &get_lnprob,
            self);

    mca_run(self->chain_data.chain,
            self->chain_data.mca_a,
            self->chain_data.burnin_chain,
            &get_lnprob,
            self);

    guess=mca_chain_free(guess);
}
*/

struct mca_chain *gmix_mcmc_guess_simple(
        double row, double col,
        double T, double counts,
        size_t nwalkers)
{
    size_t npars=6;
    double centers[6], widths[6];

    centers[0]=row;
    centers[1]=col;
    centers[2]=0.;
    centers[3]=0.;
    centers[4]=T;
    centers[5]=counts;

    widths[0] = 0.1;
    widths[1] = 0.1;
    widths[2] = 0.05;
    widths[3] = 0.05;
    widths[4] = 0.1*centers[4];
    widths[5] = 0.1*centers[5];

    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}





struct mca_chain *gmix_mcmc_guess_turb_full(
        double row, double col,
        double T, double counts,
        size_t nwalkers)
{
    size_t ngauss=2;
    size_t npars=2*ngauss+4;

    // note alloca, stack allocated
    double *centers=alloca(npars*sizeof(double));
    double *widths=alloca(npars*sizeof(double));

    centers[0]=row;
    centers[1]=col;
    centers[2]=0.;
    centers[3]=0.;
    centers[4]=T*0.58;
    centers[5]=T*1.62;
    centers[6]=counts*0.60;
    centers[7]=counts*0.40;

    widths[0] = 0.1;
    widths[1] = 0.1;
    widths[2] = 0.05;
    widths[3] = 0.05;
    widths[4] = 0.1*centers[4];
    widths[5] = 0.1*centers[5];
    widths[6] = 0.1*centers[6];
    widths[7] = 0.1*centers[7];

    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}



// this is the more generic one
struct mca_chain *gmix_mcmc_make_guess_coellip(double *centers, 
                                               double *widths,
                                               size_t npars, 
                                               size_t nwalkers)
{
    if ( ( (npars-4) % 2 ) != 0 ) {
        fprintf(stderr,
                "gmix_mcmc error: pars are wrong size for coelliptical\n");
        exit(EXIT_FAILURE);
    }
    return mca_make_guess(centers,
                          widths,
                          npars, 
                          nwalkers);
}


void lognormal_set(struct lognormal *self, double mean, double sigma)
{
    self->mean=mean;
    self->sigma=sigma;

    double rat=(sigma*sigma)/(mean*mean);
    double logvar = log(1 + rat);

    self->logmean = log(mean) - 0.5*logvar;
    self->logivar = 1./logvar;
}

double lognormal_lnprob(
        struct lognormal *self,
        double x)
{
    double logx = log(x);
    double ldiff = logx-self->logmean;
    double chi2 = -0.5*self->logivar*ldiff*ldiff;

    return chi2 - logx;
}
double lognormal_prob(
        struct lognormal *self,
        double x)
{
    return exp(lognormal_lnprob(self,x));
}



