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

#include "satox/wallet/wallet_manager.hpp"
#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace satox::wallet {
namespace test {

class WalletManagerBenchmark : public benchmark::Fixture {
protected:
    void SetUp(const benchmark::State& state) override {
        NetworkConfig config;
        config.network_id = "satoxcoin";
        config.chain_id = 9007;
        config.rpc_url = "http://localhost:7777";
        config.p2p_port = 60777;
        config.timeout = 30;
        config.max_retries = 3;
        
        manager_.initialize(config);
    }

    void TearDown(const benchmark::State& state) override {
        manager_.shutdown();
    }

    WalletManager manager_;
};

// Key Generation Benchmarks
BENCHMARK_DEFINE_F(WalletManagerBenchmark, GenerateKeyPair)(benchmark::State& state) {
    for (auto _ : state) {
        KeyPair keyPair;
        manager_.generateKeyPair(keyPair);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, GenerateKeyPair);

BENCHMARK_DEFINE_F(WalletManagerBenchmark, DeriveKeyPair)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    for (auto _ : state) {
        KeyPair keyPair;
        manager_.deriveKeyPair(walletId, keyPair);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, DeriveKeyPair);

// Address Generation Benchmarks
BENCHMARK_DEFINE_F(WalletManagerBenchmark, DeriveAddress)(benchmark::State& state) {
    KeyPair keyPair;
    manager_.generateKeyPair(keyPair);
    for (auto _ : state) {
        manager_.deriveAddress(keyPair.publicKey);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, DeriveAddress);

// Transaction Benchmarks
BENCHMARK_DEFINE_F(WalletManagerBenchmark, CreateTransaction)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    KeyPair keyPair;
    manager_.deriveKeyPair(walletId, keyPair);
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    for (auto _ : state) {
        Transaction tx;
        tx.from = address;
        tx.to = address;
        tx.amount = 100000;
        tx.fee = 1000;
        manager_.createTransaction(walletId, tx);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, CreateTransaction);

BENCHMARK_DEFINE_F(WalletManagerBenchmark, SignTransaction)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    KeyPair keyPair;
    manager_.deriveKeyPair(walletId, keyPair);
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    Transaction tx;
    tx.from = address;
    tx.to = address;
    tx.amount = 100000;
    tx.fee = 1000;
    manager_.createTransaction(walletId, tx);
    
    for (auto _ : state) {
        std::vector<uint8_t> signature;
        manager_.signTransaction(walletId, tx, signature);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, SignTransaction);

// Asset Benchmarks
BENCHMARK_DEFINE_F(WalletManagerBenchmark, CreateAsset)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    
    for (auto _ : state) {
        Asset asset;
        asset.name = "BENCHMARK_ASSET";
        asset.symbol = "BENCH";
        asset.amount = 1000000;
        asset.owner = walletId;
        manager_.createAsset(walletId, asset);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, CreateAsset);

BENCHMARK_DEFINE_F(WalletManagerBenchmark, TransferAsset)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    KeyPair keyPair;
    manager_.deriveKeyPair(walletId, keyPair);
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    Asset asset;
    asset.name = "BENCHMARK_ASSET";
    asset.symbol = "BENCH";
    asset.amount = 1000000;
    asset.owner = walletId;
    manager_.createAsset(walletId, asset);
    
    for (auto _ : state) {
        manager_.transferAsset(walletId, "BENCHMARK_ASSET", address, 100000);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, TransferAsset);

// IPFS Benchmarks
BENCHMARK_DEFINE_F(WalletManagerBenchmark, AddIPFSData)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    std::vector<uint8_t> data(1024, 0); // 1KB of data
    
    for (auto _ : state) {
        std::string cid;
        manager_.addIPFSData(walletId, data, cid);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, AddIPFSData);

BENCHMARK_DEFINE_F(WalletManagerBenchmark, GetIPFSData)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    std::vector<uint8_t> data(1024, 0);
    std::string cid;
    manager_.addIPFSData(walletId, data, cid);
    
    for (auto _ : state) {
        std::vector<uint8_t> retrievedData;
        manager_.getIPFSData(walletId, cid, retrievedData);
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, GetIPFSData);

// Concurrency Benchmarks
BENCHMARK_DEFINE_F(WalletManagerBenchmark, ConcurrentTransactions)(benchmark::State& state) {
    std::string walletId = manager_.createWallet("benchmark_wallet");
    KeyPair keyPair;
    manager_.deriveKeyPair(walletId, keyPair);
    std::string address = manager_.deriveAddress(keyPair.publicKey);
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        for (int i = 0; i < state.range(0); i++) {
            threads.emplace_back([&]() {
                Transaction tx;
                tx.from = address;
                tx.to = address;
                tx.amount = 10000;
                tx.fee = 1000;
                std::vector<uint8_t> signature;
                manager_.signTransaction(walletId, tx, signature);
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
    }
}
BENCHMARK_REGISTER_F(WalletManagerBenchmark, ConcurrentTransactions)
    ->RangeMultiplier(2)
    ->Range(1, 32);

} // namespace test
} // namespace satox::wallet 