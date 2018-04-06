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

#include <stdio.h>
#include "blake2s.h"

__attribute__((aligned(4)))
const unsigned char data1[BLAKE2S_BLOCKBYTES + 1] = "The quick brown fox jumps over the lazy dog";

__attribute__((aligned(4)))
const unsigned char data2[] = "The quick brown fox jumps over the lazy dog";

int main(int argc, char **argv) {
    unsigned char result[32];

    blake2s(result, (const uint32_t*)data1, sizeof(data1) - 1);
    printf("Result: ");
    for (size_t i=0; i<sizeof(result); i++) {
        printf("%02x", result[i]);
    }
    printf("\n");

#if BLAKE2S_STREAM
    blake2s(result, (const uint32_t*)data2, sizeof(data2) - 1);
    printf("Result: ");
    for (size_t i=0; i<sizeof(result); i++) {
        printf("%02x", result[i]);
    }
    printf("\n");
#endif
}
