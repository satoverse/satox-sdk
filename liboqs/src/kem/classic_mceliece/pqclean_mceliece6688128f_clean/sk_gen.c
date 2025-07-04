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
  This file is for secret-key generation
*/

#include "sk_gen.h"

#include "controlbits.h"
#include "crypto_declassify.h"
#include "crypto_uint16.h"
#include "gf.h"
#include "params.h"
#include "randombytes.h"
#include "util.h"

static inline crypto_uint16 gf_is_zero_declassify(gf t) {
    crypto_uint16 mask = crypto_uint16_zero_mask(t);
    crypto_declassify(&mask, sizeof mask);
    return mask;
}

/* input: f, element in GF((2^m)^t) */
/* output: out, minimal polynomial of f */
/* return: 0 for success and -1 for failure */
int genpoly_gen(gf *out, gf *f) {
    int i, j, k, c;

    gf mat[ SYS_T + 1 ][ SYS_T ];
    gf mask, inv, t;

    // fill matrix

    mat[0][0] = 1;

    for (i = 1; i < SYS_T; i++) {
        mat[0][i] = 0;
    }

    for (i = 0; i < SYS_T; i++) {
        mat[1][i] = f[i];
    }

    for (j = 2; j <= SYS_T; j++) {
        GF_mul(mat[j], mat[j - 1], f);
    }

    // gaussian

    for (j = 0; j < SYS_T; j++) {
        for (k = j + 1; k < SYS_T; k++) {
            mask = gf_iszero(mat[ j ][ j ]);

            for (c = j; c < SYS_T + 1; c++) {
                mat[ c ][ j ] ^= mat[ c ][ k ] & mask;
            }

        }

        if ( gf_is_zero_declassify(mat[ j ][ j ]) ) { // return if not systematic
            return -1;
        }

        inv = gf_inv(mat[j][j]);

        for (c = j; c < SYS_T + 1; c++) {
            mat[ c ][ j ] = gf_mul(mat[ c ][ j ], inv) ;
        }

        for (k = 0; k < SYS_T; k++) {
            if (k != j) {
                t = mat[ j ][ k ];

                for (c = j; c < SYS_T + 1; c++) {
                    mat[ c ][ k ] ^= gf_mul(mat[ c ][ j ], t);
                }
            }
        }
    }

    for (i = 0; i < SYS_T; i++) {
        out[i] = mat[ SYS_T ][ i ];
    }

    return 0;
}
