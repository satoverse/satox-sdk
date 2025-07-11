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
/// @file blas_matrix_neon.h
/// @brief linear algebra functions for matrix operations, specialized for neon instruction set.
///

#ifndef _BLAS_MATRIX_NEON_H_
#define _BLAS_MATRIX_NEON_H_


#include "stdint.h"
#include "params.h"


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
#define gf16mat_prod_multab_neon PQOV_NAMESPACE(gf16mat_prod_multab_neon)
void gf16mat_prod_multab_neon( uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b_multab );

/// @brief  c = mat_a * b , GF(16)
///
/// @param[out]   c         - the output vector c
/// @param[in]   mat_a       - the matrix a.
/// @param[in]   a_h_byte    - the size of column vectors in a.
/// @param[in]   a_w        - the width of matrix a.
/// @param[in]   b           - the vector b.
///
#define gf16mat_prod_neon PQOV_NAMESPACE(gf16mat_prod_neon)
void gf16mat_prod_neon( uint8_t *c, const uint8_t *mat_a, unsigned a_h_byte, unsigned a_w, const uint8_t *b );


/// @brief Gaussian elimination to row-echelon form for a linear system, in GF(16)
///
/// @param[in,out]  sq_col_mat_a     - input of col-major-sq-matrix a. output of row-major-sq-matrix.
/// @param[in,out]  constant         - constant terms of the linear system.
/// @param[in]      len              - the dimension of input matrix a.
/// @return   1(true) if success. 0(false) if the matrix is singular.
///
#define gf16mat_gaussian_elim_neon PQOV_NAMESPACE(gf16mat_gaussian_elim_neon)
unsigned gf16mat_gaussian_elim_neon(uint8_t *sq_col_mat_a, uint8_t *constant, unsigned len);

/// @brief Performing back-substituion to solve a linear system, in GF(16)
///
/// @param[in,out]  constant   - constant terms of the linear system.
/// @param[in]   sq_row_mat_a  - row echelon form of the square row-major matrix a.
/// @param[in]   len           - the dimension of input matrix a.
///
#define gf16mat_back_substitute_neon PQOV_NAMESPACE(gf16mat_back_substitute_neon)
void gf16mat_back_substitute_neon( uint8_t *constant, const uint8_t *sq_row_mat_a, unsigned len);


///////////////////////////////  GF( 256 ) ////////////////////////////////////////////////////


/// @brief  c = matA * b , GF(256)
///
/// @param[out]   c         - the output vector c
/// @param[in]   matA       - the matrix A.
/// @param[in]   n_A_vec_byte    - the size of column vectors in A.
/// @param[in]   n_A_width       - the widht of matrix A.
/// @param[in]   b_multab        - the vector b, in multiplication tables.
///
#define gf256mat_prod_multab_neon PQOV_NAMESPACE(gf256mat_prod_multab_neon)
void gf256mat_prod_multab_neon( uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *multab );

/// @brief  c = matA * b , GF(256)
///
/// @param[out]   c         - the output vector c
/// @param[in]   matA       - the matrix A.
/// @param[in]   n_A_vec_byte   - the size of column vectors in a.
/// @param[in]   n_A_width    - the width of matrix a.
/// @param[in]   b           - the vector b.
///
#define gf256mat_prod_neon PQOV_NAMESPACE(gf256mat_prod_neon)
void gf256mat_prod_neon( uint8_t *c, const uint8_t *matA, unsigned n_A_vec_byte, unsigned n_A_width, const uint8_t *b );


/// @brief Gaussian elimination to row-echelon form for a linear system, in GF(256)
///
/// @param[in,out]  sq_col_mat_a     - input of col-major-sq-matrix a. output of row-major-sq-matrix.
/// @param[in,out]  constant         - constant terms of the linear system.
/// @param[in]      len              - the dimension of input matrix a.
/// @return   1(true) if success. 0(false) if the matrix is singular.
///
#define gf256mat_gaussian_elim_neon PQOV_NAMESPACE(gf256mat_gaussian_elim_neon)
unsigned gf256mat_gaussian_elim_neon(uint8_t *sq_col_mat_a, uint8_t *constant, unsigned len);

/// @brief Performing back-substituion to solve a linear system, in GF(256)
///
/// @param[in,out]  constant   - constant terms of the linear system.
/// @param[in]   sq_row_mat_a  - row echelon form of the square row-major matrix a.
/// @param[in]   len           - the dimension of input matrix a.
///
#define gf256mat_back_substitute_neon PQOV_NAMESPACE(gf256mat_back_substitute_neon)
void gf256mat_back_substitute_neon( uint8_t *constant, const uint8_t *sq_row_mat_a, unsigned len);



#ifdef  __cplusplus
}
#endif



#endif //  _BLAS_MATRIX_NEON_H_

