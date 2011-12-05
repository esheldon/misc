#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "healpix.h"
#include "tree.h"

#include <unistd.h>

/* 
 * build a tree in healpix pixel space. We add the index
 * of each object
 *
 * ra/dec in degress, search radius in radians
 *
 */
struct tree_node* build_tree(struct hpix* hpix, struct f64stack* ra, struct f64stack* dec, double radius_arcsec) {

    double radius_radians = radius_arcsec/3600.*D2R;
    struct tree_node* tree=NULL;

    struct i64stack* listpix = i64stack_new(0);

    assert(ra->size == dec->size);

    for (size_t i=0; i<ra->size; i++) {
        hpix_disc_intersect(
                hpix, 
                ra->data[i], dec->data[i],
                radius_radians, 
                listpix);

        int64* ptr=listpix->data;
        while (ptr < listpix->data + listpix->size) {
            tree_insert(&tree, *ptr, i);
            ptr++;
        }
    }
    listpix=i64stack_delete(listpix);

    return tree;
}

void read_radec(const char* fname, struct f64stack* ra, struct f64stack* dec) {
    double tra=0, tdec=0;
    FILE* fptr=fopen(fname, "r");
    if (fptr==NULL) {
        fprintf(stderr,"Could not open file: %s\n", fname);
        exit(EXIT_FAILURE);
    }

    while (2 == fscanf(fptr, "%lf %lf", &tra, &tdec)) {
       f64stack_push(ra, tra); 
       f64stack_push(dec, tdec); 
    }

    fclose(fptr);
    return;
}

void find_pairs(struct hpix* hpix, struct tree_node* tree, struct f64stack* ra, struct f64stack* dec) {

    double* tra  = ra->data;
    double* tdec = dec->data;
    for (size_t ind=0; i<ra->size; i++) {

        int64 hpixid = hpix_eq2pix(hpix, src->ra, src->dec);

        struct tree_node* node = tree_find(tree, hpixid);

        if (node != NULL) {
            for (size_t i=0; i<node->indices->size; i++) {
                // index into other list
                ind2 = node->indices[i];
            }
        }

        tra++;
        tdec++;
    }
}

int main(int argc, char** argv) {

    if (argc < 4) {
        fprintf(stderr,"usage: smatch radius_degrees file1 file2\n");
        fprintf(stderr,"    put smaller list first\n");
        exit(EXIT_FAILURE);
    }

    int64 nside = 64;
    double radius_arcsec = atof(argv[1]);

    struct healpix* hpix = hpix_new(nside);

    const char* file1 = argv[2];
    const char* file2 = argv[3];

    struct f64stack* ra1=f64stack_new(0);
    struct f64stack* dec1=f64stack_new(0);
    struct f64stack* ra2=f64stack_new(0);
    struct f64stack* dec2=f64stack_new(0);

    wlog("Reading %s\n", file1);
    read_radec(file1, ra1, dec1);

    wlog("memory after read...\n");
    sleep(10);

    wlog("building tree on first ra/dec list\n");
    struct tree_node* tree = build_tree(hpix, ra1, dec1, radius_arcsec);

    wlog("memory after build tree...\n");
    sleep(10);

    ra1=f64stack_delete(ra1);
    dec1=f64stack_delete(dec1);
    wlog("memory after free ra1,dec1...\n");
    sleep(10);

    wlog("Reading %s\n", file2);
    read_radec(file2, ra2, dec2);

    wlog("cleaning up\n");
    tree = tree_delete(tree);

}
