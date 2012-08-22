#ifndef _CONFIG_H_GUARD
#define _CONFIG_H_GUARD

#include <stdint.h>
enum cfg_type {
    CFG_DOUBLE=0,
    CFG_LONG,
    CFG_STRING,
    CFG_DBLARR,
    CFG_LONARR,
    CFG_STRARR,
    CFG_UNKNOWN
};
enum cfg_status_code {
    CFG_SUCCESS=0,
    CFG_PARSE_BLANK,
    CFG_PARSE_COMMENT,
    CFG_PARSE_FAILURE,
    CFG_NOT_FOUND,
    CFG_TYPE_ERROR
};
struct cfg_entry {
    char *name;
    char *type_name; // do not free
    enum cfg_type type;

    // for scalars
    double dbl;
    long lng;
    char *str;

    // for arrays
    size_t size;
    double *dblarr;
    long *lonarr;
    char **strarr;
};
struct cfg_list {
    size_t size;
    size_t capacity;
    struct cfg_entry **data;
};

#define CFG_ENTRY_NAME(entry) (entry)->name
#define CFG_ENTRY_IS_DOUBLE(entry) ( (entry)->type == CFG_DOUBLE )
#define CFG_ENTRY_DOUBLE(entry) (entry)->dbl

#define CFG_ENTRY_IS_LONG(entry) ( (entry)->type == CFG_LONG )
#define CFG_ENTRY_LONG(entry) (entry)->lng

#define CFG_ENTRY_IS_STRING(entry) ( (entry)->type == CFG_STRING )
#define CFG_ENTRY_STRING(entry) (entry)->str

#define CFG_ENTRY_IS_TYPE(entry, type) ( (entry)->type == (type) )
#define CFG_ENTRY_IS_DBLARR(entry) ( (entry)->type == CFG_DBLARR )
#define CFG_ENTRY_DBLARR(entry) (entry)->dblarr
#define CFG_ENTRY_IS_LONARR(entry) ( (entry)->type == CFG_LONARR )
#define CFG_ENTRY_LONARR(entry) (entry)->lonarr
#define CFG_ENTRY_IS_STRARR(entry) ( (entry)->type == CFG_STRARR )
#define CFG_ENTRY_STRARR(entry) (entry)->strarr

#define CFG_ENTRY_ARRSIZE(entry) (entry)->size

#define CFG_SIZE(cfg_list) (cfg_list)->size
#define CFG_CAPACITY(cfg_list) (cfg_list)->capacity
#define CFG_ENTRY(cfg_list, i) (cfg_list)->data[(i)]

const char* cfg_status_string(enum cfg_status_code status);
char **cfg_strarr_del(char **arr, size_t size);

struct cfg_list *cfg_parse(const char* filename, enum cfg_status_code *status);
struct cfg_list *cfg_list_del(struct cfg_list *list);
double cfg_get_double(const struct cfg_list *list, 
                      const char* name, 
                      enum cfg_status_code *status);
long cfg_get_long(const struct cfg_list *list, 
                  const char* name, 
                  enum cfg_status_code *status);

/* returns a copy, you must free */
char *cfg_get_string(const struct cfg_list *list, 
                     const char* name, 
                     enum cfg_status_code *status);
/* copy at most n characters into input string */
void cfg_copy_string(const struct cfg_list *list, 
                     const char* name, 
                     char *out,
                     size_t nmax,
                     enum cfg_status_code *status);


/* These return a copies, you must free the result */
double *cfg_get_dblarr(const struct cfg_list *list, 
                       const char* name, 
                       size_t *size,
                       enum cfg_status_code *status);

long *cfg_get_lonarr(const struct cfg_list *list, 
                     const char* name, 
                     size_t *size,
                     enum cfg_status_code *status);


void cfg_print(struct cfg_list *list, FILE *stream);

// You can use cfg_strarr_del convenience function to free this
char **cfg_get_strarr(const struct cfg_list *list, 
                      const char* name, 
                      size_t *size,
                      enum cfg_status_code *status);
#endif

