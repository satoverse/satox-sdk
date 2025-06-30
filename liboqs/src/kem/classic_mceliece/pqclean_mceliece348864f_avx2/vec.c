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

#include "vec.h"

/* input: v, an element in GF(2^m)[y]/(y^64+y^3+y+z) in bitsliced form */
/* input: a, an element in GF(2^m)[y]/(y^64+y^3+y+z) as an array of coefficients */
/* output: out, the product of v and a in bitsliced form */
void vec_GF_mul(vec out[ GFBITS ], vec v[ GFBITS ], gf a[ SYS_T ]) {
    int i, j;
    vec buf[GFBITS][2], prod[GFBITS], tmp[GFBITS];

    const vec allone = -1;

    // polynomial multiplication

    for (i = 0; i < GFBITS; i++) {
        buf[i][0] = 0;
        buf[i][1] = 0;
    }

    for (i = SYS_T - 1; i >= 0; i--) {
        for (j = 0; j < GFBITS; j++) {
            buf[j][1] <<= 1;
            buf[j][1] |= buf[j][0] >> 63;
            buf[j][0] <<= 1;
        }

        vec_mul_gf(prod, v, a[i]);

        for (j = 0; j < GFBITS; j++) {
            buf[j][0] ^= prod[j];
        }
    }

    // reduction modulo y^64 + y^3 + y + z

    for (i = 0; i < GFBITS; i++) {
        prod[i] = buf[i][1] & (allone << 3);
    }

    vec_mul_gf(tmp, prod, 2);

    for (i = 0; i < GFBITS; i++) {
        buf[i][1] ^= prod[i] >> (SYS_T - 3);
        buf[i][0] ^= prod[i] << (64 - (SYS_T - 3));
        buf[i][1] ^= prod[i] >> (SYS_T - 1);
        buf[i][0] ^= prod[i] << (64 - (SYS_T - 1));
        buf[i][0] ^= tmp[i];
    }

    //

    for (i = 0; i < GFBITS; i++) {
        prod[i] = buf[i][1] & 0x7;
    }

    vec_mul_gf(tmp, prod, 2);

    for (i = 0; i < GFBITS; i++) {
        buf[i][0] ^= prod[i] << (64 - (SYS_T - 3));
        buf[i][0] ^= prod[i] << (64 - (SYS_T - 1));
        buf[i][0] ^= tmp[i];
    }

    //

    for (i = 0; i < GFBITS; i++) {
        out[i] = buf[i][0];
    }
}
