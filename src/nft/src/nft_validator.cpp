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

#include "satox/nft/nft_validator.hpp"
#include <regex>
#include <chrono>
#include <algorithm>

namespace satox {
namespace nft {

NFTValidator& NFTValidator::getInstance() {
    static NFTValidator instance;
    return instance;
}

bool NFTValidator::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    // Add default validation rules
    validationRules_["required_fields"] = [this](const NFTManager::NFT& nft) {
        return validateRequiredFields(nft);
    };

    validationRules_["metadata_fields"] = [this](const NFTManager::NFT& nft) {
        return validateMetadataFields(nft.metadata);
    };

    validationRules_["contract_address"] = [this](const NFTManager::NFT& nft) {
        return validateContractAddress(nft.contractAddress);
    };

    validationRules_["token_uri"] = [this](const NFTManager::NFT& nft) {
        return validateTokenURI(nft.tokenURI);
    };

    validationRules_["timestamp"] = [this](const NFTManager::NFT& nft) {
        return validateTimestamp(nft.createdAt);
    };

    initialized_ = true;
    return true;
}

void NFTValidator::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    validationRules_.clear();
    initialized_ = false;
}

NFTValidator::ValidationResult NFTValidator::validateNFT(const NFTManager::NFT& nft) {
    ValidationResult result;
    result.isValid = true;
    result.message = "NFT validation successful";

    // Run all validation rules
    for (const auto& [ruleName, rule] : validationRules_) {
        if (!rule(nft)) {
            result.isValid = false;
            result.errors.push_back("Validation rule '" + ruleName + "' failed");
        }
    }

    if (!result.isValid) {
        result.message = "NFT validation failed";
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateNFTMetadata(const NFTManager::NFTMetadata& metadata) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Metadata validation successful";

    if (!validateMetadataFields(metadata)) {
        result.isValid = false;
        result.message = "Metadata validation failed";
        result.errors.push_back("Invalid metadata fields");
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateNFTTransfer(const std::string& nftId, const std::string& from, const std::string& to) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Transfer validation successful";

    if (!validateOwnership(nftId, from)) {
        result.isValid = false;
        result.errors.push_back("Invalid ownership");
    }

    if (!validateTransferability(nftId)) {
        result.isValid = false;
        result.errors.push_back("NFT is not transferable");
    }

    if (result.isValid) {
        result.message = "Transfer validation successful";
    } else {
        result.message = "Transfer validation failed";
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateNFTBurn(const std::string& nftId, const std::string& owner) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Burn validation successful";

    if (!validateOwnership(nftId, owner)) {
        result.isValid = false;
        result.errors.push_back("Invalid ownership");
    }

    if (!validateBurnability(nftId)) {
        result.isValid = false;
        result.errors.push_back("NFT is not burnable");
    }

    if (result.isValid) {
        result.message = "Burn validation successful";
    } else {
        result.message = "Burn validation failed";
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateNFTRoyalty(const std::string& nftId, const std::string& recipient, uint32_t basisPoints) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Royalty validation successful";

    if (!validateContractAddress(recipient)) {
        result.isValid = false;
        result.errors.push_back("Invalid royalty recipient address");
    }

    if (!validateRoyaltyBasisPoints(basisPoints)) {
        result.isValid = false;
        result.errors.push_back("Invalid royalty basis points");
    }

    if (result.isValid) {
        result.message = "Royalty validation successful";
    } else {
        result.message = "Royalty validation failed";
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateBatchNFTs(const std::vector<NFTManager::NFT>& nfts) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Batch NFT validation successful";

    for (const auto& nft : nfts) {
        auto nftResult = validateNFT(nft);
        if (!nftResult.isValid) {
            result.isValid = false;
            result.errors.insert(result.errors.end(), nftResult.errors.begin(), nftResult.errors.end());
        }
    }

    if (!result.isValid) {
        result.message = "Batch NFT validation failed";
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateBatchTransfers(const std::vector<std::string>& nftIds, const std::string& from, const std::string& to) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Batch transfer validation successful";

    for (const auto& nftId : nftIds) {
        auto transferResult = validateNFTTransfer(nftId, from, to);
        if (!transferResult.isValid) {
            result.isValid = false;
            result.errors.insert(result.errors.end(), transferResult.errors.begin(), transferResult.errors.end());
        }
    }

    if (!result.isValid) {
        result.message = "Batch transfer validation failed";
    }

    return result;
}

NFTValidator::ValidationResult NFTValidator::validateBatchBurns(const std::vector<std::string>& nftIds, const std::string& owner) {
    ValidationResult result;
    result.isValid = true;
    result.message = "Batch burn validation successful";

    for (const auto& nftId : nftIds) {
        auto burnResult = validateNFTBurn(nftId, owner);
        if (!burnResult.isValid) {
            result.isValid = false;
            result.errors.insert(result.errors.end(), burnResult.errors.begin(), burnResult.errors.end());
        }
    }

    if (!result.isValid) {
        result.message = "Batch burn validation failed";
    }

    return result;
}

bool NFTValidator::validateSchema(const nlohmann::json& schema, const nlohmann::json& data) {
    try {
        // Basic schema validation
        if (!schema.is_object() || !data.is_object()) {
            return false;
        }

        // Check required fields
        if (schema.contains("required")) {
            for (const auto& field : schema["required"]) {
                if (!data.contains(field)) {
                    return false;
                }
            }
        }

        // Check field types
        if (schema.contains("properties")) {
            for (const auto& [field, fieldSchema] : schema["properties"].items()) {
                if (data.contains(field)) {
                    if (fieldSchema.contains("type")) {
                        const std::string& type = fieldSchema["type"];
                        if (type == "string" && !data[field].is_string()) return false;
                        if (type == "number" && !data[field].is_number()) return false;
                        if (type == "boolean" && !data[field].is_boolean()) return false;
                        if (type == "array" && !data[field].is_array()) return false;
                        if (type == "object" && !data[field].is_object()) return false;
                    }
                }
            }
        }

        return true;
    } catch (const std::exception& e) {
        lastError_ = {1, std::string("Schema validation error: ") + e.what()};
        return false;
    }
}

bool NFTValidator::validateMetadataSchema(const nlohmann::json& metadata) {
    // Define the required metadata schema
    nlohmann::json schema = {
        {"type", "object"},
        {"required", {"name", "description", "image"}},
        {"properties", {
            {"name", {"type", "string"}},
            {"description", {"type", "string"}},
            {"image", {"type", "string"}},
            {"attributes", {"type", "array"}}
        }}
    };

    return validateSchema(schema, metadata);
}

void NFTValidator::addValidationRule(const std::string& ruleName, std::function<bool(const NFTManager::NFT&)> rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    validationRules_[ruleName] = rule;
}

void NFTValidator::removeValidationRule(const std::string& ruleName) {
    std::lock_guard<std::mutex> lock(mutex_);
    validationRules_.erase(ruleName);
}

bool NFTValidator::hasValidationRule(const std::string& ruleName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return validationRules_.find(ruleName) != validationRules_.end();
}

NFTValidator::Error NFTValidator::getLastError() const {
    return lastError_;
}

void NFTValidator::clearLastError() {
    lastError_ = {0, ""};
}

// Private helper methods
bool NFTValidator::validateRequiredFields(const NFTManager::NFT& nft) {
    return !nft.id.empty() &&
           !nft.contractAddress.empty() &&
           !nft.owner.empty() &&
           !nft.creator.empty() &&
           !nft.tokenURI.empty() &&
           nft.createdAt > 0;
}

bool NFTValidator::validateMetadataFields(const NFTManager::NFTMetadata& metadata) {
    return !metadata.name.empty() &&
           !metadata.description.empty() &&
           !metadata.image.empty();
}

bool NFTValidator::validateOwnership(const std::string& nftId, const std::string& owner) {
    // This would typically check against the blockchain state
    // For now, we'll just validate the address format
    return validateContractAddress(owner);
}

bool NFTValidator::validateTransferability(const std::string& nftId) {
    // This would typically check against the NFT's properties
    // For now, we'll assume all NFTs are transferable
    return true;
}

bool NFTValidator::validateBurnability(const std::string& nftId) {
    // This would typically check against the NFT's properties
    // For now, we'll assume all NFTs are burnable
    return true;
}

bool NFTValidator::validateRoyaltyBasisPoints(uint32_t basisPoints) {
    return basisPoints <= 10000; // Maximum 100%
}

bool NFTValidator::validateContractAddress(const std::string& address) {
    // Basic Ethereum address validation
    std::regex pattern("^0x[0-9a-fA-F]{40}$");
    return std::regex_match(address, pattern);
}

bool NFTValidator::validateTokenURI(const std::string& uri) {
    // Basic URI validation
    return !uri.empty() && (uri.starts_with("http://") || uri.starts_with("https://") || uri.starts_with("ipfs://"));
}

bool NFTValidator::validateTimestamp(uint64_t timestamp) {
    auto now = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return timestamp <= now_seconds;
}

} // namespace nft
} // namespace satox 