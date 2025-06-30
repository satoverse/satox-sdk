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
/// @file gf16_tabs.h
/// @brief Defining the constants for performing GF arithmetics.
///

#ifndef _GF16_TABS_NEON_H_
#define _GF16_TABS_NEON_H_

#include <stdint.h>
#include "params.h"

#ifdef  __cplusplus
extern  "C" {
#endif

#define __0_f PQOV_NAMESPACE(__0_f)
extern const unsigned char __0_f[];
#define __gf16_reduce PQOV_NAMESPACE(__gf16_reduce)
extern const unsigned char __gf16_reduce[];

#define __gf16_inv PQOV_NAMESPACE(__gf16_inv)
extern const unsigned char __gf16_inv[];
#define __gf16_squ PQOV_NAMESPACE(__gf16_squ)
extern const unsigned char __gf16_squ[];
#define __gf16_mulbase PQOV_NAMESPACE(__gf16_mulbase)
extern const unsigned char __gf16_mulbase[];
#define __gf16_mul PQOV_NAMESPACE(__gf16_mul)
extern const unsigned char __gf16_mul[];

#define __gf256_bit8_11_reduce PQOV_NAMESPACE(__gf256_bit8_11_reduce)
extern const unsigned char __gf256_bit8_11_reduce[];
#define __gf256_bit12_15_reduce PQOV_NAMESPACE(__gf256_bit12_15_reduce)
extern const unsigned char __gf256_bit12_15_reduce[];
#define __gf256_squ PQOV_NAMESPACE(__gf256_squ)
extern const unsigned char __gf256_squ[];
#define __gf256_mulbase PQOV_NAMESPACE(__gf256_mulbase)
extern const unsigned char __gf256_mulbase[];
#define __gf256_mul PQOV_NAMESPACE(__gf256_mul)
extern const unsigned char __gf256_mul[];


#ifdef  __cplusplus
}
#endif



#endif // _GF16_TABS_NEON_H_
