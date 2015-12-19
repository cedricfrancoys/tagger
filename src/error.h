/* error.c - interface for hashing error handling.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/> 
*/

#ifndef ERROR_H
#define ERROR_H 1

#include <stdarg.h>

#define ERROR_RECOVERABLE   0   // nothing to do
#define ERROR_ENV           1   // faulty environment
#define ERROR_INIT          2   // memory allocation error
#define ERROR_USAGE         3   // argument missing or mismatch 
#define ERROR_CHARSET       4   // charset conversion error

#define ERROR_COUNT         5

#define TRACE_QUIET         0
#define TRACE_NORMAL        1
#define TRACE_DEBUG         2

void raise_error(int status, char* template, ...);
void trace(int flag, char* template, ...);

#endif