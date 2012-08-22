#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/*
 * return pointer to first non whitespace character
 */
static ssize_t find_nonwhite(char *str)
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
static ssize_t find_white(char *str)
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


/*

int test_strings()
{
    int i=0;
    char delim[]=" ";
    char quote[]="\"";

    char line[] = "string[] sarr =\"hello there\" \"world\"   \"goodbye\"";
    char *type=NULL, *name=NULL, *str=NULL, *token=NULL;


    i = find_nonwhite(str);

    if (i == -1) {
        fprintf(stderr,"line is blank\n");
        return 0;
    }

    str=&line[i];
    if (str[0] == '#') {
        fprintf(stderr,"line is a comment\n");
        return 0;
    }

    type = strtok(str, " ");
    if (0 != strcmp(type,"string[]")) {
        fprintf(stderr,"expected string[] declaration, got '%s'\n", type);
        exit(1);
    }

    printf("type: '%s'\n", type);

    name = rstrip(strtok(NULL, "="));
    printf("name: '%s'\n", name);

    token = strtok(NULL, "\"");
    printf("token: '%s'\n", token);
    token = strtok(NULL, "\"");
    printf("token: '%s'\n", token);
    token = strtok(NULL, "\"");
    printf("token: '%s'\n", token);
    token = strtok(NULL, "\"");
    printf("token: '%s'\n", token);
    token = strtok(NULL, "\"");
    printf("token: '%s'\n", token);
    token = strtok(NULL, "\"");
    printf("token: '%s'\n", token);
    return 0;

    for (i=1, str=line; ; i++, str=NULL) {
        token = strtok(str, delim);
        if (token == NULL) {
            break;
        }
        printf("found token: '%s'\n", token);
        printf("line p:  %p\n", line);
        printf("token p: %p\n", token);
    }

    free(name);
    return 0;
}

int test_nums()
{
    int j=0;
    char delim[]=" ";
    char quote[]="\"";

    char line[] = " float x=3.5 2.7 1.8";
    //char line[] = "  \t  ";
    char *type=NULL, *name=NULL, *str=NULL, *token=NULL;
    int i=0;

    str=line;

    i = find_nonwhite(str);
    if (i == -1) {
        fprintf(stderr,"line is blank\n");
        return 0;
    }
    str = &line[i];

    name = rstrip(strtok(str, "="));
    printf("name: '%s'\n", name);

    token = strtok(NULL, " ");
    printf("token: '%s'\n", token);
    token = strtok(NULL, " ");
    printf("token: '%s'\n", token);
    token = strtok(NULL, " ");
    printf("token: '%s'\n", token);
    token = strtok(NULL, " ");
    printf("token: '%s'\n", token);

    free(name);
    return 0;
}
*/

enum cfg_types {
    CFG_DOUBLE,
    CFG_LONG,
    CFG_STRING,
    CFG_DBLARR,
    CFG_LONARR,
    CFG_STRARR,
    CFG_UNKNOWN
};
struct cfg_entry {
    char *name;
    char *type_name;
    enum cfg_types type;

    // for scalars
    double dbl;
    long lng;
    char *str;

    // for arrays
    size_t size;
    double *dblarr;
    long *lngarr;
    char **strarr;
};
struct cfg_entries {
    size_t size;
    size_t capacity;
    struct cfg_entry *data;
};

#define CFG_ENTRY_IS_DOUBLE(entry) ( (entry)->type == CFG_DOUBLE )
#define CFG_ENTRY_DOUBLE(entry) entry->dbl

#define CFG_ENTRY_IS_LONG(entry) ( (entry)->type == CFG_LONG )
#define CFG_ENTRY_LONG(entry) entry->lng

#define CFG_ENTRY_IS_STRING(entry) ( (entry)->type == CFG_STRING )
#define CFG_ENTRY_STRING(entry) entry->str

#define CFG_ENTRY_IS_DBLARR(entry) ( (entry)->type == CFG_DBLARR )
#define CFG_ENTRY_DBLARR(entry) entry->dblarr

#define CFG_ENTRY_ARRSIZE(entry) entry->size

