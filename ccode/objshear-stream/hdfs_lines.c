/*
 * usage is similar to gnu getline in libc, except the 
 * newline character is *not* included. This is because
 * the underlying java routine strips the newline.
 *
 * // you can start NULL but you must have len=0. Internally the
 * data are created and realloced as needed.  You can also
 * pre-create the array before sending.
 *
 * char* linebuff=NULL;
 * size_t len=0;
 *
 * while( hdfs_getline(&linebuff, &len, hdfs_file) >= 0 ) {
 *     // work with line
 * }
 * free(linebuff);
 *
 *
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "hdfs_lines.h"



ssize_t hdfs_getline(char **lineptr, size_t *length, hdfsFile f)
{

    //Get the JNIEnv* corresponding to current thread
    JNIEnv* env = getJNIEnv();
    if (env == NULL) {
        errno = EINTERNAL;
        return -1;
    }

    //Error checking... make sure that this file is 'readable'
    if (f->type != INPUT) {
        fprintf(stderr, "Cannot read from a non-InputStream object!\n");
        errno = EINVAL;
        return -1;
    }

    //Parameters
    jobject jInputStream = (jobject)(f ? f->file : NULL);

    jthrowable jExc = NULL;
    jstring jstr;
    jvalue jval;

    if (invokeMethod(env, &jval, &jExc, INSTANCE, jInputStream, 
                     "org/apache/hadoop/fs/FSDataInputStream",
                     "readLine", "()Ljava/lang/String;") != 0) {
        errno = errnoFromException(jExc, env, "org.apache.hadoop.fs."
                                   "FSDataInputStream::readLine");
        return -1;
    } else {

        jstr = (jstring) jval.l;

        if (jstr == NULL) {
            return -1;
        } else {
            const char* cstr = (*env)->GetStringUTFChars(env, jstr, NULL);

            size_t read_size = strlen(cstr);
            // include 1 for null byte
            size_t full_size = read_size+1;

            if (*lineptr==NULL) {
                *lineptr = calloc(full_size, sizeof(char));
                *length = full_size;
            } else {
                if (*length < full_size) {
                    *lineptr = realloc(*lineptr, full_size);
                    *length = full_size;
                }
            }

            strncpy(*lineptr, cstr, *length);
            (*env)->ReleaseStringUTFChars(env, jstr, cstr);

            return read_size;
        }
    }
}


// I copied this from the hdfs library, it was static there too
/**
 * Helper function to translate an exception into a meaningful errno value.
 * @param exc: The exception.
 * @param env: The JNIEnv Pointer.
 * @param method: The name of the method that threw the exception. This
 * may be format string to be used in conjuction with additional arguments.
 * @return Returns a meaningful errno value if possible, or EINTERNAL if not.
 */
int errnoFromException(jthrowable exc, JNIEnv *env, const char *method, ...)
{
    va_list ap;
    int errnum = 0;
    char *excClass = NULL;

    if (exc == NULL)
        goto default_error;

    if ((excClass = classNameOfObject((jobject) exc, env)) == NULL) {
      errnum = EINTERNAL;
      goto done;
    }

    if (!strcmp(excClass, "org.apache.hadoop.security."
                "AccessControlException")) {
        errnum = EACCES;
        goto done;
    }

    if (!strcmp(excClass, "org.apache.hadoop.hdfs.protocol."
                "QuotaExceededException")) {
        errnum = EDQUOT;
        goto done;
    }

    if (!strcmp(excClass, "java.io.FileNotFoundException")) {
        errnum = ENOENT;
        goto done;
    }

    //TODO: interpret more exceptions; maybe examine exc.getMessage()

default_error:

    //Can't tell what went wrong, so just punt
    (*env)->ExceptionDescribe(env);
    fprintf(stderr, "Call to ");
    va_start(ap, method);
    vfprintf(stderr, method, ap);
    va_end(ap);
    fprintf(stderr, " failed!\n");
    errnum = EINTERNAL;

done:

    (*env)->ExceptionClear(env);

    if (excClass != NULL)
        free(excClass);

    return errnum;
}





