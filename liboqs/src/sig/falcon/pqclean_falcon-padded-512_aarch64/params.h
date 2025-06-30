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

#define FALCON_LOGN 9

#define FALCON_N (1 << FALCON_LOGN)
#define FALCON_Q 12289
#define FALCON_QINV (-12287) // pow(12289, -1, pow(2, 16)) - pow(2, 16)
#define FALCON_V 5461        // Barrett reduction
#define FALCON_MONT 4091     // pow(2, 16, 12289)
#define FALCON_MONT_BR 10908 // (4091 << 16)//q//2

#define FALCON_NINV_MONT 128    // pow(512, -1, 12289) * pow(2, 16, 12289)
#define FALCON_NINV_MONT_BR 341 // (128 << 16) //q // 2
#define FALCON_LOG2_NINV_MONT 7

#endif
