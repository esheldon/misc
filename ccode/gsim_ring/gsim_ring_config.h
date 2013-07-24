#ifndef _GSIM_RING_CONFIG_HEADER_GUARD
#define _GSIM_RING_CONFIG_HEADER_GUARD

#include <stdio.h>
#include "gmix.h"
#include "dist.h"
#include "shape.h"

#define GSIM_RING_MAXPARS 6
#define GSIM_RING_MAXNAME 20
#define GSIM_RING_MAXARR 100

#define GSIM_CONFIG_BAD_ARRAY 0x1

struct gsim_ring_config {

    // object pairs

    char obj_model_name[GSIM_RING_MAXNAME];
    enum gmix_model obj_model;

    char shape_prior_name[GSIM_RING_MAXNAME];
    enum dist shape_prior;
    double shape_prior_pars[GSIM_RING_MAXPARS];
    size_t shape_prior_npars;

    char T_prior_name[GSIM_RING_MAXNAME];
    enum dist T_prior;
    double T_prior_pars[GSIM_RING_MAXPARS];
    size_t T_prior_npars;

    char counts_prior_name[GSIM_RING_MAXNAME];
    enum dist counts_prior;
    double counts_prior_pars[GSIM_RING_MAXPARS];
    size_t counts_prior_npars;

    char cen_prior_name[GSIM_RING_MAXNAME];
    enum dist cen_prior;
    double cen_prior_pars[GSIM_RING_MAXPARS];
    size_t cen_prior_npars;

    // psf

    char psf_model_name[GSIM_RING_MAXNAME];
    enum gmix_model psf_model;
    double psf_T;
    struct shape psf_shape;
    double psf_s2n;

    // shear
    struct shape shear;
};

long gsim_ring_config_load(struct gsim_ring_config *self, const char *name);
void gsim_ring_config_print(const struct gsim_ring_config *self, FILE *stream);


#endif
