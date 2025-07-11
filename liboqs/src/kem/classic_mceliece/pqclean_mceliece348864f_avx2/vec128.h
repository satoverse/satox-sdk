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

#ifndef VEC128_H
#define VEC128_H
/*
  This file is for functions related to 128-bit vectors
  including functions for bitsliced field operations
*/

#include "namespace.h"

#define vec128_mul_asm CRYPTO_NAMESPACE(vec128_mul_asm)
#define vec128_mul_GF CRYPTO_NAMESPACE(vec128_mul_GF)

#include "params.h"

#include <smmintrin.h>
#include <stdint.h>

typedef __m128i vec128;

static inline vec128 vec128_set1_16b(uint16_t a) {
    return _mm_set1_epi16(a);
}

static inline vec128 vec128_setzero(void) {
    return _mm_setzero_si128();
}

#define vec128_extract(a, i) ((uint64_t) _mm_extract_epi64((vec128) (a), (i)))

static inline int vec128_testz(vec128 a) {
    return _mm_testz_si128(a, a);
}

static inline vec128 vec128_and(vec128 a, vec128 b) {
    return _mm_and_si128(a, b);
}

static inline vec128 vec128_xor(vec128 a, vec128 b) {
    return _mm_xor_si128(a, b);
}

static inline vec128 vec128_or(vec128 a, vec128 b) {
    return _mm_or_si128(a, b);
}

#define vec128_sll_2x(a, s) ((vec128) _mm_slli_epi64((vec128) (a), (s)))
#define vec128_srl_2x(a, s) ((vec128) _mm_srli_epi64((vec128) (a), (s)))

static inline vec128 vec128_set2x(uint64_t a0, uint64_t a1) {
    return _mm_set_epi64x(a1, a0);
}

static inline vec128 vec128_unpack_low(vec128 a, vec128 b) {
    return _mm_unpacklo_epi64(a, b);
}

static inline vec128 vec128_unpack_high(vec128 a, vec128 b) {
    return _mm_unpackhi_epi64(a, b);
}

static inline vec128 vec128_setbits(uint64_t a) {
    return _mm_set1_epi64x(-a);
}

static inline void vec128_copy(vec128 *dest, vec128 *src) {
    int i;

    for (i = 0; i < GFBITS; i++) {
        dest[i] = src[i];
    }
}

static inline void vec128_add(vec128 *c, vec128 *a, vec128 *b) {
    int i;

    for (i = 0; i < GFBITS; i++) {
        c[i] = vec128_xor(a[i], b[i]);
    }
}

static inline vec128 vec128_or_reduce(vec128 *a) {
    int i;
    vec128 ret;

    ret = a[0];
    for (i = 1; i < GFBITS; i++) {
        ret = vec128_or(ret, a[i]);
    }

    return ret;
}

extern void vec128_mul_asm(vec128 *, vec128 *, const vec128 *, int);

/* bitsliced field multiplications */
static inline void vec128_mul(vec128 *h, vec128 *f, const vec128 *g) {
    vec128_mul_asm(h, f, g, 16);
}

#endif
