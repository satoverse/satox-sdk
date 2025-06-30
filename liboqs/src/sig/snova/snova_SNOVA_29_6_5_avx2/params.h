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

#ifndef PARAMS_H
#define PARAMS_H

#ifndef sk_is_seed
#define sk_is_seed 1
#endif

#ifndef PK_EXPAND_SHAKE
#define PK_EXPAND_SHAKE 1
#endif

#ifndef v_SNOVA
#define v_SNOVA 24
#endif

#ifndef o_SNOVA
#define o_SNOVA 5
#endif

#ifndef l_SNOVA
#define l_SNOVA 4
#endif

// Optimisation level.  0: Reference, 1:Optimised, 2:Vectorized
#ifndef OPTIMISATION
#if __AVX2__ || __ARM_NEON
#define OPTIMISATION 2
#else
#define OPTIMISATION 1
#endif
#endif

#endif
