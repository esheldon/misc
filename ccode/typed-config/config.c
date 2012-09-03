#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "config.h"

char *cfg_names[]= {
    "double",
    "long",
    "string",
    "double[]",
    "long[]",
    "string[]"
};
char *cfg_status_names[]= {
    "SUCCESS",
    "PARSE_BLANK",
    "PARSE_COMMENT",
    "PARSE_FAILURE",
    "NOT_FOUND",
    "TYPE_ERROR"
};



/*
 * some exported helper functions
 */

/* do not free! */
const char* cfg_status_string(enum cfg_status status)
{
    int imax=sizeof(cfg_status_names)-1;

    if (status < 0 || status > imax) {
        return NULL;
    } else {
        return cfg_status_names[status];
    }
}
char **cfg_strarr_del(char **arr, size_t size)
{
    size_t i=0;
    if (arr) {
        for (i=0; i<size; i++) {
            free(arr[i]);
            arr[i]=NULL;
        }
        free(arr);
    }
    return NULL;
}

/*
 * return pointer to first non whitespace character
 */
static ssize_t find_nonwhite(const char *str)
{
    ssize_t len=strlen(str);
    ssize_t i=0, loc=-1;

    for (i=0; i<len; i++) {
        if (!isspace(str[i])) {
            loc=i;
            break;
        }
    }
    return loc;
}
static ssize_t find_white(const char *str)
{
    ssize_t len=strlen(str);
    ssize_t i=0, loc=-1;

    for (i=0; i<len; i++) {
        if (isspace(str[i])) {
            loc=i;
            break;
        }
    }
    return loc;
}

/* find white space and cut off with a null char */
static void rstrip_inplace(char *str)
{
    ssize_t i=0;
    i = find_white(str);
    if (i != -1) {
        str[i] = '\0';
    }
}

static enum cfg_type cfg_string2type(const char *type_str)
{
    enum cfg_type type;
    if (0 == strcmp(type_str, "double")) {
        type = CFG_DOUBLE;
    } else if (0 == strcmp(type_str,"long")) {
        type = CFG_LONG;
    } else if (0 == strcmp(type_str,"string")) {
        type = CFG_STRING;
    } else if (0 == strcmp(type_str,"double[]")) {
        type = CFG_DBLARR;
    } else if (0 == strcmp(type_str,"long[]")) {
        type = CFG_LONARR;
    } else if (0 == strcmp(type_str, "string[]")) {
        type = CFG_STRARR;
    } else {
        fprintf(stderr,"Unsupported type '%s'\n", type_str);
        type = CFG_UNKNOWN;
    }
    return type;
}
static struct cfg_entry *cfg_entry_new(const char *name, enum cfg_type type)
{
    struct cfg_entry *entry=NULL;

    entry=calloc(1, sizeof(struct cfg_entry));
    if (NULL == entry) {
        fprintf(stderr,"Failed to create config entry\n");
        exit(1);
    }

    entry->type = type;
    entry->type_name = cfg_names[type];//strdup(type_name);
    entry->name = strdup(name);

    return entry;
}

static struct cfg_entry *cfg_entry_del(struct cfg_entry *entry)
{
    if (entry) {
        // We always zero memory on creation, so this is OK
        free(entry->name);
        free(entry->str);
        free(entry->dblarr);
        free(entry->lonarr);

        if (entry->strarr != NULL) {
            entry->strarr = cfg_strarr_del(entry->strarr,entry->size);
        }
        free(entry);
    }
    return NULL;
}


