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

bool KeyManager::computeSHA256(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const {
    hash.resize(32);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;
    
    bool ok = EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) == 1 &&
              EVP_DigestUpdate(ctx, data.data(), data.size()) == 1;
    unsigned int len = 0;
    ok = ok && EVP_DigestFinal_ex(ctx, hash.data(), &len) == 1;
    
    EVP_MD_CTX_free(ctx);
    if (!ok || len != 32) return false;
    return true;
}

bool KeyManager::computeRIPEMD160(const std::vector<uint8_t>& data, std::vector<uint8_t>& hash) const {
    hash.resize(20);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return false;
    
    bool ok = EVP_DigestInit_ex(ctx, EVP_ripemd160(), nullptr) == 1 &&
              EVP_DigestUpdate(ctx, data.data(), data.size()) == 1;
    unsigned int len = 0;
    ok = ok && EVP_DigestFinal_ex(ctx, hash.data(), &len) == 1;
    
    EVP_MD_CTX_free(ctx);
    if (!ok || len != 20) return false;
    return true;
} 