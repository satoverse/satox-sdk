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

#include <string>
#include <vector>
#include <memory>
#include <openssl/sha.h>

namespace satox {
namespace core {

class MerkleTree {
public:
    MerkleTree();
    ~MerkleTree() = default;

    // Prevent copying
    MerkleTree(const MerkleTree&) = delete;
    MerkleTree& operator=(const MerkleTree&) = delete;

    // Allow moving
    MerkleTree(MerkleTree&&) noexcept = default;
    MerkleTree& operator=(MerkleTree&&) noexcept = default;

    // Tree operations
    void buildTree(const std::vector<std::string>& transactions);
    std::string getRoot() const;
    std::vector<std::string> getProof(const std::string& transaction) const;
    bool verifyProof(const std::string& transaction,
                    const std::string& root,
                    const std::vector<std::string>& proof) const;

private:
    struct Node {
        std::string hash;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        std::string data;

        explicit Node(const std::string& h) : hash(h), left(nullptr), right(nullptr) {}
    };

    std::shared_ptr<Node> root_;

    // Optimized hash calculation methods
    static std::string calculateHash(const std::string& data);
    static std::string calculateHashOptimized(const std::string& data);
    static std::string combineHashes(const std::string& left, const std::string& right);
    static std::string combineHashesOptimized(const std::string& left, const std::string& right);

    // Tree building and traversal methods
    void buildTreeRecursive(std::vector<std::shared_ptr<Node>>& nodes);
    std::vector<std::string> getProofRecursive(const std::shared_ptr<Node>& node,
                                             const std::string& transaction,
                                             std::vector<std::string>& proof) const;
};

} // namespace core
} // namespace satox 