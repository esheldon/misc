/*
 
    Algorithm is simple:

    Start with a guess for the N gaussians.

    Then the new estimate for the gaussian weight "p" for gaussian gi is

        pnew[gi] = sum_pix( gi[pix]/gtot[pix]*imnorm[pix] )

    where imnorm is image/sum(image) and gtot[pix] is
        
        gtot[pix] = sum_gi(gi[pix]) + nsky

    and nsky is the sky/sum(image)
    
    These new p[gi] can then be used to update the mean and covariance
    as well.  To update the mean in coordinate x

        mx[gi] = sum_pix( gi[pix]/gtot[pix]*imnorm[pix]*x )/pnew[gi]
 
    where x is the pixel value in either row or column.

    Similarly for the covariance for coord x and coord y.

        cxy = sum_pix(  gi[pix]/gtot[pix]*imnorm[pix]*(x-xc)*(y-yc) )/pcen[gi]

    setting x==y gives the diagonal terms.

    Then repeat until some tolerance in the moments is achieved.

    These calculations can be done very efficiently within a single loop,
    with a pixel lookup only once per loop.
    
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gmix_em.h"
#include "fmath.h"
#include "image.h"
#include "gmix.h"
#include "mtx2.h"

void gmix_em_run(struct gmix_em* self,
                 struct image *image, 
                 struct gmix *gmix)
{
    double wmomlast=0, wmom=0;
    double sky     = IM_SKY(image);
    double counts  = image_get_counts(image);
    size_t npoints = IM_SIZE(image);

    struct gmix_em_iter *iter_struct = iter_new(gmix->size);

    iter_struct->nsky = sky/counts;
    iter_struct->psky = sky/(counts/npoints);


    wmomlast=-9999;
    self->numiter=0;
    self->flags=0;
    while (self->numiter < self->maxiter) {
        if (self->verbose > 1) gmix_print(gmix,stderr);
 
        self->flags = gmix_get_sums(self, image, gmix, iter_struct);
        if (self->flags!=0)
            goto _gmix_em_bail;

        self->flags |= gmix_em_gmix_set_fromiter(gmix, iter_struct);
        if (self->flags!=0)
            goto _gmix_em_bail;

        iter_struct->psky = iter_struct->skysum;
        iter_struct->nsky = iter_struct->psky/npoints;

        wmom = gmix_wmomsum(gmix);
        wmom /= iter_struct->psum;
        self->fdiff = fabs((wmom-wmomlast)/wmom);

        if (self->fdiff < self->tol) {
            break;
        }
        wmomlast = wmom;
        self->numiter++;
    }

_gmix_em_bail:
    if (self->maxiter == self->numiter) {
        self->flags += GMIX_EM_MAXIT;
    }
    if (self->flags!=0 && self->verbose) 
        wlog("error found at iter %lu\n", self->numiter);

    iter_struct = iter_free(iter_struct);
}
/*
 
 Find the weighted average center

 for j gaussians

     munew = sum_j( C_j^-1 p_j mu_j )/sum( C_j^-1 p_j )

 where the mus are mean vectors and the C are the covarance
 matrices.

 The following would be a lot simpler if we use vec2 and mtx2
 types in the gaussian!  Maybe some other day.

 */
int gmix_wmean_center(const struct gmix* gmix, struct vec2* mu_new)
{
    int status=1;
    struct vec2 mu_Cinvp, mu_Cinvpsum;
    struct mtx2 Cinvpsum, Cinvpsum_inv, C, Cinvp;
    size_t i=0;

    memset(&Cinvpsum,0,sizeof(struct mtx2));
    memset(&mu_Cinvpsum,0,sizeof(struct vec2));

    const struct gauss2* gauss = gmix->data;
    for (i=0; i<gmix->size; i++) {
        // p*C^-1
        mtx2_set(&C, gauss->irr, gauss->irc, gauss->icc);
        if (!mtx2_invert(&C, &Cinvp)) {
            wlog("gmix_fix_centers: zero determinant found in C\n");
            status=0;
            goto _gmix_wmean_center_bail;
        }
        mtx2_sprodi(&Cinvp, gauss->p);

        // running sum of p*C^-1
        mtx2_sumi(&Cinvpsum, &Cinvp);

        // set the center as a vec2
        vec2_set(&mu_Cinvp, gauss->row, gauss->col);
        // p*C^-1 * mu in place on mu
        mtx2_vec2prodi(&Cinvp, &mu_Cinvp);

        // running sum of p*C^-1 * mu
        vec2_sumi(&mu_Cinvpsum, &mu_Cinvp);
        gauss++;
    }

    if (!mtx2_invert(&Cinvpsum, &Cinvpsum_inv)) {
        wlog("gmix_fix_centers: zero determinant found in Cinvpsum\n");
        status=0;
        goto _gmix_wmean_center_bail;
    }

    mtx2_vec2prod(&Cinvpsum_inv, &mu_Cinvpsum, mu_new);

_gmix_wmean_center_bail:
    return status;
}

