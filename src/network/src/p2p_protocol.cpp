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

#include "p2p_protocol.hpp"
#include <openssl/sha.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace satox {

// Helper function to calculate double SHA256
std::vector<uint8_t> doubleSHA256(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> hash1(SHA256_DIGEST_LENGTH);
    std::vector<uint8_t> hash2(SHA256_DIGEST_LENGTH);
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash1.data(), &sha256);
    
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, hash1.data(), hash1.size());
    SHA256_Final(hash2.data(), &sha256);
    
    return hash2;
}

uint32_t calculateChecksum(const std::vector<uint8_t>& data) {
    auto hash = doubleSHA256(data);
    uint32_t checksum;
    std::memcpy(&checksum, hash.data(), sizeof(checksum));
    return checksum;
}

std::string messageTypeToString(P2PMessageType type) {
    switch (type) {
        case P2PMessageType::VERSION: return "version";
        case P2PMessageType::VERACK: return "verack";
        case P2PMessageType::INV: return "inv";
        case P2PMessageType::GETDATA: return "getdata";
        case P2PMessageType::BLOCK: return "block";
        case P2PMessageType::TX: return "tx";
        case P2PMessageType::GETBLOCKS: return "getblocks";
        case P2PMessageType::GETHEADERS: return "getheaders";
        case P2PMessageType::HEADERS: return "headers";
        case P2PMessageType::PING: return "ping";
        case P2PMessageType::PONG: return "pong";
        case P2PMessageType::REJECT: return "reject";
        case P2PMessageType::MEMPOOL: return "mempool";
        case P2PMessageType::FILTERLOAD: return "filterload";
        case P2PMessageType::FILTERADD: return "filteradd";
        case P2PMessageType::FILTERCLEAR: return "filterclear";
        case P2PMessageType::MERKLEBLOCK: return "merkleblock";
        case P2PMessageType::ALERT: return "alert";
        case P2PMessageType::SENDHEADERS: return "sendheaders";
        case P2PMessageType::FEEFILTER: return "feefilter";
        case P2PMessageType::SENDCMPCT: return "sendcmpct";
        case P2PMessageType::CMPCTBLOCK: return "cmpctblock";
        case P2PMessageType::GETBLOCKTXN: return "getblocktxn";
        case P2PMessageType::BLOCKTXN: return "blocktxn";
        case P2PMessageType::GETCFILTERS: return "getcfilters";
        case P2PMessageType::CFILTER: return "cfilter";
        case P2PMessageType::GETCFHEADERS: return "getcfheaders";
        case P2PMessageType::CFHEADERS: return "cfheaders";
        case P2PMessageType::GETCFCHECKPT: return "getcfcheckpt";
        case P2PMessageType::CFCHECKPT: return "cfcheckpt";
        case P2PMessageType::WTXIDRELAY: return "wtxidrelay";
        default: throw std::runtime_error("Unknown message type");
    }
}

P2PMessageType stringToMessageType(const std::string& str) {
    if (str == "version") return P2PMessageType::VERSION;
    if (str == "verack") return P2PMessageType::VERACK;
    if (str == "inv") return P2PMessageType::INV;
    if (str == "getdata") return P2PMessageType::GETDATA;
    if (str == "block") return P2PMessageType::BLOCK;
    if (str == "tx") return P2PMessageType::TX;
    if (str == "getblocks") return P2PMessageType::GETBLOCKS;
    if (str == "getheaders") return P2PMessageType::GETHEADERS;
    if (str == "headers") return P2PMessageType::HEADERS;
    if (str == "ping") return P2PMessageType::PING;
    if (str == "pong") return P2PMessageType::PONG;
    if (str == "reject") return P2PMessageType::REJECT;
    if (str == "mempool") return P2PMessageType::MEMPOOL;
    if (str == "filterload") return P2PMessageType::FILTERLOAD;
    if (str == "filteradd") return P2PMessageType::FILTERADD;
    if (str == "filterclear") return P2PMessageType::FILTERCLEAR;
    if (str == "merkleblock") return P2PMessageType::MERKLEBLOCK;
    if (str == "alert") return P2PMessageType::ALERT;
    if (str == "sendheaders") return P2PMessageType::SENDHEADERS;
    if (str == "feefilter") return P2PMessageType::FEEFILTER;
    if (str == "sendcmpct") return P2PMessageType::SENDCMPCT;
    if (str == "cmpctblock") return P2PMessageType::CMPCTBLOCK;
    if (str == "getblocktxn") return P2PMessageType::GETBLOCKTXN;
    if (str == "blocktxn") return P2PMessageType::BLOCKTXN;
    if (str == "getcfilters") return P2PMessageType::GETCFILTERS;
    if (str == "cfilter") return P2PMessageType::CFILTER;
    if (str == "getcfheaders") return P2PMessageType::GETCFHEADERS;
    if (str == "cfheaders") return P2PMessageType::CFHEADERS;
    if (str == "getcfcheckpt") return P2PMessageType::GETCFCHECKPT;
    if (str == "cfcheckpt") return P2PMessageType::CFCHECKPT;
    if (str == "wtxidrelay") return P2PMessageType::WTXIDRELAY;
    throw std::runtime_error("Unknown message type string");
}

