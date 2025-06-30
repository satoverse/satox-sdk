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

#include "crypto_int32.h"

crypto_int32 crypto_int32_negative_mask(crypto_int32 crypto_int32_x) {
    return crypto_int32_x >> 31;
}

crypto_int32 crypto_int32_nonzero_mask(crypto_int32 crypto_int32_x) {
    return crypto_int32_negative_mask(crypto_int32_x) | crypto_int32_negative_mask(-crypto_int32_x);
}

crypto_int32 crypto_int32_zero_mask(crypto_int32 crypto_int32_x) {
    return ~crypto_int32_nonzero_mask(crypto_int32_x);
}

crypto_int32 crypto_int32_positive_mask(crypto_int32 crypto_int32_x) {
    crypto_int32 crypto_int32_z = -crypto_int32_x;
    crypto_int32_z ^= crypto_int32_x & crypto_int32_z;
    return crypto_int32_negative_mask(crypto_int32_z);
}

crypto_int32 crypto_int32_unequal_mask(crypto_int32 crypto_int32_x, crypto_int32 crypto_int32_y) {
    crypto_int32 crypto_int32_xy = crypto_int32_x ^ crypto_int32_y;
    return crypto_int32_nonzero_mask(crypto_int32_xy);
}

crypto_int32 crypto_int32_equal_mask(crypto_int32 crypto_int32_x, crypto_int32 crypto_int32_y) {
    return ~crypto_int32_unequal_mask(crypto_int32_x, crypto_int32_y);
}

crypto_int32 crypto_int32_smaller_mask(crypto_int32 crypto_int32_x, crypto_int32 crypto_int32_y) {
    crypto_int32 crypto_int32_xy = crypto_int32_x ^ crypto_int32_y;
    crypto_int32 crypto_int32_z = crypto_int32_x - crypto_int32_y;
    crypto_int32_z ^= crypto_int32_xy & (crypto_int32_z ^ crypto_int32_x);
    return crypto_int32_negative_mask(crypto_int32_z);
}

crypto_int32 crypto_int32_min(crypto_int32 crypto_int32_x, crypto_int32 crypto_int32_y) {
    crypto_int32 crypto_int32_xy = crypto_int32_y ^ crypto_int32_x;
    crypto_int32 crypto_int32_z = crypto_int32_y - crypto_int32_x;
    crypto_int32_z ^= crypto_int32_xy & (crypto_int32_z ^ crypto_int32_y);
    crypto_int32_z = crypto_int32_negative_mask(crypto_int32_z);
    crypto_int32_z &= crypto_int32_xy;
    return crypto_int32_x ^ crypto_int32_z;
}

crypto_int32 crypto_int32_max(crypto_int32 crypto_int32_x, crypto_int32 crypto_int32_y) {
    crypto_int32 crypto_int32_xy = crypto_int32_y ^ crypto_int32_x;
    crypto_int32 crypto_int32_z = crypto_int32_y - crypto_int32_x;
    crypto_int32_z ^= crypto_int32_xy & (crypto_int32_z ^ crypto_int32_y);
    crypto_int32_z = crypto_int32_negative_mask(crypto_int32_z);
    crypto_int32_z &= crypto_int32_xy;
    return crypto_int32_y ^ crypto_int32_z;
}

void crypto_int32_minmax(crypto_int32 *crypto_int32_a, crypto_int32 *crypto_int32_b) {
    crypto_int32 crypto_int32_x = *crypto_int32_a;
    crypto_int32 crypto_int32_y = *crypto_int32_b;
    crypto_int32 crypto_int32_xy = crypto_int32_y ^ crypto_int32_x;
    crypto_int32 crypto_int32_z = crypto_int32_y - crypto_int32_x;
    crypto_int32_z ^= crypto_int32_xy & (crypto_int32_z ^ crypto_int32_y);
    crypto_int32_z = crypto_int32_negative_mask(crypto_int32_z);
    crypto_int32_z &= crypto_int32_xy;
    *crypto_int32_a = crypto_int32_x ^ crypto_int32_z;
    *crypto_int32_b = crypto_int32_y ^ crypto_int32_z;
}
