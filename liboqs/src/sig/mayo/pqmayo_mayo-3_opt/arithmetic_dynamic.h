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

#ifndef ARITHMETIC_DYNAMIC_H
#define ARITHMETIC_DYNAMIC_H

#include <stdint.h>
#include <simple_arithmetic.h>

static
inline void m_vec_copy(int m_vec_limbs, const uint64_t *in, uint64_t *out) {
    for (int i = 0; i < m_vec_limbs; i++) {
        out[i] = in[i];
    }
}

static
inline void m_vec_add(int m_vec_limbs, const uint64_t *in, uint64_t *acc) {
    for (int i = 0; i < m_vec_limbs; i++) {
        acc[i] ^= in[i];
    }
}

static inline
void m_vec_mul_add(int m_vec_limbs, const uint64_t *in, unsigned char a, uint64_t *acc) {
    for(int i=0; i < m_vec_limbs; i++){
        acc[i] ^= gf16v_mul_u64(in[i], a);
    }
}

inline
static void m_vec_mul_add_x (int m_vec_limbs, const uint64_t *in, uint64_t *acc) {
    uint64_t mask_msb = 0x8888888888888888ULL;
    for(int i=0; i < m_vec_limbs; i++){
        uint64_t t = in[i] & mask_msb;
        acc[i] ^= ((in[i] ^ t) << 1) ^ ((t >> 3) * 3);
    }
}

inline
static void m_vec_mul_add_x_inv (int m_vec_limbs, const uint64_t *in, uint64_t *acc) {
    uint64_t mask_lsb = 0x1111111111111111ULL;
    for(int i=0; i < m_vec_limbs; i++){
        uint64_t t = in[i] & mask_lsb;
        acc[i] ^= ((in[i] ^ t) >> 1) ^ (t * 9);
    }
}

static
inline void m_vec_multiply_bins (int m_vec_limbs, uint64_t *bins, uint64_t *out) {

    m_vec_mul_add_x_inv (m_vec_limbs, bins +  5 * m_vec_limbs, bins +  10 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins + 11 * m_vec_limbs, bins + 12 * m_vec_limbs);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  10 * m_vec_limbs, bins +  7 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins + 12 * m_vec_limbs, bins +  6 * m_vec_limbs);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  7 * m_vec_limbs, bins +  14 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins +  6 * m_vec_limbs, bins +  3 * m_vec_limbs);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  14 * m_vec_limbs, bins +  15 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins +  3 * m_vec_limbs, bins +  8 * m_vec_limbs);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  15 * m_vec_limbs, bins +  13 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins +  8 * m_vec_limbs, bins +  4 * m_vec_limbs);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +  13 * m_vec_limbs, bins +  9 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins +  4 * m_vec_limbs, bins +  2 * m_vec_limbs);
    m_vec_mul_add_x_inv (m_vec_limbs, bins +   9 * m_vec_limbs, bins +  1 * m_vec_limbs);
    m_vec_mul_add_x (m_vec_limbs, bins +  2 * m_vec_limbs, bins +  1 * m_vec_limbs);
    m_vec_copy (m_vec_limbs, bins + m_vec_limbs, out);
}

#endif