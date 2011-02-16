struct test {
    int x;
    float y;
};

struct F8Vector {
    size_t size;
    double* data;
};

struct F8Vector* F8VectorAlloc(size_t size);
struct F8Vector* F8VectorRange(size_t size);
void F8VectorPrintSome(struct F8Vector* vec, size_t n);

