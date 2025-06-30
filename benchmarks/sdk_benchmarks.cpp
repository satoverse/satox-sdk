#include <benchmark/benchmark.h>
#include <satox/sdk.hpp>
#include <satox/blockchain.hpp>
#include <satox/asset.hpp>
#include <satox/nft.hpp>
#include <satox/ipfs.hpp>
#include <satox/security.hpp>
#include <memory>
#include <vector>
#include <string>
#include <random>

using namespace satox;

class SDKBenchmark {
protected:
    std::shared_ptr<SDK> sdk;
    std::shared_ptr<BlockchainManager> blockchain_manager;
    std::shared_ptr<AssetManager> asset_manager;
    std::shared_ptr<NFTManager> nft_manager;
    std::shared_ptr<IPFSManager> ipfs_manager;
    std::shared_ptr<SecurityManager> security_manager;

    // Test data
    std::vector<std::string> asset_ids;
    std::vector<std::string> nft_ids;
    std::vector<std::string> ipfs_hashes;

    void SetUp() {
        // Initialize SDK with test configuration
        SDKConfig config;
        config.network = "mainnet";
        config.data_dir = "./benchmark_data";
        config.rpc_endpoint = "http://localhost:7777";
        config.rpc_username = "satox";
        config.rpc_password = "satox";
        config.enable_sync = true;
        config.sync_interval = 1000;

        sdk = std::make_shared<SDK>();
        sdk->initialize(config);

        // Get managers
        blockchain_manager = sdk->get_blockchain_manager();
        asset_manager = sdk->get_asset_manager();
        nft_manager = sdk->get_nft_manager();
        ipfs_manager = sdk->get_ipfs_manager();
        security_manager = sdk->get_security_manager();

        // Initialize security
        security_manager->initialize();
        security_manager->set_policy(SecurityPolicy::HIGH);

        // Prepare test data
        prepareTestData();
    }

    void TearDown() {
        // Cleanup
        security_manager->shutdown();
        sdk->shutdown();
    }

    void prepareTestData() {
        // Create test assets
        for (int i = 0; i < 100; ++i) {
            AssetMetadata metadata;
            metadata.name = "Benchmark Asset " + std::to_string(i);
            metadata.symbol = "BA" + std::to_string(i);
            metadata.total_supply = 1000000;
            metadata.decimals = 8;
            metadata.reissuable = true;

            auto asset_id = asset_manager->create_asset(metadata);
            if (!asset_id.empty()) {
                asset_ids.push_back(asset_id);
            }
        }

        // Create test NFTs
        for (int i = 0; i < 100; ++i) {
            // Generate random image data
            std::vector<uint8_t> image_data(1024);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            for (auto& byte : image_data) {
                byte = static_cast<uint8_t>(dis(gen));
            }

            auto ipfs_hash = ipfs_manager->add_data(image_data);
            if (!ipfs_hash.empty()) {
                ipfs_hashes.push_back(ipfs_hash);

                NFTMetadata metadata;
                metadata.name = "Benchmark NFT " + std::to_string(i);
                metadata.description = "Benchmark NFT Description " + std::to_string(i);
                metadata.image = "ipfs://" + ipfs_hash;
                metadata.attributes = {};

                auto nft_id = nft_manager->create_nft(metadata);
                if (!nft_id.empty()) {
                    nft_ids.push_back(nft_id);
                }
            }
        }
    }
};

// Blockchain benchmarks
static void BM_GetBlockchainInfo(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        auto info = benchmark.blockchain_manager->get_blockchain_info();
        benchmark::DoNotOptimize(info);
    }

    benchmark.TearDown();
}
BENCHMARK(BM_GetBlockchainInfo);

static void BM_GetBlock(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        auto block = benchmark.blockchain_manager->get_block(state.range(0));
        benchmark::DoNotOptimize(block);
    }

    benchmark.TearDown();
}
BENCHMARK(BM_GetBlock)->Range(1, 1000);

