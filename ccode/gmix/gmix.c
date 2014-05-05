#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gmix.h"
#include "shape.h"

enum gmix_model gmix_string2model(const char *model_name, long *flags)
{
    enum gmix_model model=0;
    if (0==strcmp(model_name,"GMIX_FULL")) {
        model=GMIX_FULL;
    } else if (0==strcmp(model_name,"GMIX_COELLIP")) {
        model=GMIX_COELLIP;
    } else if (0==strcmp(model_name,"GMIX_TURB")) {
        model=GMIX_TURB;
    } else if (0==strcmp(model_name,"GMIX_EXP")) {
        model=GMIX_EXP;
    } else if (0==strcmp(model_name,"GMIX_DEV")) {
        model=GMIX_DEV;
    } else if (0==strcmp(model_name,"GMIX_BD")) {
        model=GMIX_BD;

    } else if (0==strcmp(model_name,"GMIX_EXP_SHEAR")) {
        model=GMIX_EXP_SHEAR;
    } else if (0==strcmp(model_name,"GMIX_DEV_SHEAR")) {
        model=GMIX_DEV_SHEAR;


    } else {
        *flags |= GMIX_BAD_MODEL;
    }
    return model;
}

long gmix_get_simple_npars(enum gmix_model model, long *flags)
{
    long npars=-1;
    switch (model) {
        case GMIX_EXP:
            npars=6;
            break;
        case GMIX_DEV:
            npars=6;
            break;
        case GMIX_BD:
            npars=8;
            break;
        case GMIX_TURB:
            npars=6;
            break;

        case GMIX_EXP_SHEAR:
            npars=8;
            break;
        case GMIX_DEV_SHEAR:
            npars=8;
            break;


        default:
            fprintf(stderr, "bad simple gmix model type: %u\n", model);
            *flags |= GMIX_BAD_MODEL;
            break;
    }
    return npars;

}
long gmix_get_simple_ngauss(enum gmix_model model, long *flags)
{
    long ngauss=0;
    switch (model) {

        case GMIX_EXP:
            ngauss=6;
            break;
        case GMIX_DEV:
            ngauss=10;
            break;

        case GMIX_BD:
            ngauss=16;
            break;
        case GMIX_TURB:
            ngauss=3;
            break;

        case GMIX_EXP_SHEAR:
            ngauss=6;
            break;
        case GMIX_DEV_SHEAR:
            ngauss=10;
            break;



        default:
            fprintf(stderr, "bad simple gmix model type: %u\n", model);
            *flags |= GMIX_BAD_MODEL;
            ngauss=-1;
            break;
    }

    return ngauss;
}

long gmix_get_coellip_ngauss(long npars, long *flags)
{
    long ngauss=0;

    if ( ((npars-4) % 2) != 0) {
        fprintf(stderr, "bad npars for coellip: %ld", npars);
        ngauss = -1;
        *flags |= GMIX_WRONG_NPARS; 
    } else {
        ngauss = (npars-4)/2;
    }
    return ngauss;

}
long gmix_get_full_ngauss(long npars, long *flags)
{
    long ngauss=0;

    if ( (npars % 6) != 0) {
        fprintf(stderr, "bad npars for full: %ld", npars);
        ngauss = -1;
        *flags |= GMIX_WRONG_NPARS; 
    } else {
        ngauss = npars/6;
    }
    return ngauss;

}


struct gmix* gmix_new(size_t ngauss, long *flags)
{
    struct gmix*self=NULL;
    if (ngauss == 0) {
        fprintf(stderr,"number of gaussians must be > 0\n");
        *flags |= GMIX_ZERO_GAUSS;
        return NULL;
    }

    self = calloc(1, sizeof(struct gmix));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct gmix\n");
        exit(EXIT_FAILURE);
    }

    gmix_resize(self, ngauss);
    return self;

}

void gmix_resize(struct gmix *self, size_t size)
{
    if (self->size != size) {
        self->data = realloc(self->data, size*sizeof(struct gauss2));
        if (!self->data) {
            fprintf(stderr,"error: could not allocate %lu struct gauss2, aborting\n",size);
            self=gmix_free(self);
            exit(EXIT_FAILURE);
        }
    }
    self->size=size;
}

struct gmix* gmix_new_empty_simple(enum gmix_model model, long *flags)
{
    struct gmix*self=NULL;
    long ngauss=0;

    ngauss=gmix_get_simple_ngauss(model, flags);

