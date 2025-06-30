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
  This file is for Niederreiter decryption
*/

#include "decrypt.h"
#include <stdio.h>

#include "benes.h"
#include "bm.h"
#include "gf.h"
#include "params.h"
#include "root.h"
#include "synd.h"
#include "util.h"

/* Niederreiter decryption with the Berlekamp decoder */
/* intput: sk, secret key */
/*         c, ciphertext */
/* output: e, error vector */
/* return: 0 for success; 1 for failure */
int decrypt(unsigned char *e, const unsigned char *sk, const unsigned char *c) {
    int i, w = 0;
    uint16_t check;

    unsigned char r[ SYS_N / 8 ];

    gf g[ SYS_T + 1 ];
    gf L[ SYS_N ];

    gf s[ SYS_T * 2 ];
    gf s_cmp[ SYS_T * 2 ];
    gf locator[ SYS_T + 1 ];
    gf images[ SYS_N ];

    gf t;

    //

    for (i = 0; i < SYND_BYTES; i++) {
        r[i] = c[i];
    }
    for (i = SYND_BYTES; i < SYS_N / 8; i++) {
        r[i] = 0;
    }

    for (i = 0; i < SYS_T; i++) {
        g[i] = load_gf(sk);
        sk += 2;
    }
    g[ SYS_T ] = 1;

    support_gen(L, sk);

    synd(s, g, L, r);

    bm(locator, s);

    root(images, locator, L);

    //

    for (i = 0; i < SYS_N / 8; i++) {
        e[i] = 0;
    }

    for (i = 0; i < SYS_N; i++) {
        t = gf_iszero(images[i]) & 1;

        e[ i / 8 ] |= t << (i % 8);
        w += t;

    }

    synd(s_cmp, g, L, e);

    //

    check = (uint16_t)w;
    check ^= SYS_T;

    for (i = 0; i < SYS_T * 2; i++) {
        check |= s[i] ^ s_cmp[i];
    }

    check -= 1;
    check >>= 15;

    return check ^ 1;
}
