#ifndef _GVEC_HEADER_GUARD
#define _GVEC_HEADER_GUARD

struct gauss {
    double p;
    double row;
    double col;
    double irr;
    double irc;
    double icc;
};

struct gvec {
    size_t size;
    struct gauss* data;
};

struct gvec *gvec_new(size_t n);
struct gvec *gvec_free(struct gvec *self);
struct gvec *gvec_copy(struct gvec *self);
void gvec_print(FILE* fptr, struct gvec *self);

#endif
