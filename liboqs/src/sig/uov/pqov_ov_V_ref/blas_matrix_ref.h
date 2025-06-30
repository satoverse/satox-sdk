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
/// @file blas_matrix_ref.h
/// @brief linear algebra functions for matrix op.
///
#ifndef _BLAS_MATRIX_REF_H_
#define _BLAS_MATRIX_REF_H_

#include <stdint.h>



#ifdef  __cplusplus
extern  "C" {
#endif


///////////////// Section: multiplications  ////////////////////////////////

// matrix-vector
#define gf16mat_prod_ref PQOV_NAMESPACE(gf16mat_prod_ref)
void gf16mat_prod_ref(uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b);

#define gf256mat_prod_ref PQOV_NAMESPACE(gf256mat_prod_ref)
void gf256mat_prod_ref(uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b);

/////////////////////////////////////////////////////
#define gf256mat_gaussian_elim_ref PQOV_NAMESPACE(gf256mat_gaussian_elim_ref)
unsigned gf256mat_gaussian_elim_ref(uint8_t *sq_col_mat_a, uint8_t *constant, unsigned len);

#define gf256mat_back_substitute_ref PQOV_NAMESPACE(gf256mat_back_substitute_ref)
void gf256mat_back_substitute_ref( uint8_t *constant, const uint8_t *sq_row_mat_a, unsigned len);

#define gf16mat_gaussian_elim_ref PQOV_NAMESPACE(gf16mat_gaussian_elim_ref)
unsigned gf16mat_gaussian_elim_ref(uint8_t *sq_col_mat_a, uint8_t *constant, unsigned len);

#define gf16mat_back_substitute_ref PQOV_NAMESPACE(gf16mat_back_substitute_ref)
void gf16mat_back_substitute_ref( uint8_t *constant, const uint8_t *sq_row_mat_a, unsigned len);



#ifdef  __cplusplus
}
#endif

#endif  // _BLAS_MATRIX_REF_H_

