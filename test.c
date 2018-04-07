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
#include <string.h>
#include <stdlib.h>
#include "blake2s.h"

__attribute__((aligned(4)))
const uint8_t data1[BLAKE2S_BLOCKBYTES + 1] = "The quick brown fox jumps over the lazy dog";

__attribute__((aligned(4)))
const uint8_t data2[] = "The quick brown fox jumps over the lazy dog";

void test(const uint8_t *data, size_t len) {
    unsigned char result[32];

    int err = blake2s(result, (const uint32_t*)data, len);
    if (err) {
        printf("blake2s: error\n");
        return;
    }

    for (size_t i = 0; i < sizeof(result); i++) {
        printf("%02x", result[i]);
    }
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc > 1) {
        for (size_t i = 1; i < argc; i++) {
            size_t len = strlen(argv[i]) / 2;
            if (!BLAKE2S_STREAM && len % BLAKE2S_BLOCKBYTES != 0) {
                printf("skip: inputs must be block aligned\n");
                continue;
            }
            if (!BLAKE2S_STREAM && len == 0) {
                printf("skip: input is zero-length\n");
                continue;
            }

            // Read hex-encoded data
            uint8_t *buf = malloc(len);
            for (size_t j = 0; j < len; j++) {
                sscanf(&argv[i][j*2], "%2hhx", &buf[j]);
            };
            test(buf, len);
            free(buf);
        }
    } else {
        test(data1, sizeof(data1) - 1);
        if (BLAKE2S_STREAM) {
            test(data2, sizeof(data2) - 1);
        }
        if (BLAKE2S_STREAM) {
            test(NULL, 0);
        }
    }
}
