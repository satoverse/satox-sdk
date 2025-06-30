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

#ifndef CONFIG_H
#define CONFIG_H

//#define DILITHIUM_MODE 2
#define DILITHIUM_RANDOMIZED_SIGNING
//#define USE_RDPMC
//#define DBENCH

#ifndef DILITHIUM_MODE
#define DILITHIUM_MODE 2
#endif

#if DILITHIUM_MODE == 2
#define CRYPTO_ALGNAME "ML-DSA-44"
#define DILITHIUM_NAMESPACETOP pqcrystals_ml_dsa_44_ref
#define DILITHIUM_NAMESPACE(s) pqcrystals_ml_dsa_44_ref_##s
#elif DILITHIUM_MODE == 3
#define CRYPTO_ALGNAME "ML-DSA-65"
#define DILITHIUM_NAMESPACETOP pqcrystals_ml_dsa_65_ref
#define DILITHIUM_NAMESPACE(s) pqcrystals_ml_dsa_65_ref_##s
#elif DILITHIUM_MODE == 5
#define CRYPTO_ALGNAME "ML-DSA-87"
#define DILITHIUM_NAMESPACETOP pqcrystals_ml_dsa_87_ref
#define DILITHIUM_NAMESPACE(s) pqcrystals_ml_dsa_87_ref_##s
#endif

#endif