    if (ngauss <= 0) {
        return NULL;
    }

    self=gmix_new(ngauss,flags);
    return self;
}

struct gmix* gmix_new_empty_coellip(long npars, long *flags)
{
    struct gmix *self=NULL;
    long ngauss=0;
    ngauss=gmix_get_coellip_ngauss(npars, flags);
    if (*flags != 0 || ngauss <= 0) {
        return NULL;
    }
    self = gmix_new(ngauss, flags);
    return self;
}

struct gmix* gmix_new_empty_full(long npars, long *flags)
{
    struct gmix *self=NULL;
    long ngauss=0;
    ngauss=gmix_get_full_ngauss(npars, flags);
    if (ngauss <= 0) {
        return NULL;
    }
    self = gmix_new(ngauss, flags);
    return self;
}

static void simple_pars_to_shape(enum gmix_model model,
                                 const double *pars,
                                 size_t npars,
                                 enum shape_system system,
                                 struct shape *shape,
                                 long *flags)
{

    // just as a check on the pars
    long npars_expected = gmix_get_simple_npars(model, flags);
    if (*flags != 0) {
        goto _simple_pars_to_shapes_bail;
    }
    if (npars_expected != npars) {
        *flags |= GMIX_WRONG_NPARS;
        goto _simple_pars_to_shapes_bail;
    }

    int ret=0;
    switch (system) {
        case SHAPE_SYSTEM_ETA:
            ret=shape_set_eta(shape, pars[2], pars[3]);
            break;
        case SHAPE_SYSTEM_G:
            ret=shape_set_g(shape, pars[2], pars[3]);
            break;
        case SHAPE_SYSTEM_E:
            ret=shape_set_e(shape, pars[2], pars[3]);
            break;
        default:
            fprintf(stderr, "bad shape system: %u\n", system);
            *flags |= GMIX_BAD_MODEL;
    }

    if (ret == 0) {
        *flags |= SHAPE_RANGE_ERROR;
    }

_simple_pars_to_shapes_bail:
    return;
}

static void simple_pars_to_shape_with_shear(enum gmix_model model,
                                            const double *pars,
                                            size_t npars,
                                            enum shape_system system,
                                            struct shape *shape,
                                            struct shape *shear,
                                            long *flags)
{

    // just as a check on the pars
    long npars_expected = gmix_get_simple_npars(model, flags);
    if (*flags != 0) {
        goto _simple_pars_to_shapes_bail;
    }
    if (npars_expected != npars) {
        *flags |= GMIX_WRONG_NPARS;
        goto _simple_pars_to_shapes_bail;
    }

    int ret=0;
    switch (system) {
        case SHAPE_SYSTEM_ETA:
            ret=shape_set_eta(shape, pars[2], pars[3]);
            break;
        case SHAPE_SYSTEM_G:
            ret=shape_set_g(shape, pars[2], pars[3]);
            break;
        case SHAPE_SYSTEM_E:
            ret=shape_set_e(shape, pars[2], pars[3]);
            break;
        default:
            fprintf(stderr, "bad shape system: %u\n", system);
            *flags |= GMIX_BAD_MODEL;
    }

    if (ret == 0) {
        *flags |= SHAPE_RANGE_ERROR;
    }

    ret=shape_set_g(shear, pars[npars-2], pars[npars-1]);

    if (ret == 0) {
        *flags |= SHAPE_RANGE_ERROR;
    }

_simple_pars_to_shapes_bail:
    return;
}


static void coellip_pars_to_shape(const double *pars, size_t npars, struct shape *shape, long *flags)
{

    // just as a check on the pars
    long ngauss = gmix_get_coellip_ngauss(npars, flags);
    if (*flags != 0 || ngauss <= 0) {
        goto _coellip_pars_to_shapes_bail;
    }

    if (!shape_set_eta(shape, pars[2], pars[3])) {
        *flags |= SHAPE_RANGE_ERROR;
        goto _coellip_pars_to_shapes_bail;
    }

_coellip_pars_to_shapes_bail:
    return;
}

