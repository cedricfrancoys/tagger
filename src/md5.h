/* md5.c - interface for checking and setting program environment.

    This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
    MD5 Message-Digest Algorithm (RFC 1321).
    <http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5>

    Copyright (c) 2001 Alexander Peslyak <solar at openwall.com> and it is hereby released to the
    general public under the following terms:

    Redistribution and use in source and binary forms, with or without
    modification, are permitted.
    There's ABSOLUTELY NO WARRANTY, express or implied.
    
    See md5.c for more information. */
 
#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#elif !defined(_MD5_H)
#define _MD5_H
 
/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;
 
typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;
 
extern void MD5_Init(MD5_CTX *ctx);
extern void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
extern void MD5_Final(unsigned char *result, MD5_CTX *ctx);
 
#endif