/*
 * calculate the mean covariance matrix
 *
 *   sum(p*Covar)/sum(p)
 */
void gmix_wmean_covar(const struct gmix* gmix, struct mtx2 *cov)
{
    double psum=0.0;
    struct gauss2 *gauss=gmix->data;
    struct gauss2 *end=gmix->data+gmix->size;

    mtx2_sprodi(cov, 0.0);
    
    for (; gauss != end; gauss++) {
        psum += gauss->p;
        cov->m11 += gauss->p*gauss->irr;
        cov->m12 += gauss->p*gauss->irc;
        cov->m22 += gauss->p*gauss->icc;
    }

    cov->m11 /= psum;
    cov->m12 /= psum;
    cov->m22 /= psum;
}


static void set_means(struct gmix *gmix, struct vec2 *cen)
{
    size_t i=0;
    for (i=0; i<gmix->size; i++) {
        gmix->data[i].row = cen->v1;
        gmix->data[i].col = cen->v2;
    }
}

/*
 * this could be cleaned up, some repeated code
 */
void gmix_em_cocenter_run(struct gmix_em* self,
                          struct image *image, 
                          struct gmix *gmix)
{
    double wmomlast=0, wmom=0;
    double sky     = IM_SKY(image);
    double counts  = image_get_counts(image);
    size_t npoints = IM_SIZE(image);
    struct vec2 cen_new;
    struct gmix *gcopy=NULL;
    struct gmix_em_iter *iter_struct = iter_new(gmix->size);
    long flags=0;

    gcopy = gmix_new(gmix->size, &flags);
    self->flags |= flags;
    if (self->flags!=0)
        goto _gmix_em_cocenter_bail;

    iter_struct->nsky = sky/counts;
    iter_struct->psky = sky/(counts/npoints);


    wmomlast=-9999;
    self->numiter=0;
    self->flags=0;
    while (self->numiter < self->maxiter) {
        if (self->verbose > 1) gmix_print(gmix,stderr);

        // first pass to get centers
        self->flags = gmix_get_sums(self, image, gmix, iter_struct);
        if (self->flags!=0)
            goto _gmix_em_cocenter_bail;

        // copy for getting centers only
        gmix_copy(gmix, gcopy,&flags);
        self->flags |= gmix_em_gmix_set_fromiter(gcopy, iter_struct);

        if (!gmix_wmean_center(gcopy, &cen_new)) {
            self->flags += GMIX_EM_NEGATIVE_DET_COCENTER;
            goto _gmix_em_cocenter_bail;
        }
        set_means(gmix, &cen_new);

        // now that we have fixed centers, we re-calculate everything
        self->flags = gmix_get_sums(self, image, gmix, iter_struct);
        if (self->flags!=0)
            goto _gmix_em_cocenter_bail;
 

        self->flags |= gmix_em_gmix_set_fromiter(gmix, iter_struct);
        // we only wanted to update the moments, set these back.
        // Should do with extra par in above function or something
        set_means(gmix, &cen_new);

        iter_struct->psky = iter_struct->skysum;
        iter_struct->nsky = iter_struct->psky/npoints;

        wmom = gmix_wmomsum(gmix);
        wmom /= iter_struct->psum;
        self->fdiff = fabs((wmom-wmomlast)/wmom);

        if (self->fdiff < self->tol) {
            break;
        }
        wmomlast = wmom;
        self->numiter++;
    }

_gmix_em_cocenter_bail:
    if (self->maxiter == self->numiter) {
        self->flags += GMIX_EM_MAXIT;
    }
    if (self->flags!=0 && self->verbose) wlog("error found at iter %lu\n", self->numiter);

    gcopy = gmix_free(gcopy);
    iter_struct = iter_free(iter_struct);

}