void gmix_pars_fill(struct gmix_pars *self,
                    const double *pars,
                    size_t npars,
                    enum shape_system system,
                    long *flags)
{
    if (self->size != npars) {
        self->data = realloc(self->data, npars*sizeof(double));
        if (self->data ==NULL) {
            fprintf(stderr,"could not allocate %ld doubles: %s: %d\n",
                    npars, __FILE__,__LINE__);
            exit(1);
        }
    }

    self->size = npars;
    memcpy(self->data, pars, npars*sizeof(double));

    // for these models we set a shape
    if (self->model==GMIX_COELLIP) {

        coellip_pars_to_shape(pars, npars, &self->shape, flags);

    } else if (self->model == GMIX_EXP   || 
               self->model == GMIX_DEV   ||
               self->model == GMIX_BD    ||
               self->model == GMIX_TURB) {

        simple_pars_to_shape(self->model, pars, npars, system, &self->shape,
                             flags);

    } else if (self->model == GMIX_EXP_SHEAR   || 
               self->model == GMIX_DEV_SHEAR) {

        simple_pars_to_shape_with_shear(self->model, pars, npars, system,
                                        &self->shape, &self->shear, flags);

    } else {
        *flags |= GMIX_BAD_MODEL;
    }

}

struct gmix_pars *gmix_pars_new(enum gmix_model model,
                                const double *pars,
                                size_t npars,
                                enum shape_system system,
                                long *flags)
{
    struct gmix_pars *self=NULL;

    self=calloc(1, sizeof(struct gmix_pars));
    if (!self) {
        fprintf(stderr,"could not allocate struct gmix_pars: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    self->model=model;
    gmix_pars_fill(self, pars, npars, system, flags);
    if (*flags != 0) {
        self=gmix_pars_free(self);
    }

    return self;
}

struct gmix_pars *gmix_pars_free(struct gmix_pars *self)
{
    if (self) {
        free(self->data);
        self->data=NULL;

        free(self);
        self=NULL;
    }
    return self;
}

void gmix_pars_print(const struct gmix_pars *self, FILE *stream)
{
    fprintf(stream,"model: %d\n", self->model);
    fprintf(stream,"shape:\n");
    shape_show(&self->shape, stream);
    fprintf(stream,"pars: ");
    for (long i=0; i<self->size; i++) {
        fprintf(stream,"%g ", self->data[i]);
    }
    fprintf(stream,"\n");
}

struct gmix* gmix_new_model(const struct gmix_pars *pars, long *flags)
{
    struct gmix *self=NULL;

    if (pars->model==GMIX_COELLIP) {
        self = gmix_new_empty_coellip(pars->size, flags);
    } else if (pars->model==GMIX_FULL) {
        self = gmix_new_empty_full(pars->size, flags);
    } else {
        self = gmix_new_empty_simple(pars->model, flags);
    }
    if (!self) {
        return NULL;
    }
    if (*flags != 0) {
        fprintf(stderr,"error making empty model\n");
        goto _gmix_new_model_bail;
    }

    gmix_fill_model(self, pars, flags);
    if (*flags) {
        fprintf(stderr,"error filling new model\n");
    }

_gmix_new_model_bail:
    if (*flags != 0) {
        self=gmix_free(self);
    }

    return self;
}

struct gmix* gmix_new_model_from_array(enum gmix_model model,
                                       const double *pars,
                                       long npars,
                                       enum shape_system system,
                                       long *flags)
{
    struct gmix *self=NULL;

    struct gmix_pars *gmix_pars=gmix_pars_new(model, pars, npars, system, flags);
    if (*flags != 0) {
        goto _gmix_new_model_from_array_bail;
    }

    self=gmix_new_model(gmix_pars, flags);

_gmix_new_model_from_array_bail:
    gmix_pars=gmix_pars_free(gmix_pars);
    return self;

}

/* no error checking except on shape */
static void fill_simple(struct gmix *self,
                              const struct gmix_pars *pars,
                              const double *Fvals, 
                              const double *pvals,
                              long *flags)
{
    double row=0, col=0;
    double T=0, T_i=0;
    double counts=0, counts_i=0;

    struct gauss2 *gauss=NULL;

    long i=0;

    row    = pars->data[0];
    col    = pars->data[1];
    T      = pars->data[4];
    counts = pars->data[5];

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];

        T_i = T*Fvals[i];
        counts_i=counts*pvals[i];

        gauss2_set(gauss,
                   counts_i,
                   row, col, 
                   (T_i/2.)*(1-pars->shape.e1), 
                   (T_i/2.)*pars->shape.e2,
                   (T_i/2.)*(1+pars->shape.e1),
                   flags);
        if (*flags) {
            return;
        }
    }

    return;
}


