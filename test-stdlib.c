//  BLAKE2 - size-optimized implementations
//
//  Copyright 2012, Samuel Neves <sneves@dei.uc.pt> (original work)
//  Copyright 2018, Ayke van Laethem
//
//  You may use this under the terms of the CC0, the OpenSSL Licence, or
//  the Apache Public License 2.0, at your option. The terms of these
//  licenses can be found at:
//
//  - CC0 1.0 Universal : http://creativecommons.org/publicdomain/zero/1.0
//  - OpenSSL license   : https://www.openssl.org/source/license.html
//  - Apache 2.0        : http://www.apache.org/licenses/LICENSE-2.0
//
//  More information about the BLAKE2 hash function can be found at
//  https://blake2.net.

#include <string.h>

// Very compact implementations of builtin/stdlib functions

void *memset(void *s, int c, size_t n) {
    unsigned char *buf = s;
    for (void *end = buf + n; buf != end; buf++) {
        *buf = c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *destbuf = dest;
    const unsigned char *srcbuf = src;
    for (size_t i = 0; i < n; i++) {
        destbuf[i] = srcbuf[i];
    }
    return dest;
}
