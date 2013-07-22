#include <stdio.h>
#include <stdlib.h>
#include "gmix.h"
#include "dist.h"
#include "shape.h"
#include "gsim_ring_config.h"

static void load_dblarr(struct cfg *cfg,
                        const char *key,
                        double *data,
                        size_t *size,
                        enum cfg_status *status,
                        long *flags)
{
    double *tdata=NULL;

    tdata = cfg_get_dblarr(cfg, key, size, status);
    if (*status) goto _load_s2n_bail;

    if (*size > GSIM_RING_MAXARR ) {
        fprintf(stderr,"error, %s %s has %lu elements, > GSIM_RING_MAXARR = %d\n",
                key, *size, GSIM_RING_MAXARR );
        *flags= GSIM_CONFIG_BAD_ARRAY;
        goto _load_dblarr_bail;
    }

    memcpy(data, tdata, (*size)*sizeof(double));

_load_prior_bail:
    free(tdata);tdata=NULL;
}


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
long gsim_ring_config_load(struct gsim_ring_config *self, const char *name)
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

    load_dblarr(cfg, strcpy(key,"s2n"), self->s2n, self->n_s2n, &status, &flags);
    if (status || flags)  goto _gmix_mcmc_config_read_bail;

    // obj model conversion
    tstr = cfg_get_string(cfg,strcpy(key,"obj_model"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;
    self->obj_model = gmix_string2model(tstr, &flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->obj_model_name,tstr);
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

    // psf model conversion
    tstr = cfg_get_string(cfg,strcpy(key,"psf_model"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;
    self->psf_model = gmix_string2model(tstr, &flags);
    if (flags) goto _gmix_mcmc_config_read_bail;
    strcpy(self->psf_model_name,tstr);
    free(tstr);tstr=NULL;

    self->psf_T = cfg_get_long(cfg,strcpy(key,"psf_T"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->psf_eta1 = cfg_get_long(cfg,strcpy(key,"psf_eta1"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;
    self->psf_eta2 = cfg_get_long(cfg,strcpy(key,"psf_eta2"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;

    self->shear1 = cfg_get_long(cfg,strcpy(key,"shear1"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;
    self->shear2 = cfg_get_long(cfg,strcpy(key,"shear2"),&status);
    if (status) goto _gmix_mcmc_config_read_bail;


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
    fprintf(stream,"s2n:\n");
    fprintf(stream,"    ");
    for (long i=0; i<self->n_s2n; i++) {
        fprintf(stream,"%g ",self->s2n[i]);
    }
    fprintf(stream,"\n");


    fprintf(stream,"obj_model:    %s (%u)\n", self->obj_model_name, self->obj_model);

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


    fprintf(stream,"cen_prior:    %s\n", self->cen_prior_name);
    fprintf(stream,"    ");
    for (long i=0; i<self->cen_prior_npars; i++) {
        fprintf(stream,"%g ",self->cen_prior_pars[i]);
    }
    fprintf(stream,"\n");

    fprintf(stream,"psf_model:    %s (%u)\n", self->psf_model_name, self->psf_model);
    fprintf(stream,"psf_T:        %g\n", self->psf_T);
    fprintf(stream,"psf_eta1:     %g\n", self->psf_eta1);
    fprintf(stream,"psf_eta2:     %g\n", self->psf_eta2);

    fprintf(stream,"shear1:       %g\n", self->shear1);
    fprintf(stream,"shear2:       %g\n", self->shear2);


}
