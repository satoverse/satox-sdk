#include <napi.h>
#include <satox/asset/asset_manager.hpp>
#include <nlohmann/json.hpp>

class AssetManagerWrapper : public Napi::ObjectWrap<AssetManagerWrapper> {
private:
    satox::asset::AssetManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "AssetManager", {
            InstanceMethod("initialize", &AssetManagerWrapper::Initialize),
            InstanceMethod("shutdown", &AssetManagerWrapper::Shutdown),
            InstanceMethod("getLastError", &AssetManagerWrapper::GetLastError),
            InstanceMethod("createAsset", &AssetManagerWrapper::CreateAsset),
            InstanceMethod("getAsset", &AssetManagerWrapper::GetAsset),
            InstanceMethod("getAllAssets", &AssetManagerWrapper::GetAllAssets),
            InstanceMethod("getAssetsByOwner", &AssetManagerWrapper::GetAssetsByOwner),
            InstanceMethod("transferAsset", &AssetManagerWrapper::TransferAsset),
            InstanceMethod("getAssetBalance", &AssetManagerWrapper::GetAssetBalance),
        });

        exports.Set("AssetManager", func);
        return exports;
    }

    AssetManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AssetManagerWrapper>(info) {
        manager_ = &satox::asset::AssetManager::getInstance();
    }

    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            bool result = manager_->initialize("mainnet");
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

    Napi::Value CreateAsset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected asset object").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            Napi::Object assetObj = info[0].As<Napi::Object>();
            
            satox::asset::Asset asset;
            asset.id = assetObj.Get("id").As<Napi::String>();
            asset.name = assetObj.Get("name").As<Napi::String>();
            asset.symbol = assetObj.Get("symbol").As<Napi::String>();
            asset.owner = assetObj.Get("owner").As<Napi::String>();
            asset.total_supply = assetObj.Get("totalSupply").As<Napi::Number>().Uint32Value();
            asset.decimals = assetObj.Get("decimals").As<Napi::Number>().Uint32Value();
            asset.status = assetObj.Get("status").As<Napi::String>();
            
            bool result = manager_->createAsset(asset);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAsset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected asset ID").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string assetId = info[0].As<Napi::String>();
            auto asset = manager_->getAsset(assetId);
            
            if (asset) {
                Napi::Object assetObj = Napi::Object::New(env);
                assetObj.Set("id", asset->id);
                assetObj.Set("name", asset->name);
                assetObj.Set("symbol", asset->symbol);
                assetObj.Set("owner", asset->owner);
                assetObj.Set("totalSupply", asset->total_supply);
                assetObj.Set("decimals", asset->decimals);
                assetObj.Set("status", asset->status);
                return assetObj;
            } else {
                return env.Null();
            }
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAllAssets(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        try {
            auto assets = manager_->getAllAssets();
            Napi::Array result = Napi::Array::New(env, assets.size());
            
            for (size_t i = 0; i < assets.size(); i++) {
                Napi::Object assetObj = Napi::Object::New(env);
                assetObj.Set("id", assets[i].id);
                assetObj.Set("name", assets[i].name);
                assetObj.Set("symbol", assets[i].symbol);
                assetObj.Set("owner", assets[i].owner);
                assetObj.Set("totalSupply", assets[i].total_supply);
                assetObj.Set("decimals", assets[i].decimals);
                assetObj.Set("status", assets[i].status);
                result.Set(i, assetObj);
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAssetsByOwner(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1) {
            Napi::TypeError::New(env, "Expected owner address").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string owner = info[0].As<Napi::String>();
            auto assets = manager_->getAssetsByOwner(owner);
            Napi::Array result = Napi::Array::New(env, assets.size());
            
            for (size_t i = 0; i < assets.size(); i++) {
                Napi::Object assetObj = Napi::Object::New(env);
                assetObj.Set("id", assets[i].id);
                assetObj.Set("name", assets[i].name);
                assetObj.Set("symbol", assets[i].symbol);
                assetObj.Set("owner", assets[i].owner);
                assetObj.Set("totalSupply", assets[i].total_supply);
                assetObj.Set("decimals", assets[i].decimals);
                assetObj.Set("status", assets[i].status);
                result.Set(i, assetObj);
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value TransferAsset(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 4) {
            Napi::TypeError::New(env, "Expected asset ID, from address, to address, and amount").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string assetId = info[0].As<Napi::String>();
            std::string from = info[1].As<Napi::String>();
            std::string to = info[2].As<Napi::String>();
            uint64_t amount = info[3].As<Napi::Number>().Uint32Value();
            
            bool result = manager_->transferAsset(assetId, from, to, amount);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetAssetBalance(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 2) {
            Napi::TypeError::New(env, "Expected asset ID and address").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        try {
            std::string assetId = info[0].As<Napi::String>();
            std::string address = info[1].As<Napi::String>();
            
            uint64_t balance = manager_->getAssetBalance(assetId, address);
            return Napi::Number::New(env, balance);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
};

Napi::Object InitAssetManager(Napi::Env env, Napi::Object exports) {
    AssetManagerWrapper::Init(env, exports);
    return exports;
} 