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

#ifndef GF_H
#define GF_H
/*
  This file is for functions for field arithmetic
*/

#include "namespace.h"

#define gf_add CRYPTO_NAMESPACE(gf_add)
#define gf_frac CRYPTO_NAMESPACE(gf_frac)
#define gf_inv CRYPTO_NAMESPACE(gf_inv)
#define gf_iszero CRYPTO_NAMESPACE(gf_iszero)
#define gf_mul2 CRYPTO_NAMESPACE(gf_mul2)
#define gf_mul CRYPTO_NAMESPACE(gf_mul)
#define GF_mul CRYPTO_NAMESPACE(GF_mul)

#include "params.h"

#include <stdint.h>

typedef uint16_t gf;

gf gf_iszero(gf a);
gf gf_add(gf in0, gf in1);
gf gf_mul(gf in0, gf in1);
gf gf_frac(gf den, gf num);
gf gf_inv(gf in);

void GF_mul(gf *out, gf *in0, gf *in1);

/* 2 field multiplications */
static inline uint64_t gf_mul2(gf a, gf b0, gf b1) {
    int i;

    uint64_t tmp = 0;
    uint64_t t0;
    uint64_t t1;
    uint64_t t;
    uint64_t mask = 0x0000000100000001;

    t0 = a;
    t1 = b1;
    t1 = (t1 << 32) | b0;

    for (i = 0; i < GFBITS; i++) {
        tmp ^= t0 * (t1 & mask);
        mask += mask;
    }

    //

    t = tmp & 0x007FC000007FC000;
    tmp ^= (t >> 9) ^ (t >> 12);

    t = tmp & 0x0000300000003000;
    tmp ^= (t >> 9) ^ (t >> 12);

    return tmp & 0x00000FFF00000FFF;
}

#endif
