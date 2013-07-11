#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "gmix.h"
#include "prob.h"
#include "dist.h"

#include "gmix_mcmc_config.h"

struct gmix_mcmc_config *gmix_mcmc_config_read(const char *name, enum cfg_status *status)
{
    long flags=0;
    char key[100];
    char *tstr=NULL;

    struct gmix_mcmc_config *self=NULL;

    struct cfg *cfg=cfg_read(name, status);
    if (*status) {
        fprintf(stderr,"Config Error: %s\n", cfg_status_string(*status));
        goto _gmix_mcmc_config_read_bail;
    }

    self=calloc(1, sizeof(struct gmix_mcmc_config));
    if (!self)  {
        fprintf(stderr, "failed to allocate struct gmix_mcmc_config: %s: %d",
                    __FILE__,__LINE__);
        exit(1);
    }

    self->nwalkers = cfg_get_long(cfg,strcpy(key,"nwalkers"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    self->burnin = cfg_get_long(cfg,strcpy(key,"burnin"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    self->nstep = cfg_get_long(cfg,strcpy(key,"nstep"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    self->mca_a = cfg_get_double(cfg,strcpy(key,"mca_a"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    self->psf_ngauss = cfg_get_long(cfg,strcpy(key,"psf_ngauss"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    self->em_maxiter = cfg_get_long(cfg,strcpy(key,"em_maxiter"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    self->em_tol = cfg_get_double(cfg,strcpy(key,"em_tol"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;

    // fitmodel conversion
    tstr = cfg_get_string(cfg,strcpy(key,"fitmodel"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;
    self->fitmodel = gmix_string2model(tstr, &flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->fitmodel_name,tstr);
    free(tstr);tstr=NULL;

    // prob_type conversion
    tstr = cfg_get_string(cfg,strcpy(key,"prob_type"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;
    self->prob_type = prob_string2type(tstr,&flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->prob_type_name,tstr);
    free(tstr);tstr=NULL;

    // shape_prior conversion
    tstr = cfg_get_string(cfg,strcpy(key,"shape_prior"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;
    self->shape_prior = dist_string2dist(tstr,&flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->shape_prior_name,tstr);
    free(tstr);tstr=NULL;

    // T_prior conversion
    tstr = cfg_get_string(cfg,strcpy(key,"shape_prior"),status);
    tstr = cfg_get_string(cfg,strcpy(key,"T_prior"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;
    self->T_prior = dist_string2dist(tstr,&flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->T_prior_name,tstr);
    free(tstr);tstr=NULL;

    // counts_prior conversion
    tstr = cfg_get_string(cfg,strcpy(key,"counts_prior"),status);
    if (*status) goto _gmix_mcmc_config_read_bail;
    self->counts_prior = dist_string2dist(tstr,&flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->counts_prior_name,tstr);
    free(tstr);tstr=NULL;


_gmix_mcmc_config_read_bail:

    cfg=cfg_free(cfg);
    if (*status != 0 || flags != 0) {
        free(tstr);tstr=NULL;
        self=gmix_mcmc_config_free(self);
    }

    return self;
}

struct gmix_mcmc_config *gmix_mcmc_config_free(struct gmix_mcmc_config *self)
{
    free(self);
    self=NULL;
    return self;
}

void gmix_mcmc_config_print(const struct gmix_mcmc_config *self, FILE *stream)
{
    fprintf(stream,"nwalkers:     %ld\n", self->nwalkers);
    fprintf(stream,"burnin:       %ld\n", self->burnin);
    fprintf(stream,"nstep:        %ld\n", self->nstep);
    fprintf(stream,"mca_a:        %g\n", self->mca_a);
    fprintf(stream,"psf_ngauss:   %ld\n", self->psf_ngauss);
    fprintf(stream,"em_maxiter:   %ld\n", self->em_maxiter);
    fprintf(stream,"em_tol:       %g\n", self->em_tol);

    fprintf(stream,"fitmodel:     %s\n", self->fitmodel_name);
    fprintf(stream,"prob_type:    %s\n", self->prob_type_name);
    fprintf(stream,"shape_prior:  %s\n", self->shape_prior_name);
    fprintf(stream,"T_prior:      %s\n", self->T_prior_name);
    fprintf(stream,"counts_prior: %s\n", self->counts_prior_name);
}
