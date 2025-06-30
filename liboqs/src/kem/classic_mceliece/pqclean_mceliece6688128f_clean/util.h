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

#ifndef UTIL_H
#define UTIL_H
/*
  This file is for loading/storing data in a little-endian fashion
*/

#include "namespace.h"

#define bitrev CRYPTO_NAMESPACE(bitrev)
#define load4 CRYPTO_NAMESPACE(load4)
#define load8 CRYPTO_NAMESPACE(load8)
#define load_gf CRYPTO_NAMESPACE(load_gf)
#define store8 CRYPTO_NAMESPACE(store8)
#define store_gf CRYPTO_NAMESPACE(store_gf)

#include "gf.h"
#include <inttypes.h>

void store_gf(unsigned char *dest, gf a);
uint16_t load_gf(const unsigned char *src);

uint32_t load4(const unsigned char *in);

void store8(unsigned char *out, uint64_t in);
uint64_t load8(const unsigned char *in);

gf bitrev(gf a);

#endif
