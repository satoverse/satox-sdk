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
  This file is for the Berlekamp-Massey algorithm
  see http://crypto.stanford.edu/~mironov/cs359/massey.pdf
*/

#include "bm.h"
#include "params.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

/* the Berlekamp-Massey algorithm */
/* input: s, sequence of field elements */
/* output: out, minimal polynomial of s */
void bm(gf *out, gf *s) {
    int i;

    uint16_t N = 0;
    uint16_t L = 0;
    uint16_t mle;
    uint16_t mne;

    gf T[ SYS_T + 1  ];
    gf C[ SYS_T + 1 ];
    gf B[ SYS_T + 1 ];

    gf b = 1, d, f;

    //

    for (i = 0; i < SYS_T + 1; i++) {
        C[i] = B[i] = 0;
    }

    B[1] = C[0] = 1;

    //

    for (N = 0; N < 2 * SYS_T; N++) {
        d = 0;

        for (i = 0; i <= min(N, SYS_T); i++) {
            d ^= gf_mul(C[i], s[ N - i]);
        }

        mne = d;
        mne -= 1;
        mne >>= 15;
        mne -= 1;
        mle = N;
        mle -= 2 * L;
        mle >>= 15;
        mle -= 1;
        mle &= mne;

        for (i = 0; i <= SYS_T; i++) {
            T[i] = C[i];
        }

        f = gf_frac(b, d);

        for (i = 0; i <= SYS_T; i++) {
            C[i] ^= gf_mul(f, B[i]) & mne;
        }

        L = (L & ~mle) | ((N + 1 - L) & mle);

        for (i = 0; i <= SYS_T; i++) {
            B[i] = (B[i] & ~mle) | (T[i] & mle);
        }

        b = (b & ~mle) | (d & mle);

        for (i = SYS_T; i >= 1; i--) {
            B[i] = B[i - 1];
        }
        B[0] = 0;
    }

    for (i = 0; i <= SYS_T; i++) {
        out[i] = C[ SYS_T - i ];
    }
}
