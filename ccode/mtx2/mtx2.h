/*
   A library for length two vectors and symmetric 2x2 matrices
*/
#ifndef _MTRX2_HEADER_GUARD
#define _MTRX2_HEADER_GUARD

struct vec2 {
    double v1;
    double v2;
    double norm;
};

// symmetric 2x2 matrix
struct mtx2 {
    double m11;
    double m12;
    double m22;
    double det;
};

// vector of symmetric 2x2 matrices
struct vmtx2 {
    size_t size;
    struct mtx2* d;
};

struct vec2 *vec2_new(void);
struct vec2 *vec2_fromdata(double v1, double v2);
struct vec2 *vec2_free(struct vec2 *self);
// print as column vector
void vec2_print(struct vec2 *self, FILE* fptr, const char* fmt);
void vec2_set(struct vec2 *self, double v1, double v2);

// vec2 * scalar.  res can be the same object as vec
void vec2_sprod(struct vec2 *vec, 
                double scalar, 
                struct vec2 *res);

// vec2 * scalar in place
void vec2_sprodi(struct vec2 *self, double scalar);


// vec1 + vec2.  res can be same as vec1 or vec2
void vec2_sum(struct vec2 *vec1, 
              struct vec2 *vec2, 
              struct vec2 *res);
// self + vec in place
void vec2_sumi(struct vec2 *self, struct vec2 *vec);




struct mtx2 *mtx2_new(void);
struct mtx2 *mtx2_fromdata(double m11, double m12, double m22);
struct mtx2 *mtx2_free(struct mtx2 *self);
void mtx2_set(struct mtx2 *self, double m11, double m12, double m22);
void mtx2_print(struct mtx2 *self, FILE* fptr, const char* fmt);

// product M X scalar.  res can be the same object as mat
// can be inlined
void mtx2_sprod(struct mtx2 *mat, 
                double scalar, 
                struct mtx2 *res);
// product M X scalar in place.
// can be inlined
void mtx2_sprodi(struct mtx2 *self, double scalar);


// product M X M
// not yet implemented
void mtx2_prod(struct mtx2 *mat1, 
               struct mtx2 *mat2, 
               struct mtx2 *result);


// sum M + M self and result can be the same object
void mtx2_sum(struct mtx2 *self, 
              struct mtx2 *mat, 
              struct mtx2 *res);
// sum M + M in place
void mtx2_sumi(struct mtx2 *self, 
               struct mtx2 *mat);





// product M X V vec and res must be different objects
// we can inline this eventually
void mtx2_vec2prod(struct mtx2 *mat, 
                   struct vec2 *vec, 
                   struct vec2 *res);


// product M X V stored in place in vec.  This is slower.
void mtx2_vec2prodi(struct mtx2 *mat, 
                    struct vec2 *self);



// set the matrix elements and it's determinant
void mtx2_set(struct mtx2* self, double m11, double m12, double m22);

// invert the matrix and put the values into the input matrix
// returns 0 if determinant is zero else 1
int mtx2_invert(struct mtx2* self, struct mtx2 *res);

// new vector of matrices
// not yet implemented
struct vmtx2 *vmtx2_new(size_t n);

// sum the matrices in the vector and place them into mat
// not yet implemented
void vmtx2_sum(struct vmtx2 *self, struct mtx2 *mat);

#endif
