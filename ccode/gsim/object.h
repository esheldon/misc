#ifndef _OBJECT_HGUARD
#define _OBJECT_HGUARD

struct object {
    char model[20];
    double row;
    double col;
    double e1;
    double e2;
    double T;
    double counts;

    char psf_model[20];
    double psf_e1;
    double psf_e2;
    double psf_T;
};

int object_read_one(struct object *self, FILE *fobj);
void object_write_one(struct object *self, FILE* fobj);

#endif
