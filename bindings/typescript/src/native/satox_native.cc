#include <napi.h>
#include <memory>
#include <string>
#include <vector>

// Include the complex AssetManager and other managers
#include "satox/asset/asset_manager.hpp"
#include "satox/blockchain/blockchain_manager.hpp"
#include "satox/network/network_manager.hpp"
#include "satox/security/security_manager.hpp"
#include "satox/nft/nft_manager.hpp"
#include "satox/ipfs/ipfs_manager.hpp"

// Forward declaration
namespace satox::blockchain {
    struct NetworkConfig;
}

// BlockchainManager wrapper
class BlockchainManagerWrapper : public Napi::ObjectWrap<BlockchainManagerWrapper> {
private:
    satox::blockchain::BlockchainManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "BlockchainManager", {
            InstanceMethod("initialize", &BlockchainManagerWrapper::Initialize),
            InstanceMethod("shutdown", &BlockchainManagerWrapper::Shutdown),
            InstanceMethod("getLastError", &BlockchainManagerWrapper::GetLastError),
            InstanceMethod("getCurrentHeight", &BlockchainManagerWrapper::GetCurrentHeight),
            InstanceMethod("getBalance", &BlockchainManagerWrapper::GetBalance),
            InstanceMethod("getStatistics", &BlockchainManagerWrapper::GetStatistics),
        });

        exports.Set("BlockchainManager", func);
        return exports;
    }

    BlockchainManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BlockchainManagerWrapper>(info) {
        manager_ = new satox::blockchain::BlockchainManager();
    }

    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            // For now, just return true without actual initialization
            // TODO: Implement proper NetworkConfig initialization
            return Napi::Boolean::New(env, true);
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

    Napi::Value GetCurrentHeight(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            uint64_t height = manager_->getCurrentHeight();
            return Napi::Number::New(env, static_cast<double>(height));
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetBalance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string address = info[0].As<Napi::String>();
            double balance = manager_->getBalance(address);
            return Napi::Number::New(env, balance);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetStatistics(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            // TODO: Fix getStats method call
            // auto stats = manager_->getStats();
            // std::string stats_str = stats.dump();
            Napi::Object result = Napi::Object::New(env);
            result.Set("data", "Statistics not available yet");
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    ~BlockchainManagerWrapper() {
        if (manager_) {
            delete manager_;
        }
    }
};

// AssetManager wrapper
class AssetManagerWrapper : public Napi::ObjectWrap<AssetManagerWrapper> {
private:
    satox::asset::AssetManager& manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "AssetManager", {
            InstanceMethod("createAsset", &AssetManagerWrapper::CreateAsset),
            InstanceMethod("getAssetMetadata", &AssetManagerWrapper::GetAssetMetadata),
            InstanceMethod("getAssetBalance", &AssetManagerWrapper::GetAssetBalance),
            InstanceMethod("transferAsset", &AssetManagerWrapper::TransferAsset),
            InstanceMethod("getAssetStats", &AssetManagerWrapper::GetAssetStats),
        });

        exports.Set("AssetManager", func);
        return exports;
    }

    AssetManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AssetManagerWrapper>(info), manager_(satox::asset::AssetManager::getInstance()) {
    }

    Napi::Value CreateAsset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 5) {
            Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string owner_address = info[0].As<Napi::String>();
            std::string name = info[1].As<Napi::String>();
            std::string symbol = info[2].As<Napi::String>();
            uint64_t initial_supply = info[3].As<Napi::Number>().Uint32Value();
            bool reissuable = info[4].As<Napi::Boolean>().Value();

            // Create AssetCreationRequest
            satox::asset::AssetManager::AssetCreationRequest request;
            request.owner_address = owner_address;
            request.type = satox::asset::AssetManager::AssetType::SUB_ASSET;
            request.metadata.name = name;
            request.metadata.symbol = symbol;
            request.metadata.decimals = 8;
            request.metadata.total_supply = initial_supply;
            request.metadata.reissuable = reissuable;
            request.initial_supply = initial_supply;
            request.reissuable = reissuable;

            bool result = manager_.createAsset(request);
            
            Napi::Object result_obj = Napi::Object::New(env);
            result_obj.Set("success", Napi::Boolean::New(env, result));
            if (result) {
                result_obj.Set("assetName", Napi::String::New(env, name));
            }
            
            return result_obj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, std::string("Asset creation failed: ") + e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAssetMetadata(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Asset name required").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string asset_name = info[0].As<Napi::String>();
            
            satox::asset::AssetManager::AssetMetadata metadata;
            bool success = manager_.getAssetMetadata(asset_name, metadata);
            
            Napi::Object result = Napi::Object::New(env);
            if (success) {
                result.Set("name", Napi::String::New(env, metadata.name));
                result.Set("symbol", Napi::String::New(env, metadata.symbol));
                result.Set("description", Napi::String::New(env, metadata.description));
                result.Set("ipfs_hash", Napi::String::New(env, metadata.ipfs_hash));
                result.Set("decimals", Napi::Number::New(env, metadata.decimals));
                result.Set("total_supply", Napi::Number::New(env, metadata.total_supply));
                result.Set("reissuable", Napi::Boolean::New(env, metadata.reissuable));
            } else {
                result.Set("error", Napi::String::New(env, "Asset not found"));
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, std::string("Get asset metadata failed: ") + e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAssetBalance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Address and asset name required").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string address = info[0].As<Napi::String>();
            std::string asset_name = info[1].As<Napi::String>();

            uint64_t balance = 0;
            bool success = manager_.getAssetBalance(address, asset_name, balance);
            
            Napi::Object result = Napi::Object::New(env);
            if (success) {
                result.Set("balance", Napi::Number::New(env, balance));
            } else {
                result.Set("error", Napi::String::New(env, "Failed to get balance"));
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, std::string("Get asset balance failed: ") + e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value TransferAsset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 4) {
            Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string from_address = info[0].As<Napi::String>();
            std::string to_address = info[1].As<Napi::String>();
            std::string asset_name = info[2].As<Napi::String>();
            uint64_t amount = info[3].As<Napi::Number>().Uint32Value();

            satox::asset::AssetManager::AssetTransferRequest request;
            request.from_address = from_address;
            request.to_address = to_address;
            request.asset_name = asset_name;
            request.amount = amount;
            request.memo = "";

            bool result = manager_.transferAsset(request);
            
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, std::string("Asset transfer failed: ") + e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAssetStats(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            auto stats = manager_.getAssetStats();
            
            Napi::Object result = Napi::Object::New(env);
            result.Set("total_assets", Napi::Number::New(env, stats.total_assets));
            result.Set("active_assets", Napi::Number::New(env, stats.active_assets));
            result.Set("frozen_assets", Napi::Number::New(env, stats.frozen_assets));
            result.Set("destroyed_assets", Napi::Number::New(env, stats.destroyed_assets));
            result.Set("total_transfers", Napi::Number::New(env, stats.total_transfers));
            result.Set("total_reissues", Napi::Number::New(env, stats.total_reissues));
            result.Set("total_burns", Napi::Number::New(env, stats.total_burns));
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, std::string("Get asset stats failed: ") + e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
};

// SecurityManager wrapper
class SecurityManagerWrapper : public Napi::ObjectWrap<SecurityManagerWrapper> {
private:
    satox::security::SecurityManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "SecurityManager", {
            InstanceMethod("initialize", &SecurityManagerWrapper::Initialize),
            InstanceMethod("shutdown", &SecurityManagerWrapper::Shutdown),
            InstanceMethod("validateToken", &SecurityManagerWrapper::ValidateToken),
            InstanceMethod("validateInput", &SecurityManagerWrapper::ValidateInput),
            InstanceMethod("checkPermission", &SecurityManagerWrapper::CheckPermission),
        });

        exports.Set("SecurityManager", func);
        return exports;
    }

    SecurityManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<SecurityManagerWrapper>(info) {
        manager_ = new satox::security::SecurityManager();
    }

    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            bool result = manager_->initialize();
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

    Napi::Value ValidateToken(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string token = info[0].As<Napi::String>();
            bool result = manager_->validateToken(token);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ValidateInput(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 2) {
            Napi::Error::New(env, "Expected 2 arguments").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string input = info[0].As<Napi::String>();
            std::string type = info[1].As<Napi::String>();
            bool result = manager_->validateInput(input, type);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value CheckPermission(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 3) {
            Napi::Error::New(env, "Expected 3 arguments").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string user = info[0].As<Napi::String>();
            std::string resource = info[1].As<Napi::String>();
            std::string action = info[2].As<Napi::String>();
            bool result = manager_->checkPermission(user, resource, action);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    ~SecurityManagerWrapper() {
        if (manager_) {
            delete manager_;
        }
    }
};

// NFTManager wrapper
class NFTManagerWrapper : public Napi::ObjectWrap<NFTManagerWrapper> {
private:
    satox::nft::NFTManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "NFTManager", {
            InstanceMethod("initialize", &NFTManagerWrapper::Initialize),
            InstanceMethod("shutdown", &NFTManagerWrapper::Shutdown),
            InstanceMethod("createNFT", &NFTManagerWrapper::CreateNFT),
            InstanceMethod("getNFT", &NFTManagerWrapper::GetNFT),
            InstanceMethod("getLastError", &NFTManagerWrapper::GetLastError),
        });

        exports.Set("NFTManager", func);
        return exports;
    }

    NFTManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NFTManagerWrapper>(info) {
        manager_ = &satox::nft::NFTManager::getInstance();
    }

    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            bool result = manager_->initialize();
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

    Napi::Value CreateNFT(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 3) {
            Napi::Error::New(env, "Expected 3 arguments").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string contractAddress = info[0].As<Napi::String>();
            Napi::Object metadataObj = info[1].As<Napi::Object>();
            std::string creator = info[2].As<Napi::String>();
            
            satox::nft::NFTManager::NFTMetadata metadata;
            metadata.name = metadataObj.Get("name").As<Napi::String>();
            metadata.description = metadataObj.Get("description").As<Napi::String>();
            metadata.image = metadataObj.Get("image").As<Napi::String>();
            metadata.externalUrl = metadataObj.Get("externalUrl").As<Napi::String>();
            
            std::string nftId = manager_->createNFT(contractAddress, metadata, creator);
            return Napi::String::New(env, nftId);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetNFT(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string id = info[0].As<Napi::String>();
            satox::nft::NFTManager::NFT nft = manager_->getNFT(id);
            
            Napi::Object result = Napi::Object::New(env);
            result.Set("id", nft.id);
            result.Set("contractAddress", nft.contractAddress);
            result.Set("owner", nft.owner);
            result.Set("creator", nft.creator);
            result.Set("tokenId", nft.tokenId);
            result.Set("tokenURI", nft.tokenURI);
            result.Set("isTransferable", nft.isTransferable);
            result.Set("isBurnable", nft.isBurnable);
            result.Set("royaltyRecipient", nft.royaltyRecipient);
            result.Set("royaltyBasisPoints", nft.royaltyBasisPoints);
            
            // Metadata
            Napi::Object metadata = Napi::Object::New(env);
            metadata.Set("name", nft.metadata.name);
            metadata.Set("description", nft.metadata.description);
            metadata.Set("image", nft.metadata.image);
            metadata.Set("externalUrl", nft.metadata.externalUrl);
            result.Set("metadata", metadata);
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetLastError(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            satox::nft::NFTManager::Error error = manager_->getLastError();
            Napi::Object result = Napi::Object::New(env);
            result.Set("code", error.code);
            result.Set("message", error.message);
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
};

// IPFSManager wrapper
class IPFSManagerWrapper : public Napi::ObjectWrap<IPFSManagerWrapper> {
private:
    satox::ipfs::IPFSManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "IPFSManager", {
            InstanceMethod("initialize", &IPFSManagerWrapper::Initialize),
            InstanceMethod("addFile", &IPFSManagerWrapper::AddFile),
            InstanceMethod("getFile", &IPFSManagerWrapper::GetFile),
            InstanceMethod("pinFile", &IPFSManagerWrapper::PinFile),
            InstanceMethod("unpinFile", &IPFSManagerWrapper::UnpinFile),
            InstanceMethod("getLastError", &IPFSManagerWrapper::GetLastError),
        });

        exports.Set("IPFSManager", func);
        return exports;
    }

    IPFSManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<IPFSManagerWrapper>(info) {
        manager_ = new satox::ipfs::IPFSManager();
    }

    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            satox::ipfs::IPFSManager::Config config;
            config.api_endpoint = "http://127.0.0.1:5001";
            
            bool result = manager_->initialize(config);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value AddFile(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected file path argument").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string filePath = info[0].As<Napi::String>();
            std::string hash;
            
            bool result = manager_->addFile(filePath, hash);
            
            Napi::Object result_obj = Napi::Object::New(env);
            result_obj.Set("success", Napi::Boolean::New(env, result));
            if (result) {
                result_obj.Set("hash", Napi::String::New(env, hash));
            }
            
            return result_obj;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetFile(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 2) {
            Napi::Error::New(env, "Expected hash and output path arguments").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string hash = info[0].As<Napi::String>();
            std::string outputPath = info[1].As<Napi::String>();
            
            bool result = manager_->getFile(hash, outputPath);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value PinFile(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected hash argument").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string hash = info[0].As<Napi::String>();
            bool result = manager_->pinFile(hash);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value UnpinFile(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected hash argument").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string hash = info[0].As<Napi::String>();
            bool result = manager_->unpinFile(hash);
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

    ~IPFSManagerWrapper() {
        if (manager_) {
            delete manager_;
        }
    }
};

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    BlockchainManagerWrapper::Init(env, exports);
    AssetManagerWrapper::Init(env, exports);
    SecurityManagerWrapper::Init(env, exports);
    NFTManagerWrapper::Init(env, exports);
    IPFSManagerWrapper::Init(env, exports);
    return exports;
}

NODE_API_MODULE(satox_native, Init) 