P2PMessage::P2PMessage(P2PMessageType type, const std::vector<uint8_t>& data) {
    header.magic = 0x52415645; // Satoxcoin magic number
    std::string cmd = messageTypeToString(type);
    std::copy(cmd.begin(), cmd.end(), header.command);
    header.length = data.size();
    header.checksum = calculateChecksum(data);
    payload = data;
}

std::vector<uint8_t> serializeMessage(const P2PMessage& message) {
    std::vector<uint8_t> result;
    result.reserve(sizeof(P2PMessageHeader) + message.payload.size());
    
    // Serialize header
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&message.header.magic),
                 reinterpret_cast<const uint8_t*>(&message.header.magic) + sizeof(message.header.magic));
    result.insert(result.end(), message.header.command,
                 message.header.command + sizeof(message.header.command));
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&message.header.length),
                 reinterpret_cast<const uint8_t*>(&message.header.length) + sizeof(message.header.length));
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&message.header.checksum),
                 reinterpret_cast<const uint8_t*>(&message.header.checksum) + sizeof(message.header.checksum));
    
    // Add payload
    result.insert(result.end(), message.payload.begin(), message.payload.end());
    
    return result;
}

P2PMessage deserializeMessage(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(P2PMessageHeader)) {
        throw std::runtime_error("Message too short");
    }
    
    P2PMessage message;
    size_t offset = 0;
    
    // Deserialize header
    std::memcpy(&message.header.magic, &data[offset], sizeof(message.header.magic));
    offset += sizeof(message.header.magic);
    
    std::memcpy(message.header.command, &data[offset], sizeof(message.header.command));
    offset += sizeof(message.header.command);
    
    std::memcpy(&message.header.length, &data[offset], sizeof(message.header.length));
    offset += sizeof(message.header.length);
    
    std::memcpy(&message.header.checksum, &data[offset], sizeof(message.header.checksum));
    offset += sizeof(message.header.checksum);
    
    // Verify magic number
    if (message.header.magic != 0x52415645) {
        throw std::runtime_error("Invalid magic number");
    }
    
    // Get payload
    if (data.size() < offset + message.header.length) {
        throw std::runtime_error("Message payload too short");
    }
    
    message.payload.assign(data.begin() + offset, data.begin() + offset + message.header.length);
    
    // Verify checksum
    uint32_t calculated_checksum = calculateChecksum(message.payload);
    if (calculated_checksum != message.header.checksum) {
        throw std::runtime_error("Invalid checksum");
    }
    
    return message;
}

