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

#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Fixed constants that cannot be changed.
#define BLAKE2S_BLOCKBYTES (64)

// Constants that may be changed to accomodate application needs.
#define BLAKE2S_STREAM   (0)  // When disabled, inputs must be aligned to BLAKE2S_BLOCKBYTES
#define BLAKE2S_OUTLEN   (32) // Length of digest (32 is the secure/maximum/standard length)
#define BLAKE2S_ERRCHECK (0)  // Enable error checking
#define BLAKE2S_ALIGNED  (1)  // Inputs are always 32-bit aligned.
#define BLAKE2S_MAX4GB   (1)  // Input length won't be larger than 4GB (32-bits).

typedef struct {
  uint32_t h[8];
  uint32_t t[2];
  uint32_t f[2];
  uint8_t  buf[BLAKE2S_BLOCKBYTES];
  size_t   buflen;
  uint8_t  last_node;
} blake2s_state;

// Simple API.
// Restrictions:
//   * If BLAKE2S_ALIGNED is set, 'in' MUST be aligned on words (4 bytes).
//   * If BLAKE2S_STREAM is unset, 'inlen' MUST be a multiple of BLAKE2S_BLOCKBYTES.
int blake2s(void *out, const void *in, size_t inlen);
int blake2s_blocks(void *out, const uint8_t in[BLAKE2S_BLOCKBYTES], size_t inblocks);

#if defined(__cplusplus)
}
#endif
