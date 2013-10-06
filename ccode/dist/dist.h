/*
   A set of distributions
*/
#ifndef _PRIORS_HEADER_GUARD
#define _PRIORS_HEADER_GUARD

#define DIST_LOG_LOWVAL -9.999e9
#define DIST_LOG_MINARG 1.0e-300

#define DIST_BAD_DIST 0x1
#define DIST_WRONG_NPARS 0x2

//#include "VEC.h"

#include "shape.h"

enum dist {
    // 1d
    DIST_GAUSS,
    DIST_LOGNORMAL,
    // 2d
    DIST_G_BA,
    DIST_GMIX3_ETA 
};


struct dist_base {
    enum dist dist_type;
};

// these should always be value types, so they can be copied
// that means static sized fields
struct dist_gauss {
    enum dist dist_type;
    double mean;
    double sigma;
    double ivar;
};

struct dist_lognorm {
    enum dist dist_type;
    double mean;
    double sigma;

    double logmean;
    double logvar;
    double logsigma;
    double logivar;
};

struct dist_g_ba {
    enum dist dist_type;
    double sigma;
    double ivar;

    double maxval;
};

// 3 round gaussians centered at zero
// evaluation is easy,
//    prob = sum_i( N_i*exp( -0.5*ivar*( eta_1^2 + eta_2^2 ) )
// where N_i is amp_i/(2*PI)*ivar

struct dist_gmix3_eta_data {
    double p;
    double pnorm;
    double sigma;
    double ivar;
};

struct dist_gmix3_eta {
    enum dist dist_type;
    size_t size;
    struct dist_gmix3_eta_data data[3]; 
    double p_normalized[3];
};


enum dist dist_string2dist(const char *dist_name, long *flags);
long dist_get_npars(enum dist dist_type, long *flags);


struct dist_gauss *dist_gauss_new(double mean, double sigma);
void dist_gauss_fill(struct dist_gauss *self, double mean, double sigma);
double dist_gauss_lnprob(const struct dist_gauss *self, double x);
void dist_gauss_print(const struct dist_gauss *self, FILE *stream);
double dist_gauss_sample(const struct dist_gauss *self);

struct dist_lognorm *dist_lognorm_new(double mean, double sigma);
void dist_lognorm_fill(struct dist_lognorm *self, double mean, double sigma);
double dist_lognorm_lnprob(const struct dist_lognorm *self, double x);
void dist_lognorm_print(const struct dist_lognorm *self, FILE *stream);
double dist_lognorm_sample(const struct dist_lognorm *self);

struct dist_g_ba *dist_g_ba_new(double sigma);
void dist_g_ba_fill(struct dist_g_ba *self, double sigma);
double dist_g_ba_lnprob(const struct dist_g_ba *self, const struct shape *shape);
double dist_g_ba_prob(const struct dist_g_ba *self, const struct shape *shape);
void dist_g_ba_sample(const struct dist_g_ba *self, struct shape *shape);

double dist_g_ba_pj(const struct dist_g_ba *self,
                    const struct shape *shape,
                    const struct shape *shear,
                    long *flags);

void dist_g_ba_pqr(const struct dist_g_ba *self,
                   const struct shape *shape,
                   double *P,
                   double *Q1,
                   double *Q2,
                   double *R11,
                   double *R12,
                   double *R22);
void dist_g_ba_pqr_num(const struct dist_g_ba *self,
                       const struct shape *shape,
                       double *P,
                       double *Q1,
                       double *Q2,
                       double *R11,
                       double *R12,
                       double *R22,
                       long *flags);

void dist_g_ba_dbyg_num(const struct dist_g_ba *self,
                        const struct shape *shape,
                        double *P, double *dbydg1, double *dbydg2,
                        long *flags);

void dist_g_ba_print(const struct dist_g_ba *self, FILE *stream);

void dist_gmix3_eta_fill(struct dist_gmix3_eta *self,
                         double sigma1, double sigma2, double sigma3,
                         double p1, double p2, double p3);



struct dist_gmix3_eta *dist_gmix3_eta_new(double sigma1,
                                          double sigma2,
                                          double sigma3,
                                          double p1,
                                          double p2,
                                          double p3);
                                          
double dist_gmix3_eta_lnprob(const struct dist_gmix3_eta *self, const struct shape *shape);
double dist_gmix3_eta_prob(const struct dist_gmix3_eta *self, const struct shape *shape);
void dist_gmix3_eta_sample(const struct dist_gmix3_eta *self, struct shape *shape);

double dist_gmix3_eta_pj(const struct dist_gmix3_eta *self,
                         const struct shape *shape,
                         const struct shape *shear,
                         long *flags);

void dist_gmix3_eta_pqr(const struct dist_gmix3_eta *self,
                        const struct shape *shape,
                        double *P,
                        double *Q1,
                        double *Q2,
                        double *R11,
                        double *R12,
                        double *R22,
                        long *flags);

void dist_gmix3_eta_print(const struct dist_gmix3_eta *self, FILE *stream);


#endif