enum cfg_types cfg_string2type(const char *type_str)
{
    enum cfg_types type;
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
static struct cfg_entry *cfg_entry_new(const char *name, enum cfg_types type)
{
    struct cfg_entry *entry=NULL;
    entry=calloc(1, sizeof(struct cfg_entry));
    if (NULL == entry) {
        fprintf(stderr,"Failed to create config entry\n");
        exit(1);
    }

    entry->type = type;
    entry->name = strdup(name);

    return entry;
}
static struct cfg_entry *cfg_entry_del(struct cfg_entry *entry)
{
    size_t i=0;
    if (entry) {
        // We always zero memory on creation, so this is OK
        free(entry->name);
        free(entry->type_name);
        free(entry->str);
        free(entry->dblarr);
        free(entry->lngarr);

        if (entry->strarr != NULL) {
            for (i=0; i<entry->size; i++) {
                free(entry->strarr[i]);
            }
        }
    }
    return NULL;
}


static double extract_double(char *str, int *status)
{
    char *endptr=NULL;
    double val=0;

    *status=0;

    endptr=str;
    val = strtod(str, &endptr);
    if (endptr == str) {
        fprintf(stderr,"Failed to convert data to a double: '%s'\n", str);
        *status=1;
    }
    return val;
}
static double *extract_dblarr(char *str, size_t *size, int *status)
{
    char *ptr=NULL, *endptr=NULL;
    double val=0;
    double *out=NULL;
    *size=0;

    *status=0;

    ptr=str;
    endptr=ptr;
    while (1) {
        val = strtod(ptr, &endptr);
        if (endptr == ptr) {
            break;
        }
        *size += 1;
        out = realloc(out, *size);
        if (!out) {
            fprintf(stderr,"failed to allocate string data\n");
            exit(1);
        }
        out[(*size)-1] = val;
        ptr = endptr;
    }

    if ((*size) == 0) {
        fprintf(stderr,"failed to convert any doubles in data section: '%s'\n", str);
        *status=1;
    }
    return out;
}
static long extract_long(char *str, int *status)
{
    char *endptr=NULL;
    long val=0;

    *status=0;

    endptr=str;
    val = strtol(str, &endptr, 10);
    if (endptr == str) {
        fprintf(stderr,"Failed to convert data to a long: '%s'\n", str);
        *status=1;
    }
    return val;
}
/*
 * extract a string from between quoted and copy into str.
 * str should not be allocated before calling this function
 */
static char *extract_string(char *data, int *status)
{
    char *ptr1=NULL, *ptr2=NULL;
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


/*
 * data should be from the entry line, pointing after the equals sign
 */
static struct cfg_entry *cfg_entry_fromdata(const char *name, enum cfg_types type, char *data)
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
    }

    if (status) {
        entry = cfg_entry_del(entry);
    }
    return entry;
}



