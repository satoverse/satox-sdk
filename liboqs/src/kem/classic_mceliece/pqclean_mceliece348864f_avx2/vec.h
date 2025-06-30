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

#ifndef VEC_H
#define VEC_H

#include "namespace.h"

#define vec_GF_mul CRYPTO_NAMESPACE(vec_GF_mul)
#define vec_mul_asm CRYPTO_NAMESPACE(vec_mul_asm)
#define vec_mul_sp_asm CRYPTO_NAMESPACE(vec_mul_sp_asm)

#include "gf.h"
#include "params.h"

#include <stdint.h>

typedef uint64_t vec;

extern void vec_mul_asm(vec *, const vec *, const vec *);

static inline void vec_mul(vec *h, const vec *f, const vec *g) {
    vec_mul_asm(h, f, g);
}

extern void vec_mul_sp_asm(vec *, const vec *, const vec *);

static inline void vec_mul_sp(vec *h, const vec *f, const vec *g) {
    vec_mul_sp_asm(h, f, g);
}

static inline void vec_add(vec *h, const vec *f, const vec *g) {
    int b;

    for (b = 0; b < GFBITS; b++) {
        h[b] = f[b] ^ g[b];
    }
}

static inline void vec_mul_gf(vec out[ GFBITS ], vec v[ GFBITS ], gf a) {
    int i;

    vec bits[GFBITS];

    for (i = 0; i < GFBITS; i++) {
        bits[i] = -((a >> i) & 1);
    }

    vec_mul_asm(out, v, bits);
}

void vec_GF_mul(vec out[GFBITS], vec v[GFBITS], gf a[SYS_T]);

#endif
