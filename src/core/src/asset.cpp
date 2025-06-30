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

#include "satox/core/asset.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace satox {
namespace core {

AssetManager::AssetManager() : initialized_(false) {}

AssetManager::~AssetManager() {
    if (initialized_) {
        cleanup();
    }
}

void AssetManager::initialize() {
    if (initialized_) {
        throw std::runtime_error("AssetManager already initialized");
    }
    initialized_ = true;
}

Asset AssetManager::createAsset(
    const std::string& name,
    const std::string& symbol,
    const std::string& owner,
    const std::string& totalSupply,
    const std::string& decimals) {
    
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (name.empty() || symbol.empty() || owner.empty() || totalSupply.empty()) {
        throw std::runtime_error("Name, symbol, owner, and total supply are required");
    }

    Asset asset;
    asset.name = name;
    asset.symbol = symbol;
    asset.owner = owner;
    asset.totalSupply = totalSupply;
    asset.decimals = decimals;
    asset.createdAt = std::chrono::system_clock::now();
    asset.status = "active";

    // Generate asset ID
    asset.id = generateAssetId(asset);

    // Initialize balance for owner
    balances_[asset.id][owner] = totalSupply;

    // Store asset
    assets_[asset.id] = asset;

    return asset;
}

bool AssetManager::updateAsset(const Asset& asset) {
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (!validateAsset(asset)) {
        return false;
    }

    if (assets_.find(asset.id) == assets_.end()) {
        return false;
    }

    assets_[asset.id] = asset;
    return true;
}

bool AssetManager::deleteAsset(const std::string& assetId) {
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (assetId.empty()) {
        throw std::runtime_error("Asset ID is required");
    }

    if (assets_.find(assetId) == assets_.end()) {
        return false;
    }

    assets_.erase(assetId);
    balances_.erase(assetId);
    return true;
}

Asset AssetManager::getAsset(const std::string& assetId) {
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (assetId.empty()) {
        throw std::runtime_error("Asset ID is required");
    }

    if (assets_.find(assetId) == assets_.end()) {
        throw std::runtime_error("Asset not found");
    }

    return assets_[assetId];
}

std::vector<Asset> AssetManager::getAssetsByOwner(const std::string& owner) {
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (owner.empty()) {
        throw std::runtime_error("Owner address is required");
    }

    std::vector<Asset> ownerAssets;
    for (const auto& pair : assets_) {
        if (pair.second.owner == owner) {
            ownerAssets.push_back(pair.second);
        }
    }

    return ownerAssets;
}

std::vector<Asset> AssetManager::getAllAssets() {
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    std::vector<Asset> allAssets;
    for (const auto& pair : assets_) {
        allAssets.push_back(pair.second);
    }

    return allAssets;
}

bool AssetManager::transferAsset(
    const std::string& assetId,
    const std::string& from,
    const std::string& to,
    const std::string& amount) {
    
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (assetId.empty() || from.empty() || to.empty() || amount.empty()) {
        throw std::runtime_error("Asset ID, from, to, and amount are required");
    }

    if (assets_.find(assetId) == assets_.end()) {
        return false;
    }

    auto& assetBalances = balances_[assetId];
    if (assetBalances.find(from) == assetBalances.end()) {
        return false;
    }

    // Check if sender has enough balance
    if (assetBalances[from] < amount) {
        return false;
    }

    // Update balances
    assetBalances[from] = std::to_string(std::stod(assetBalances[from]) - std::stod(amount));
    assetBalances[to] = std::to_string(std::stod(assetBalances[to]) + std::stod(amount));

    return true;
}

std::string AssetManager::getAssetBalance(const std::string& assetId, const std::string& address) {
    if (!initialized_) {
        throw std::runtime_error("AssetManager not initialized");
    }

    if (assetId.empty() || address.empty()) {
        throw std::runtime_error("Asset ID and address are required");
    }

    if (assets_.find(assetId) == assets_.end()) {
        return "0";
    }

    auto& assetBalances = balances_[assetId];
    if (assetBalances.find(address) == assetBalances.end()) {
        return "0";
    }

    return assetBalances[address];
}

void AssetManager::cleanup() {
    if (!initialized_) {
        return;
    }

    assets_.clear();
    balances_.clear();
    initialized_ = false;
}

std::string AssetManager::generateAssetId(const Asset& asset) {
    std::stringstream ss;
    ss << asset.name << asset.symbol << asset.owner 
       << asset.totalSupply << asset.createdAt.time_since_epoch().count();

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

bool AssetManager::validateAsset(const Asset& asset) {
    if (asset.id.empty() || asset.name.empty() || asset.symbol.empty() || 
        asset.owner.empty() || asset.totalSupply.empty()) {
        return false;
    }

    // Add more validation as needed
    return true;
}

} // namespace core
} // namespace satox 