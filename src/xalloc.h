/* xalloc.c - interface for memory allocation with error checking.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
    
    Following macros are adapted from implementations defined 
    in xmalloc.c, xalloc.h and xalloc-oversized.h
    released under GPL 3 by Free Software Foundation, Inc. 
    See credits below. */

/*   
   xalloc.h -- malloc with out of memory checking   

    Copyright (C) 1990-2000, 2002-2006, 2008-2011 Free Software Foundation, Inc.
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>   
*/

# include <stddef.h>   


/* Functions defined in xalloc.c
*/
void *xmalloc (size_t s);
void *xzalloc (size_t s);
void *xcalloc (size_t n, size_t s);
void *xrealloc (void *p, size_t s);
void *xmemdup (void const *p, size_t s);
char *xstrdup (char const *str);  
   
   
/* In the following macros, T must be an elementary or structure/union or
   typedef'ed type, or a pointer to such a type.  To apply one of the
   following macros to a function pointer or array type, you need to typedef
   it first and use the typedef name.  */

/* Allocate an object of type T dynamically, with error checking.  */
/* extern t *XMALLOC (typename t); */
# define XMALLOC(t) ((t *) xmalloc (sizeof (t)))

/* Allocate memory for N elements of type T, with error checking.  */
/* extern t *XNMALLOC (size_t n, typename t); */
# define XNMALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xmalloc (n) : xnmalloc (n, sizeof (t))))

/* Allocate an object of type T dynamically, with error checking,
   and zero it.  */
/* extern t *XZALLOC (typename t); */
# define XZALLOC(t) ((t *) xzalloc (sizeof (t)))

/* Allocate memory for N elements of type T, with error checking,
   and zero it.  */
/* extern t *XCALLOC (size_t n, typename t); */
# define XCALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xzalloc (n) : xcalloc (n, sizeof (t))))

/* Return 1 if an array of N objects, each of size S, cannot exist due
   to size arithmetic overflow.  S must be positive and N must be
   nonnegative.  This is a macro, not an inline function, so that it
   works correctly even when SIZE_MAX < N.

   By gnulib convention, SIZE_MAX represents overflow in size
   calculations, so the conservative dividend to use here is
   SIZE_MAX - 1, since SIZE_MAX might represent an overflowed value.
   However, malloc (SIZE_MAX) fails on all known hosts where
   sizeof (ptrdiff_t) <= sizeof (size_t), so do not bother to test for
   exactly-SIZE_MAX allocations on such hosts; this avoids a test and
   branch when S is known to be 1.  */
#define xalloc_oversized(n, s) ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))