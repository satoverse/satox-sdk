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

#ifndef TRANSPOSE_H
#define TRANSPOSE_H
/*
  This file is for matrix transposition
*/

#include "namespace.h"

#define transpose_64x128_sp_asm CRYPTO_NAMESPACE(transpose_64x128_sp_asm)
#define transpose_64x256_sp_asm CRYPTO_NAMESPACE(transpose_64x256_sp_asm)
#define transpose_64x64_asm CRYPTO_NAMESPACE(transpose_64x64_asm)

#include "vec128.h"
#include "vec256.h"

extern void transpose_64x64_asm(uint64_t *);
extern void transpose_64x128_sp_asm(vec128 *);

/* input: in, a 64x64 matrix over GF(2) */
/* output: out, transpose of in */
static inline void transpose_64x64(uint64_t *in) {
    transpose_64x64_asm(in);
}

static inline void transpose_64x128_sp(vec128 *in) {
    transpose_64x128_sp_asm(in);
}

extern void transpose_64x256_sp_asm(vec256 *);

static inline void transpose_64x256_sp(vec256 *in) {
    transpose_64x256_sp_asm(in);
}

#endif
