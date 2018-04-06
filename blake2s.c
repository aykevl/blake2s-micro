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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "blake2s.h"

#define NATIVE_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

// Some logging helper macros, for debugging.
#ifdef DEBUG
#define LOG(s) puts("  " s);
#define LOGF(s, ...) printf("  " s "\n", ##__VA_ARGS__)
#else
#define LOG(s)
#define LOGF(s, ...)
#endif

static const uint32_t blake2s_IV[8] = {
    0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
    0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

// These are the permutations. It packs two permutations per byte, see the
// first row.
static const uint8_t blake2s_sigma[10][8] = {
    { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef },
    { 0xea, 0x48, 0x9f, 0xd6, 0x1c, 0x02, 0xb7, 0x53 },
    { 0xb8, 0xc0, 0x52, 0xfd, 0xae, 0x36, 0x71, 0x94 },
    { 0x79, 0x31, 0xdc, 0xbe, 0x26, 0x5a, 0x40, 0xf8 },
    { 0x90, 0x57, 0x24, 0xaf, 0xe1, 0xbc, 0x68, 0x3d },
    { 0x2c, 0x6a, 0x0b, 0x83, 0x4d, 0x75, 0xfe, 0x19 },
    { 0xc5, 0x1f, 0xed, 0x4a, 0x07, 0x63, 0x92, 0x8b },
    { 0xdb, 0x7e, 0xc1, 0x39, 0x50, 0xf4, 0x86, 0x2a },
    { 0x6f, 0xe9, 0xb3, 0x08, 0xc2, 0xd7, 0x14, 0xa5 },
    { 0xa2, 0x84, 0x76, 0x15, 0xfb, 0x9e, 0x3c, 0xd0 },
};

static inline uint32_t load32(const void *src) {
#if NATIVE_LITTLE_ENDIAN && BLAKE2S_ALIGNED
    return *(uint32_t*)src;
#elif NATIVE_LITTLE_ENDIAN
    uint32_t w;
    memcpy(&w, src, sizeof w);
    return w;
#else // fallback that always works
    const uint8_t *p = (const uint8_t *)src;
    return ((uint32_t)(p[0]) <<  0) |
           ((uint32_t)(p[1]) <<  8) |
           ((uint32_t)(p[2]) << 16) |
           ((uint32_t)(p[3]) << 24) ;
#endif
}

static inline void store32(void *dst, uint32_t w) {
#if NATIVE_LITTLE_ENDIAN && BLAKE2S_ALIGNED
    *(uint32_t*)dst = w;
#elif NATIVE_LITTLE_ENDIAN
    memcpy(dst, &w, sizeof w);
#else // fallback
    uint8_t *p = (uint8_t *)dst;
    p[0] = (uint8_t)(w >>  0);
    p[1] = (uint8_t)(w >>  8);
    p[2] = (uint8_t)(w >> 16);
    p[3] = (uint8_t)(w >> 24);
#endif
}

static inline void store16(void *dst, uint16_t w) {
#if NATIVE_LITTLE_ENDIAN && BLAKE2S_ALIGNED
    *(uint16_t*)dst = w;
#elif NATIVE_LITTLE_ENDIAN
    memcpy(dst, &w, sizeof w);
#else // fallback
    uint8_t *p = (uint8_t *)dst;
    *p++ = (uint8_t)w; w >>= 8;
    *p++ = (uint8_t)w;
#endif
}

// Rotate right by the given amount.
static inline uint32_t rotr32(const uint32_t w, const unsigned c) {
    return ( w >> c ) | ( w << ( 32 - c ) );
}

// Erase memory, even if the compiler would like to optimize it away.
static inline void secure_zero_memory(void *v, size_t n) {
    static void *(*const volatile memset_v)(void *, int, size_t) = &memset;
    memset_v(v, 0, n);
}

void blake2s_set_lastnode(blake2s_state *S) {
    S->f[1] = (uint32_t)-1;
}

int blake2s_is_lastblock(const blake2s_state *S) {
    return S->f[0] != 0;
}

void blake2s_set_lastblock(blake2s_state *S) {
    LOG("lastblock");
    if (S->last_node) blake2s_set_lastnode(S);

    S->f[0] = (uint32_t)-1;
}

static void blake2s_increment_counter(blake2s_state *S, const uint32_t inc) {
    LOGF("increment: %d", inc);
    S->t[0] += inc;
    #if !BLAKE2S_MAX4GB
    S->t[1] += ( S->t[0] < inc );
    #endif
}

static void blake2s_set_IV(uint32_t *buf) {
    for (size_t i = 0; i < 8; i++) {
        buf[i] = blake2s_IV[i];
    }
}

int blake2s_update(blake2s_state *S, const void *in, size_t inlen);

// blake2s initialization without key
int blake2s_init(blake2s_state *S) {
    memset(S, 0, sizeof(blake2s_state));

    blake2s_set_IV(S->h);

    // set depth, fanout and digest length
    S->h[0] ^= (1UL << 24) | (1UL << 16) | BLAKE2S_OUTLEN;

    return 0;
}

#define G(a,b,c,d,m1,m2)       \
    do {                       \
        a = a + b + m1;        \
        d = rotr32(d ^ a, 16); \
        c = c + d;             \
        b = rotr32(b ^ c, 12); \
        a = a + b + m2;        \
        d = rotr32(d ^ a, 8);  \
        c = c + d;             \
        b = rotr32(b ^ c, 7);  \
    } while(0)

static void blake2s_round(size_t r, const uint32_t m[16], uint32_t v[16]) {
#if 1 || !defined(__thumb__)
    // Pure C implementation.

    for (size_t i = 0; i < 8; i++) {
        size_t bit4 = i / 4; // 0, 0, 0, 0, 1, 1, 1, 1

        // Calculate the following table dynamically:
        //   a:    b:    c:     d:
        //   v[0]  v[4]  v[ 8]  v[12]
        //   v[1]  v[5]  v[ 9]  v[13]
        //   v[2]  v[6]  v[10]  v[14]
        //   v[3]  v[7]  v[11]  v[15]
        //   v[0]  v[5]  v[10]  v[15]
        //   v[1]  v[6]  v[11]  v[12]
        //   v[2]  v[7]  v[ 8]  v[13]
        //   v[3]  v[4]  v[ 9]  v[14]
        uint32_t *a = &v[(i + bit4 * 0) % 4 + 0];
        uint32_t *b = &v[(i + bit4 * 1) % 4 + 4];
        uint32_t *c = &v[(i + bit4 * 2) % 4 + 8];
        uint32_t *d = &v[(i + bit4 * 2 + bit4) % 4 + 12];

        const uint8_t sigma = blake2s_sigma[r][i];
        uint32_t m1 = m[sigma >> 4];
        uint32_t m2 = m[sigma & 0xf];
        G(*a, *b, *c, *d, m1, m2);
    }
#else
    // Implementation for ARM Cortex-M microcontrollers (including M0).
    // It turns out this isn't more compact than what GCC produces.
    //
    // Register to variable mapping:
    //   r0: v
    //   r1: m
    //   r2: m1, sigma_row
    //   r3: m2, i
    //   r4: a
    //   r5: b
    //   r6: c, bit4
    //   r7: d
    register uint32_t *v_reg __asm__("r0") = v;
    register const uint32_t *m_reg __asm__("r1") = m;
    register const uint8_t *sigma_row_reg __asm__("r2") = blake2s_sigma[r];
    __asm__ __volatile__(
        ".syntax unified\n"

        "movs r3, #0\n"   // for i in 0..8
        "subround:"

        // Calculate addresses for a, b, c, d
        "movs r6, r3\n"
        "lsrs r6, #2\n"

        "movs r4, r3\n"   // a = i
        "lsls r4, #30\n"  // a %= 4
        "lsrs r4, #28\n"  // b align

        "movs r5, r3\n"   // b = i
        "adds r5, r6\n"   // b += bit4 * 1
        "lsls r5, #30\n"  // b %= 4
        "lsrs r5, #28\n"  // b align
        "adds r5, #16\n"  // b += 16 (index += 4)

        "movs r7, r6\n"   // d = bit4
        "lsls r6, #1\n"   // c = bit4 * 2
        "adds r7, r6\n"   // d += c

        "adds r6, r3\n"   // c += i
        "lsls r6, #30\n"  // c %= 4
        "lsrs r6, #28\n"  // c align
        "adds r6, #32\n"  // b += 32 (index += 8)

        "adds r7, r3\n"   // d += i
        "lsls r7, #30\n"  // d %= 4
        "lsrs r7, #28\n"  // d align
        "adds r7, #48\n"  // b += 48 (index += 12)

        "push {r2, r3}\n" // save sigma_row and i

        // Read m1 and m2 from the m array, using indexes from
        // sigma_row.
        "ldrb r2, [%[sigma_row], r3]\n" // read 'sigma' value (2 nibbles)
        "movs r3, r2\n"

        "lsls r3, #28\n"                // take lower 4 bits
        "lsrs r3, #26\n"                // align
        "ldr  r3, [%[m], r3]\n"         // read m2

        "lsrs r2, #4\n"                 // take higher 4 bits
        "lsls r2, #2\n"                 // align
        "ldr  r2, [%[m], r2]\n"         // read m1

        "push {r4, r5, r6, r7}\n"       // save a, b, c, d pointers
        "ldr  r4, [%[v], r4]\n"         // dereference a, b, c, d
        "ldr  r5, [%[v], r5]\n"
        "ldr  r6, [%[v], r6]\n"
        "ldr  r7, [%[v], r7]\n"

        // G:

        "adds r4, r2\n"  // a += m1
        "adds r4, r5\n"  // a += b
        "eors r7, r4\n"  // d ^= a
        "movs r2, #16\n" // tmp = 16
        "rors r7, r2\n"  // d = rotr32(d, tmp)
        "adds r6, r7\n"  // c += d
        "eors r5, r6\n"  // b ^= c
        "movs r2, #12\n" // tmp = 12
        "rors r5, r2\n"  // b = rotr32(b, tmp)

        "adds r4, r3\n"  // a += m2
        "adds r4, r5\n"  // a += b
        "eors r7, r4\n"  // d ^= a
        "movs r3, #8\n"  // tmp = 8
        "rors r7, r3\n"  // d = rotr32(d, tmp)
        "adds r6, r7\n"  // c += d
        "eors r5, r6\n"  // b ^= c
        "movs r3, #7\n"  // tmp = 7
        "rors r5, r3\n"  // b = rotr32(b, tmp)

        // Put values back in a, b, c, d
        "pop  {r2, r3}\n"
        "str  r4, [%[v], r2]\n"
        "str  r5, [%[v], r3]\n"
        "pop  {r2, r3}\n"
        "str  r6, [%[v], r2]\n"
        "str  r7, [%[v], r3]\n"

        "pop  {r2, r3}\n" // v and i

        "adds r3, #1\n"   // i += 1
        "cmp  r3, #8\n"   // if (i == 0)
        "bne  subround\n" //   break

        : [v]"+r"(v_reg)
        : [sigma_row]"r"(sigma_row_reg), [m]"r"(m_reg)
        : "r3", "r4", "r5", "r6", "r7");

#endif
}

static void blake2s_compress(blake2s_state *S, const uint8_t in[BLAKE2S_BLOCKBYTES]) {
    LOG("compress");

    #if BLAKE2S_ALIGNED
    const uint32_t *m = (const uint32_t*)in;
    #else
    uint32_t m[16];
    memcpy(m, in, sizeof(m));
    #endif

    uint32_t v[16];
    memcpy(v, S->h, 8 * sizeof(v[0]));

    blake2s_set_IV(&v[8]);
    v[12] ^= S->t[0];
    v[13] ^= S->t[1];
    v[14] ^= S->f[0];
    v[15] ^= S->f[1];

    for (size_t r = 0; r < 10; r++) {
        blake2s_round(r, m, v);
    }

    for (size_t i = 0; i < 8; i++) {
        S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];
    }
}

