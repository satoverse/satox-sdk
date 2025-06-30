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

#ifndef PQCLEAN_MCELIECE6688128_AVX2_crypto_uint64_h
#define PQCLEAN_MCELIECE6688128_AVX2_crypto_uint64_h

#include <inttypes.h>
typedef uint64_t crypto_uint64;

typedef int64_t crypto_uint64_signed;

#include "namespace.h"

#define crypto_uint64_signed_negative_mask CRYPTO_NAMESPACE(crypto_uint64_signed_negative_mask)
crypto_uint64_signed crypto_uint64_signed_negative_mask(crypto_uint64_signed crypto_uint64_signed_x);
#define crypto_uint64_nonzero_mask CRYPTO_NAMESPACE(crypto_uint64_nonzero_mask)
crypto_uint64 crypto_uint64_nonzero_mask(crypto_uint64 crypto_uint64_x);
#define crypto_uint64_zero_mask CRYPTO_NAMESPACE(crypto_uint64_zero_mask)
crypto_uint64 crypto_uint64_zero_mask(crypto_uint64 crypto_uint64_x);
#define crypto_uint64_unequal_mask CRYPTO_NAMESPACE(crypto_uint64_unequal_mask)
crypto_uint64 crypto_uint64_unequal_mask(crypto_uint64 crypto_uint64_x, crypto_uint64 crypto_uint64_y);
#define crypto_uint64_equal_mask CRYPTO_NAMESPACE(crypto_uint64_equal_mask)
crypto_uint64 crypto_uint64_equal_mask(crypto_uint64 crypto_uint64_x, crypto_uint64 crypto_uint64_y);
#define crypto_uint64_smaller_mask CRYPTO_NAMESPACE(crypto_uint64_smaller_mask)
crypto_uint64 crypto_uint64_smaller_mask(crypto_uint64 crypto_uint64_x, crypto_uint64 crypto_uint64_y);
#define crypto_uint64_min CRYPTO_NAMESPACE(crypto_uint64_min)
crypto_uint64 crypto_uint64_min(crypto_uint64 crypto_uint64_x, crypto_uint64 crypto_uint64_y);
#define crypto_uint64_max CRYPTO_NAMESPACE(crypto_uint64_max)
crypto_uint64 crypto_uint64_max(crypto_uint64 crypto_uint64_x, crypto_uint64 crypto_uint64_y);
#define crypto_uint64_minmax CRYPTO_NAMESPACE(crypto_uint64_minmax)
void crypto_uint64_minmax(crypto_uint64 *crypto_uint64_a, crypto_uint64 *crypto_uint64_b);

#endif