// can use for with and without shear because shear is at the end
static void fill_dev10(struct gmix *self, const struct gmix_pars *pars, long *flags)
{
    static const long ngauss_expected=10;

    if (pars->model != GMIX_DEV && pars->model != GMIX_DEV_SHEAR) {
        fprintf(stderr,"dev10: expected model==GMIX_DEV or GMIX_DEV_SHEAR, got %u\n",
                pars->model);
        *flags |= GMIX_BAD_MODEL;
        return;
    }
    gmix_resize(self, ngauss_expected);

    // from Hogg & Lang, normalized
    static const double Fvals[10] = 
        {2.9934935706271918e-07, 
         3.4651596338231207e-06, 
         2.4807910570562753e-05, 
         0.00014307404300535354, 
         0.000727531692982395, 
         0.003458246439442726, 
         0.0160866454407191, 
         0.077006776775654429, 
         0.41012562102501476, 
         2.9812509778548648};
    static const double pvals[10] = 
        {6.5288960012625658e-05, 
         0.00044199216814302695, 
         0.0020859587871659754, 
         0.0075913681418996841, 
         0.02260266219257237, 
         0.056532254390212859, 
         0.11939049233042602, 
         0.20969545753234975, 
         0.29254151133139222, 
         0.28905301416582552};

    fill_simple(self, pars, Fvals, pvals, flags);
}



// can use for with and without shear because shear is at the end
static void fill_exp6(struct gmix *self, const struct gmix_pars *pars, long *flags)
{
    static const long ngauss_expected=6;

    if (pars->model != GMIX_EXP && pars->model != GMIX_EXP_SHEAR) {
        fprintf(stderr,"exp6: expected model==GMIX_EXP or  GMIX_EXP_SHEAR, got %u\n",
                pars->model);
        *flags |= GMIX_BAD_MODEL;
        return;
    }
    gmix_resize(self, ngauss_expected);

    // from Hogg & Lang, normalized
    static const double Fvals[6] = 
        {0.002467115141477932, 
         0.018147435573256168, 
         0.07944063151366336, 
         0.27137669897479122, 
         0.79782256866993773, 
         2.1623306025075739};
    static const double pvals[6] = 
        {0.00061601229677880041, 
         0.0079461395724623237, 
         0.053280454055540001, 
         0.21797364640726541, 
         0.45496740582554868, 
         0.26521634184240478};

    fill_simple(self, pars, Fvals, pvals, flags);
}



void gmix_fill_turb3(struct gmix *self, const struct gmix_pars *pars, long *flags)
{
    static const long ngauss_expected=3;
    if (pars->model != GMIX_TURB) {
        fprintf(stderr,"turb3: expected model==GMIX_TURB, got %u\n", pars->model);
        *flags |= GMIX_BAD_MODEL;
        return;
    }

    gmix_resize(self, ngauss_expected);

    static const double Fvals[3] = 
        {0.5793612389470884,1.621860687127999,7.019347162356363};
    static const double pvals[3] = 
        {0.596510042804182,0.4034898268889178,1.303069003078001e-07};

    fill_simple(self, pars, Fvals, pvals, flags);
}

static void fill_coellip(struct gmix *self, const struct gmix_pars *pars, long *flags)
{
    long Tstart=0, Astart=0;
    double row=0, col=0, Ti=0, Ai=0;
    struct gauss2 *gauss=NULL;

    if (pars->model != GMIX_COELLIP) {
        fprintf(stderr,"expected model==GMIX_COELLIP, got %u\n", pars->model);
        *flags |= GMIX_BAD_MODEL;
        return;
    }

    long ngauss=gmix_get_coellip_ngauss(pars->size,flags);
    if (ngauss <= 0) {
        *flags |= GMIX_ZERO_GAUSS;
        return;
    }
    gmix_resize(self, ngauss);

    row=pars->data[0];
    col=pars->data[1];

    Tstart=4;
    Astart=Tstart+ngauss;

    for (long i=0; i<self->size; i++) {
        gauss = &self->data[i];

        Ti = pars->data[Tstart+i];
        Ai = pars->data[Astart+i];

        gauss2_set(gauss,
                   Ai,
                   row, 
                   col, 
                   (Ti/2.)*(1-pars->shape.e1),
                   (Ti/2.)*pars->shape.e2,
                   (Ti/2.)*(1+pars->shape.e1),
                   flags);
        if (*flags) {
            return;
        }
    }

    return;
}



