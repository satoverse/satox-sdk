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
/// @file blas_matrix_sse.h
/// @brief linear algebra functions for matrix operations, specialized for SSSE3 instruction set.
///

#ifndef _BLAS_MATRIX_SSE_H_
#define _BLAS_MATRIX_SSE_H_


#include "stdint.h"



#ifdef  __cplusplus
extern  "C" {
#endif





///////////////////////////////  GF( 16 ) ////////////////////////////////////////////////////

/// @brief  c = matA * b , GF(16)
///
/// @param[out]   c         - the output vector c
/// @param[in]   matA       - the matrix A.
/// @param[in]   n_A_vec_byte    - the size of column vectors in A.
/// @param[in]   n_A_width       - the width of matrix A.
/// @param[in]   b_multab        - the vector b, in multiplication tables.
///
void gf16mat_prod_multab_sse( uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b_multab );


/// @brief  c = mat_a * b , GF(16)
///
/// @param[out]   c         - the output vector c
/// @param[in]   mat_a       - the matrix a.
/// @param[in]   a_h_byte    - the size of column vectors in a.
/// @param[in]   a_w        - the width of matrix a.
/// @param[in]   b           - the vector b.
///
void gf16mat_prod_sse( uint8_t *c, const uint8_t *mat_a, unsigned a_h_byte, unsigned a_w, const uint8_t *b );



///////////////////////////////  GF( 256 ) ////////////////////////////////////////////////////


//void gf256mat_prod_add_multab_sse( __m128i * r , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * multab );


/// @brief  c = matA * b , GF(256)
///
/// @param[out]   c         - the output vector c
/// @param[in]   matA       - the matrix A.
/// @param[in]   n_A_vec_byte    - the size of column vectors in A.
/// @param[in]   n_A_width       - the widht of matrix A.
/// @param[in]   b_multab        - the vector b, in multiplication tables.
///
void gf256mat_prod_multab_sse( uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *multab );

//void gf256mat_prod_add_sse( __m128i * r , const uint8_t * matA , unsigned n_A_vec_byte , unsigned n_A_width , const uint8_t * b );


/// @brief  c = matA * b , GF(256)
///
/// @param[out]   c         - the output vector c
/// @param[in]   matA       - the matrix A.
/// @param[in]   n_A_vec_byte   - the size of column vectors in a.
/// @param[in]   n_A_width    - the width of matrix a.
/// @param[in]   b           - the vector b.
///
void gf256mat_prod_sse( uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b );



#ifdef  __cplusplus
}
#endif



#endif //  _BLAS_MATRIX_SSE_H_

