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

#include <stdint.h>
#include <string.h>

#include "merkle.h"

#include "address.h"
#include "params.h"
#include "utils.h"
#include "utilsx4.h"
#include "wots.h"
#include "wotsx4.h"

/*
 * This generates a Merkle signature (WOTS signature followed by the Merkle
 * authentication path).
 */
void merkle_sign(uint8_t *sig, unsigned char *root,
                 const spx_ctx *ctx,
                 uint32_t wots_addr[8], uint32_t tree_addr[8],
                 uint32_t idx_leaf) {
    unsigned char *auth_path = sig + SPX_WOTS_BYTES;
    uint32_t tree_addrx4[4 * 8] = { 0 };
    int j;
    struct leaf_info_x4 info = { 0 };
    unsigned steps[ SPX_WOTS_LEN ];

    info.wots_sig = sig;
    chain_lengths(steps, root);
    info.wots_steps = steps;

    for (j = 0; j < 4; j++) {
        set_type(&tree_addrx4[8 * j], SPX_ADDR_TYPE_HASHTREE);
        set_type(&info.leaf_addr[8 * j], SPX_ADDR_TYPE_WOTS);
        set_type(&info.pk_addr[8 * j], SPX_ADDR_TYPE_WOTSPK);
        copy_subtree_addr(&tree_addrx4[8 * j], tree_addr);
        copy_subtree_addr(&info.leaf_addr[8 * j], wots_addr);
        copy_subtree_addr(&info.pk_addr[8 * j], wots_addr);
    }

    info.wots_sign_leaf = idx_leaf;

    treehashx4(root, auth_path, ctx,
               idx_leaf, 0,
               SPX_TREE_HEIGHT,
               wots_gen_leafx4,
               tree_addrx4, &info);
}

/* Compute root node of the top-most subtree. */
void merkle_gen_root(unsigned char *root, const spx_ctx *ctx) {
    /* We do not need the auth path in key generation, but it simplifies the
       code to have just one treehash routine that computes both root and path
       in one function. */
    unsigned char auth_path[SPX_TREE_HEIGHT * SPX_N + SPX_WOTS_BYTES];
    uint32_t top_tree_addr[8] = {0};
    uint32_t wots_addr[8] = {0};

    set_layer_addr(top_tree_addr, SPX_D - 1);
    set_layer_addr(wots_addr, SPX_D - 1);

    merkle_sign(auth_path, root, ctx,
                wots_addr, top_tree_addr,
                ~0U /* ~0 means "don't bother generating an auth path */ );
}