static void fill_full(struct gmix *self, const struct gmix_pars *pars, long *flags)
{
    long ngauss=0;
    struct gauss2 *gauss=NULL;

    long i=0, beg=0;

    if (pars->model != GMIX_FULL) {
        fprintf(stderr,"expected model==GMIX_EXP, got %u\n", pars->model);
        *flags |= GMIX_BAD_MODEL;
        return;
    }

    ngauss=gmix_get_full_ngauss(pars->size,flags);
    if (ngauss <= 0) {
        *flags |= GMIX_ZERO_GAUSS;
        return;
    }
    gmix_resize(self, ngauss);

    for (i=0; i<ngauss; i++) {
        gauss = &self->data[i];

        beg = i*6;

        gauss2_set(gauss,
                   pars->data[beg+0],  // p
                   pars->data[beg+1],  // row
                   pars->data[beg+2],  // col
                   pars->data[beg+3],  // irr
                   pars->data[beg+4],  // irc
                   pars->data[beg+5],
                   flags); // icc
        if (*flags) {
            return;
        }
    }

    return;
}

static void fill_bd(struct gmix *self, const struct gmix_pars *pars, long *flags)
{
    static const long 
        npars_exp=6, npars_dev=6, ngauss_exp=6, ngauss_dev=10,
        ngauss_expected=16;

    double pars_exp[6], pars_dev[6];
    struct gmix *gmix_exp=NULL, *gmix_dev=NULL;

    if (pars->model != GMIX_BD) {
        fprintf(stderr,"BD: expected model==GMIX_BD, got %u\n", pars->model);
        *flags |= GMIX_BAD_MODEL;
        return;
    }

    gmix_resize(self, ngauss_expected);

    pars_dev[0] = pars->data[0];
    pars_dev[1] = pars->data[1];
    pars_dev[2] = pars->data[2];
    pars_dev[3] = pars->data[3];
    pars_dev[4] = pars->data[4];
    pars_dev[5] = pars->data[6];

    pars_exp[0] = pars->data[0];
    pars_exp[1] = pars->data[1];
    pars_exp[2] = pars->data[2];
    pars_exp[3] = pars->data[3];
    pars_exp[4] = pars->data[5];
    pars_exp[5] = pars->data[7];

    struct gmix_pars *gmix_pars_dev = gmix_pars_new(GMIX_DEV, pars_dev, npars_dev, pars->shape_system, flags);
    struct gmix_pars *gmix_pars_exp = gmix_pars_new(GMIX_EXP, pars_exp, npars_exp, pars->shape_system, flags);

    gmix_dev = gmix_new_model(gmix_pars_dev, flags);
    if (*flags) {
        return;
    }
    gmix_exp = gmix_new_model(gmix_pars_exp, flags);
    if (*flags) {
        return;
    }

    memcpy(self->data,
           gmix_dev->data,
           ngauss_dev*sizeof(struct gauss2));
    memcpy(self->data+ngauss_dev,
           gmix_exp->data,
           ngauss_exp*sizeof(struct gauss2));

    gmix_pars_dev = gmix_pars_free(gmix_pars_dev);
    gmix_pars_exp = gmix_pars_free(gmix_pars_exp);

    gmix_dev=gmix_free(gmix_dev);
    gmix_exp=gmix_free(gmix_exp);

    return;
}



/*
   fill is provided so we aren't constantly hitting the heap
*/
void gmix_fill_model(struct gmix *self,
                     const struct gmix_pars *pars,
                     long *flags)
{
    switch (pars->model) {
        case GMIX_EXP:
            fill_exp6(self, pars, flags);
            break;
        case GMIX_DEV:
            fill_dev10(self, pars, flags);
            break;

        case GMIX_EXP_SHEAR:
            fill_exp6(self, pars, flags);
            break;
        case GMIX_DEV_SHEAR:
            fill_dev10(self, pars, flags);
            break;


        case GMIX_BD:
            fill_bd(self, pars, flags);
            break;
        case GMIX_COELLIP:
            fill_coellip(self, pars, flags);
            break;
        case GMIX_FULL:
            fill_full(self, pars, flags);
            break;
        case GMIX_TURB:
            gmix_fill_turb3(self, pars, flags);
            break;
        default:
            fprintf(stderr, "bad simple gmix model type: %u\n", pars->model);
            *flags |= GMIX_BAD_MODEL;
            break;
    }

    return;
}