static double extract_double(const char *str, int *status)
{
    char *endptr=NULL;
    double val=0;

    *status=0;

    endptr=(char*) str;
    val = strtod(str, &endptr);
    if (endptr == str) {
        fprintf(stderr,"Failed to convert data to a double: '%s'\n", str);
        *status=1;
    }
    return val;
}
static double *extract_dblarr(const char *str, size_t *size, int *status)
{
    const char *ptr=NULL;
    char **endptr=NULL;
    double val=0;
    double *out=NULL;
    *size=0;

    *status=0;

    ptr=str;
    endptr=(char**) &str;
    while (1) {
        val = strtod(ptr, endptr);
        if (*endptr == ptr) {
            break;
        }
        *size += 1;
        out = realloc(out, (*size)*sizeof(double));
        if (!out) {
            fprintf(stderr,"failed to allocate double array data\n");
            exit(1);
        }
        out[(*size)-1] = val;
        ptr = (const char*) *endptr;
    }

    if ((*size) == 0) {
        fprintf(stderr,"failed to convert any doubles in data section: '%s'\n", str);
        *status=1;
    }
    return out;
}
static long extract_long(const char *str, int *status)
{
    char *endptr=NULL;
    long val=0;

    *status=0;

    endptr=(char*) str;
    val = strtol(str, &endptr, 10);
    if (endptr == str) {
        fprintf(stderr,"Failed to convert data to a long: '%s'\n", str);
        *status=1;
    }
    return val;
}
static long *extract_lonarr(const char *str, size_t *size, int *status)
{
    const char *ptr=NULL; 
    char *endptr=NULL;
    long val=0;
    long *out=NULL;
    *size=0;

    *status=0;

    ptr=str;
    endptr=(char *) ptr;
    while (1) {
        val = strtol(ptr, &endptr, 10);
        if (endptr == ptr) {
            break;
        }
        *size += 1;
        out = realloc(out, (*size)*sizeof(long));
        if (!out) {
            fprintf(stderr,"failed to allocate long array data\n");
            exit(1);
        }
        out[(*size)-1] = val;
        ptr = (const char*) endptr;
    }

    if ((*size) == 0) {
        fprintf(stderr,"failed to convert any longs in data section: '%s'\n", str);
        *status=1;
    }
    return out;
}

/*
 * extract a string from between quoted and copy into str.
 * str should not be allocated before calling this function
 */
static char *extract_string(const char *data, int *status)
{
    const char *ptr1=NULL, *ptr2=NULL;
    char *output=NULL;

    *status=0;

    ptr1 = strchr(data, '"');
    if (NULL==ptr1) {
        fprintf(stderr,"Failed find opening string quote in: '%s'\n", data);
        *status=1;
        goto _cfg_extract_string_bail;
    }
    // skip past the "
    ptr1++;
    if (ptr1[0] == '\n' || ptr1[0] == '\0') {
        fprintf(stderr,"Failed to find closing string quote in: '%s'\n", data);
        *status=1;
        goto _cfg_extract_string_bail;
    }
    ptr2 = strchr(ptr1, '"');
    if (NULL==ptr2) {
        fprintf(stderr,"Failed to find closing string quote in: '%s'\n", data);
        *status=1;
        goto _cfg_extract_string_bail;
    }

    output=calloc(ptr2-ptr1+1, sizeof(char));
    strncpy(output, ptr1, ptr2-ptr1);

_cfg_extract_string_bail:
    return output;
}
static char **extract_strarr(const char *data, size_t *size, int *status)
{
    const char *ptr1=NULL, *ptr2=NULL, *str=NULL;
    char **out=NULL;

    *status=0;

    str=data;
    while (1) {
        ptr1 = strchr(str, '"');
        if (NULL==ptr1) {
            break;
        }
        // skip past the "
        ptr1++;
        if (ptr1[0] == '\n' || ptr1[0] == '\0') {
            fprintf(stderr,"Failed to find closing string quote in: '%s'\n", data);
            *status=1;
            break;
        }
        ptr2 = strchr(ptr1, '"');
        if (NULL==ptr2) {
            fprintf(stderr,"Failed to find closing string quote in: '%s'\n", data);
            *status=1;
            break;
        }

        *size += 1;
        out = realloc(out, (*size)*sizeof(char*));
        if (!out) {
            fprintf(stderr,"failed to allocate string array pointers\n");
            exit(1);
        }
        out[(*size)-1] = calloc(ptr2-ptr1+1, sizeof(char));
        if (out[(*size)-1] == NULL) {
            fprintf(stderr,"failed to allocate string array data\n");
            exit(1);
        }
        strncpy(out[(*size)-1], ptr1, ptr2-ptr1);

        str = ptr2+1;
    }

    if ((*size) == 0) {
        fprintf(stderr,"failed to convert any strings in data section: '%s'\n", str);
        *status=1;
    } else if (*status) {
        out=cfg_strarr_del(out, *size);
    }

    return out;
}
/*
 * data should be from the entry line, pointing after the equals sign
 */
