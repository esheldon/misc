#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "matrix.h"
#include "defs.h"

struct vec2 *vec2_new()
{
    struct vec2 *self=calloc(1,sizeof(struct vec2));
    if (self==NULL) {
        wlog("could not allocate vec2 struct\n");
        return NULL;
    }
    return self;
}
struct vec2 *vec2_fromdata(double v1, double v2)
{
    struct vec2 *self=vec2_new();
    if (self==NULL) {
        return self;
    }

    self->v1=v1;
    self->v2=v2;
    self->norm = sqrt(v1*v1+v2*v2);
    return self;
}
struct vec2 *vec2_free(struct vec2 *self)
{
    free(self);
    return NULL;
}


void vec2_set(struct vec2 *self, double v1, double v2)
{
    self->v1=v1;
    self->v2=v2;
    self->norm = sqrt(v1*v1+v2*v2);
}

// print as column vector
void vec2_print(struct vec2 *self, FILE* fptr, const char* fmt)
{
    fprintf(fptr,fmt,self->v1);
    fprintf(fptr,"\n");
    fprintf(fptr,fmt,self->v2);
    fprintf(fptr,"\n");
}

// vec2 * scalar.  res can be the same object as vec
void vec2_sprod(struct vec2 *vec, 
                double scalar, 
                struct vec2 *res)
{
    res->v1 = vec->v1*scalar;
    res->v2 = vec->v2*scalar;
    res->norm = vec->norm*scalar;
}
// vec2 * scalar in place
void vec2_sprodi(struct vec2 *self, double scalar)
{
    self->v1 *= scalar;
    self->v2 *= scalar;
    self->norm *= scalar;
}


// vec1 + vec2.  res can be same as vec1 or vec2
void vec2_sum(struct vec2 *vec1, 
              struct vec2 *vec2, 
              struct vec2 *res)
{
    res->v1 = vec1->v1 + vec2->v1;
    res->v2 = vec1->v2 + vec2->v2;
    res->norm = sqrt(res->v1*res->v1 + res->v2*res->v2);
}
// self + vec in place
void vec2_sumi(struct vec2 *self, struct vec2 *vec)
{
    self->v1 += vec->v1;
    self->v2 += vec->v2;
    self->norm = sqrt(self->v1*self->v1 + self->v2*self->v2);
}



struct mtx2 *mtx2_new()
{
    struct mtx2 *self=calloc(1,sizeof(struct mtx2));
    if (self==NULL) {
        wlog("could not allocate mtx2 struct\n");
        return NULL;
    }
    return self;
}
struct mtx2 *mtx2_fromdata(double m11, double m12, double m22)
{
    struct mtx2 *self=mtx2_new();
    if (self==NULL) {
        return self;
    }

    self->m11=m11;
    self->m12=m12;
    self->m22=m22;
    self->det = m11*m22-m12*m12;
    return self;
}
struct mtx2 *mtx2_free(struct mtx2 *self)
{
    free(self);
    return NULL;
}


void mtx2_set(struct mtx2 *self, double m11, double m12, double m22)
{
    self->m11=m11;
    self->m12=m12;
    self->m22=m22;
    self->det = m11*m22-m12*m12;
}

int mtx2_invert(struct mtx2* self, struct mtx2 *res)
{
    int status=1;
    if (self->det == 0) {
        status=0;
    }
    res->m11 =  self->m22/self->det;
    res->m12 = -self->m12/self->det;
    res->m22 =  self->m11/self->det;
    res->det = res->m11*res->m22-res->m12*res->m12;
    return status;
}

// product M X scalar
void mtx2_sprod(struct mtx2 *mat, 
                double scalar, 
                struct mtx2 *res)
{
    res->m11 = mat->m11*scalar;
    res->m12 = mat->m12*scalar;
    res->m22 = mat->m22*scalar;
    res->det = mat->det*scalar*scalar;
}
// product M X scalar
void mtx2_sprodi(struct mtx2 *self, 
                 double scalar)
{
    self->m11 *= scalar;
    self->m12 *= scalar;
    self->m22 *= scalar;
    self->det *= scalar*scalar;
}

// sum M + M self and result can be the same object
void mtx2_sum(struct mtx2 *self, 
              struct mtx2 *mat, 
              struct mtx2 *res)
{
    res->m11 = self->m11 + mat->m11;
    res->m12 = self->m12 + mat->m12;
    res->m22 = self->m22 + mat->m22;
    res->det = res->m11*res->m22-res->m12*res->m12;
}

// sum M + M in place
void mtx2_sumi(struct mtx2 *self, 
               struct mtx2 *mat)
{
    self->m11 += mat->m11;
    self->m12 += mat->m12;
    self->m22 += mat->m22;
    self->det = self->m11*self->m22-self->m12*self->m12;
}



// product M X V vec and res must be different objects
// we can inline this eventually
void mtx2_vec2prod(struct mtx2 *mat, 
                   struct vec2 *vec, 
                   struct vec2 *res)
{
    res->v1 = mat->m11*vec->v1 + mat->m12*vec->v2;
    res->v2 = mat->m12*vec->v1 + mat->m22*vec->v2;
    res->norm = sqrt(res->v1*res->v1 + res->v2*res->v2);
}

// product M X V stored in place in vec.
void mtx2_vec2prodi(struct mtx2 *mat, 
                    struct vec2 *self)
{
    double v1, v2;
    v1 = mat->m11*self->v1 + mat->m12*self->v2;
    v2 = mat->m12*self->v1 + mat->m22*self->v2;
    self->v1 = v1;
    self->v2 = v2;
    self->norm = sqrt(v1*v1 + v2*v2);
}

void mtx2_print(struct mtx2 *self, FILE* fptr, const char* fmt)
{
    fprintf(fptr,fmt,self->m11);
    fprintf(fptr," ");
    fprintf(fptr,fmt,self->m12);
    fprintf(fptr,"\n");
    fprintf(fptr,fmt,self->m12);
    fprintf(fptr," ");
    fprintf(fptr,fmt,self->m22);
    fprintf(fptr,"\n");
}

