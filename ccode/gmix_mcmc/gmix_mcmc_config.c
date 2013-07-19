#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "gmix.h"
#include "prob.h"
#include "dist.h"

#include "gmix_mcmc_config.h"

static void load_prior_data(struct cfg *cfg, 
                            const char *dist_key, // e.g. "shape_prior"
                            const char *dist_pars_key, // e.g. "shape_prior_pars"
                            enum dist *dist_type,
                            char *dist_name,
                            double *pars,
                            size_t *npars,
                            enum cfg_status *status,
                            long *flags)
{
    char *tstr=NULL;
    double *tpars=NULL;

    tstr = cfg_get_string(cfg,dist_key,status);
    if (*status) goto _load_prior_bail;

    *dist_type = dist_string2dist(tstr,flags);
    if (*flags) goto _load_prior_bail;

    strncpy(dist_name,tstr,GMIX_MCMC_MAXNAME);

    tpars = cfg_get_dblarr(cfg, dist_pars_key, npars, status);
    if (*status) goto _load_prior_bail;

    long npars_expected = dist_get_npars(*dist_type, flags);
    if (*npars != npars_expected) {
        fprintf(stderr,"for prior %s expected %ld pars, got %lu\n",
                dist_name, npars_expected, *npars);
        *flags= DIST_WRONG_NPARS;
        goto _load_prior_bail;
    }

    if (*npars > GMIX_MCMC_MAXPARS ) {
        fprintf(stderr,"error, prior %s has %lu pars, but GMIX_MCMC_MAXPARS is %d\n",
                dist_name, *npars, GMIX_MCMC_MAXPARS );
        *flags= DIST_WRONG_NPARS;
        goto _load_prior_bail;
    }

    memcpy(pars, tpars, (*npars)*sizeof(double));

_load_prior_bail:
    free(tstr);tstr=NULL;
    free(tpars);tpars=NULL;
}

// a giant mess of error checking
long gmix_mcmc_config_load(struct gmix_mcmc_config *self, const char *name)
{
    long flags=0;
    enum cfg_status status=0;
    char key[100];
    char *tstr=NULL;

    struct cfg *cfg=NULL;

    cfg=cfg_read(name, &status);
    if (status) {
        fprintf(stderr,"Config Error: %s\n", cfg_status_string(status));
        goto _gmix_mcmc_config_read_bail;
    }

    self->nwalkers = cfg_get_long(cfg,strcpy(key,"nwalkers"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->burnin = cfg_get_long(cfg,strcpy(key,"burnin"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->nstep = cfg_get_long(cfg,strcpy(key,"nstep"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->mca_a = cfg_get_double(cfg,strcpy(key,"mca_a"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->psf_ngauss = cfg_get_long(cfg,strcpy(key,"psf_ngauss"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->em_maxiter = cfg_get_long(cfg,strcpy(key,"em_maxiter"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->em_tol = cfg_get_double(cfg,strcpy(key,"em_tol"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    // fitmodel conversion
    tstr = cfg_get_string(cfg,strcpy(key,"fitmodel"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;
    self->fitmodel = gmix_string2model(tstr, &flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->fitmodel_name,tstr);
    free(tstr);tstr=NULL;
    self->nmodel=1;

    // prob_type conversion
    tstr = cfg_get_string(cfg,strcpy(key,"prob_type"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;
    self->prob_type = prob_string2type(tstr,&flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->prob_type_name,tstr);
    free(tstr);tstr=NULL;

    // shape_prior conversion
    load_prior_data(cfg,"shape_prior","shape_prior_pars",
                    &self->shape_prior, self->shape_prior_name,
                    self->shape_prior_pars, &self->shape_prior_npars,
                    &status, &flags);
    if (status || flags)  goto _gmix_mcmc_config_read_bail;


    // T_prior conversion
    load_prior_data(cfg,"T_prior","T_prior_pars",
                    &self->T_prior, self->T_prior_name,
                    self->T_prior_pars, &self->T_prior_npars,
                    &status, &flags);
    if (status || flags)  goto _gmix_mcmc_config_read_bail;


    // counts_prior conversion
    load_prior_data(cfg,"counts_prior","counts_prior_pars",
                    &self->counts_prior, self->counts_prior_name,
                    self->counts_prior_pars, &self->counts_prior_npars,
                    &status, &flags);
    if (status || flags)  goto _gmix_mcmc_config_read_bail;


    // cen_prior conversion
    load_prior_data(cfg,"cen_prior","cen_prior_pars",
                    &self->cen_prior, self->cen_prior_name,
                    self->cen_prior_pars, &self->cen_prior_npars,
                    &status, &flags);
    if (status || flags)  goto _gmix_mcmc_config_read_bail;


    self->npars = gmix_get_simple_npars(self->fitmodel, &flags);
    if (flags != 0) goto _gmix_mcmc_config_read_bail;

_gmix_mcmc_config_read_bail:

    cfg=cfg_free(cfg);
    free(tstr);tstr=NULL;
    if (flags | status) {
        fprintf(stderr,"Config Error for key '%s': %s\n", key,cfg_status_string(status));
    }

    return (flags | status);
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
    fprintf(stream,"npars:        %ld\n",self->npars);

    fprintf(stream,"prob_type:    %s\n", self->prob_type_name);

    fprintf(stream,"shape_prior:  %s\n", self->shape_prior_name);
    fprintf(stream,"    ");
    for (long i=0; i<self->shape_prior_npars; i++) {
        fprintf(stream,"%g ",self->shape_prior_pars[i]);
    }
    fprintf(stream,"\n");

    fprintf(stream,"T_prior:      %s\n", self->T_prior_name);
    fprintf(stream,"    ");
    for (long i=0; i<self->T_prior_npars; i++) {
        fprintf(stream,"%g ",self->T_prior_pars[i]);
    }
    fprintf(stream,"\n");


    fprintf(stream,"counts_prior: %s\n", self->counts_prior_name);
    fprintf(stream,"    ");
    for (long i=0; i<self->counts_prior_npars; i++) {
        fprintf(stream,"%g ",self->counts_prior_pars[i]);
    }
    fprintf(stream,"\n");


    fprintf(stream,"cen_prior: %s\n", self->cen_prior_name);
    fprintf(stream,"    ");
    for (long i=0; i<self->cen_prior_npars; i++) {
        fprintf(stream,"%g ",self->cen_prior_pars[i]);
    }
    fprintf(stream,"\n");


}
