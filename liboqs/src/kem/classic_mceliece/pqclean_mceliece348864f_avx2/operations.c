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

#include "operations.h"

#include "controlbits.h"
#include "crypto_hash.h"
#include "decrypt.h"
#include "params.h"
#include "pk_gen.h"
#include "randombytes.h"
#include "sk_gen.h"
#include "util.h"

#include <stdint.h>
#include <string.h>

/* Include last because of issues with unistd.h's encrypt definition */
#include "encrypt.h"

int crypto_kem_enc(
    unsigned char *c,
    unsigned char *key,
    const unsigned char *pk
) {
    unsigned char e[ SYS_N / 8 ];
    unsigned char one_ec[ 1 + SYS_N / 8 + SYND_BYTES ] = {1};

    //

    encrypt(c, pk, e);

    memcpy(one_ec + 1, e, SYS_N / 8);
    memcpy(one_ec + 1 + SYS_N / 8, c, SYND_BYTES);

    crypto_hash_32b(key, one_ec, sizeof(one_ec));

    return 0;
}

int crypto_kem_dec(
    unsigned char *key,
    const unsigned char *c,
    const unsigned char *sk
) {
    int i;

    unsigned char ret_decrypt = 0;

    uint16_t m;

    unsigned char e[ SYS_N / 8 ];
    unsigned char preimage[ 1 + SYS_N / 8 + SYND_BYTES ];
    unsigned char *x = preimage;
    const unsigned char *s = sk + 40 + IRR_BYTES + COND_BYTES;

    //

    ret_decrypt = (unsigned char)decrypt(e, sk + 40, c);

    m = ret_decrypt;
    m -= 1;
    m >>= 8;

    *x++ = m & 1;
    for (i = 0; i < SYS_N / 8; i++) {
        *x++ = (~m & s[i]) | (m & e[i]);
    }

    for (i = 0; i < SYND_BYTES; i++) {
        *x++ = c[i];
    }

    crypto_hash_32b(key, preimage, sizeof(preimage));

    return 0;
}

int crypto_kem_keypair
(
    unsigned char *pk,
    unsigned char *sk
) {
    int i;
    unsigned char seed[ 33 ] = {64};
    unsigned char r[ SYS_N / 8 + (1 << GFBITS)*sizeof(uint32_t) + SYS_T * 2 + 32 ];
    unsigned char *rp, *skp;
    uint64_t pivots;

    gf f[ SYS_T ]; // element in GF(2^mt)
    gf irr[ SYS_T ]; // Goppa polynomial
    uint32_t perm[ 1 << GFBITS ]; // random permutation as 32-bit integers
    int16_t pi[ 1 << GFBITS ]; // random permutation

    randombytes(seed + 1, 32);

    while (1) {
        rp = &r[ sizeof(r) - 32 ];
        skp = sk;

        // expanding and updating the seed

        shake(r, sizeof(r), seed, 33);
        memcpy(skp, seed + 1, 32);
        skp += 32 + 8;
        memcpy(seed + 1, &r[ sizeof(r) - 32 ], 32);

        // generating irreducible polynomial

        rp -= sizeof(f);

        for (i = 0; i < SYS_T; i++) {
            f[i] = load_gf(rp + i * 2);
        }

        if (genpoly_gen(irr, f)) {
            continue;
        }

        for (i = 0; i < SYS_T; i++) {
            store_gf(skp + i * 2, irr[i]);
        }

        skp += IRR_BYTES;

        // generating permutation

        rp -= sizeof(perm);

        for (i = 0; i < (1 << GFBITS); i++) {
            perm[i] = load4(rp + i * 4);
        }

        if (pk_gen(pk, skp - IRR_BYTES, perm, pi, &pivots)) {
            continue;
        }

        controlbitsfrompermutation(skp, pi, GFBITS, 1 << GFBITS);
        skp += COND_BYTES;

        // storing the random string s

        rp -= SYS_N / 8;
        memcpy(skp, rp, SYS_N / 8);

        // storing positions of the 32 pivots

        store8(sk + 32, pivots);

        break;
    }

    return 0;
}
