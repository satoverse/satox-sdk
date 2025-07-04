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

// SPDX-License-Identifier: Apache-2.0

int pivot_col_rounded = pivot_col/16;

int pivot_row_lower_bound = MAYO_MAX(0, pivot_col + nrows - MAX_COLS);
int pivot_row_upper_bound = MAYO_MIN(nrows - 1, pivot_col - MAX_COLS + ncols);
/* the pivot row is guaranteed to be between these lower and upper bounds if A has full rank*/

/* zero out pivot row */
for (int i = pivot_col_rounded; i < NEON_REGS_PER_ROW; i++) {
    _pivot_row[i] = vmovq_n_u8(0);
}

/* try to get a pivot row in constant time */
unsigned char pivot = 0;
uint32_t pivot_is_zero = -1;
for (int row = pivot_row_lower_bound;
        row <= MAYO_MIN(nrows - 1, pivot_row_upper_bound + 32); row++) {
    uint32_t is_pivot_row = ~ct_compare_32(row, pivot_row);
    uint32_t below_pivot_row = ct_is_greater_than(row, pivot_row);
    uint8x16_t mask = vmovq_n_u8( is_pivot_row | (below_pivot_row & pivot_is_zero) );
    for (int j = pivot_col_rounded; j < NEON_REGS_PER_ROW; j++) {
        _pivot_row[j] ^= mask & A_neon[row * NEON_REGS_PER_ROW + j];
    }
    pivot = pivot_row_bytes[pivot_col];
    pivot_is_zero = ~ct_compare_32((int) pivot, 0);
}

/* multiply pivot row by inverse of pivot */
inverse = inverse_f(pivot);
uint8x16_t inverse_multab = gf16v_get_multab(inverse);

for (int j = pivot_col_rounded; j < NEON_REGS_PER_ROW; j++) {
    _pivot_row[j] = vqtbl1q_u8(inverse_multab, _pivot_row[j]);
}

/* conditionally write pivot row to the correct row, if there is a nonzero pivot */
/* eliminate entries below pivot */
for (int row = pivot_row_lower_bound; row < nrows; row++) {
    unsigned char below_pivot =  (unsigned char) (ct_is_greater_than(row, pivot_row));
    unsigned char elt_to_elim = A_bytes[row*NEON_REGS_PER_ROW*16 + pivot_col];

    uint8x16_t multab = gf16v_get_multab(below_pivot & elt_to_elim);
    if (row <= pivot_row_upper_bound) {
        uint8x16_t mask = vmovq_n_u8(~ct_compare_32(row, pivot_row) & ~pivot_is_zero);
        for (int col = pivot_col_rounded; col < NEON_REGS_PER_ROW; col++) {
            A_neon[row*NEON_REGS_PER_ROW + col] = 	vbslq_u8(mask, _pivot_row[col], A_neon[row*NEON_REGS_PER_ROW + col]) ^
                                                    vqtbl1q_u8(multab, _pivot_row[col]);
        }
    } else {
        for (int j = pivot_col_rounded; j < NEON_REGS_PER_ROW; j++) {
            A_neon[row*NEON_REGS_PER_ROW + j] ^= vqtbl1q_u8(multab, _pivot_row[j]);
        }
    }
}

pivot_row += (-(int32_t)(~pivot_is_zero));

