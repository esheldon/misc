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


ssize_t hdfs_getline(hdfsFile f, char **lineptr, size_t *length)
{

    if (f==NULL || f->type != INPUT) {
        fprintf(stderr, "Cannot read from a non-InputStream object!\n");
        errno = EINVAL;
        return -1;
    }

    //Get the JNIEnv* corresponding to current thread
    JNIEnv* env = getJNIEnv();
    if (env == NULL) {
        errno = EINTERNAL;
        return -1;
    }

    //Parameters
    jobject jInputStream = f->file;

    jthrowable jExc = NULL;
    jstring jstr;
    jvalue jval;

    if (invokeMethod(env, &jval, &jExc, INSTANCE, jInputStream, 
                     "org/apache/hadoop/fs/FSDataInputStream",
                     "readLine", "()Ljava/lang/String;") != 0) {
        fprintf(stderr,"Call to FSDataInputStream::readLine failed");
        errno = EINTERNAL;
        (*env)->ExceptionClear(env);
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


// convenience functions with error handling
hdfsFS hdfs_connect(void) {
    hdfsFS fs = hdfsConnect("default", 0);
    if (!fs) {
        fprintf(stderr, "Failed to connect to hdfs\n");
        exit(EXIT_FAILURE);
    } 
    return fs;
}
hdfsFile hdfs_open(hdfsFS fs, const char* url, int mode, tSize buffsize) {
    hdfsFile hdfs_file = hdfsOpenFile(fs, url, mode, buffsize, 0, 0);
    if (!hdfs_file) {
        fprintf(stderr, "Failed to open hdfs url %s\n", url);
        exit(EXIT_FAILURE);
    }
    return hdfs_file;
}

// test if the name begins with hdfs://
int is_in_hdfs(const char* name) {
    int name_len=0;
    name_len = strlen(name);
    if (name_len < 7 ) {
        return 0;
    }

    return (0==strncmp("hdfs://", name, 7));
}
