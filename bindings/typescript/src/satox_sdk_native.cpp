/**
 * @file satox_sdk_native.cpp
 * @brief Native Node.js addon for Satox SDK using N-API
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include <napi.h>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

// Include the main SDK header
#include "satox/satox.hpp"

// Include individual manager headers
#include "satox/core/core_manager.hpp"
#include "satox/database/database_manager.hpp"
#include "satox/security/security_manager.hpp"
#include "satox/wallet/wallet_manager.hpp"
#include "satox/asset/asset_manager.hpp"
#include "satox/nft/nft_manager.hpp"
#include "satox/blockchain/blockchain_manager.hpp"
#include "satox/ipfs/ipfs_manager.hpp"
#include "satox/network/network_manager.hpp"

using json = nlohmann::json;

// Helper function to convert JSON to N-API object
Napi::Object JsonToNapiObject(Napi::Env env, const json& j) {
  Napi::Object obj = Napi::Object::New(env);
  
  for (auto it = j.begin(); it != j.end(); ++it) {
    if (it.value().is_string()) {
      obj.Set(it.key(), it.value().get<std::string>());
    } else if (it.value().is_number()) {
      if (it.value().is_number_integer()) {
        obj.Set(it.key(), it.value().get<int64_t>());
      } else {
        obj.Set(it.key(), it.value().get<double>());
      }
    } else if (it.value().is_boolean()) {
      obj.Set(it.key(), it.value().get<bool>());
    } else if (it.value().is_null()) {
      obj.Set(it.key(), env.Null());
    } else if (it.value().is_array()) {
      Napi::Array arr = Napi::Array::New(env, it.value().size());
      for (size_t i = 0; i < it.value().size(); i++) {
        if (it.value()[i].is_string()) {
          arr.Set(i, it.value()[i].get<std::string>());
        } else if (it.value()[i].is_number()) {
          if (it.value()[i].is_number_integer()) {
            arr.Set(i, it.value()[i].get<int64_t>());
          } else {
            arr.Set(i, it.value()[i].get<double>());
          }
        } else if (it.value()[i].is_boolean()) {
          arr.Set(i, it.value()[i].get<bool>());
        } else if (it.value()[i].is_object()) {
          arr.Set(i, JsonToNapiObject(env, it.value()[i]));
        }
      }
      obj.Set(it.key(), arr);
    } else if (it.value().is_object()) {
      obj.Set(it.key(), JsonToNapiObject(env, it.value()));
    }
  }
  
  return obj;
}

// Helper function to convert N-API object to JSON
json NapiObjectToJson(Napi::Object obj) {
  json j = json::object();
  
  Napi::Array keys = obj.GetPropertyNames();
  for (uint32_t i = 0; i < keys.Length(); i++) {
    std::string key = keys.Get(i).As<Napi::String>();
    Napi::Value value = obj.Get(key);
    
    if (value.IsString()) {
      j[key] = value.As<Napi::String>();
    } else if (value.IsNumber()) {
      j[key] = value.As<Napi::Number>().DoubleValue();
    } else if (value.IsBoolean()) {
      j[key] = value.As<Napi::Boolean>();
    } else if (value.IsNull() || value.IsUndefined()) {
      j[key] = nullptr;
    } else if (value.IsArray()) {
      Napi::Array arr = value.As<Napi::Array>();
      json arrJson = json::array();
      for (uint32_t j = 0; j < arr.Length(); j++) {
        Napi::Value arrValue = arr.Get(j);
        if (arrValue.IsString()) {
          arrJson.push_back(arrValue.As<Napi::String>());
        } else if (arrValue.IsNumber()) {
          arrJson.push_back(arrValue.As<Napi::Number>().DoubleValue());
        } else if (arrValue.IsBoolean()) {
          arrJson.push_back(arrValue.As<Napi::Boolean>());
        } else if (arrValue.IsObject()) {
          arrJson.push_back(NapiObjectToJson(arrValue.As<Napi::Object>()));
        }
      }
      j[key] = arrJson;
    } else if (value.IsObject()) {
      j[key] = NapiObjectToJson(value.As<Napi::Object>());
    }
  }
  
  return j;
}

// SDK initialization function
Napi::Value Initialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Configuration object required");
    }
    
    if (!info[0].IsObject()) {
      throw std::runtime_error("Configuration must be an object");
    }
    
    Napi::Object configObj = info[0].As<Napi::Object>();
    json config = NapiObjectToJson(configObj);
    
    satox::SDK& sdk = satox::SDK::getInstance();
    bool success = sdk.initialize(config);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", success);
    
    if (!success) {
      result.Set("error", sdk.getLastError());
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// SDK shutdown function
Napi::Value Shutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    satox::SDK& sdk = satox::SDK::getInstance();
    sdk.shutdown();
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// Get last error function
Napi::Value GetLastError(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    satox::SDK& sdk = satox::SDK::getInstance();
    std::string error = sdk.getLastError();
    return Napi::String::New(env, error);
  } catch (const std::exception& e) {
    return Napi::String::New(env, e.what());
  }
}

// Health check function
Napi::Value HealthCheck(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    satox::SDK& sdk = satox::SDK::getInstance();
    bool healthy = sdk.is_initialized();
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", healthy);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// Wallet Manager Functions
Napi::Value WalletManagerInitialize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Configuration string required");
    }
    
    std::string configStr = info[0].As<Napi::String>();
    json config = json::parse(configStr);
    
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    bool success = manager.initialize(config);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", success);
    
    if (!success) {
      result.Set("error", manager.getLastError());
    }
    
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value WalletManagerShutdown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    manager.shutdown();
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value WalletManagerHealthCheck(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    bool healthy = manager.isInitialized();
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", healthy);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value WalletManagerCreateWallet(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Wallet name required");
    }
    
    std::string name = info[0].As<Napi::String>();
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    std::string walletId = manager.createWallet(name);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("wallet_id", walletId);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value WalletManagerGenerateAddress(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Wallet ID required");
    }
    
    std::string walletId = info[0].As<Napi::String>();
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    std::string address = manager.generateAddress(walletId);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("address", address);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value WalletManagerGetBalance(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Wallet ID required");
    }
    
    std::string walletId = info[0].As<Napi::String>();
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    double balance = manager.getBalance(walletId);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("balance", balance);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value WalletManagerSendTransaction(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 4) {
      throw std::runtime_error("from_address, to_address, amount, and fee required");
    }
    
    std::string fromAddress = info[0].As<Napi::String>();
    std::string toAddress = info[1].As<Napi::String>();
    double amount = info[2].As<Napi::Number>().DoubleValue();
    double fee = info[3].As<Napi::Number>().DoubleValue();
    
    satox::wallet::WalletManager& manager = satox::wallet::WalletManager::getInstance();
    std::string txId = manager.sendTransaction(fromAddress, toAddress, amount, fee);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("transaction_id", txId);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// Security Manager Functions
Napi::Value SecurityManagerGeneratePQCKeyPair(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Key ID required");
    }
    
    std::string keyId = info[0].As<Napi::String>();
    satox::security::SecurityManager& manager = satox::security::SecurityManager::getInstance();
    auto [publicKey, privateKey] = manager.generatePQCKeyPair(keyId);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("public_key", publicKey);
    result.Set("private_key", privateKey);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value SecurityManagerSignWithPQC(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 2) {
      throw std::runtime_error("Key ID and data required");
    }
    
    std::string keyId = info[0].As<Napi::String>();
    std::string data = info[1].As<Napi::String>();
    
    satox::security::SecurityManager& manager = satox::security::SecurityManager::getInstance();
    std::string signature = manager.signWithPQC(keyId, data);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("signature", signature);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

Napi::Value SecurityManagerVerifyWithPQC(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 3) {
      throw std::runtime_error("Key ID, data, and signature required");
    }
    
    std::string keyId = info[0].As<Napi::String>();
    std::string data = info[1].As<Napi::String>();
    std::string signature = info[2].As<Napi::String>();
    
    satox::security::SecurityManager& manager = satox::security::SecurityManager::getInstance();
    bool isValid = manager.verifyWithPQC(keyId, data, signature);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("valid", isValid);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// Asset Manager Functions
Napi::Value AssetManagerCreateAsset(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("Asset metadata required");
    }
    
    Napi::Object metadataObj = info[0].As<Napi::Object>();
    json metadata = NapiObjectToJson(metadataObj);
    
    satox::asset::AssetMetadata assetMetadata;
    if (metadata.contains("name")) assetMetadata.name = metadata["name"];
    if (metadata.contains("symbol")) assetMetadata.symbol = metadata["symbol"];
    if (metadata.contains("total_supply")) assetMetadata.totalSupply = metadata["total_supply"];
    if (metadata.contains("decimals")) assetMetadata.decimals = metadata["decimals"];
    
    satox::asset::AssetManager& manager = satox::asset::AssetManager::getInstance();
    std::string assetId = manager.createAsset(assetMetadata);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("asset_id", assetId);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// NFT Manager Functions
Napi::Value NFTManagerCreateNFT(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  
  try {
    if (info.Length() < 1) {
      throw std::runtime_error("NFT metadata required");
    }
    
    Napi::Object metadataObj = info[0].As<Napi::Object>();
    json metadata = NapiObjectToJson(metadataObj);
    
    satox::nft::NFTMetadata nftMetadata;
    if (metadata.contains("name")) nftMetadata.name = metadata["name"];
    if (metadata.contains("description")) nftMetadata.description = metadata["description"];
    if (metadata.contains("metadata")) nftMetadata.metadata = metadata["metadata"];
    
    satox::nft::NFTManager& manager = satox::nft::NFTManager::getInstance();
    std::string nftId = manager.createNFT(nftMetadata);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", true);
    result.Set("nft_id", nftId);
    return result;
  } catch (const std::exception& e) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("success", false);
    result.Set("error", e.what());
    return result;
  }
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  // SDK functions
  exports.Set("initialize", Napi::Function::New(env, Initialize));
  exports.Set("shutdown", Napi::Function::New(env, Shutdown));
  exports.Set("getLastError", Napi::Function::New(env, GetLastError));
  exports.Set("healthCheck", Napi::Function::New(env, HealthCheck));
  
  // Wallet Manager functions
  exports.Set("walletManagerInitialize", Napi::Function::New(env, WalletManagerInitialize));
  exports.Set("walletManagerShutdown", Napi::Function::New(env, WalletManagerShutdown));
  exports.Set("walletManagerHealthCheck", Napi::Function::New(env, WalletManagerHealthCheck));
  exports.Set("walletManagerCreateWallet", Napi::Function::New(env, WalletManagerCreateWallet));
  exports.Set("walletManagerGenerateAddress", Napi::Function::New(env, WalletManagerGenerateAddress));
  exports.Set("walletManagerGetBalance", Napi::Function::New(env, WalletManagerGetBalance));
  exports.Set("walletManagerSendTransaction", Napi::Function::New(env, WalletManagerSendTransaction));
  
  // Security Manager functions
  exports.Set("securityManagerGeneratePQCKeyPair", Napi::Function::New(env, SecurityManagerGeneratePQCKeyPair));
  exports.Set("securityManagerSignWithPQC", Napi::Function::New(env, SecurityManagerSignWithPQC));
  exports.Set("securityManagerVerifyWithPQC", Napi::Function::New(env, SecurityManagerVerifyWithPQC));
  
  // Asset Manager functions
  exports.Set("assetManagerCreateAsset", Napi::Function::New(env, AssetManagerCreateAsset));
  
  // NFT Manager functions
  exports.Set("nftManagerCreateNFT", Napi::Function::New(env, NFTManagerCreateNFT));
  
  return exports;
}

NODE_API_MODULE(satox_sdk_native, Init) 