/*
static void cfg_entry_clear(struct cfg_entry *entry)
{
    size_t i=0;
    if (entry) {
        // We always zero memory on creation, so this is OK
        free(entry->name);
        free(entry->type_name);
        free(entry->str);
        free(entry->dblarr);
        free(entry->larr);

        if (entry->strarr != NULL) {
            for (i=0; i<entry->size; i++) {
                free(entry->strarr[i]);
            }
        }
    }
}
*/
/*
static struct cfg_entries *cfg_entries_new()
{
    struct cfg_entries *vec=NULL;

    vec = calloc(1, sizeof(struct cfg_entries));
    if (vec == NULL) {
        fprintf(stderr,"Failed to allocate config entries\n");
        exit(1);
    }
    vec->data = calloc(1, sizeof(struct cfg_entry));
    if (vec->data == NULL) {
        fprintf(stderr,"Failed to allocate config entries\n");
        exit(1);
    }
    vec->capacity=1;
    return vec;
}
static void cfg_entries_append(struct cfg_entries *entries, struct cfg_entry *entry)
{
    if (entries->size == entries->capacity) {
        entries_realloc(entries, entries->capacity*2);
    }
}
static struct cfg_entries *cfg_entries_del(struct cfg_entries *entries)
{
    size_t i=0, j=0;
    if (entries) {
        if (entries->data) {
            for (i=0; i<entries->size; i++) {
                cfg_entry_clear(&entries->data[i]);
            }
            free(entries->data);
        }
        free(entries);
    }
    return NULL;
}
*/
static int process_line(char* line)
{
    ssize_t i=0;

    char type_str[9]={0}, name[80]={0};
    char *str=NULL, *ptr=NULL;
    enum cfg_types type=CFG_UNKNOWN;

    struct cfg_entry *entry = NULL;

    i = find_nonwhite(line);

    if (i == -1) {
        fprintf(stderr,"line is blank\n");
        return 0;
    }

    str=&line[i];
    if (str[0] == '#') {
        fprintf(stderr,"line is a comment\n");
        return 0;
    }

    // if we get here, we have found a real word
    i = find_white(str);
    strncpy(type_str, str, i);
    printf("type string: '%s'\n", type_str);

    type = cfg_string2type(type_str);
    if (type == CFG_UNKNOWN) {
        return 0;
    }
    printf("type num: %d\n", type);

    str = &str[i];

    i = find_nonwhite(str);
    if (i == -1) {
        fprintf(stderr,"Nothing found after type declaration in entry '%s'\n", line);
        return 0;
    }
    str = &str[i];

    ptr = strchr(str, '=');
    if (NULL==ptr) {
        fprintf(stderr,"no assignment found in entry '%s'\n", line);
        return 0;
    }
    if (ptr == str) {
        fprintf(stderr,"no variable name found in entry '%s'\n", line);
        return 0;
    }
    
    strncpy(name, str, ptr-str);
    // cut off white space at the end with a null
    rstrip_inplace(name);

    // no data?  Note could still be whitespace
    if (ptr[1] == '\n' || ptr[1] == '\0') {
        fprintf(stderr,"no data section found in entry '%s'\n", line);
        return 0;
    }

    str = &ptr[1];
    i = find_nonwhite(str);
    if (i == -1) {
        fprintf(stderr,"no data section found in entry '%s'\n", line);
        return 0;
    }

    // name gets trimmed of whitespace in here
    entry = cfg_entry_fromdata(name, type, str);
    if (!entry) {
        fprintf(stderr,"Could not parse line\n");
        return 0;
    }
    /*
     * below here we have memory allocation
     */
    printf("name: '%s'\n", entry->name);
    if (CFG_ENTRY_IS_DOUBLE(entry)) {
        printf("double: %.16g\n", CFG_ENTRY_DOUBLE(entry));
    } else if (CFG_ENTRY_IS_LONG(entry)) {
        printf("long: %ld\n", CFG_ENTRY_LONG(entry));
    } else if (CFG_ENTRY_IS_STRING(entry)) {
        printf("string: '%s'\n", CFG_ENTRY_STRING(entry));
    } else if (CFG_ENTRY_IS_DBLARR(entry)) {
        double *darr;
        size_t n=CFG_ENTRY_ARRSIZE(entry), i=0;
        printf("found %lu doubles\n", n);
        darr=CFG_ENTRY_DBLARR(entry);
        for (i=0; i<n; i++) {
            printf("  darr[%lu]: %.16g\n",i,darr[i]);
        }
    }

    entry=cfg_entry_del(entry);

    return 1;
}


int main(int argc, char *argv[])
{
    char dbl_line[] = " double x=3.5";
    char long_line[] = " long l = 7";
    char str_line[] = " string str = \"stuff\"";

    char dblarr_line[] = "double[] darr=3.5 2.77 8.00";
    char bad_line[] = "double[] darr=3";

    //char sarr_line[] = "string[] sarr =\"hello there\" \"world\"   \"goodbye\"";
    //char iarr_line[] = "int x= 5 8 7 10";
    //char bad_line[] = "string[] = \"hello there\" \"world\"   \"goodbye\"";
    //char bad_line[] = " string[] sarr = \"stuff\" \"things\"";

    //process_line(sarr_line);
    process_line(dbl_line);
    process_line(long_line);
    process_line(str_line);
    process_line(dblarr_line);
    process_line(bad_line);
    
    return 0;
}