std::vector<uint8_t> serializeVersionMessage(const VersionMessage& msg) {
    std::vector<uint8_t> result;
    result.reserve(100); // Approximate size
    
    // Version
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.version),
                 reinterpret_cast<const uint8_t*>(&msg.version) + sizeof(msg.version));
    
    // Services
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.services),
                 reinterpret_cast<const uint8_t*>(&msg.services) + sizeof(msg.services));
    
    // Timestamp
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.timestamp),
                 reinterpret_cast<const uint8_t*>(&msg.timestamp) + sizeof(msg.timestamp));
    
    // Address receiving
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.addr_recv_services),
                 reinterpret_cast<const uint8_t*>(&msg.addr_recv_services) + sizeof(msg.addr_recv_services));
    // IP address (16 bytes)
    std::vector<uint8_t> ip_bytes(16, 0);
    if (!msg.addr_recv_ip.empty()) {
        // Convert IP string to bytes (simplified)
        std::string ip = msg.addr_recv_ip;
        std::replace(ip.begin(), ip.end(), '.', ' ');
        std::istringstream iss(ip);
        int a, b, c, d;
        iss >> a >> b >> c >> d;
        ip_bytes[10] = 0xff;
        ip_bytes[11] = 0xff;
        ip_bytes[12] = a;
        ip_bytes[13] = b;
        ip_bytes[14] = c;
        ip_bytes[15] = d;
    }
    result.insert(result.end(), ip_bytes.begin(), ip_bytes.end());
    
    // Port
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.addr_recv_port),
                 reinterpret_cast<const uint8_t*>(&msg.addr_recv_port) + sizeof(msg.addr_recv_port));
    
    // Address from
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.addr_from_services),
                 reinterpret_cast<const uint8_t*>(&msg.addr_from_services) + sizeof(msg.addr_from_services));
    // IP address (16 bytes)
    ip_bytes.assign(16, 0);
    if (!msg.addr_from_ip.empty()) {
        // Convert IP string to bytes (simplified)
        std::string ip = msg.addr_from_ip;
        std::replace(ip.begin(), ip.end(), '.', ' ');
        std::istringstream iss(ip);
        int a, b, c, d;
        iss >> a >> b >> c >> d;
        ip_bytes[10] = 0xff;
        ip_bytes[11] = 0xff;
        ip_bytes[12] = a;
        ip_bytes[13] = b;
        ip_bytes[14] = c;
        ip_bytes[15] = d;
    }
    result.insert(result.end(), ip_bytes.begin(), ip_bytes.end());
    
    // Port
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.addr_from_port),
                 reinterpret_cast<const uint8_t*>(&msg.addr_from_port) + sizeof(msg.addr_from_port));
    
    // Nonce
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.nonce),
                 reinterpret_cast<const uint8_t*>(&msg.nonce) + sizeof(msg.nonce));
    
    // User agent
    uint8_t user_agent_size = static_cast<uint8_t>(msg.user_agent.size());
    result.push_back(user_agent_size);
    result.insert(result.end(), msg.user_agent.begin(), msg.user_agent.end());
    
    // Start height
    result.insert(result.end(), reinterpret_cast<const uint8_t*>(&msg.start_height),
                 reinterpret_cast<const uint8_t*>(&msg.start_height) + sizeof(msg.start_height));
    
    // Relay
    result.push_back(msg.relay ? 1 : 0);
    
    return result;
}

VersionMessage deserializeVersionMessage(const std::vector<uint8_t>& data) {
    VersionMessage msg;
    size_t offset = 0;
    
    // Version
    std::memcpy(&msg.version, &data[offset], sizeof(msg.version));
    offset += sizeof(msg.version);
    
    // Services
    std::memcpy(&msg.services, &data[offset], sizeof(msg.services));
    offset += sizeof(msg.services);
    
    // Timestamp
    std::memcpy(&msg.timestamp, &data[offset], sizeof(msg.timestamp));
    offset += sizeof(msg.timestamp);
    
    // Address receiving
    std::memcpy(&msg.addr_recv_services, &data[offset], sizeof(msg.addr_recv_services));
    offset += sizeof(msg.addr_recv_services);
    
    // IP address (16 bytes)
    std::vector<uint8_t> ip_bytes(data.begin() + offset, data.begin() + offset + 16);
    offset += 16;
    if (ip_bytes[10] == 0xff && ip_bytes[11] == 0xff) {
        msg.addr_recv_ip = std::to_string(ip_bytes[12]) + "." +
                          std::to_string(ip_bytes[13]) + "." +
                          std::to_string(ip_bytes[14]) + "." +
                          std::to_string(ip_bytes[15]);
    }
    
    // Port
    std::memcpy(&msg.addr_recv_port, &data[offset], sizeof(msg.addr_recv_port));
    offset += sizeof(msg.addr_recv_port);
    
    // Address from
    std::memcpy(&msg.addr_from_services, &data[offset], sizeof(msg.addr_from_services));
    offset += sizeof(msg.addr_from_services);
    
    // IP address (16 bytes)
    ip_bytes.assign(data.begin() + offset, data.begin() + offset + 16);
    offset += 16;
    if (ip_bytes[10] == 0xff && ip_bytes[11] == 0xff) {
        msg.addr_from_ip = std::to_string(ip_bytes[12]) + "." +
                          std::to_string(ip_bytes[13]) + "." +
                          std::to_string(ip_bytes[14]) + "." +
                          std::to_string(ip_bytes[15]);
    }
    
    // Port
    std::memcpy(&msg.addr_from_port, &data[offset], sizeof(msg.addr_from_port));
    offset += sizeof(msg.addr_from_port);
    
    // Nonce
    std::memcpy(&msg.nonce, &data[offset], sizeof(msg.nonce));
    offset += sizeof(msg.nonce);
    
    // User agent
    uint8_t user_agent_size = data[offset++];
    msg.user_agent.assign(data.begin() + offset, data.begin() + offset + user_agent_size);
    offset += user_agent_size;
    
    // Start height
    std::memcpy(&msg.start_height, &data[offset], sizeof(msg.start_height));
    offset += sizeof(msg.start_height);
    
    // Relay
    msg.relay = data[offset] != 0;
    
    return msg;
}

// Additional serialization/deserialization functions for other message types
// would be implemented similarly...

} // namespace satox 