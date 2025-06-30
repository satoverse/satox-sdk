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

#include "satox/nft/nft_manager.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace satox {
namespace nft {

class NFTValidator {
public:
    static NFTValidator& getInstance();

    // Prevent copying
    NFTValidator(const NFTValidator&) = delete;
    NFTValidator& operator=(const NFTValidator&) = delete;

    // Initialization and cleanup
    bool initialize();
    void shutdown();

    // Validation Methods
    struct ValidationResult {
        bool isValid;
        std::string message;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };

    // NFT Validation
    ValidationResult validateNFT(const NFTManager::NFT& nft);
    ValidationResult validateNFTMetadata(const NFTManager::NFTMetadata& metadata);
    ValidationResult validateNFTTransfer(const std::string& nftId, const std::string& from, const std::string& to);
    ValidationResult validateNFTBurn(const std::string& nftId, const std::string& owner);
    ValidationResult validateNFTRoyalty(const std::string& nftId, const std::string& recipient, uint32_t basisPoints);

    // Batch Validation
    ValidationResult validateBatchNFTs(const std::vector<NFTManager::NFT>& nfts);
    ValidationResult validateBatchTransfers(const std::vector<std::string>& nftIds, const std::string& from, const std::string& to);
    ValidationResult validateBatchBurns(const std::vector<std::string>& nftIds, const std::string& owner);

    // Schema Validation
    bool validateSchema(const nlohmann::json& schema, const nlohmann::json& data);
    bool validateMetadataSchema(const nlohmann::json& metadata);

    // Custom Validation Rules
    void addValidationRule(const std::string& ruleName, std::function<bool(const NFTManager::NFT&)> rule);
    void removeValidationRule(const std::string& ruleName);
    bool hasValidationRule(const std::string& ruleName) const;

    // Error Handling
    struct Error {
        int code;
        std::string message;
    };
    Error getLastError() const;
    void clearLastError();

private:
    NFTValidator() = default;
    ~NFTValidator() = default;

    // Internal helper methods
    bool validateRequiredFields(const NFTManager::NFT& nft);
    bool validateMetadataFields(const NFTManager::NFTMetadata& metadata);
    bool validateOwnership(const std::string& nftId, const std::string& owner);
    bool validateTransferability(const std::string& nftId);
    bool validateBurnability(const std::string& nftId);
    bool validateRoyaltyBasisPoints(uint32_t basisPoints);
    bool validateContractAddress(const std::string& address);
    bool validateTokenURI(const std::string& uri);
    bool validateTimestamp(uint64_t timestamp);

    // Member variables
    std::mutex mutex_;
    std::unordered_map<std::string, std::function<bool(const NFTManager::NFT&)>> validationRules_;
    Error lastError_;
    bool initialized_ = false;
};

} // namespace nft
} // namespace satox 