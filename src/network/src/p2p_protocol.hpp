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

#include <cstdint>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace satox {

// Satoxcoin network magic number
constexpr uint32_t SATOX_MAGIC = 0x53545843; // "STXC"

// Protocol version
constexpr int32_t PROTOCOL_VERSION = 70015;

// User agent string
constexpr const char* USER_AGENT = "/Satoxcoin:1.0.0/";

// P2P message types
enum class P2PMessageType : uint8_t {
    VERSION = 0,
    VERACK = 1,
    ADDR = 2,
    INV = 3,
    GETDATA = 4,
    GETBLOCKS = 5,
    GETHEADERS = 6,
    TX = 7,
    BLOCK = 8,
    HEADERS = 9,
    GETADDR = 10,
    MEMPOOL = 11,
    PING = 12,
    PONG = 13,
    REJECT = 14,
    SENDHEADERS = 15,
    FEEFILTER = 16,
    SENDCMPCT = 17,
    CMPCTBLOCK = 18,
    GETBLOCKTXN = 19,
    BLOCKTXN = 20,
    // Satoxcoin specific messages
    ASSET = 21,
    GETASSET = 22,
    ASSETALLOCATION = 23,
    GETASSETALLOCATION = 24,
    IPFS = 25,
    GETIPFS = 26,
    NFT = 27,
    GETNFT = 28
};

// P2P message header structure
struct P2PMessageHeader {
    uint32_t magic;           // Network magic number (0x52415645 for Satoxcoin)
    char command[12];         // Command name
    uint32_t length;          // Payload length
    uint32_t checksum;        // First 4 bytes of double SHA256 of payload

    P2PMessageHeader() : magic(0x52415645), length(0), checksum(0) {
        std::fill(command, command + 12, 0);
    }
};

// P2P message structure
struct P2PMessage {
    P2PMessageHeader header;
    std::vector<uint8_t> payload;

    P2PMessage() = default;
    P2PMessage(P2PMessageType type, const std::vector<uint8_t>& data);
};

// Version message structure
struct VersionMessage {
    int32_t version;
    uint64_t services;
    int64_t timestamp;
    uint64_t addr_recv_services;
    std::string addr_recv_ip;
    uint16_t addr_recv_port;
    uint64_t addr_from_services;
    std::string addr_from_ip;
    uint16_t addr_from_port;
    uint64_t nonce;
    std::string user_agent;
    int32_t start_height;
    bool relay;
    // Satoxcoin specific fields
    bool asset_support;
    bool ipfs_support;
    bool nft_support;

    VersionMessage() : version(70015), services(0), timestamp(0),
                      addr_recv_services(0), addr_recv_port(0),
                      addr_from_services(0), addr_from_port(0),
                      nonce(0), start_height(0), relay(true),
                      asset_support(false), ipfs_support(false), nft_support(false) {}
};

// Inventory vector structure
struct InvVector {
    uint32_t type;
    uint256 hash;

    InvVector() : type(0) {}
};

// Block header structure
struct BlockHeader {
    int32_t version;
    uint256 prev_block;
    uint256 merkle_root;
    uint32_t timestamp;
    uint32_t bits;
    uint32_t nonce;

    BlockHeader() : version(0), timestamp(0), bits(0), nonce(0) {}
};

// Transaction structure
struct Transaction {
    int32_t version;
    std::vector<TxIn> inputs;
    std::vector<TxOut> outputs;
    uint32_t locktime;

    Transaction() : version(1), locktime(0) {}
};

// Transaction input structure
struct TxIn {
    OutPoint prevout;
    std::vector<uint8_t> script_sig;
    uint32_t sequence;

    TxIn() : sequence(0xffffffff) {}
};

// Transaction output structure
struct TxOut {
    int64_t value;
    std::vector<uint8_t> script_pubkey;

    TxOut() : value(0) {}
};

// OutPoint structure
struct OutPoint {
    uint256 hash;
    uint32_t n;

    OutPoint() : n(0) {}
};

// Function declarations
std::vector<uint8_t> serializeMessage(const P2PMessage& message);
P2PMessage deserializeMessage(const std::vector<uint8_t>& data);
std::vector<uint8_t> serializeVersionMessage(const VersionMessage& msg);
VersionMessage deserializeVersionMessage(const std::vector<uint8_t>& data);
std::vector<uint8_t> serializeInvVector(const InvVector& inv);
InvVector deserializeInvVector(const std::vector<uint8_t>& data);
std::vector<uint8_t> serializeBlockHeader(const BlockHeader& header);
BlockHeader deserializeBlockHeader(const std::vector<uint8_t>& data);
std::vector<uint8_t> serializeTransaction(const Transaction& tx);
Transaction deserializeTransaction(const std::vector<uint8_t>& data);

// Helper functions
std::string messageTypeToString(P2PMessageType type);
P2PMessageType stringToMessageType(const std::string& str);
uint32_t calculateChecksum(const std::vector<uint8_t>& data);

} // namespace satox 