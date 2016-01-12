/* xalloc.c - interface for memory allocation with error checking.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
    
    Following functions are adapted from implementations defined in xmalloc.c
    released under GPL 3 by Free Software Foundation, Inc. 
    See credits below. 
*/

/*   
   xmalloc.c -- malloc with out of memory checking   

    Copyright (C) 1990-2000, 2002-2006, 2008-2011 Free Software Foundation, Inc.
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>   
*/
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xalloc.h"

/* Allocate N bytes of memory dynamically, with error checking.  
*/
void* xmalloc (size_t n) {
    void *p = malloc(n);
    if (!p && n != 0) {
        fprintf(stderr, "malloc failed, memory exhausted\n");
        abort ();
    }
    return p;
}

/* Allocate S bytes of zeroed memory dynamically, with error checking.
   There's no need for xnzalloc (N, S), since it would be equivalent
   to xcalloc (N, S).  */
void *xzalloc (size_t s) {
  return memset (xmalloc (s), 0, s);
}

/* Allocate zeroed memory for N elements of S bytes, with error
 checking.  S must be nonzero.  
*/
void* xcalloc (size_t n, size_t s) {
    void *p;
    /* Test for overflow, since some calloc implementations don't have
     proper overflow checks. */
     
    if(xalloc_oversized (n, s)
      || (! (p = calloc (n, s)) && (n != 0))) {
        fprintf(stderr, "calloc failed, memory exhausted\n");
        abort ();        
    }
    return p;
}

/* Change the size of an allocated block of memory P to N bytes,
 with error checking.  
*/
void* xrealloc (void *p, size_t n) {
    if (!n && p) {
        /* The GNU and C99 realloc behaviors disagree here.  Act like
        GNU, even if the underlying realloc is C99.  */
        free (p);
        return NULL;
    }
    p = realloc (p, n);
    if (!p && n != 0) {
        fprintf(stderr, "realloc failed, memory exhausted\n");
        abort ();
    }
    return p;
}

/* Clone an object P of size S, with error checking.  There's no need
   for xnmemdup (P, N, S), since xmemdup (P, N * S) works without any
   need for an arithmetic overflow check.  */
void* xmemdup (void const *p, size_t s) {
  return memcpy (xmalloc (s), p, s);
}

/* Clone STRING.  */
char* xstrdup (char const *string) {
  return xmemdup (string, strlen (string) + 1);
}