static struct cfg_entry *cfg_entry_fromdata(const char *name, enum cfg_type type, const char *data)
{
    int status=0;
    struct cfg_entry *entry=NULL;
    entry = cfg_entry_new(name, type);

    switch (entry->type) {
        case CFG_DOUBLE:
            CFG_ENTRY_DOUBLE(entry) = extract_double(data, &status);
            break;
        case CFG_LONG:
            CFG_ENTRY_LONG(entry) = extract_long(data, &status);
            break;
        case CFG_STRING:
            CFG_ENTRY_STRING(entry) = extract_string(data, &status);
            break;

        case CFG_DBLARR:
            CFG_ENTRY_DBLARR(entry) = extract_dblarr(data, &CFG_ENTRY_ARRSIZE(entry), &status);
            break;
        case CFG_LONARR:
            CFG_ENTRY_LONARR(entry) = extract_lonarr(data, &CFG_ENTRY_ARRSIZE(entry), &status);
            break;
        case CFG_STRARR:
            CFG_ENTRY_STRARR(entry) = extract_strarr(data, &CFG_ENTRY_ARRSIZE(entry), &status);
            break;
        case CFG_UNKNOWN:
            fprintf(stderr,"cannot read CFG_UNKNOWN type\n");
            status=1;
            break;
    }

    if (status) {
        entry = cfg_entry_del(entry);
    }
    return entry;
}


static void cfg_entry_print(struct cfg_entry *entry, FILE *stream)
{
    size_t n=0, i=0;

    if (!entry) {
        return;
    }
    fprintf(stream,"%s ", entry->type_name);
    fprintf(stream,"%s = ", entry->name);

    if (CFG_ENTRY_IS_DOUBLE(entry)) {
        fprintf(stream,"%.16g", CFG_ENTRY_DOUBLE(entry));
    } else if (CFG_ENTRY_IS_LONG(entry)) {
        fprintf(stream,"%ld", CFG_ENTRY_LONG(entry));
    } else if (CFG_ENTRY_IS_STRING(entry)) {
        fprintf(stream,"\"%s\"", CFG_ENTRY_STRING(entry));
    } else if (CFG_ENTRY_IS_DBLARR(entry)) {
        double *darr;
        darr=CFG_ENTRY_DBLARR(entry);
        n=CFG_ENTRY_ARRSIZE(entry);
        for (i=0; i<n; i++) {
            fprintf(stream,"%.16g",darr[i]);
            if (i < (n-1))
                fprintf(stream," ");
        }
    } else if (CFG_ENTRY_IS_LONARR(entry)) {
        long *larr;
        larr=CFG_ENTRY_LONARR(entry);
        n=CFG_ENTRY_ARRSIZE(entry);
        for (i=0; i<n; i++) {
            fprintf(stream,"%ld",larr[i]);
            if (i < (n-1))
                fprintf(stream," ");
        }
    } else if (CFG_ENTRY_IS_STRARR(entry)) {
        char **sarr;
        sarr=CFG_ENTRY_STRARR(entry);
        n=CFG_ENTRY_ARRSIZE(entry);
        for (i=0; i<n; i++) {
            fprintf(stream,"\"%s\"",sarr[i]);
            if (i < (n-1))
                fprintf(stream," ");
        }
    }

    fprintf(stream,"\n");

}

