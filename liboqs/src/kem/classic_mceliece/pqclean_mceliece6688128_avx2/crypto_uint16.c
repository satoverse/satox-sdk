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

#include "crypto_uint16.h"

crypto_uint16_signed crypto_uint16_signed_negative_mask(crypto_uint16_signed crypto_uint16_signed_x) {
    return crypto_uint16_signed_x >> 15;
}

crypto_uint16 crypto_uint16_nonzero_mask(crypto_uint16 crypto_uint16_x) {
    return crypto_uint16_signed_negative_mask(crypto_uint16_x) | crypto_uint16_signed_negative_mask(-crypto_uint16_x);
}

crypto_uint16 crypto_uint16_zero_mask(crypto_uint16 crypto_uint16_x) {
    return ~crypto_uint16_nonzero_mask(crypto_uint16_x);
}

crypto_uint16 crypto_uint16_unequal_mask(crypto_uint16 crypto_uint16_x, crypto_uint16 crypto_uint16_y) {
    crypto_uint16 crypto_uint16_xy = crypto_uint16_x ^ crypto_uint16_y;
    return crypto_uint16_nonzero_mask(crypto_uint16_xy);
}

crypto_uint16 crypto_uint16_equal_mask(crypto_uint16 crypto_uint16_x, crypto_uint16 crypto_uint16_y) {
    return ~crypto_uint16_unequal_mask(crypto_uint16_x, crypto_uint16_y);
}

crypto_uint16 crypto_uint16_smaller_mask(crypto_uint16 crypto_uint16_x, crypto_uint16 crypto_uint16_y) {
    crypto_uint16 crypto_uint16_xy = crypto_uint16_x ^ crypto_uint16_y;
    crypto_uint16 crypto_uint16_z = crypto_uint16_x - crypto_uint16_y;
    crypto_uint16_z ^= crypto_uint16_xy & (crypto_uint16_z ^ crypto_uint16_x ^ (((crypto_uint16) 1) << 15));
    return crypto_uint16_signed_negative_mask(crypto_uint16_z);
}

crypto_uint16 crypto_uint16_min(crypto_uint16 crypto_uint16_x, crypto_uint16 crypto_uint16_y) {
    crypto_uint16 crypto_uint16_xy = crypto_uint16_y ^ crypto_uint16_x;
    crypto_uint16 crypto_uint16_z = crypto_uint16_y - crypto_uint16_x;
    crypto_uint16_z ^= crypto_uint16_xy & (crypto_uint16_z ^ crypto_uint16_y ^ (((crypto_uint16) 1) << 15));
    crypto_uint16_z = crypto_uint16_signed_negative_mask(crypto_uint16_z);
    crypto_uint16_z &= crypto_uint16_xy;
    return crypto_uint16_x ^ crypto_uint16_z;
}

crypto_uint16 crypto_uint16_max(crypto_uint16 crypto_uint16_x, crypto_uint16 crypto_uint16_y) {
    crypto_uint16 crypto_uint16_xy = crypto_uint16_y ^ crypto_uint16_x;
    crypto_uint16 crypto_uint16_z = crypto_uint16_y - crypto_uint16_x;
    crypto_uint16_z ^= crypto_uint16_xy & (crypto_uint16_z ^ crypto_uint16_y ^ (((crypto_uint16) 1) << 15));
    crypto_uint16_z = crypto_uint16_signed_negative_mask(crypto_uint16_z);
    crypto_uint16_z &= crypto_uint16_xy;
    return crypto_uint16_y ^ crypto_uint16_z;
}

void crypto_uint16_minmax(crypto_uint16 *crypto_uint16_a, crypto_uint16 *crypto_uint16_b) {
    crypto_uint16 crypto_uint16_x = *crypto_uint16_a;
    crypto_uint16 crypto_uint16_y = *crypto_uint16_b;
    crypto_uint16 crypto_uint16_xy = crypto_uint16_y ^ crypto_uint16_x;
    crypto_uint16 crypto_uint16_z = crypto_uint16_y - crypto_uint16_x;
    crypto_uint16_z ^= crypto_uint16_xy & (crypto_uint16_z ^ crypto_uint16_y ^ (((crypto_uint16) 1) << 15));
    crypto_uint16_z = crypto_uint16_signed_negative_mask(crypto_uint16_z);
    crypto_uint16_z &= crypto_uint16_xy;
    *crypto_uint16_a = crypto_uint16_x ^ crypto_uint16_z;
    *crypto_uint16_b = crypto_uint16_y ^ crypto_uint16_z;
}
