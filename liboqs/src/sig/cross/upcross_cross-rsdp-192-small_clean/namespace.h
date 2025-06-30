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


#pragma once

#define CROSS_NAMESPACE(s) PQCLEAN_CROSSRSDP192SMALL_CLEAN_ ## s

#define crypto_sign                             CROSS_NAMESPACE(crypto_sign)
#define crypto_sign_keypair                     CROSS_NAMESPACE(crypto_sign_keypair)
#define crypto_sign_open                        CROSS_NAMESPACE(crypto_sign_open)
#define crypto_sign_signature                   CROSS_NAMESPACE(crypto_sign_signature)
#define crypto_sign_verify                      CROSS_NAMESPACE(crypto_sign_verify)

#define CROSS_keygen                            CROSS_NAMESPACE(CROSS_keygen)
#define CROSS_sign                              CROSS_NAMESPACE(CROSS_sign)
#define CROSS_verify                            CROSS_NAMESPACE(CROSS_verify)
#define expand_digest_to_fixed_weight           CROSS_NAMESPACE(expand_digest_to_fixed_weight)
#define gen_seed_tree                           CROSS_NAMESPACE(gen_seed_tree)
#define pack_fp_syn                             CROSS_NAMESPACE(pack_fp_syn)
#define pack_fp_vec                             CROSS_NAMESPACE(pack_fp_vec)
#define pack_fz_rsdp_g_vec                      CROSS_NAMESPACE(pack_fz_rsdp_g_vec)
#define pack_fz_vec                             CROSS_NAMESPACE(pack_fz_vec)
#define rebuild_leaves                          CROSS_NAMESPACE(rebuild_leaves)
#define rebuild_tree                            CROSS_NAMESPACE(rebuild_tree)
#define recompute_root                          CROSS_NAMESPACE(recompute_root)
#define seed_leaves                             CROSS_NAMESPACE(seed_leaves)
#define seed_path                               CROSS_NAMESPACE(seed_path)
#define tree_proof                              CROSS_NAMESPACE(tree_proof)
#define tree_root                               CROSS_NAMESPACE(tree_root)
#define unpack_fp_syn                           CROSS_NAMESPACE(unpack_fp_syn)
#define unpack_fp_vec                           CROSS_NAMESPACE(unpack_fp_vec)
#define unpack_fz_rsdp_g_vec                    CROSS_NAMESPACE(unpack_fz_rsdp_g_vec)
#define unpack_fz_vec                           CROSS_NAMESPACE(unpack_fz_vec)
