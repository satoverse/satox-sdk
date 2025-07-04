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

// SPDX-License-Identifier: Apache-2.0

#ifndef ARITHMETIC_FIXED_H
#define ARITHMETIC_FIXED_H

#include <stdint.h>
#include <mayo.h>
#include <simple_arithmetic.h>

// This implements arithmetic for vectors of X field elements in Z_2[x]/(x^4+x+1)

static
inline void m_vec_copy (int m_vec_limbs, const uint64_t *in, uint64_t *out) {
    (void) m_vec_limbs;
    for (size_t i = 0; i < M_VEC_LIMBS_MAX; i++)
    {
        out[i] = in[i];
    }
}

static
inline void m_vec_add (int m_vec_limbs, const uint64_t *in, uint64_t *acc) {
    (void) m_vec_limbs;
    for (size_t i = 0; i < M_VEC_LIMBS_MAX; i++)
    {
        acc[i] ^= in[i];
    }
}

static 
inline void m_vec_mul_add (int m_vec_limbs, const uint64_t *in, unsigned char a, uint64_t *acc) {
    (void) m_vec_limbs;
    uint32_t tab = mul_table(a);

    uint64_t lsb_ask = 0x1111111111111111ULL;

    for(int i=0; i < M_VEC_LIMBS_MAX ;i++){
        acc[i] ^= ( in[i]       & lsb_ask) * (tab & 0xff)
                ^ ((in[i] >> 1) & lsb_ask) * ((tab >> 8)  & 0xf)
                ^ ((in[i] >> 2) & lsb_ask) * ((tab >> 16) & 0xf)
                ^ ((in[i] >> 3) & lsb_ask) * ((tab >> 24) & 0xf);
    }
}

inline
static void m_vec_mul_add_x (int m_vec_limbs, const uint64_t *in, uint64_t *acc) {
    (void) m_vec_limbs;
    uint64_t mask_msb = 0x8888888888888888ULL;
    for(int i=0; i < M_VEC_LIMBS_MAX; i++){
        uint64_t t = in[i] & mask_msb;
        acc[i] ^= ((in[i] ^ t) << 1) ^ ((t >> 3) * 3);
    }
}

inline
static void m_vec_mul_add_x_inv (int m_vec_limbs, const uint64_t *in, uint64_t *acc) {
    (void) m_vec_limbs;
    uint64_t mask_lsb = 0x1111111111111111ULL;
    for(int i=0; i < M_VEC_LIMBS_MAX; i++){
        uint64_t t = in[i] & mask_lsb;
        acc[i] ^= ((in[i] ^ t) >> 1) ^ (t * 9);
    }
}

static 
inline void m_vec_multiply_bins (int m_vec_limbs, uint64_t *bins, uint64_t *out) {
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  5 * M_VEC_LIMBS_MAX, bins +  10 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins + 11 * M_VEC_LIMBS_MAX, bins + 12 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  10 * M_VEC_LIMBS_MAX, bins +  7 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins + 12 * M_VEC_LIMBS_MAX, bins +  6 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  7 * M_VEC_LIMBS_MAX, bins +  14 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins +  6 * M_VEC_LIMBS_MAX, bins +  3 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  14 * M_VEC_LIMBS_MAX, bins +  15 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins +  3 * M_VEC_LIMBS_MAX, bins +  8 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  15 * M_VEC_LIMBS_MAX, bins +  13 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins +  8 * M_VEC_LIMBS_MAX, bins +  4 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  13 * M_VEC_LIMBS_MAX, bins +  9 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins +  4 * M_VEC_LIMBS_MAX, bins +  2 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +   9 * M_VEC_LIMBS_MAX, bins +  1 * M_VEC_LIMBS_MAX);
    m_vec_mul_add_x (m_vec_limbs, bins +  2 * M_VEC_LIMBS_MAX, bins +  1 * M_VEC_LIMBS_MAX);
    m_vec_copy (m_vec_limbs, bins + M_VEC_LIMBS_MAX, out);
}

#endif
