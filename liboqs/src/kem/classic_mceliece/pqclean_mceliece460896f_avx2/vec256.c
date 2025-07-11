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

/*
  This file is for functions related to 256-bit vectors
  including functions for bitsliced field operations
*/

#include "vec256.h"

/* bitsliced field squarings */
void vec256_sq(vec256 *out, vec256 *in) {
    int i;
    vec256 result[GFBITS], t;

    t = vec256_xor(in[11], in[12]);

    result[0] = vec256_xor(in[0], in[11]);
    result[1] = vec256_xor(in[7], t);
    result[2] = vec256_xor(in[1], in[7]);
    result[3] = vec256_xor(in[8], t);
    result[4] = vec256_xor(in[2], in[7]);
    result[4] = vec256_xor(result[4], in[8]);
    result[4] = vec256_xor(result[4], t);
    result[5] = vec256_xor(in[7], in[9]);
    result[6] = vec256_xor(in[3], in[8]);
    result[6] = vec256_xor(result[6], in[9]);
    result[6] = vec256_xor(result[6], in[12]);
    result[7] = vec256_xor(in[8], in[10]);
    result[8] = vec256_xor(in[4], in[9]);
    result[8] = vec256_xor(result[8], in[10]);
    result[9] = vec256_xor(in[9], in[11]);
    result[10] = vec256_xor(in[5], in[10]);
    result[10] = vec256_xor(result[10], in[11]);
    result[11] = vec256_xor(in[10], in[12]);
    result[12] = vec256_xor(in[6], t);

    for (i = 0; i < GFBITS; i++) {
        out[i] = result[i];
    }
}

/* bitsliced field inverses */
void vec256_inv(vec256 *out, vec256 *in) {
    vec256 tmp_11[ GFBITS ];
    vec256 tmp_1111[ GFBITS ];

    vec256_copy(out, in);

    vec256_sq(out, out);
    vec256_mul(tmp_11, out, in); // ^11

    vec256_sq(out, tmp_11);
    vec256_sq(out, out);
    vec256_mul(tmp_1111, out, tmp_11); // ^1111

    vec256_sq(out, tmp_1111);
    vec256_sq(out, out);
    vec256_sq(out, out);
    vec256_sq(out, out);
    vec256_mul(out, out, tmp_1111); // ^11111111

    vec256_sq(out, out);
    vec256_sq(out, out);
    vec256_sq(out, out);
    vec256_sq(out, out);
    vec256_mul(out, out, tmp_1111); // ^111111111111

    vec256_sq(out, out); // ^1111111111110
}
