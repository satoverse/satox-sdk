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

// SPDX-License-Identifier: CC0 OR Apache-2.0
/// @file blas_comm.h
/// @brief Common functions for linear algebra.
///
#ifndef _BLAS_COMM_H_
#define _BLAS_COMM_H_

#include <stdint.h>

//IF_CRYPTO_CORE:define CRYPTO_NAMESPACE


/// @brief get an element from GF(16) vector .
///
/// @param[in]  a         - the input vector a.
/// @param[in]  i         - the index in the vector a.
/// @return  the value of the element.
///
static inline uint8_t gf16v_get_ele(const uint8_t *a, unsigned i) {
    uint8_t r = a[i >> 1];
    return (i & 1) ? (r >> 4) : (r & 0xf);
}

/// @brief set an element for a GF(16) vector .
///
/// @param[in,out]   a   - the vector a.
/// @param[in]  i        - the index in the vector a.
/// @param[in]  v        - the value for the i-th element in vector a.
/// @return  the value of the element.
///
static inline uint8_t gf16v_set_ele(uint8_t *a, unsigned i, uint8_t v) {
    uint8_t ai = a[i >> 1];
    uint8_t i_1_or_16 = (i & 1) * 15 + 1; // 0 -> 1 , 1 -> 16
    ai &= ~(0xf * i_1_or_16); // 0 -> clear lower nibble, 1 -> clear high nibble.
    // v &= 0xf;
    a[i >> 1] = ai + v * i_1_or_16;
    return v;
}


/// @brief get an element from GF(256) vector .
///
/// @param[in]  a         - the input vector a.
/// @param[in]  i         - the index in the vector a.
/// @return  the value of the element.
///
static inline uint8_t gf256v_get_ele(const uint8_t *a, unsigned i) {
    return a[i];
}


/// @brief set an element for a GF(256) vector .
///
/// @param[in,out]   a   - the vector a.
/// @param[in]  i        - the index in the vector a.
/// @param[in]  v        - the value for the i-th element in vector a.
/// @return  the value of the element.
///
static inline uint8_t gf256v_set_ele(uint8_t *a, unsigned i, uint8_t v) {
    a[i] = v;
    return v;
}


/// @brief check if a vector is 0.
///
/// @param[in]   a          - the vector a.
/// @param[in]  _num_byte   - number of bytes for the vector a.
/// @return  1(true) if a is 0. 0(false) else.
///
static inline unsigned gf256v_is_zero(const uint8_t *a, unsigned _num_byte) {
    uint8_t r = 0;
    while ( _num_byte-- ) {
        r |= a[0];
        a++;
    }
    return (0 == r);
}

#include "blas.h"
//
//  gf256v_add() should be defined in blas.h
//

/// @brief set a vector to 0.
///
/// @param[in,out]   b      - the vector b.
/// @param[in]  _num_byte   - number of bytes for the vector b.
///
static inline void gf256v_set_zero(uint8_t *b, unsigned _num_byte) {
    memset(b, 0, _num_byte);
}



#endif  // _BLAS_COMM_H_

