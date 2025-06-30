#include <napi.h>
#include "satox/core/blockchain_manager.hpp"
#include <nlohmann/json.hpp>

class BlockchainManagerWrapper : public Napi::ObjectWrap<BlockchainManagerWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "BlockchainManager", {
            InstanceMethod("initialize", &BlockchainManagerWrapper::Initialize),
            InstanceMethod("shutdown", &BlockchainManagerWrapper::Shutdown),
            InstanceMethod("connect", &BlockchainManagerWrapper::Connect),
            InstanceMethod("disconnect", &BlockchainManagerWrapper::Disconnect),
            InstanceMethod("isConnected", &BlockchainManagerWrapper::IsConnected),
            InstanceMethod("getState", &BlockchainManagerWrapper::GetState),
            InstanceMethod("getBlock", &BlockchainManagerWrapper::GetBlock),
            InstanceMethod("getBlockByHeight", &BlockchainManagerWrapper::GetBlockByHeight),
            InstanceMethod("getLatestBlock", &BlockchainManagerWrapper::GetLatestBlock),
            InstanceMethod("getBlocks", &BlockchainManagerWrapper::GetBlocks),
            InstanceMethod("getTransaction", &BlockchainManagerWrapper::GetTransaction),
            InstanceMethod("broadcastTransaction", &BlockchainManagerWrapper::BroadcastTransaction),
            InstanceMethod("validateTransaction", &BlockchainManagerWrapper::ValidateTransaction),
            InstanceMethod("getTransactionsByBlock", &BlockchainManagerWrapper::GetTransactionsByBlock),
            InstanceMethod("getInfo", &BlockchainManagerWrapper::GetInfo),
            InstanceMethod("getCurrentHeight", &BlockchainManagerWrapper::GetCurrentHeight),
            InstanceMethod("getBestBlockHash", &BlockchainManagerWrapper::GetBestBlockHash),
            InstanceMethod("getDifficulty", &BlockchainManagerWrapper::GetDifficulty),
            InstanceMethod("getNetworkHashRate", &BlockchainManagerWrapper::GetNetworkHashRate),
            InstanceMethod("getStats", &BlockchainManagerWrapper::GetStats),
            InstanceMethod("resetStats", &BlockchainManagerWrapper::ResetStats),
            InstanceMethod("enableStats", &BlockchainManagerWrapper::EnableStats),
            InstanceMethod("getLastError", &BlockchainManagerWrapper::GetLastError),
            InstanceMethod("clearLastError", &BlockchainManagerWrapper::ClearLastError),
        });

        exports.Set("BlockchainManager", func);
        return exports;
    }

    BlockchainManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BlockchainManagerWrapper>(info) {
        manager_ = &satox::core::BlockchainManager::getInstance();
    }