#undef G
#undef ROUND

int blake2s_update(blake2s_state *S, const void *pin, size_t inlen) {
    if (inlen == 0) return 0; // nothing to do

    if (BLAKE2S_ERRCHECK && !BLAKE2S_STREAM) {
        return -1;
    }

    const uint8_t * in = (const uint8_t *)pin;
    size_t left = S->buflen;
    size_t fill = BLAKE2S_BLOCKBYTES - left;
    if (inlen > fill) {
        S->buflen = 0;
        memcpy(S->buf + left, in, fill); // Fill buffer
        blake2s_increment_counter(S, BLAKE2S_BLOCKBYTES);
        blake2s_compress(S, S->buf);
        in += fill; inlen -= fill;
        while (inlen > BLAKE2S_BLOCKBYTES) {
            blake2s_increment_counter(S, BLAKE2S_BLOCKBYTES);
            blake2s_compress(S, in);
            in += BLAKE2S_BLOCKBYTES;
            inlen -= BLAKE2S_BLOCKBYTES;
        }
    }
    memcpy(S->buf + S->buflen, in, inlen);
    S->buflen += inlen;
    return 0;
}

int blake2s_final(blake2s_state *S, void *out) {
    if (BLAKE2S_ERRCHECK && BLAKE2S_STREAM && blake2s_is_lastblock(S)) return -1;

    #if BLAKE2S_STREAM
    blake2s_increment_counter(S, (uint32_t)S->buflen);
    blake2s_set_lastblock(S);
    memset(S->buf + S->buflen, 0, BLAKE2S_BLOCKBYTES - S->buflen); // Padding
    blake2s_compress(S, S->buf);
    #endif

    memcpy(out, S->h, BLAKE2S_OUTLEN);
    return 0;
}

