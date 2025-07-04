/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SHA512AVX_H
#define SHA512AVX_H

#include <immintrin.h>
#include <stdint.h>

#include "params.h"

typedef struct SHA512state4x {
    __m256i s[8];
    unsigned char msgblocks[4 * 128];
    unsigned int datalen;
    unsigned long long msglen;
} sha512x4ctx;

#define sha512_init4x SPX_NAMESPACE(sha512_init4x)
void sha512_init4x(sha512x4ctx *ctx);

#define sha512_transform4x SPX_NAMESPACE(sha512_transform4x)
void sha512_transform4x(
    sha512x4ctx *ctx,
    const unsigned char *d0,
    const unsigned char *d1,
    const unsigned char *d2,
    const unsigned char *d3);

#define sha512x4_seeded SPX_NAMESPACE(sha512x4_seeded)
void sha512x4_seeded(
    unsigned char *out0,
    unsigned char *out1,
    unsigned char *out2,
    unsigned char *out3,
    const sha512x4ctx *seed,
    const unsigned char *in0,
    const unsigned char *in1,
    const unsigned char *in2,
    const unsigned char *in3,
    unsigned long long inlen);

#define sha512_ctx_clone4x SPX_NAMESPACE(sha512_ctx_clone4x)
void sha512_ctx_clone4x(sha512x4ctx *out, const sha512x4ctx *in);

/**
 * Note that inlen should be sufficiently small that it still allows for
 * an array to be allocated on the stack. Typically 'in' is merely a seed.
 * Outputs outlen number of bytes
 */
#define mgf1x4_512 SPX_NAMESPACE(mgf1x4_512)
void mgf1x4_512(unsigned char *outx4, unsigned long outlen,
                const unsigned char *in0,
                const unsigned char *in1,
                const unsigned char *in2,
                const unsigned char *in3,
                unsigned long inlen);

#endif
