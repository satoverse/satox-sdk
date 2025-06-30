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

#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <unordered_map>

namespace satox {

class RPCManager {
public:
    RPCManager();
    ~RPCManager();

    // Prevent copying
    RPCManager(const RPCManager&) = delete;
    RPCManager& operator=(const RPCManager&) = delete;

    // Allow moving
    RPCManager(RPCManager&&) = default;
    RPCManager& operator=(RPCManager&&) = default;

    // Core functionality
    bool initialize();
    void shutdown();
    bool isRunning() const;

    // RPC method handling
    nlohmann::json handleRequest(const nlohmann::json& request);
    void registerMethod(const std::string& method, 
                       std::function<nlohmann::json(const nlohmann::json&)> handler);

private:
    // Built-in RPC methods
    nlohmann::json getInfo(const nlohmann::json& params);
    nlohmann::json getBlockchainInfo(const nlohmann::json& params);
    nlohmann::json getNetworkInfo(const nlohmann::json& params);
    nlohmann::json getBlockCount(const nlohmann::json& params);
    nlohmann::json getBlockHash(const nlohmann::json& params);
    nlohmann::json getBlock(const nlohmann::json& params);
    nlohmann::json getRawTransaction(const nlohmann::json& params);
    nlohmann::json sendRawTransaction(const nlohmann::json& params);
    nlohmann::json getMempoolInfo(const nlohmann::json& params);
    nlohmann::json getMiningInfo(const nlohmann::json& params);
    nlohmann::json getDifficulty(const nlohmann::json& params);
    nlohmann::json getConnectionCount(const nlohmann::json& params);
    nlohmann::json getPeerInfo(const nlohmann::json& params);
    nlohmann::json getBalance(const nlohmann::json& params);
    nlohmann::json listUnspent(const nlohmann::json& params);
    nlohmann::json createRawTransaction(const nlohmann::json& params);
    nlohmann::json signRawTransaction(const nlohmann::json& params);
    nlohmann::json validateAddress(const nlohmann::json& params);
    nlohmann::json getNewAddress(const nlohmann::json& params);
    nlohmann::json getReceivedByAddress(const nlohmann::json& params);
    nlohmann::json getTransaction(const nlohmann::json& params);
    nlohmann::json listTransactions(const nlohmann::json& params);
    nlohmann::json backupWallet(const nlohmann::json& params);
    nlohmann::json importWallet(const nlohmann::json& params);
    nlohmann::json dumpPrivKey(const nlohmann::json& params);
    nlohmann::json importPrivKey(const nlohmann::json& params);
    nlohmann::json getWalletInfo(const nlohmann::json& params);
    nlohmann::json getAddressesByAccount(const nlohmann::json& params);
    nlohmann::json getAccount(const nlohmann::json& params);
    nlohmann::json getAccountAddress(const nlohmann::json& params);
    nlohmann::json getAddressesByLabel(const nlohmann::json& params);
    nlohmann::json getReceivedByLabel(const nlohmann::json& params);
    nlohmann::json listLabels(const nlohmann::json& params);
    nlohmann::json getAddressInfo(const nlohmann::json& params);
    nlohmann::json getBlockTemplate(const nlohmann::json& params);
    nlohmann::json submitBlock(const nlohmann::json& params);
    nlohmann::json getMempoolEntry(const nlohmann::json& params);
    nlohmann::json getTxOut(const nlohmann::json& params);
    nlohmann::json getTxOutSetInfo(const nlohmann::json& params);
    nlohmann::json verifyChain(const nlohmann::json& params);
    nlohmann::json getChainTips(const nlohmann::json& params);
    nlohmann::json getChainTxStats(const nlohmann::json& params);
    nlohmann::json getNetTotals(const nlohmann::json& params);
    nlohmann::json getNetworkHashPS(const nlohmann::json& params);
    nlohmann::json getMemoryInfo(const nlohmann::json& params);
    nlohmann::json getRPCInfo(const nlohmann::json& params);
    nlohmann::json help(const nlohmann::json& params);
    nlohmann::json stop(const nlohmann::json& params);
    nlohmann::json uptime(const nlohmann::json& params);

    // Satoxcoin-specific methods
    nlohmann::json issueAsset(const nlohmann::json& params);
    nlohmann::json reissueAsset(const nlohmann::json& params);
    nlohmann::json transferAsset(const nlohmann::json& params);
    nlohmann::json listAssets(const nlohmann::json& params);
    nlohmann::json getAssetInfo(const nlohmann::json& params);
    nlohmann::json getAssetAllocation(const nlohmann::json& params);
    nlohmann::json getAssetHistory(const nlohmann::json& params);
    nlohmann::json getAssetBalances(const nlohmann::json& params);
    nlohmann::json getAssetTransactions(const nlohmann::json& params);
    nlohmann::json getAssetAddresses(const nlohmann::json& params);
    nlohmann::json getAssetAddressBalances(const nlohmann::json& params);
    nlohmann::json getAssetAddressTransactions(const nlohmann::json& params);
    nlohmann::json getAssetAddressHistory(const nlohmann::json& params);
    nlohmann::json getAssetAddressAllocations(const nlohmann::json& params);
    nlohmann::json getAssetAddressAllocationHistory(const nlohmann::json& params);
    nlohmann::json getAssetAddressAllocationBalances(const nlohmann::json& params);
    nlohmann::json getAssetAddressAllocationTransactions(const nlohmann::json& params);
    nlohmann::json getAssetAddressAllocationHistory(const nlohmann::json& params);
    nlohmann::json getIPFSHash(const nlohmann::json& params);
    nlohmann::json getIPFSData(const nlohmann::json& params);
    nlohmann::json getIPFSHistory(const nlohmann::json& params);
    nlohmann::json getIPFSBalances(const nlohmann::json& params);
    nlohmann::json getIPFSTransactions(const nlohmann::json& params);
    nlohmann::json getIPFSAddresses(const nlohmann::json& params);
    nlohmann::json getIPFSAddressBalances(const nlohmann::json& params);
    nlohmann::json getIPFSAddressTransactions(const nlohmann::json& params);
    nlohmann::json getIPFSAddressHistory(const nlohmann::json& params);
    nlohmann::json getNFTInfo(const nlohmann::json& params);
    nlohmann::json getNFTHistory(const nlohmann::json& params);
    nlohmann::json getNFTBalances(const nlohmann::json& params);
    nlohmann::json getNFTTransactions(const nlohmann::json& params);
    nlohmann::json getNFTAddresses(const nlohmann::json& params);
    nlohmann::json getNFTAddressBalances(const nlohmann::json& params);
    nlohmann::json getNFTAddressTransactions(const nlohmann::json& params);
    nlohmann::json getNFTAddressHistory(const nlohmann::json& params);

    // Helper functions
    nlohmann::json createErrorResponse(int code, const std::string& message);
    nlohmann::json createSuccessResponse(const nlohmann::json& result);
    bool validateParams(const nlohmann::json& params, const std::vector<std::string>& required);
    bool validateAddressFormat(const std::string& address);
    bool validateAmount(double amount);

    std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> method_handlers_;
    bool running_;
};

} // namespace satox 