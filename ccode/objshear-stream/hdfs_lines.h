#include <stdlib.h>

#include <hdfsJniHelper.h>
#include <hdfs.h>

#ifndef _HDFS_LINES
#define _HDFS_LINES


ssize_t hdfs_getline(char **lineptr, size_t *length, hdfsFile f);
int errnoFromException(jthrowable exc, JNIEnv *env, const char *method, ...);


/*
 * Read *lines* from hdfs
 */
struct hdfs_lines {
    hdfsFS fs;          /* the hadoop file system instance */
    hdfsFile file;      /* the file within the hdfs */

    size_t buffsize;    /* size of the buffer */
    size_t nread;       /* size of the latest read */
    size_t curpos;      /* where we are in reading off characters from this buffer
                           curpos=buffsize means we reached the end and need to get more
                           data from the file */
    char* buff;         /* buffer to hold the data read in a chunk of buffsize */

    // The use interacts with these objects
    char* line;         /* just the line, copied from the buffered input */
    size_t size;        /* number of characters in line before newline */
    size_t totsize;     /* total number of characters in line array */
};


struct hdfs_lines* hdfs_lines_new(hdfsFS fs, hdfsFile f, size_t buffsize);
size_t hdfs_lines_read(struct hdfs_lines* lbuff);

struct hdfs_lines* hdfs_lines_delete(struct hdfs_lines* hl);

char hdfs_lines_nextchar(struct hdfs_lines* lbuff);
void hdfs_lines_realloc(struct hdfs_lines* lbuff);
void hdfs_lines_pushchar(struct hdfs_lines* lbuff, char c);
void hdfs_lines_clear(struct hdfs_lines* lbuff);
size_t hdfs_lines_next(struct hdfs_lines* lbuff);

// these are just simple helper functions with error handling
hdfsFS hdfs_connect(void);
hdfsFile hdfs_open(hdfsFS fs, const char* url, int mode, int buffsize);

#endif
