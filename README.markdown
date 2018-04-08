# BLAKE2s for embedded devices

This is a size-optimized implementation of the [BLAKE2s hash
function](https://blake2.net/). With all optimizations/constraints enabled, it
can archieve a code size of about 500 bytes.

**Note**: although this implementation is based on the reference implementation
and it has been tested on all relevant test vectors, I cannot guarantee it will
work in all cases as it has been modified quite a lot.

Things that are different from the original reference implementation:

  * By default, inputs are assumed to be aligned on word boundaries and the
    output digest length is fixed at compile time. Also, by default the input
    must be given in multiples of the block size (64 bytes). Most of these
    changes can be disabled.
  * Unrolled loops are re-rolled to save code size.
  * No support for keyed hashing or salting, yet. This is something that would
    be nice to have, if it can be disabled by default.
  * Code relies on modern features of C, to make it easier to read. We don't
    live in the 90's, so we can use variable declarations directly in a `for`
    loop for example. This also means using `#pragma once` in header files.
    This is non-standard, but supported by
    [basically every compiler](https://en.wikipedia.org/wiki/Pragma_once#Portability).
  * Code has been reformatted, mostly following the
[MicroPython coding conventions](https://github.com/micropython/micropython/blob/master/CODECONVENTIONS.md#c-code-conventions).

The original license of the reference source code has been kept, so you can use
it under the CC0 1.0, OpenSSL, or Apache 2.0 license.


## Usage

Usage is simple:

```c
#include "blake2s.h"

// this is the message to be hashed
__attribute__((aligned(4)))
const uint8_t data[BLAKE2S_BLOCKBYTES] = {0, 1, 2, 3}; // block length must be > 0 and a multiple of 64

void main() {
    uint8_t result[32];

    // hash the data
    blake2s(result, data, sizeof(data));

    // do something with the result
    for (size_t i = 0; i < sizeof(result); i++) {
        printf("%02x", result[i]);
    }
    printf("\n");
}
```


## Performance

The primary goal of this library is small size, but I've also done some
performance testing.


| System    | Optimization level | size (bytes) | kB/s         |
| --------- | ------------------ | ------------ | ------------ |
| Cortex-M0 | GCC: `-Os -flto`   | 498          | ?            |
| Cortex-M4 | GCC: `-Os -flto`   | 550          | ~500 (64MHz) |


## Options

There are several options in blake2s.h with feature/codesize tradeoffs:

| Option              | Effect |
| ------------------- | ------ |
| `BLAKE2S_OUTLEN`    | The digest length, usually 32. |
| `BLAKE2S_STREAM`    | Allow non-block-sized inputs. When this is disabled, the
length passed to `blake2s()` must be a non-zero multiple of 64. |
| `BLAKE2S_ERRCHECK`  | Check and return an error on invalid parameters. It is a programming error (invalid parameter) if an error is returned - for example passing NULL as output. |
| `BLAKE2S_UNALIGNED` | Allow unaligned inputs. Processors that require aligned reads (most 32-bit microcontrollers, few high-end CPUs) will choke when this option is disabled and unaligned input data is passed to `blake2s()`. |
| `BLAKE2S_64BIT`     | Allow the total length of the hashed data to be larger than 4GB. This is only relevant on 64-bit processors. |


## TODO

  * Check against all applicable test vectors.
  * Expand API.
