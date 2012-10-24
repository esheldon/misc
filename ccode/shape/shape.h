#ifndef _SHAPE_HEADER_GUARD
#define _SHAPE_HEADER_GUARD

struct shape {
    double g1;
    double g2;

    double e1;
    double e2;
};

struct shape *shape_new_e1e2(double e1, double e2);
struct shape *shape_new_g1g2(double g1, double g2);

// return NULL, use as sh=shape_free(sh);
struct shape *shape_free(struct shape *self);

// for human viewing, write both
// e1: %.16g e2: %.16g
// g1: %.16g g2: %.16g
void shape_show(struct shape *self, FILE *fptr);

// just write e1,e2 to the file
// %.16g %.16g
void shape_write(struct shape *self, FILE *fptr);


// set e1,e2, also keeping g1,g2 consistent
// returns 0 if g or e >= 1, else 1
int shape_set_e1e2(struct shape *self, double e1, double e2);

// set g1,g2, also keeping e1,e2 consistent
// returns 0 if g or e >= 1, else 1
int shape_set_g1g2(struct shape *self, double g1, double g2);

// create a new shape and place in it self+shear
// returns NULL if failure (e.g. e >= 1)
struct shape *shape_add(struct shape *self, struct shape *shear);

// return 0 if failure (e.g. e>=1)
int shape_add_inplace(struct shape *self, struct shape *shear);

#endif
