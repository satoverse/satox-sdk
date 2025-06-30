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

#ifndef PQCLEAN_MCELIECE6688128_AVX2_crypto_uint32_h
#define PQCLEAN_MCELIECE6688128_AVX2_crypto_uint32_h

#include <inttypes.h>
typedef uint32_t crypto_uint32;

typedef int32_t crypto_uint32_signed;

#include "namespace.h"

#define crypto_uint32_signed_negative_mask CRYPTO_NAMESPACE(crypto_uint32_signed_negative_mask)
crypto_uint32_signed crypto_uint32_signed_negative_mask(crypto_uint32_signed crypto_uint32_signed_x);
#define crypto_uint32_nonzero_mask CRYPTO_NAMESPACE(crypto_uint32_nonzero_mask)
crypto_uint32 crypto_uint32_nonzero_mask(crypto_uint32 crypto_uint32_x);
#define crypto_uint32_zero_mask CRYPTO_NAMESPACE(crypto_uint32_zero_mask)
crypto_uint32 crypto_uint32_zero_mask(crypto_uint32 crypto_uint32_x);
#define crypto_uint32_unequal_mask CRYPTO_NAMESPACE(crypto_uint32_unequal_mask)
crypto_uint32 crypto_uint32_unequal_mask(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y);
#define crypto_uint32_equal_mask CRYPTO_NAMESPACE(crypto_uint32_equal_mask)
crypto_uint32 crypto_uint32_equal_mask(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y);
#define crypto_uint32_smaller_mask CRYPTO_NAMESPACE(crypto_uint32_smaller_mask)
crypto_uint32 crypto_uint32_smaller_mask(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y);
#define crypto_uint32_min CRYPTO_NAMESPACE(crypto_uint32_min)
crypto_uint32 crypto_uint32_min(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y);
#define crypto_uint32_max CRYPTO_NAMESPACE(crypto_uint32_max)
crypto_uint32 crypto_uint32_max(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y);
#define crypto_uint32_minmax CRYPTO_NAMESPACE(crypto_uint32_minmax)
void crypto_uint32_minmax(crypto_uint32 *crypto_uint32_a, crypto_uint32 *crypto_uint32_b);

#endif
