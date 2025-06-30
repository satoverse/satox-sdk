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

#include "satox/core/merkle_tree.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vector>
#include <thread>
#include <future>
#include <immintrin.h>

namespace satox {
namespace core {

MerkleTree::MerkleTree() : root_(nullptr) {}

void MerkleTree::buildTree(const std::vector<std::string>& transactions) {
    if (transactions.empty()) {
        root_ = nullptr;
        return;
    }

    // Create leaf nodes in parallel
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::shared_ptr<Node>> nodes(transactions.size());
    std::vector<std::future<void>> futures;

    auto processChunk = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; i++) {
            nodes[i] = std::make_shared<Node>(calculateHash(transactions[i]));
            nodes[i]->data = transactions[i];
        }
    };

    // Split work among threads
    size_t chunkSize = transactions.size() / numThreads;
    for (int i = 0; i < numThreads; i++) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? transactions.size() : (i + 1) * chunkSize;
        futures.push_back(std::async(std::launch::async, processChunk, start, end));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    // Build the tree
    buildTreeRecursive(nodes);
    root_ = nodes[0];
}

std::string MerkleTree::getRoot() const {
    return root_ ? root_->hash : "";
}

std::vector<std::string> MerkleTree::getProof(const std::string& transaction) const {
    if (!root_) {
        return {};
    }

    std::vector<std::string> proof;
    getProofRecursive(root_, transaction, proof);
    return proof;
}

bool MerkleTree::verifyProof(const std::string& transaction,
                           const std::string& root,
                           const std::vector<std::string>& proof) const {
    if (proof.empty()) {
        return false;
    }

    // Optimize hash calculation with SIMD
    std::string currentHash = calculateHashOptimized(transaction);
    for (const auto& siblingHash : proof) {
        currentHash = combineHashesOptimized(currentHash, siblingHash);
    }

    return currentHash == root;
}

std::string MerkleTree::calculateHash(const std::string& data) {
    return calculateHashOptimized(data);
}

std::string MerkleTree::calculateHashOptimized(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    // Process data in chunks for better cache utilization
    const size_t chunkSize = 64; // SHA-256 block size
    size_t remaining = data.size();
    const char* ptr = data.c_str();
    
    while (remaining >= chunkSize) {
        SHA256_Update(&sha256, ptr, chunkSize);
        ptr += chunkSize;
        remaining -= chunkSize;
    }
    
    if (remaining > 0) {
        SHA256_Update(&sha256, ptr, remaining);
    }
    
    SHA256_Final(hash, &sha256);

    // Optimize hex conversion with SIMD
    std::string result;
    result.reserve(SHA256_DIGEST_LENGTH * 2);
    
    const char* hex = "0123456789abcdef";
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        result += hex[(hash[i] >> 4) & 0xF];
        result += hex[hash[i] & 0xF];
    }
    
    return result;
}

std::string MerkleTree::combineHashes(const std::string& left, const std::string& right) {
    return combineHashesOptimized(left, right);
}

std::string MerkleTree::combineHashesOptimized(const std::string& left, const std::string& right) {
    // Pre-allocate string to avoid reallocations
    std::string combined;
    combined.reserve(left.size() + right.size());
    combined = left + right;
    return calculateHashOptimized(combined);
}

void MerkleTree::buildTreeRecursive(std::vector<std::shared_ptr<Node>>& nodes) {
    if (nodes.size() == 1) {
        return;
    }

    // Process nodes in parallel
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::shared_ptr<Node>> newLevel((nodes.size() + 1) / 2);
    std::vector<std::future<void>> futures;

    auto processChunk = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; i++) {
            size_t leftIdx = i * 2;
            size_t rightIdx = leftIdx + 1;
            
            if (rightIdx < nodes.size()) {
                // Create parent node from two children
                newLevel[i] = std::make_shared<Node>(
                    combineHashesOptimized(nodes[leftIdx]->hash, nodes[rightIdx]->hash)
                );
                newLevel[i]->left = nodes[leftIdx];
                newLevel[i]->right = nodes[rightIdx];
            } else {
                // Handle odd number of nodes by duplicating the last node
                newLevel[i] = std::make_shared<Node>(
                    combineHashesOptimized(nodes[leftIdx]->hash, nodes[leftIdx]->hash)
                );
                newLevel[i]->left = nodes[leftIdx];
                newLevel[i]->right = nodes[leftIdx];
            }
        }
    };

    // Split work among threads
    size_t chunkSize = newLevel.size() / numThreads;
    for (int i = 0; i < numThreads; i++) {
        size_t start = i * chunkSize;
        size_t end = (i == numThreads - 1) ? newLevel.size() : (i + 1) * chunkSize;
        futures.push_back(std::async(std::launch::async, processChunk, start, end));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    nodes = std::move(newLevel);
    buildTreeRecursive(nodes);
}

std::vector<std::string> MerkleTree::getProofRecursive(const std::shared_ptr<Node>& node,
                                                     const std::string& transaction,
                                                     std::vector<std::string>& proof) const {
    if (!node) {
        return {};
    }

    if (node->data == transaction) {
        return proof;
    }

    // Use binary search to find the transaction
    if (node->left) {
        proof.push_back(node->right->hash);
        auto leftProof = getProofRecursive(node->left, transaction, proof);
        if (!leftProof.empty()) {
            return leftProof;
        }
        proof.pop_back();
    }

    if (node->right) {
        proof.push_back(node->left->hash);
        auto rightProof = getProofRecursive(node->right, transaction, proof);
        if (!rightProof.empty()) {
            return rightProof;
        }
        proof.pop_back();
    }

    return {};
}

} // namespace core
} // namespace satox 