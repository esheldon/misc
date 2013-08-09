#ifndef _SHAPE_HEADER_GUARD
#define _SHAPE_HEADER_GUARD

#define SHAPE_RANGE_ERROR 0x1
#define SHAPE_MAX 0.999999999999999

enum shape_system {
    SHAPE_SYSTEM_ETA,
    SHAPE_SYSTEM_G,
    SHAPE_SYSTEM_E
};

struct shape {
    double g1;
    double g2;

    double e1;
    double e2;

    double eta1;
    double eta2;
};

//struct shape *shape_new(void);
//struct shape *shape_new_e(double e1, double e2);
//struct shape *shape_new_g(double g1, double g2);

// return NULL, use as sh=shape_free(sh);
//struct shape *shape_free(struct shape *self);

// for human viewing, write both
// e1: %.16g e2: %.16g
// g1: %.16g g2: %.16g
void shape_show(const struct shape *self, FILE *fptr);

// just write e1,e2 to the file
// %.16g %.16g
void shape_write_e(const struct shape *self, FILE *fptr);
void shape_write_g(const struct shape *self, FILE *fptr);
void shape_read_e(struct shape *self, FILE *fptr);
void shape_read_g(struct shape *self, FILE *fptr);


// set e1,e2 as well as g and eta
// returns 0 if out of domain
int shape_set_e(struct shape *self, double e1, double e2);

// set g1,g2 as well as e and eta
// returns 0 if out of domain
int shape_set_g(struct shape *self, double g1, double g2);

// set eta1,eta2 as well as g and e
int shape_set_eta(struct shape *self, double eta1, double eta2);

double shape_get_theta(const struct shape *self);

void shape_rotate(struct shape *self, double theta_radians);

// create a new shape and place in it self+shear
// returns NULL if failure (e.g. e >= 1)
//struct shape *shape_add(struct shape *self, const struct shape *shear);

// return 0 if failure (e.g. e>=1)
int shape_add_inplace(struct shape *self, const struct shape *shear);

//    jacobian of the transformation
//        |des/deo|_{-shear}

double shape_detas_by_detao_jacob(const struct shape *shape,
                                  const struct shape *shear, long *flags);
double shape_dgs_by_dgo_jacob(const struct shape *shape, const struct shape *shear);
double shape_dgs_by_dgo_jacob_num(const struct shape *shape,
                                  const struct shape *shear, long *flags);

#endif
