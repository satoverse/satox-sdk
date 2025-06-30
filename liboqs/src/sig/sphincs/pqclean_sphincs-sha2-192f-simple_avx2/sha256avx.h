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

#ifndef SHA256AVX_H
#define SHA256AVX_H

#include <immintrin.h>
#include <stdint.h>

#include "params.h"

typedef struct SHA256state {
    __m256i s[8];
    unsigned char msgblocks[8 * 64];
    unsigned int datalen;
    unsigned long long msglen;
} sha256x8ctx;

#define sha256_ctx_clone8x SPX_NAMESPACE(sha256_ctx_clone8x)
void sha256_ctx_clone8x(sha256x8ctx *out, const sha256x8ctx *in);

#define sha256_init8x SPX_NAMESPACE(sha256_init8x)
void sha256_init8x(sha256x8ctx *ctx);

#define sha256_final8x SPX_NAMESPACE(sha256_final8x)
void sha256_final8x(sha256x8ctx *ctx,
                    unsigned char *out0,
                    unsigned char *out1,
                    unsigned char *out2,
                    unsigned char *out3,
                    unsigned char *out4,
                    unsigned char *out5,
                    unsigned char *out6,
                    unsigned char *out7);

#define sha256_transform8x SPX_NAMESPACE(sha256_transform8x)
void sha256_transform8x(sha256x8ctx *ctx,
                        const unsigned char *data0,
                        const unsigned char *data1,
                        const unsigned char *data2,
                        const unsigned char *data3,
                        const unsigned char *data4,
                        const unsigned char *data5,
                        const unsigned char *data6,
                        const unsigned char *data7);

#endif