struct hdfs_lines* hdfs_lines_new(hdfsFS fs, hdfsFile file, size_t buffsize) {
    struct hdfs_lines* l=NULL;

    if (buffsize <= 0) {
        fprintf(stderr,"buffer size must be > 0");
        exit(EXIT_FAILURE);
    }

    l = calloc(1, sizeof(struct hdfs_lines));
    if (l == NULL) {
        fprintf(stderr,"Failed to allocate line buffer");
        exit(EXIT_FAILURE);
    }


    l->buff=calloc(buffsize+1, sizeof(char));
    l->line=calloc(buffsize+1, sizeof(char));

    if (l->buff == NULL || l->line == NULL) {
        fprintf(stderr,"Failed to allocate line buffer");
        exit(EXIT_FAILURE);
    }

    l->buffsize=buffsize;
    l->totsize=buffsize;
    l->size=0;

    // means read next time we enter next()
    l->curpos=buffsize;

    l->fs = fs;
    l->file = file;

    return l;
}

// usage:  hl = hdfs_lines_delete(hl);
struct hdfs_lines* hdfs_lines_delete(struct hdfs_lines* hl) {
    if (hl != NULL) {
        free(hl->buff);
        free(hl->line);
    }
    free(hl);
    return NULL;
}


size_t hdfs_lines_read(struct hdfs_lines* hl) {
    size_t nread=0;

    nread=hdfsRead(hl->fs, hl->file, hl->buff, hl->buffsize);
    hl->curpos=0;
    hl->nread=nread;
    return nread;
}

char hdfs_lines_nextchar(struct hdfs_lines* hl) {
    size_t nread=0;
    char c='\0';

    if (hl->curpos == hl->buffsize) {
        nread = hdfs_lines_read(hl);
        if (nread == 0) {
            return c;
        }
    }
    c = hl->buff[hl->curpos];
    hl->curpos++;

    return c;
}

void hdfs_lines_realloc(struct hdfs_lines* hl) {
    size_t newsize=0;

    // realloc line to twice the size
    newsize = 2*hl->totsize;
    hl->line = realloc(hl->line, newsize);
    if (hl->line == NULL) {
        fprintf(stderr,"failed to realloce line to size %ld\n", newsize);
        exit(EXIT_FAILURE);
    }
    hl->totsize=newsize;
}
void hdfs_lines_pushchar(struct hdfs_lines* hl, char c) {
    if (hl->size == hl->totsize) {
        // make twice as big
        hdfs_lines_realloc(hl);
    }
    hl->line[hl->size] = c;
    // if we push a null char, size doesn't change
    if (c != '\0') {
        hl->size++;
    }
}

void hdfs_lines_clear(struct hdfs_lines* hl) {
    hl->size=0;
    hl->line[0] = '\0';
}

/* get characters until we hit a newline or we run out of data
 * data are read into the buffer in buffsize chunks as needed
 */
size_t hdfs_lines_next(struct hdfs_lines* hl) {
    char c='\0';
    
    hdfs_lines_clear(hl);

    while (c != '\n') {
        c = hdfs_lines_nextchar(hl);
        hdfs_lines_pushchar(hl, c);
        if (c == '\0') {
            return hl->size;
        }
    }
    hdfs_lines_pushchar(hl, '\0');

    return hl->size;
}



// convenience functions with error handling
hdfsFS hdfs_connect(void) {
    hdfsFS fs = hdfsConnect("default", 0);
    if (!fs) {
        fprintf(stderr, "Failed to connect to hdfs\n");
        exit(EXIT_FAILURE);
    } 
    return fs;
}
hdfsFile hdfs_open(hdfsFS fs, const char* url, int mode, int buffsize) {
    hdfsFile hdfs_file = hdfsOpenFile(fs, url, mode, buffsize, 0, 0);
    if (!hdfs_file) {
        fprintf(stderr, "Failed to open hdfs url %s\n", url);
        exit(EXIT_FAILURE);
    }
    return hdfs_file;
}


