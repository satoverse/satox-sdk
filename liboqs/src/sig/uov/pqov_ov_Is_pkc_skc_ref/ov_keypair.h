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
/// @file ov_keypair.h
/// @brief Formats of key pairs and functions for generating key pairs.
/// Formats of key pairs and functions for generating key pairs.
///

#ifndef _ov_KEYPAIR_H_
#define _ov_KEYPAIR_H_


#include "params.h"

#ifdef  __cplusplus
extern  "C" {
#endif

/// alignment 1 for sturct
#pragma pack(push,1)


/// @brief public key for classic ov
///
///  public key for classic ov
///
typedef
struct {
    unsigned char pk[_PK_BYTE];
} pk_t;


/// @brief secret key for classic ov
///
/// secret key for classic ov
///
typedef
struct {
    unsigned char sk_seed[LEN_SKSEED];   ///< seed for generating secret key

    unsigned char O[_V_BYTE * _O]; ///< T map

    unsigned char P1[_PK_P1_BYTE];  ///< part of C-map, P1
    unsigned char S[_PK_P2_BYTE];                 ///< part of C-map, S
} sk_t;



/// @brief compressed public key
///
///  compressed public key
///
typedef
struct {
    unsigned char pk_seed[LEN_PKSEED];                      ///< seed for generating l1_Q1,l1_Q2
    unsigned char P3[_PK_P3_BYTE];
} cpk_t;



/// @brief compressed secret key
///
/// compressed secret key
///
typedef
struct {
    unsigned char sk_seed[LEN_SKSEED];   ///< seed for generating a part of secret key.
} csk_t;





/// restores alignment
#pragma pack(pop)


/////////////////////////////////////






#ifdef  __cplusplus
}
#endif

#endif //  _ov_KEYPAIR_H_