/*
 * parse the line and return a config entry. return NULL in the case of failure
 * or blank/comment lines.  This is a monster of cases.
 *
 * status can be
 *   CFG_SUCCESS
 *   CFG_PARSE_BLANK
 *   CFG_PARSE_COMMENT
 *   CFG_PARSE_FAILURE
 */
static struct cfg_entry *cfg_parse_line(const char* line, enum cfg_status *status)
{
    ssize_t i=0;

    char type_str[9]={0}, name[80]={0};
    const char *str=NULL, *ptr=NULL;
    enum cfg_type type=CFG_UNKNOWN;

    struct cfg_entry *entry = NULL;

    *status=CFG_SUCCESS;

    i = find_nonwhite(line);

    if (i == -1) {
        *status=CFG_PARSE_BLANK;
        goto _cfg_parse_line_bail;
    }

    str=&line[i];
    if (str[0] == '#') {
        *status=CFG_PARSE_COMMENT;
        goto _cfg_parse_line_bail;
    }

    // if we get here, we have found a real word
    i = find_white(str);
    strncpy(type_str, str, i);

    type = cfg_string2type(type_str);
    if (type == CFG_UNKNOWN) { // error message in function call
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }

    str = &str[i];

    i = find_nonwhite(str);
    if (i == -1) {
        fprintf(stderr,"Nothing found after type declaration in entry '%s'\n", line);
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }
    str = &str[i];

    ptr = strchr(str, '=');
    if (NULL==ptr) {
        fprintf(stderr,"no assignment found in entry '%s'\n", line);
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }
    if (ptr == str) {
        fprintf(stderr,"no variable name found in entry '%s'\n", line);
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }
    
    strncpy(name, str, ptr-str);
    // cut off white space at the end with a null
    rstrip_inplace(name);

    // no data?  Note could still be whitespace
    if (ptr[1] == '\n' || ptr[1] == '\0') {
        fprintf(stderr,"no data section found in entry '%s'\n", line);
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }

    str = &ptr[1];
    i = find_nonwhite(str);
    if (i == -1) {
        fprintf(stderr,"no data section found in entry '%s'\n", line);
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }

    // name gets trimmed of whitespace in here
    entry = cfg_entry_fromdata(name, type, str);
    if (!entry) {
        fprintf(stderr,"Could not parse data section in '%s'\n", line);
        *status=CFG_PARSE_FAILURE;
        goto _cfg_parse_line_bail;
    }

_cfg_parse_line_bail:
    if (CFG_SUCCESS != *status) {
        entry=cfg_entry_del(entry);
    }

    return entry;
}


/* we might want to expose this if we add the ability to
 * write new config files
 */
static struct cfg_list *cfg_new()
{
    struct cfg_list *vec=NULL;

    vec = calloc(1, sizeof(struct cfg_list));
    if (vec == NULL) {
        fprintf(stderr,"Failed to allocate config list\n");
        exit(1);
    }
    vec->data = calloc(1, sizeof(struct cfg_entry*));
    if (vec->data == NULL) {
        fprintf(stderr,"Failed to allocate config list\n");
        exit(1);
    }
    vec->capacity=1;
    return vec;
}
/*
 * Also deletes the data for each entry.
 *
 * Usage
 *     list=cfg_del(list);
 * list gets set to NULL
 */
struct cfg_list *cfg_del(struct cfg_list *list)
{
    size_t i=0;
    if (list) {
        if (list->data) {
            for (i=0; i<list->size; i++) {
                list->data[i] = cfg_entry_del(list->data[i]);
            }
            free(list->data);
        }
        free(list);
    }
    return NULL;
}

/*
 * The config list will now own this entry, don't free!
 */
static void cfg_append(struct cfg_list *list, 
                            struct cfg_entry *entry)
{
    size_t size=0, oldcap=0;
    size = CFG_SIZE(list);
    oldcap = CFG_CAPACITY(list);

