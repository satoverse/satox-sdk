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

#include "nft.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace satox {
namespace core {

NFTManager::NFTManager() : initialized_(false) {}

NFTManager::~NFTManager() {
    if (initialized_) {
        cleanup();
    }
}

void NFTManager::initialize() {
    if (initialized_) {
        throw std::runtime_error("NFTManager already initialized");
    }
    initialized_ = true;
}

NFT NFTManager::mintNFT(
    const std::string& contractAddress,
    const std::string& creator,
    const NFTMetadata& metadata,
    const std::string& uri) {
    
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (contractAddress.empty() || creator.empty() || metadata.name.empty()) {
        throw std::runtime_error("Contract address, creator, and metadata name are required");
    }

    NFT nft;
    nft.contractAddress = contractAddress;
    nft.creator = creator;
    nft.owner = creator; // Initially owned by creator
    nft.metadata = metadata;
    nft.uri = uri;
    nft.createdAt = std::chrono::system_clock::now();
    nft.status = "active";

    // Generate token ID and NFT ID
    nft.tokenId = generateTokenId(contractAddress);
    nft.id = generateNFTId(nft);

    // Initialize balance for creator
    balances_[contractAddress][creator] = "1";

    // Store NFT
    nfts_[nft.id] = nft;

    return nft;
}

bool NFTManager::transferNFT(
    const std::string& nftId,
    const std::string& from,
    const std::string& to) {
    
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (nftId.empty() || from.empty() || to.empty()) {
        throw std::runtime_error("NFT ID, from, and to addresses are required");
    }

    if (nfts_.find(nftId) == nfts_.end()) {
        return false;
    }

    NFT& nft = nfts_[nftId];
    if (nft.owner != from) {
        return false;
    }

    // Update NFT ownership
    nft.owner = to;

    // Update balances
    auto& contractBalances = balances_[nft.contractAddress];
    contractBalances[from] = "0";
    contractBalances[to] = "1";

    return true;
}

bool NFTManager::updateNFTMetadata(const std::string& nftId, const NFTMetadata& metadata) {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (nftId.empty()) {
        throw std::runtime_error("NFT ID is required");
    }

    if (nfts_.find(nftId) == nfts_.end()) {
        return false;
    }

    nfts_[nftId].metadata = metadata;
    return true;
}

NFT NFTManager::getNFT(const std::string& nftId) {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (nftId.empty()) {
        throw std::runtime_error("NFT ID is required");
    }

    if (nfts_.find(nftId) == nfts_.end()) {
        throw std::runtime_error("NFT not found");
    }

    return nfts_[nftId];
}

std::vector<NFT> NFTManager::getNFTsByOwner(const std::string& owner) {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (owner.empty()) {
        throw std::runtime_error("Owner address is required");
    }

    std::vector<NFT> ownerNFTs;
    for (const auto& pair : nfts_) {
        if (pair.second.owner == owner) {
            ownerNFTs.push_back(pair.second);
        }
    }

    return ownerNFTs;
}

std::vector<NFT> NFTManager::getNFTsByContract(const std::string& contractAddress) {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (contractAddress.empty()) {
        throw std::runtime_error("Contract address is required");
    }

    std::vector<NFT> contractNFTs;
    for (const auto& pair : nfts_) {
        if (pair.second.contractAddress == contractAddress) {
            contractNFTs.push_back(pair.second);
        }
    }

    return contractNFTs;
}

std::vector<NFT> NFTManager::getAllNFTs() {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    std::vector<NFT> allNFTs;
    for (const auto& pair : nfts_) {
        allNFTs.push_back(pair.second);
    }

    return allNFTs;
}

bool NFTManager::burnNFT(const std::string& nftId) {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (nftId.empty()) {
        throw std::runtime_error("NFT ID is required");
    }

    if (nfts_.find(nftId) == nfts_.end()) {
        return false;
    }

    const NFT& nft = nfts_[nftId];
    
    // Update balances
    auto& contractBalances = balances_[nft.contractAddress];
    contractBalances[nft.owner] = "0";

    // Remove NFT
    nfts_.erase(nftId);
    return true;
}

std::string NFTManager::getNFTBalance(const std::string& contractAddress, const std::string& address) {
    if (!initialized_) {
        throw std::runtime_error("NFTManager not initialized");
    }

    if (contractAddress.empty() || address.empty()) {
        throw std::runtime_error("Contract address and address are required");
    }

    auto& contractBalances = balances_[contractAddress];
    if (contractBalances.find(address) == contractBalances.end()) {
        return "0";
    }

    return contractBalances[address];
}

void NFTManager::cleanup() {
    if (!initialized_) {
        return;
    }

    nfts_.clear();
    balances_.clear();
    tokenIdCounters_.clear();
    initialized_ = false;
}

std::string NFTManager::generateNFTId(const NFT& nft) {
    std::stringstream ss;
    ss << nft.contractAddress << nft.tokenId << nft.creator 
       << nft.createdAt.time_since_epoch().count();

    // Generate SHA-256 hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, ss.str().c_str(), ss.str().size());
    SHA256_Final(hash, &sha256);

    // Convert to hex string
    std::stringstream hexStream;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return hexStream.str();
}

bool NFTManager::validateNFT(const NFT& nft) {
    if (nft.id.empty() || nft.tokenId.empty() || nft.contractAddress.empty() || 
        nft.owner.empty() || nft.creator.empty() || nft.metadata.name.empty()) {
        return false;
    }

    // Add more validation as needed
    return true;
}

std::string NFTManager::generateTokenId(const std::string& contractAddress) {
    // Increment counter for this contract
    uint64_t& counter = tokenIdCounters_[contractAddress];
    counter++;

    // Convert to hex string
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << counter;
    return ss.str();
}

} // namespace core
} // namespace satox 