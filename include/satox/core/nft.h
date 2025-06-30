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
#include <map>
#include <memory>
#include <chrono>

namespace satox {
namespace core {

struct NFTMetadata {
    std::string name;
    std::string description;
    std::string image;
    std::map<std::string, std::string> attributes;
    std::map<std::string, std::string> properties;
};

struct NFT {
    std::string id;
    std::string tokenId;
    std::string contractAddress;
    std::string owner;
    std::string creator;
    std::string uri;
    NFTMetadata metadata;
    std::chrono::system_clock::time_point createdAt;
    std::string status;
    std::map<std::string, std::string> additionalData;
};

class NFTManager {
public:
    NFTManager();
    ~NFTManager();

    // Prevent copying
    NFTManager(const NFTManager&) = delete;
    NFTManager& operator=(const NFTManager&) = delete;

    // Allow moving
    NFTManager(NFTManager&&) noexcept = default;
    NFTManager& operator=(NFTManager&&) noexcept = default;

    // Core functionality
    void initialize();
    NFT mintNFT(const std::string& contractAddress, const std::string& creator,
                const NFTMetadata& metadata, const std::string& uri = "");
    bool transferNFT(const std::string& nftId, const std::string& from,
                    const std::string& to);
    bool updateNFTMetadata(const std::string& nftId, const NFTMetadata& metadata);
    NFT getNFT(const std::string& nftId);
    std::vector<NFT> getNFTsByOwner(const std::string& owner);
    std::vector<NFT> getNFTsByContract(const std::string& contractAddress);
    std::vector<NFT> getAllNFTs();
    bool burnNFT(const std::string& nftId);
    std::string getNFTBalance(const std::string& contractAddress, const std::string& address);

private:
    void cleanup();
    std::string generateNFTId(const NFT& nft);
    bool validateNFT(const NFT& nft);
    std::string generateTokenId(const std::string& contractAddress);

    bool initialized_;
    std::map<std::string, NFT> nfts_;
    std::map<std::string, std::map<std::string, std::string>> balances_;
    std::map<std::string, uint64_t> tokenIdCounters_;
};

} // namespace core
} // namespace satox 