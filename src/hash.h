/* hash.h - interface for hashing elements names.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/
   
#ifndef HASH_H
#define HASH_H 1

/* Generate a MD5 digest from a given string.
*/
char* hash (char* str);

#endif