int blake2s(void *out, const void *in, size_t inlen) {
    blake2s_state S[1];

    // Verify parameters
    if (BLAKE2S_ERRCHECK && NULL == in && inlen > 0) return -1;
    if (BLAKE2S_ERRCHECK && NULL == out) return -1;

    // Initialize hash state.
    int err_init = blake2s_init(S);
    if (BLAKE2S_ERRCHECK && err_init) return -1;

    // Streaming API, where this function can get data of arbitrary length
    // as input.
    #if BLAKE2S_STREAM
    int err_update = blake2s_update(S, (const uint8_t *)in, inlen);
    if (BLAKE2S_ERRCHECK && err_update) return -1;

    // Block based API, where the input length MUST be a multiple of the
    // block size.
    #else
    if (BLAKE2S_ERRCHECK && inlen % BLAKE2S_BLOCKBYTES != 0) return -1;
    const uint8_t *inbuf = in;
    const uint8_t *inend = inbuf + inlen - BLAKE2S_BLOCKBYTES;
    for (;;) {
        blake2s_increment_counter(S, BLAKE2S_BLOCKBYTES);
        if (inbuf == inend) {
            blake2s_set_lastblock(S);
        }
        blake2s_compress(S, inbuf);
        if (inbuf == inend) {
            break;
        }
        inbuf += BLAKE2S_BLOCKBYTES;
    }
    #endif

    int err_final = blake2s_final(S, out);
    if (BLAKE2S_ERRCHECK && err_final) return -1;

    // Success!
    return 0;
}

int blake2s_blocks(void *out, const uint8_t in[BLAKE2S_BLOCKBYTES], size_t inblocks) {
    return blake2s(out, in, inblocks * BLAKE2S_BLOCKBYTES);
}
