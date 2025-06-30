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

#include "vec128.h"

/* input: v, an element in GF(2^m)[y]/(y^128+y^7+y^2+y+1) in bitsliced form */
/* input: a, an element in GF(2^m)[y]/(y^128+y^7+y^2+y+1) as an array of coefficients */
/* output: out, the product of v and a in bitsliced form */
void vec128_mul_GF(vec128 out[ GFBITS ], vec128 v[ GFBITS ], gf a[ SYS_T ]) {
    int i, j;
    uint64_t buf[GFBITS][4];
    vec128 prod[GFBITS];
    uint64_t p[GFBITS];

    // polynomial multiplication

    for (i = 0; i < GFBITS; i++) {
        buf[i][0] = 0;
        buf[i][1] = 0;
        buf[i][2] = 0;
        buf[i][3] = 0;
    }

    for (i = SYS_T - 1; i >= 0; i--) {
        for (j = 0; j < GFBITS; j++) {
            buf[j][3] <<= 1;
            buf[j][3] |= buf[j][2] >> 63;
            buf[j][2] <<= 1;
            buf[j][2] |= buf[j][1] >> 63;
            buf[j][1] <<= 1;
            buf[j][1] |= buf[j][0] >> 63;
            buf[j][0] <<= 1;
        }

        vec128_mul_gf(prod, v, a[i]);

        for (j = 0; j < GFBITS; j++) {
            buf[j][0] ^= vec128_extract(prod[j], 0);
            buf[j][1] ^= vec128_extract(prod[j], 1);
        }
    }

    // reduction modulo y^128+y^7+y^2+y+1

    for (i = 0; i < GFBITS; i++) {
        p[i] = buf[i][3];

        buf[i][2] ^= p[i] >> (SYS_T - 7 - 64);
        buf[i][1] ^= p[i] << (64 - (SYS_T - 7 - 64));
        buf[i][2] ^= p[i] >> (SYS_T - 2 - 64);
        buf[i][1] ^= p[i] << (64 - (SYS_T - 2 - 64));
        buf[i][2] ^= p[i] >> (SYS_T - 1 - 64);
        buf[i][1] ^= p[i] << (64 - (SYS_T - 1 - 64));
        buf[i][1] ^= p[i];
    }

    //

    for (i = 0; i < GFBITS; i++) {
        p[i] = buf[i][2];

        buf[i][1] ^= p[i] >> (SYS_T - 7 - 64);
        buf[i][0] ^= p[i] << (64 - (SYS_T - 7 - 64));
        buf[i][1] ^= p[i] >> (SYS_T - 2 - 64);
        buf[i][0] ^= p[i] << (64 - (SYS_T - 2 - 64));
        buf[i][1] ^= p[i] >> (SYS_T - 1 - 64);
        buf[i][0] ^= p[i] << (64 - (SYS_T - 1 - 64));
        buf[i][0] ^= p[i];
    }

    //

    for (i = 0; i < GFBITS; i++) {
        out[i] = vec128_set2x(buf[i][0], buf[i][1]);
    }
}
