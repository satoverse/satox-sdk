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

#include "crypto_uint32.h"

crypto_uint32_signed crypto_uint32_signed_negative_mask(crypto_uint32_signed crypto_uint32_signed_x) {
    return crypto_uint32_signed_x >> 31;
}

crypto_uint32 crypto_uint32_nonzero_mask(crypto_uint32 crypto_uint32_x) {
    return crypto_uint32_signed_negative_mask(crypto_uint32_x) | crypto_uint32_signed_negative_mask(-crypto_uint32_x);
}

crypto_uint32 crypto_uint32_zero_mask(crypto_uint32 crypto_uint32_x) {
    return ~crypto_uint32_nonzero_mask(crypto_uint32_x);
}

crypto_uint32 crypto_uint32_unequal_mask(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y) {
    crypto_uint32 crypto_uint32_xy = crypto_uint32_x ^ crypto_uint32_y;
    return crypto_uint32_nonzero_mask(crypto_uint32_xy);
}

crypto_uint32 crypto_uint32_equal_mask(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y) {
    return ~crypto_uint32_unequal_mask(crypto_uint32_x, crypto_uint32_y);
}

crypto_uint32 crypto_uint32_smaller_mask(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y) {
    crypto_uint32 crypto_uint32_xy = crypto_uint32_x ^ crypto_uint32_y;
    crypto_uint32 crypto_uint32_z = crypto_uint32_x - crypto_uint32_y;
    crypto_uint32_z ^= crypto_uint32_xy & (crypto_uint32_z ^ crypto_uint32_x ^ (((crypto_uint32) 1) << 31));
    return crypto_uint32_signed_negative_mask(crypto_uint32_z);
}

crypto_uint32 crypto_uint32_min(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y) {
    crypto_uint32 crypto_uint32_xy = crypto_uint32_y ^ crypto_uint32_x;
    crypto_uint32 crypto_uint32_z = crypto_uint32_y - crypto_uint32_x;
    crypto_uint32_z ^= crypto_uint32_xy & (crypto_uint32_z ^ crypto_uint32_y ^ (((crypto_uint32) 1) << 31));
    crypto_uint32_z = crypto_uint32_signed_negative_mask(crypto_uint32_z);
    crypto_uint32_z &= crypto_uint32_xy;
    return crypto_uint32_x ^ crypto_uint32_z;
}

crypto_uint32 crypto_uint32_max(crypto_uint32 crypto_uint32_x, crypto_uint32 crypto_uint32_y) {
    crypto_uint32 crypto_uint32_xy = crypto_uint32_y ^ crypto_uint32_x;
    crypto_uint32 crypto_uint32_z = crypto_uint32_y - crypto_uint32_x;
    crypto_uint32_z ^= crypto_uint32_xy & (crypto_uint32_z ^ crypto_uint32_y ^ (((crypto_uint32) 1) << 31));
    crypto_uint32_z = crypto_uint32_signed_negative_mask(crypto_uint32_z);
    crypto_uint32_z &= crypto_uint32_xy;
    return crypto_uint32_y ^ crypto_uint32_z;
}

void crypto_uint32_minmax(crypto_uint32 *crypto_uint32_a, crypto_uint32 *crypto_uint32_b) {
    crypto_uint32 crypto_uint32_x = *crypto_uint32_a;
    crypto_uint32 crypto_uint32_y = *crypto_uint32_b;
    crypto_uint32 crypto_uint32_xy = crypto_uint32_y ^ crypto_uint32_x;
    crypto_uint32 crypto_uint32_z = crypto_uint32_y - crypto_uint32_x;
    crypto_uint32_z ^= crypto_uint32_xy & (crypto_uint32_z ^ crypto_uint32_y ^ (((crypto_uint32) 1) << 31));
    crypto_uint32_z = crypto_uint32_signed_negative_mask(crypto_uint32_z);
    crypto_uint32_z &= crypto_uint32_xy;
    *crypto_uint32_a = crypto_uint32_x ^ crypto_uint32_z;
    *crypto_uint32_b = crypto_uint32_y ^ crypto_uint32_z;
}