struct gmix *gmix_free(struct gmix *self)
{
    if (self) {
        free(self->data);
        self->data=NULL;
        free(self);
        self=NULL;
    }
    return self;
}


void gmix_set_dets(struct gmix *self)
{
    struct gauss2 *gauss = NULL;
    size_t i=0;
    for (i=0; i<self->size; i++) {
        gauss = &self->data[i];
        gauss->det = gauss->irr*gauss->icc - gauss->irc*gauss->irc;
    }
}

long gmix_verify(const struct gmix *self)
{
    size_t i=0;
    const struct gauss2 *gauss=NULL;

    if (!self || !self->data) {
        fprintf(stderr,"gmix is not initialized\n");
        return 0;
    }

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];
        if (gauss->det <= 0) {
            //fprintf(stderr,"gmix_verify found det: %.16g\n", gauss->det);
            return 0;
        }
    }
    return 1;
}


void gmix_copy(const struct gmix *self, struct gmix* dest, long *flags)
{
    if (dest->size != self->size) {
        fprintf(stderr,"gmix are not same size\n");
        *flags |= GMIX_MISMATCH_SIZE;
        return;
    }
    memcpy(dest->data, self->data, self->size*sizeof(struct gauss2));
    return;
}
struct gmix *gmix_new_copy(const struct gmix *self, long *flags)
{
    struct gmix *dest=NULL;

    dest=gmix_new(self->size, flags);
    if (dest) {
        memcpy(dest->data, self->data, self->size*sizeof(struct gauss2));
    }

    return dest;
}


void gmix_print(const struct gmix *self, FILE* fptr)
{
    const struct gauss2 *gauss = NULL;
    size_t i=0;
    for (i=0; i<self->size; i++) {
        gauss = &self->data[i];
        fprintf(fptr,
             "%lu p: %9.6lf row: %9.6lf col: %9.6lf " 
             "irr: %9.6lf irc: %9.6lf icc: %9.6lf\n",
             i, 
             gauss->p, gauss->row, gauss->col,
             gauss->irr,gauss->irc, gauss->icc);
    }
}

double gmix_wmomsum(const struct gmix* self)
{
    double wmom=0;
    const struct gauss2* gauss=NULL;
    size_t i=0;
    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];
        wmom += gauss->p*(gauss->irr + gauss->icc);
    }
    return wmom;
}

void gmix_get_cen(const struct gmix *self, double *row, double *col)
{
    long i=0;
    const struct gauss2 *gauss=NULL;
    double psum=0;
    (*row)=0;
    (*col)=0;

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];

        psum += gauss->p;
        (*row) += gauss->p*gauss->row;
        (*col) += gauss->p*gauss->col;
    }

    (*row) /= psum;
    (*col) /= psum;
}

void gmix_set_cen(struct gmix *self, double row, double col)
{
    long i=0;
    struct gauss2 *gauss=NULL;

    double row_cur=0, col_cur=0;
    double row_shift=0, col_shift=0;

    gmix_get_cen(self, &row_cur, &col_cur);

    row_shift = row - row_cur;
    col_shift = col - col_cur;

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];

        gauss->row += row_shift;
        gauss->col += col_shift;
    }
}


double gmix_get_T(const struct gmix *self)
{
    long i=0;
    const struct gauss2 *gauss=NULL;
    double T=0, psum=0;

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];

        T += (gauss->irr + gauss->icc)*gauss->p;
        psum += gauss->p;
    }
    T /= psum;
    return T;
}
double gmix_get_psum(const struct gmix *self)
{
    long i=0;
    const struct gauss2 *gauss=NULL;
    double psum=0;

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];

        psum += gauss->p;
    }
    return psum;
}
void gmix_set_psum(struct gmix *self, double psum)
{
    long i=0;
    double psum_cur=0, rat=0;
    struct gauss2 *gauss=NULL;

    psum_cur=gmix_get_psum(self);
    rat=psum/psum_cur;

    for (i=0; i<self->size; i++) {
        gauss=&self->data[i];

        gauss->p *= rat;
    }
}


