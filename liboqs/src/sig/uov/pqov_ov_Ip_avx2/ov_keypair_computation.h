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
/// @file ov_keypair_computation.h
/// @brief Functions for calculating pk/sk while generating keys.
///
/// Defining an internal structure of public key.
/// Functions for calculating pk/sk for key generation.
///

#ifndef _ov_KEYPAIR_COMP_H_
#define _ov_KEYPAIR_COMP_H_

#include "ov_keypair.h"

//IF_CRYPTO_CORE:define CRYPTO_NAMESPACE

#ifdef  __cplusplus
extern  "C" {
#endif

///
/// @brief Computing parts of the sk from parts of pk and sk
///
/// @param[out] Fs       - secret key
///
#define ov_pkc_calculate_F_from_Q PQOV_NAMESPACE(ov_pkc_calculate_F_from_Q)
void ov_pkc_calculate_F_from_Q( sk_t *Fs);

///
/// @brief Computing parts of the pk from the secret key
///
/// @param[out] Qs       - parts of the pk: P3
/// @param[in]  Fs       - parts of the sk: P1, S
/// @param[in]  Ts       - parts of the sk: O
///
#define ov_pkc_calculate_Q_from_F PQOV_NAMESPACE(ov_pkc_calculate_Q_from_F)
void ov_pkc_calculate_Q_from_F( cpk_t *Qs, const sk_t *Fs, const sk_t *Ts );



///
/// @brief Computing sk S and pk P2 from P1, P2, and sk O
///
/// @param[out] F2       - S of the sk
/// @param[out] P3       - P3 of the pk
/// @param[in]  P1       - P1 of the pk
/// @param[in]  P2       - P2 of the pk
/// @param[in]  sk       - O of the sk
///
#define calculate_F2_P3 PQOV_NAMESPACE(calculate_F2_P3)
void calculate_F2_P3( unsigned char *S, unsigned char *P3, const unsigned char *P1, const unsigned char *P2, const unsigned char *sk_O );

///
/// @brief Computing sk F2 from P1, P2, and sk O
///
/// @param[out] F2       - S of the sk
/// @param[in]  P1       - P1 of the pk
/// @param[in]  P2       - P2 of the pk
/// @param[in]  sk       - O of the sk
///
#define calculate_F2 PQOV_NAMESPACE(calculate_F2)
void calculate_F2( unsigned char *S, const unsigned char *P1, const unsigned char *P2, const unsigned char *sk_O );

///
/// @brief Computing P3 of the pk from P1, P2, and sk O
///
/// @param[out] P3       - P3 of the pk
/// @param[in]  P1       - P1 of the pk
/// @param[in]  P2       - P2 of the pk
/// @param[in]  sk       - O of the sk
///
#define calculate_P3 PQOV_NAMESPACE(calculate_P3)
void calculate_P3( unsigned char *P3, const unsigned char *P1, const unsigned char *P2, const unsigned char *sk_O );




#ifdef  __cplusplus
}
#endif

#endif  // _ov_KEYPAIR_COMP_H_

