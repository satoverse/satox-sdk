#include <napi.h>
#include "satox/nft/nft_manager.hpp"

class NFTManagerWrapper : public Napi::ObjectWrap<NFTManagerWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "NFTManager", {
            InstanceMethod("initialize", &NFTManagerWrapper::Initialize),
            InstanceMethod("shutdown", &NFTManagerWrapper::Shutdown),
            InstanceMethod("getLastError", &NFTManagerWrapper::GetLastError),
            InstanceMethod("clearLastError", &NFTManagerWrapper::ClearLastError)
        });

        exports.Set("NFTManager", func);
        return exports;
    }

    NFTManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NFTManagerWrapper>(info) {
        manager_ = &satox::nft::NFTManager::getInstance();
    }

private:
    satox::nft::NFTManager* manager_;

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
};

Napi::Object InitNFTManager(Napi::Env env, Napi::Object exports) {
    NFTManagerWrapper::Init(env, exports);
    return exports;
} 