    if (size == oldcap) {
        size_t newcap=0, num_new_bytes=0;

        newcap=2*oldcap;

        list->data = \
            realloc(list->data, newcap*sizeof(struct cfg_entry*));

        if (list->data == NULL) {
            fprintf(stderr,"failed to realloc entries list\n");
            exit(1);
        }
        num_new_bytes = (newcap-oldcap)*sizeof(struct cfg_entry*);
        memset(list->data + oldcap, 0, num_new_bytes);

        CFG_CAPACITY(list) = newcap;
    }
    CFG_SIZE(list)++;
    list->data[list->size-1] = entry;
}


struct cfg_list *cfg_parse(const char* filename, enum cfg_status *status)
{
    FILE *fp=NULL;
    ssize_t nread=0;
    char *line=NULL;
    size_t len=0;
    struct cfg_entry *entry=NULL;
    struct cfg_list *list=NULL;

    fp=fopen(filename,"r");
    if (fp==NULL) {
        fprintf(stderr,"Could not open file: %s\n", filename);
        return NULL;
    }
    list = cfg_new();

    line=calloc(80,sizeof(char));
    while ((nread = getline(&line, &len, fp)) != -1) {
        entry = cfg_parse_line((const char*)line, status);
        if (*status == CFG_PARSE_FAILURE) {
            fprintf(stderr,"Could not parse config file\n");
            goto _cfg_parse_bail;
        }
        if ( ((*status) == CFG_PARSE_COMMENT) ||
             ((*status) == CFG_PARSE_BLANK) ) {
            *status=CFG_SUCCESS;
            continue;
        }
        // ownership of entry is transferred to list here
        cfg_append(list, entry);
    }
_cfg_parse_bail:
    if (*status) {
        list=cfg_del(list);
    }
    free(line);
    line=NULL;
    return list;
}

void cfg_print(struct cfg_list *list, FILE *stream)
{
    size_t i=0;
    struct cfg_entry *entry=NULL;
    if (list) {
        for (i=0; i<CFG_SIZE(list); i++) {
            entry=CFG_ENTRY(list, i);
            cfg_entry_print(entry, stream);
        }
    }
}


/* Returns a const reference to the entry if found, otherwise NULL */
static const struct cfg_entry *cfg_find(const struct cfg_list *list, const char* name)
{
    size_t i=0;
    const struct cfg_entry *entry=NULL, *tmp=NULL;

    if (!list) {
        return entry;
    }
    for (i=0; i<CFG_SIZE(list); i++) {
        tmp = CFG_ENTRY(list,i);
        if (0 == strcmp(name, CFG_ENTRY_NAME(tmp))) {
            entry=tmp;
            break;
        }
    }

    return entry;
}
/* Same as cfg_find but only return non-null if the type matches */
static const struct cfg_entry *cfg_find_type(const struct cfg_list *list, 
                                             const char* name, 
                                             enum cfg_type type,
                                             enum cfg_status *status)
{
    const struct cfg_entry *entry=NULL, *tmp=NULL;

    *status=CFG_NOT_FOUND;

    if (!list) {
        return entry;
    }

    tmp = cfg_find(list, name);
    if (tmp) {
        if (CFG_ENTRY_IS_TYPE(tmp, type)) {
            *status=CFG_SUCCESS;
            entry=tmp;
        } else {
            *status=CFG_TYPE_ERROR;
        }
    }

    return entry;
}

/* 
 * Getters
 *
 * If the name is not found zero is returned and CFG_NOT_FOUND status is set
 * If there is a type mismatch, zero is returned and CFG_TYPE_ERROR status is set
 *
 * On success, status CFG_SUCCESS==0 is returned
 */

double cfg_get_double(const struct cfg_list *list, 
                      const char* name, 
                      enum cfg_status *status)
{
    double val=0;
    const struct cfg_entry *entry=NULL;

