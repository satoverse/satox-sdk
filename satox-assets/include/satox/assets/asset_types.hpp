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
#include <cstdint>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox {
namespace assets {

enum class AssetType {
    UNKNOWN,
    TOKEN,
    NFT
};

struct AssetMetadata {
    std::string name;
    std::string symbol;
    std::string creator;
    uint64_t totalSupply = 0;
    AssetType type = AssetType::UNKNOWN;
    std::string owner;
    nlohmann::json permissions;
};

struct Asset {
    std::string id;
    AssetMetadata metadata;
    std::string owner;
    std::string status;
    bool isLocked = false;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;
    std::vector<nlohmann::json> history;
};

enum class AssetEventType {
    CREATED,
    UPDATED,
    DELETED,
    TRANSFERRED,
    LOCKED,
    UNLOCKED,
    BURNED,
    MINTED,
    PERMISSIONS_UPDATED,
    HISTORY_UPDATED
};

class AssetEventListener {
public:
    virtual void onAssetEvent(AssetEventType type, const Asset& asset) = 0;
    virtual ~AssetEventListener() = default;
};

} // namespace assets
} // namespace satox
