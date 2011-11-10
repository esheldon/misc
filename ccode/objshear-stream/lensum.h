#ifndef _lensum_HEADER
#define _lensum_HEADER

#include "defs.h"

struct lensum {
    int64 zindex;
    double weight;
    int64 totpairs;

    // get average ssh from sum(sshsum)/sum(weight)
    double sshsum;

    int64 nbin;
    int64* npair;
    double* rsum;
    double* wsum;
    double* dsum;
    double* osum;

};

struct lensums {
    size_t size;
    struct lensum* data;
};


struct lensum* lensum_new(size_t nbin);
struct lensums* lensums_new(size_t nlens, size_t nbin);

// this one we write all the data out in binary format
void lensums_write_header(size_t nlens, size_t nbin, FILE* fptr);
void lensums_write(struct lensums* lensums, FILE* fptr);

// these write the stdout
struct lensum* lensums_sum(struct lensums* lensums);
void lensums_print_sum(struct lensums* lensums);
void lensums_print_one(struct lensums* lensums, size_t index);
void lensums_print_firstlast(struct lensums* lensums);

struct lensums* lensums_delete(struct lensums* lensum);



struct lensum* lensum_new(size_t nbin);

void lensum_add(struct lensum* dest, struct lensum* src);

void lensum_write(struct lensum* lensum, FILE* fptr);
void lensum_print(struct lensum* lensum);
void lensum_clear(struct lensum* lensum);
struct lensum* lensum_delete(struct lensum* lensum);



#endif