int gmix_get_sums(struct gmix_em* self,
                  struct image *image,
                  struct gmix *gmix,
                  struct gmix_em_iter* iter)
{
    int flags=0;
    double igrat=0, imnorm=0, gtot=0, wtau=0, chi2=0;
    double u=0, v=0, uv=0, u2=0, v2=0;
    size_t i=0, col=0, row=0;
    size_t nrows=IM_NROWS(image), ncols=IM_NCOLS(image);
    struct gauss2 *gauss=NULL;
    struct gmix_em_sums *sums=NULL;

    double counts=image_get_counts(image);
    gmix_em_iter_clear(iter);
    for (row=0; row<nrows; row++) {
        for (col=0; col<ncols; col++) {

            imnorm = IM_GET(image, row, col);
            imnorm /= counts;

            gtot=0;
            gauss = &gmix->data[0];
            sums = &iter->sums[0];
            for (i=0; i<gmix->size; i++) {
                if (gauss->det <= 0) {
                    if (self->verbose) wlog("found det: %.16g\n", gauss->det);
                    flags+=GMIX_EM_NEGATIVE_DET;
                    goto _gmix_get_sums_bail;
                }

                u = row-gauss->row;
                v = col-gauss->col;

                u2 = u*u; v2 = v*v; uv = u*v;

                chi2=gauss->dcc*u2 + gauss->drr*v2 - 2.0*gauss->drc*uv;

                if (chi2 < GAUSS2_EXP_MAX_CHI2) {
                    sums->gi = gauss->norm*gauss->p*expd( -0.5*chi2 );
                } else {
                    sums->gi=0;
                }



                gtot += sums->gi;

                sums->trowsum = row*sums->gi;
                sums->tcolsum = col*sums->gi;
                sums->tu2sum  = u2*sums->gi;
                sums->tuvsum  = uv*sums->gi;
                sums->tv2sum  = v2*sums->gi;

                gauss++;
                sums++;
            }
            gtot += iter->nsky;
            igrat = imnorm/gtot;
            sums = &iter->sums[0];
            for (i=0; i<gmix->size; i++) {
                // wtau is gi[pix]/gtot[pix]*imnorm[pix]
                // which is Dave's tau*imnorm = wtau
                wtau = sums->gi*igrat;  
                //wtau = sums->gi*imnorm/gtot;  

                iter->psum += wtau;
                sums->pnew += wtau;

                // row*gi/gtot*imnorm
                sums->rowsum += sums->trowsum*igrat;
                sums->colsum += sums->tcolsum*igrat;
                sums->u2sum  += sums->tu2sum*igrat;
                sums->uvsum  += sums->tuvsum*igrat;
                sums->v2sum  += sums->tv2sum*igrat;
                sums++;
            }
            iter->skysum += iter->nsky*imnorm/gtot;

        } // rows
    } // cols

_gmix_get_sums_bail:
    return flags;
}



long gmix_em_gmix_set_fromiter(struct gmix *gmix, 
                               struct gmix_em_iter* iter)
{
    long flags=0;
    struct gmix_em_sums *sums=iter->sums;
    struct gauss2 *gauss = gmix->data;
    size_t i=0;
    for (i=0; i<gmix->size; i++) {
        double p   = sums->pnew;
        double row = sums->rowsum/sums->pnew;
        double col = sums->colsum/sums->pnew;
        double irr = sums->u2sum/sums->pnew;
        double irc = sums->uvsum/sums->pnew;
        double icc = sums->v2sum/sums->pnew;

        gauss2_set(gauss,p, row, col, irr, irc, icc,&flags);

        sums++;
        gauss++;
    }

    return flags;
}


struct gmix_em_iter *iter_new(size_t ngauss)
{
    struct gmix_em_iter *self=calloc(1,sizeof(struct gmix_em_iter));
    if (self == NULL) {
        wlog("could not allocate iter struct, bailing\n");
        exit(EXIT_FAILURE);
    }

    self->ngauss=ngauss;

    self->sums = calloc(ngauss, sizeof(struct gmix_em_sums));
    if (self->sums == NULL) {
        wlog("could not allocate iter struct, bailing\n");
        exit(EXIT_FAILURE);
    }

    return self;
}

struct gmix_em_iter *iter_free(struct gmix_em_iter *self)
{
    if (self) {
        free(self->sums);
        free(self);
    }
    return NULL;
}

/* we don't clear psky or nsky or sums */
void gmix_em_iter_clear(struct gmix_em_iter *self)
{
    self->skysum=0;
    self->psum=0;
    memset(self->sums,0,self->ngauss*sizeof(struct gmix_em_sums));
}
