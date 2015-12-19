/* error.c - interface for hashing error handling.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "xalloc.h"
#include "charset.h"
#include "error.h"

// verbose flag is defined and set in the main driver (tagger.c)
extern int verbose_flag;

static char* errors[] = {
    "",
    "faulty environment, check path and user rights",
    "memory allocation error, memory exhausted",
    "argument missing or mismatch, check args order and syntax",
    "charset conversion error, try another locale",
};

/* For all functions experiencing a non-recoverable error, are expected to set
 the global errno code and return a value indicating something went wrong.
 Then the main driver, in case of such result, outputs an error message using perror
 and exits with EXIT_FAILURE status.
*/
void raise_error(int status, char* template, ...) {
    if(verbose_flag) {
        char* str_err;
        if(errno) {
            str_err = strerror(errno);
        }
        else {
            if(status < ERROR_COUNT) str_err = errors[status];
            else str_err = errors[0];
        }
        char* msg = xmalloc(1024);
        va_list ap;
        va_start(ap, template);
        vsprintf(msg, template, ap);
        va_end(ap);
        output(stderr, msg);
        if(status == ERROR_USAGE || status == ERROR_RECOVERABLE) {
            fprintf(stderr, "\n");
        }
        else fprintf(stderr, " : %s\n", str_err);
        free(msg);
    }
    if(status != 0) {
        exit(EXIT_FAILURE);
    }
}

/* Output a message according to the state of verbose flag.
 This function is meant for debgging prpose.
*/
void trace(int flag, char* template, ...) {
   if(verbose_flag >= flag) {
        char* msg = xmalloc(1024);
        va_list ap;
        va_start(ap, template);
        vsprintf(msg, template, ap);
        va_end(ap);
        if(flag == TRACE_DEBUG) printf("DEBUG - ");
        output(stdout, msg);
        printf("\n");
        free(msg);
   }
}