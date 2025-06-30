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

#include "crypto_int16.h"

crypto_int16 crypto_int16_negative_mask(crypto_int16 crypto_int16_x) {
    return crypto_int16_x >> 15;
}

crypto_int16 crypto_int16_nonzero_mask(crypto_int16 crypto_int16_x) {
    return crypto_int16_negative_mask(crypto_int16_x) | crypto_int16_negative_mask(-crypto_int16_x);
}

crypto_int16 crypto_int16_zero_mask(crypto_int16 crypto_int16_x) {
    return ~crypto_int16_nonzero_mask(crypto_int16_x);
}

crypto_int16 crypto_int16_positive_mask(crypto_int16 crypto_int16_x) {
    crypto_int16 crypto_int16_z = -crypto_int16_x;
    crypto_int16_z ^= crypto_int16_x & crypto_int16_z;
    return crypto_int16_negative_mask(crypto_int16_z);
}

crypto_int16 crypto_int16_unequal_mask(crypto_int16 crypto_int16_x, crypto_int16 crypto_int16_y) {
    crypto_int16 crypto_int16_xy = crypto_int16_x ^ crypto_int16_y;
    return crypto_int16_nonzero_mask(crypto_int16_xy);
}

crypto_int16 crypto_int16_equal_mask(crypto_int16 crypto_int16_x, crypto_int16 crypto_int16_y) {
    return ~crypto_int16_unequal_mask(crypto_int16_x, crypto_int16_y);
}

crypto_int16 crypto_int16_smaller_mask(crypto_int16 crypto_int16_x, crypto_int16 crypto_int16_y) {
    crypto_int16 crypto_int16_xy = crypto_int16_x ^ crypto_int16_y;
    crypto_int16 crypto_int16_z = crypto_int16_x - crypto_int16_y;
    crypto_int16_z ^= crypto_int16_xy & (crypto_int16_z ^ crypto_int16_x);
    return crypto_int16_negative_mask(crypto_int16_z);
}

crypto_int16 crypto_int16_min(crypto_int16 crypto_int16_x, crypto_int16 crypto_int16_y) {
    crypto_int16 crypto_int16_xy = crypto_int16_y ^ crypto_int16_x;
    crypto_int16 crypto_int16_z = crypto_int16_y - crypto_int16_x;
    crypto_int16_z ^= crypto_int16_xy & (crypto_int16_z ^ crypto_int16_y);
    crypto_int16_z = crypto_int16_negative_mask(crypto_int16_z);
    crypto_int16_z &= crypto_int16_xy;
    return crypto_int16_x ^ crypto_int16_z;
}

crypto_int16 crypto_int16_max(crypto_int16 crypto_int16_x, crypto_int16 crypto_int16_y) {
    crypto_int16 crypto_int16_xy = crypto_int16_y ^ crypto_int16_x;
    crypto_int16 crypto_int16_z = crypto_int16_y - crypto_int16_x;
    crypto_int16_z ^= crypto_int16_xy & (crypto_int16_z ^ crypto_int16_y);
    crypto_int16_z = crypto_int16_negative_mask(crypto_int16_z);
    crypto_int16_z &= crypto_int16_xy;
    return crypto_int16_y ^ crypto_int16_z;
}

void crypto_int16_minmax(crypto_int16 *crypto_int16_a, crypto_int16 *crypto_int16_b) {
    crypto_int16 crypto_int16_x = *crypto_int16_a;
    crypto_int16 crypto_int16_y = *crypto_int16_b;
    crypto_int16 crypto_int16_xy = crypto_int16_y ^ crypto_int16_x;
    crypto_int16 crypto_int16_z = crypto_int16_y - crypto_int16_x;
    crypto_int16_z ^= crypto_int16_xy & (crypto_int16_z ^ crypto_int16_y);
    crypto_int16_z = crypto_int16_negative_mask(crypto_int16_z);
    crypto_int16_z &= crypto_int16_xy;
    *crypto_int16_a = crypto_int16_x ^ crypto_int16_z;
    *crypto_int16_b = crypto_int16_y ^ crypto_int16_z;
}
