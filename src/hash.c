/* hash.c - interface for hashing elements names.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/


#include <stdio.h>
#include <string.h>
#include "md5.h"   
#include "hash.h"

/* Return a 32 characters hash of the given string. 
 This function uses the Alexander Peslyak OpenSSL-compatible implementation 
 of MD5 Algorithm (RFC 1321) to generate a MD5 digest.
*/
char* hash(char* str) {
    MD5_CTX context;
    unsigned char digest[16];
    static char result[33];
    MD5_Init(&context);
    MD5_Update(&context, str, strlen(str));
    MD5_Final(digest, &context);
    for(int i = 0; i < 16; ++i) {
        sprintf(&result[i*2], "%02x", (unsigned int) digest[i]);
    }
    result[32] = 0;
    return result;
}