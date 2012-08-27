#ifndef _CONFIG_H_GUARD
#define _CONFIG_H_GUARD

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

//#define CFG_ASSIGN ':'
#define CFG_ASSIGN '='
#define CFG_ESCAPE '\\'
#define CFG_QUOTE '"'
#define CFG_COMMENT '#'
#define CFG_ARRAY_BEG '['
#define CFG_ARRAY_END ']'
#define CFG_ARRAY_SEP ','
#define CFG_CFG_BEG '{'
#define CFG_CFG_END '}'

enum cfg_field_type {
    CFG_TYPE_SCALAR,
    CFG_TYPE_ARRAY,
    CFG_TYPE_CFG,     // sub-config
    CFG_TYPE_UNKNOWN
};
enum cfg_data_type {
    CFG_ELTYPE_BARE,
    CFG_ELTYPE_QUOTED_STRING,
    CFG_ELTYPE_UNKNOWN
};
enum cfg_status {
    CFG_SUCCESS=0,
    CFG_READ_ERROR,
    CFG_PARSE_BLANK,
    CFG_PARSE_COMMENT,
    CFG_PARSE_FAILURE,
    CFG_EOF,
    CFG_NOT_FOUND,
    CFG_EMPTY,
    CFG_TYPE_ERROR
};

struct cfg_string {
    size_t size;
    char *data;
};
struct cfg_strvec {
    size_t size;
    size_t capacity;
    char **data;
};

struct cfg;
struct cfg_field {
    char *name;
    enum cfg_field_type field_type;
    enum cfg_data_type el_type;
    struct cfg_strvec *strvec;

    // for future expansion to sub fields
    //struct cfg_field *fields;
    struct cfg *sub;
};

struct cfg {
    size_t size;
    size_t capacity;
    struct cfg_field **fields;
};

#define CFG_SIZE(cfg) (cfg)->size
#define CFG_FIELD(cfg, i) (cfg)->fields[(i)]

#define CFG_FIELD_GET_DATA(field,i) (field)->strvec->data[(i)]
#define CFG_FIELD_GET_VEC(field) (field)->strvec
#define CFG_FIELD_GET_SUB(field) (field)->sub

#define CFG_FIELD_NAME(field) (field)->name
#define CFG_FIELD_TYPE(field) (field)->field_type
#define CFG_FIELD_ELTYPE(field) (field)->el_type
// if we support sub configs we will need to rewrite this
#define CFG_FIELD_SIZE(field) (field)->strvec->size


// public api
struct cfg *cfg_read(const char* filename, enum cfg_status *status);
struct cfg *cfg_del(struct cfg *self);
void cfg_print(struct cfg *self, FILE* stream);

/* 
 * Getters
 *
 * If the name is not found zero or NULL is returned and CFG_NOT_FOUND status
 * is set
 *
 * If there is a type mismatch, zero or NULL is returned and CFG_TYPE_ERROR
 * status is set
 *
 * Arrays must be extracted as C arrays, scalars as C scalars or else
 * a CFG_TYPE_ERROR is set.
 *
 * On success, status is set to CFG_SUCCESS==0
 */

double cfg_get_double(const struct cfg *self,
                      const char *name,
                      enum cfg_status *status);
double *cfg_get_dblarr(const struct cfg *self,
                       const char *name,
                       size_t *size,
                       enum cfg_status *status);
/*
long cfg_get_long(const struct cfg *list, 
                  const char* name, 
                  enum cfg_status *status);
*/
/* returns a copy, you must free! */
 
 /*
char *cfg_get_string(const struct cfg *list, 
                     const char* name, 
                     enum cfg_status *status);
                     */
/* copy at most n characters into input string */
/*
void cfg_copy_string(const struct cfg *list, 
                     const char* name, 
                     char *out,
                     size_t nmax,
                     enum cfg_status *status);
*/

/* These return a copies, you must free the result */
/*
double *cfg_get_dblarr(const struct cfg *list, 
                       const char* name, 
                       size_t *size,
                       enum cfg_status *status);

long *cfg_get_lonarr(const struct cfg *list, 
                     const char* name, 
                     size_t *size,
                     enum cfg_status *status);
*/
// You can use cfg_strarr_del convenience function to free this
/*
char **cfg_get_strarr(const struct cfg *list, 
                      const char* name, 
                      size_t *size,
                      enum cfg_status *status);

void cfg_print(struct cfg *list, FILE *stream);
*/
/* convert a status to a status string. Do not free! */
const char* cfg_status_string(enum cfg_status status);

/* convenience function to delete an array of strings */
char **cfg_strarr_del(char **arr, size_t size);
#endif

