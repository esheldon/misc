#include <stdlib.h>
#include <stding.h>
#include "source.h"

#ifdef SOURCE_POFZ
#include "Vector.h"
struct scat* scat_new(size_t n_source, size_t n_zlens) {
    if (n_zlens == 0) {
        printf("scat_new: n_zlens must be > 0\n");
        exit(EXIT_FAILURE);
    }
#else
struct scat* scat_new(size_t n_source) {
#endif

    if (n_source == 0) {
        printf("scat_new: size must be > 0\n");
        exit(EXIT_FAILURE);
    }

    struct scat* scat = malloc(sizeof(struct scat));
    if (scat == NULL) {
        printf("Could not allocate struct scat\n");
        exit(EXIT_FAILURE);
    }

    scat->size = n_source;

    scat->data = malloc(n_source, sizeof(struct source));
    if (scat->data == NULL) {
        printf("Could not allocate %ld sources in scat\n", n_source);
        exit(EXIT_FAILURE);
    }

#ifdef SOURCE_POFZ

    scat->zlens = f64vector_new(n_zlens);
    if (scat->zlens== NULL) {
        printf("Could not allocate %ld zlens in scat\n", n_zlens);
        exit(EXIT_FAILURE);
    }

    struct source* src = scat->data[0];
    for (size_t i=0; i<scat->size; i++) {
        src->scinv = f64vector_new(n_zlens);

        if (scat->scinv == NULL) {
            printf("Could not allocate %ld scinv in scat[%ld]\n", n_zlens, i);
            exit(EXIT_FAILURE);
        }

        // for convenience point to zlens in each source structure
        // DONT FREE!
        src->zlens = zlens;
        src++;
    }
#endif

    return scat;
}


