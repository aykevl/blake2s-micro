# BLAKE2s for embedded devices

This is a size-optimized implementation of the [BLAKE2s hash
function](https://blake2.net/). With all optimizations/constraints enabled, it
can archieve a code size of about 500 bytes.

**Warning: this implementation hasn't been verified.** It has only been tested
on very limited inputs and it hasn't been verified by someone with a
cryptographic background. Do not use it for anything serious, or accept the
risk.

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

## TODO

  * Check against all applicable test vectors.
  * Expand API.