private:
    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected config object").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            Napi::Object configObj = info[0].As<Napi::Object>();
            nlohmann::json config;
            
            // Convert JavaScript object to JSON
            if (configObj.Has("host")) {
                config["host"] = configObj.Get("host").As<Napi::String>();
            }
            if (configObj.Has("port")) {
                config["port"] = configObj.Get("port").As<Napi::Number>().Int32Value();
            }
            if (configObj.Has("enableSSL")) {
                config["enableSSL"] = configObj.Get("enableSSL").As<Napi::Boolean>().Value();
            }
            
            bool result = manager_->initialize(config);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value Shutdown(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            manager_->shutdown();
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value Connect(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected node address").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string nodeAddress = info[0].As<Napi::String>();
            bool result = manager_->connect(nodeAddress);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value Disconnect(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            bool result = manager_->disconnect();
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value IsConnected(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            bool result = manager_->isConnected();
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetState(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            auto state = manager_->getState();
            return Napi::Number::New(env, static_cast<int>(state));
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetBlock(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected block hash").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string blockHash = info[0].As<Napi::String>();
            satox::core::BlockchainManager::Block block;
            
            bool result = manager_->getBlock(blockHash, block);
            if (!result) {
                return env.Null();
            }
            
            Napi::Object blockObj = Napi::Object::New(env);
            blockObj.Set("hash", block.hash);
            blockObj.Set("previousHash", block.previousHash);
            blockObj.Set("height", block.height);
            blockObj.Set("timestamp", block.timestamp);
            blockObj.Set("merkleRoot", block.merkleRoot);
            blockObj.Set("version", block.version);
            blockObj.Set("bits", block.bits);
            blockObj.Set("nonce", block.nonce);
            
            return blockObj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetBlockByHeight(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected height").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            uint64_t height = info[0].As<Napi::Number>().Uint32Value();
            satox::core::BlockchainManager::Block block;
            
            bool result = manager_->getBlockByHeight(height, block);
            if (!result) {
                return env.Null();
            }
            
            Napi::Object blockObj = Napi::Object::New(env);
            blockObj.Set("hash", block.hash);
            blockObj.Set("previousHash", block.previousHash);
            blockObj.Set("height", block.height);
            blockObj.Set("timestamp", block.timestamp);
            blockObj.Set("merkleRoot", block.merkleRoot);
            blockObj.Set("version", block.version);
            blockObj.Set("bits", block.bits);
            blockObj.Set("nonce", block.nonce);
            
            return blockObj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetLatestBlock(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            satox::core::BlockchainManager::Block block;
            
            bool result = manager_->getLatestBlock(block);
            if (!result) {
                return env.Null();
            }
            
            Napi::Object blockObj = Napi::Object::New(env);
            blockObj.Set("hash", block.hash);
            blockObj.Set("previousHash", block.previousHash);
            blockObj.Set("height", block.height);
            blockObj.Set("timestamp", block.timestamp);
            blockObj.Set("merkleRoot", block.merkleRoot);
            blockObj.Set("version", block.version);
            blockObj.Set("bits", block.bits);
            blockObj.Set("nonce", block.nonce);
            
            return blockObj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetBlocks(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Expected startHeight and endHeight").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            uint64_t startHeight = info[0].As<Napi::Number>().Uint32Value();
            uint64_t endHeight = info[1].As<Napi::Number>().Uint32Value();
            
            std::vector<satox::core::BlockchainManager::Block> blocks = manager_->getBlocks(startHeight, endHeight);
            
            Napi::Array result = Napi::Array::New(env, blocks.size());
            for (size_t i = 0; i < blocks.size(); i++) {
                Napi::Object blockObj = Napi::Object::New(env);
                blockObj.Set("hash", blocks[i].hash);
                blockObj.Set("previousHash", blocks[i].previousHash);
                blockObj.Set("height", blocks[i].height);
                blockObj.Set("timestamp", blocks[i].timestamp);
                blockObj.Set("merkleRoot", blocks[i].merkleRoot);
                blockObj.Set("version", blocks[i].version);
                blockObj.Set("bits", blocks[i].bits);
                blockObj.Set("nonce", blocks[i].nonce);
                
                result.Set(i, blockObj);
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetTransaction(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected transaction hash").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string txHash = info[0].As<Napi::String>();
            satox::core::BlockchainManager::Transaction transaction;
            
            bool result = manager_->getTransaction(txHash, transaction);
            if (!result) {
                return env.Null();
            }
            
            Napi::Object txObj = Napi::Object::New(env);
            txObj.Set("hash", transaction.hash);
            txObj.Set("version", transaction.version);
            txObj.Set("lockTime", transaction.lockTime);
            
            return txObj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value BroadcastTransaction(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected transaction object").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            Napi::Object txObj = info[0].As<Napi::Object>();
            satox::core::BlockchainManager::Transaction transaction;
            
            transaction.hash = txObj.Get("hash").As<Napi::String>();
            transaction.version = txObj.Get("version").As<Napi::Number>().Uint32Value();
            transaction.lockTime = txObj.Get("lockTime").As<Napi::Number>().Uint32Value();
            
            bool result = manager_->broadcastTransaction(transaction);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ValidateTransaction(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected transaction object").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            Napi::Object txObj = info[0].As<Napi::Object>();
            satox::core::BlockchainManager::Transaction transaction;
            
            transaction.hash = txObj.Get("hash").As<Napi::String>();
            transaction.version = txObj.Get("version").As<Napi::Number>().Uint32Value();
            transaction.lockTime = txObj.Get("lockTime").As<Napi::Number>().Uint32Value();
            
            bool result = manager_->validateTransaction(transaction);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetTransactionsByBlock(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected block hash").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string blockHash = info[0].As<Napi::String>();
            std::vector<satox::core::BlockchainManager::Transaction> transactions = manager_->getTransactionsByBlock(blockHash);
            
            Napi::Array result = Napi::Array::New(env, transactions.size());
            for (size_t i = 0; i < transactions.size(); i++) {
                Napi::Object txObj = Napi::Object::New(env);
                txObj.Set("hash", transactions[i].hash);
                txObj.Set("version", transactions[i].version);
                txObj.Set("lockTime", transactions[i].lockTime);
                
                result.Set(i, txObj);
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetInfo(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            auto info = manager_->getInfo();
            
            Napi::Object result = Napi::Object::New(env);
            result.Set("name", info.name);
            result.Set("version", info.version);
            result.Set("type", static_cast<int>(info.type));
            result.Set("currentHeight", info.currentHeight);
            result.Set("bestBlockHash", info.bestBlockHash);
            result.Set("difficulty", info.difficulty);
            result.Set("networkHashRate", info.networkHashRate);
            result.Set("connections", info.connections);
            result.Set("isInitialBlockDownload", info.isInitialBlockDownload);
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetCurrentHeight(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            uint64_t height = manager_->getCurrentHeight();
            return Napi::Number::New(env, height);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetBestBlockHash(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            std::string hash = manager_->getBestBlockHash();
            return Napi::String::New(env, hash);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetDifficulty(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            uint64_t difficulty = manager_->getDifficulty();
            return Napi::Number::New(env, difficulty);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetNetworkHashRate(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            uint64_t hashRate = manager_->getNetworkHashRate();
            return Napi::Number::New(env, hashRate);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetStats(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            auto stats = manager_->getStats();
            
            Napi::Object result = Napi::Object::New(env);
            result.Set("totalBlocks", stats.totalBlocks);
            result.Set("totalTransactions", stats.totalTransactions);
            result.Set("connectedNodes", stats.connectedNodes);
            result.Set("averageBlockTime", stats.averageBlockTime.count());
            result.Set("averageTransactionTime", stats.averageTransactionTime.count());
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ResetStats(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            manager_->resetStats();
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value EnableStats(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected enable boolean").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            bool enable = info[0].As<Napi::Boolean>().Value();
            bool result = manager_->enableStats(enable);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetLastError(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            std::string error = manager_->getLastError();
            return Napi::String::New(env, error);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ClearLastError(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            manager_->clearLastError();
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    satox::core::BlockchainManager* manager_;
};

Napi::Object InitBlockchainManager(Napi::Env env, Napi::Object exports) {
    return BlockchainManagerWrapper::Init(env, exports);
} 