void gmix_convolve_fill(struct gmix *self, 
                        const struct gmix *obj_gmix, 
                        const struct gmix *psf_gmix,
                        long *flags)
{
    struct gauss2 *psf=NULL, *obj=NULL, *comb=NULL;

    long ntot=0, iobj=0, ipsf=0;
    double irr=0, irc=0, icc=0, psum=0;
    double row=0, col=0;
    double psf_rowcen=0, psf_colcen=0;

    ntot = obj_gmix->size*psf_gmix->size;
    if (ntot != self->size) {
        fprintf(stderr,
                "gmix (%lu) wrong size to accept convolution %ld\n",
                self->size, ntot);
        *flags |= GMIX_MISMATCH_SIZE;
        return;
    }

    gmix_get_cen(psf_gmix, &psf_rowcen, &psf_colcen);

    for (ipsf=0; ipsf<psf_gmix->size; ipsf++) {
        psf = &psf_gmix->data[ipsf];
        psum += psf->p;
    }

    comb = self->data;
    for (iobj=0; iobj<obj_gmix->size; iobj++) {
        obj = &obj_gmix->data[iobj];

        for (ipsf=0; ipsf<psf_gmix->size; ipsf++) {
            psf = &psf_gmix->data[ipsf];

            irr = obj->irr + psf->irr;
            irc = obj->irc + psf->irc;
            icc = obj->icc + psf->icc;

            // off-center psf components shift the
            // convolved center
            row = obj->row + (psf->row-psf_rowcen);
            col = obj->col + (psf->col-psf_colcen);

            gauss2_set(comb,
                       obj->p*psf->p/psum,
                       row, col, 
                       irr, irc, icc,
                       flags);
            if (*flags) {
                return;
            }

            comb++;
        }
    }

    return;
}

struct gmix *gmix_convolve(const struct gmix *obj_gmix,
                           const struct gmix *psf_gmix,
                           long *flags)
{
    struct gmix *self=NULL;
    long ntot=0;
    ntot = obj_gmix->size*psf_gmix->size;
    self= gmix_new(ntot,flags);
    if (!self) {
        return NULL;
    }

    gmix_convolve_fill(self, obj_gmix, psf_gmix,flags);
    if (*flags) {
        self=gmix_free(self);
    }
    return self;
}






/*
struct gmix *gmix_new_coellip(const struct gmix_pars *pars, long *flags)
{
    struct gmix *self=NULL;

    self=gmix_new_empty_coellip(pars->size,flags);
    if (self) {
        gmix_fill_coellip(self, pars, flags);
        if (*flags) {
            self=gmix_free(self);
        }
    }
    return self;
}
*/



void gmix_get_totals(const struct gmix *self,
                     double *row, double *col,
                     double *irr, double *irc, double *icc,
                     double *counts)
{
    double psum=0;

    *row=0; *col=0; *irr=0; *irc=0; *icc=0; *counts=0;
    for (size_t i=0; i<self->size; i++) {
        const struct gauss2 *gauss = &self->data[i];
        (*row) += gauss->p*gauss->row;
        (*col) += gauss->p*gauss->col;

        (*irr) += gauss->p*gauss->irr;
        (*irc) += gauss->p*gauss->irc;
        (*icc) += gauss->p*gauss->icc;

        psum += gauss->p;
    }

    (*row) /= psum;
    (*col) /= psum;

    (*irr) /= psum;
    (*irc) /= psum;
    (*icc) /= psum;

    (*counts) = psum;

}


struct gmix_list *gmix_list_new(size_t size, size_t ngauss, long *flags)
{
    struct gmix_list *self=NULL;

    self=calloc(1, sizeof(struct gmix_list));
    if (self==NULL) {
        fprintf(stderr,"could not allocate struct gmix_list: %s: %d\n",
                __FILE__,__LINE__);
        exit(1);
    }

    self->data = calloc(size, sizeof(struct gmix *));
    if (self->data==NULL) {
        fprintf(stderr,"could not allocate %lu pointers to struct gmix: %s: %d\n",
                size, __FILE__,__LINE__);
        exit(1);
    }

    for (size_t i=0; i<size; i++) {
        struct gmix *tmp=gmix_new(ngauss,flags);
        self->data[i] = tmp;
        if (*flags != 0) {
            goto _gmix_list_new_bail;
        }
    }

_gmix_list_new_bail:
    if (*flags != 0) {
        self=gmix_list_free(self);
    }
    return self;
}

struct gmix_list *gmix_list_free(struct gmix_list *self)
{
    if (self) {
        if (self->data) {
            for (size_t i=0; i<self->size; i++) {
                self->data[i] = gmix_free(self->data[i]);
            }
            free(self->data);
            self->data=NULL;
        }
        free(self);
        self=NULL;
    }
    return self;
}