// Asset benchmarks
static void BM_CreateAsset(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        AssetMetadata metadata;
        metadata.name = "Benchmark Asset";
        metadata.symbol = "BA";
        metadata.total_supply = 1000000;
        metadata.decimals = 8;
        metadata.reissuable = true;

        auto asset_id = benchmark.asset_manager->create_asset(metadata);
        benchmark::DoNotOptimize(asset_id);
    }

    benchmark.TearDown();
}
BENCHMARK(BM_CreateAsset);

static void BM_GetAsset(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        for (const auto& asset_id : benchmark.asset_ids) {
            auto asset = benchmark.asset_manager->get_asset(asset_id);
            benchmark::DoNotOptimize(asset);
        }
    }

    benchmark.TearDown();
}
BENCHMARK(BM_GetAsset);

// NFT benchmarks
static void BM_CreateNFT(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        // Generate random image data
        std::vector<uint8_t> image_data(1024);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (auto& byte : image_data) {
            byte = static_cast<uint8_t>(dis(gen));
        }

        auto ipfs_hash = benchmark.ipfs_manager->add_data(image_data);
        if (!ipfs_hash.empty()) {
            NFTMetadata metadata;
            metadata.name = "Benchmark NFT";
            metadata.description = "Benchmark NFT Description";
            metadata.image = "ipfs://" + ipfs_hash;
            metadata.attributes = {};

            auto nft_id = benchmark.nft_manager->create_nft(metadata);
            benchmark::DoNotOptimize(nft_id);
        }
    }

    benchmark.TearDown();
}
BENCHMARK(BM_CreateNFT);

static void BM_GetNFT(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        for (const auto& nft_id : benchmark.nft_ids) {
            auto nft = benchmark.nft_manager->get_nft(nft_id);
            benchmark::DoNotOptimize(nft);
        }
    }

    benchmark.TearDown();
}
BENCHMARK(BM_GetNFT);

// IPFS benchmarks
static void BM_AddToIPFS(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        std::vector<uint8_t> data(state.range(0));
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (auto& byte : data) {
            byte = static_cast<uint8_t>(dis(gen));
        }

        auto hash = benchmark.ipfs_manager->add_data(data);
        benchmark::DoNotOptimize(hash);
    }

    benchmark.TearDown();
}
BENCHMARK(BM_AddToIPFS)->Range(1024, 1024*1024);

static void BM_GetFromIPFS(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        for (const auto& hash : benchmark.ipfs_hashes) {
            auto data = benchmark.ipfs_manager->get_data(hash);
            benchmark::DoNotOptimize(data);
        }
    }

    benchmark.TearDown();
}
BENCHMARK(BM_GetFromIPFS);

// Security benchmarks
static void BM_SecurityOperations(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        // Test encryption
        std::vector<uint8_t> data(1024);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (auto& byte : data) {
            byte = static_cast<uint8_t>(dis(gen));
        }

        auto encrypted = benchmark.security_manager->encrypt_data(data);
        benchmark::DoNotOptimize(encrypted);

        auto decrypted = benchmark.security_manager->decrypt_data(encrypted);
        benchmark::DoNotOptimize(decrypted);
    }

    benchmark.TearDown();
}
BENCHMARK(BM_SecurityOperations);

// Concurrent operation benchmarks
static void BM_ConcurrentAssetCreation(benchmark::State& state) {
    SDKBenchmark benchmark;
    benchmark.SetUp();

    for (auto _ : state) {
        std::vector<std::future<std::string>> futures;
        for (int i = 0; i < state.range(0); ++i) {
            futures.push_back(std::async(std::launch::async, [&benchmark, i]() {
                AssetMetadata metadata;
                metadata.name = "Concurrent Asset " + std::to_string(i);
                metadata.symbol = "CA" + std::to_string(i);
                metadata.total_supply = 1000000;
                metadata.decimals = 8;
                metadata.reissuable = true;
                return benchmark.asset_manager->create_asset(metadata);
            }));
        }

        for (auto& future : futures) {
            auto asset_id = future.get();
            benchmark::DoNotOptimize(asset_id);
        }
    }

    benchmark.TearDown();
}
BENCHMARK(BM_ConcurrentAssetCreation)->Range(1, 16);

BENCHMARK_MAIN(); 