    entry = cfg_find_type(list,name,CFG_DOUBLE,status);
    if (*status==CFG_SUCCESS) {
        val = CFG_ENTRY_DOUBLE(entry);
    }
    return val;
}
long cfg_get_long(const struct cfg_list *list, 
                  const char* name, 
                  enum cfg_status *status)
{
    long val=0;
    const struct cfg_entry *entry=NULL;

    entry = cfg_find_type(list,name,CFG_LONG,status);
    if (*status==CFG_SUCCESS) {
        val = CFG_ENTRY_LONG(entry);
    }
    return val;
}
char *cfg_get_string(const struct cfg_list *list, 
                     const char* name, 
                     enum cfg_status *status)
{
    char* val=NULL;
    const struct cfg_entry *entry=NULL;

    entry = cfg_find_type(list,name,CFG_STRING,status);
    if (*status==CFG_SUCCESS) {
        val = strdup( CFG_ENTRY_STRING(entry) );
    }
    return val;
}
void cfg_copy_string(const struct cfg_list *list, 
                     const char* name, 
                     char *out,
                     size_t n,
                     enum cfg_status *status)
{
    const char* tmp=0;
    const struct cfg_entry *entry=NULL;

    entry = cfg_find_type(list,name,CFG_STRING,status);
    if (*status==CFG_SUCCESS) {
        tmp= CFG_ENTRY_STRING(entry);
        strncpy(out, tmp, n);
    }
}


double *cfg_get_dblarr(const struct cfg_list *list, 
                       const char* name, 
                       size_t *size,
                       enum cfg_status *status)
{
    double *arr=0;
    const double *tmp=NULL;
    const struct cfg_entry *entry=NULL;

    *size=0;


    entry = cfg_find_type(list,name,CFG_DBLARR,status);
    if (*status==CFG_SUCCESS) {
        tmp =  CFG_ENTRY_DBLARR(entry);
        *size = CFG_ENTRY_ARRSIZE(entry);

        arr = calloc(*size, sizeof(double));
        if (!arr) {
            fprintf(stderr,"Could not allocate double array for field '%s'\n", name);
            exit(1);
        }

        memcpy(arr, tmp, (*size)*sizeof(double));
    }

    return arr;
}
long *cfg_get_lonarr(const struct cfg_list *list, 
                     const char* name, 
                     size_t *size,
                     enum cfg_status *status)
{
    long *arr=0;
    const long *tmp=NULL;
    const struct cfg_entry *entry=NULL;

    *size=0;

    entry = cfg_find_type(list,name,CFG_LONARR,status);
    if (*status==CFG_SUCCESS) {
        tmp =  CFG_ENTRY_LONARR(entry);
        *size = CFG_ENTRY_ARRSIZE(entry);

        arr = calloc(*size, sizeof(long));
        if (!arr) {
            fprintf(stderr,"Could not allocate long array for field '%s'\n", name);
            exit(1);
        }

        memcpy(arr, tmp, (*size)*sizeof(long));
    }


    return arr;
}

char **cfg_get_strarr(const struct cfg_list *list, 
                      const char* name, 
                      size_t *size,
                      enum cfg_status *status)
{
    char **arr=0;
    const char **tmp=NULL;
    const struct cfg_entry *entry=NULL;
    size_t i;
    *size=0;

    entry = cfg_find_type(list,name,CFG_STRARR,status);
    if (*status==CFG_SUCCESS) {
        tmp =  (const char**) CFG_ENTRY_STRARR(entry);
        *size = CFG_ENTRY_ARRSIZE(entry);

        arr=(char**) calloc(*size, sizeof(char*));
        if (!arr) {
            fprintf(stderr,"Could not allocate string array for field '%s'\n", name);
            exit(1);
        }

        for (i=0; i<(*size); i++) {
            arr[i] = strdup(tmp[i]);
            if (arr[i] == NULL) {
                fprintf(stderr,"Could not allocate string array for field '%s'\n", name);
                exit(1);
            }
        }
    }

    return arr;
}

