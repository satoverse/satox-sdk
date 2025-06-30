#include <napi.h>
#include <satox/network/network_manager.hpp>

class NetworkManagerWrapper : public Napi::ObjectWrap<NetworkManagerWrapper> {
private:
    satox::network::NetworkManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "NetworkManager", {
            InstanceMethod("initialize", &NetworkManagerWrapper::Initialize),
            InstanceMethod("shutdown", &NetworkManagerWrapper::Shutdown),
            InstanceMethod("getLastError", &NetworkManagerWrapper::GetLastError),
            InstanceMethod("connect", &NetworkManagerWrapper::Connect),
            InstanceMethod("disconnect", &NetworkManagerWrapper::Disconnect),
            InstanceMethod("getPeerCount", &NetworkManagerWrapper::GetPeerCount),
        });

        exports.Set("NetworkManager", func);
        return exports;
    }

    NetworkManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NetworkManagerWrapper>(info) {
        manager_ = new satox::network::NetworkManager("p2p", 60777);
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

    Napi::Value Connect(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 2) {
            Napi::Error::New(env, "Expected 2 arguments (address, port)").ThrowAsJavaScriptException();
            return env.Null();
        }
        try {
            std::string address = info[0].As<Napi::String>();
            int port = info[1].As<Napi::Number>().Int32Value();
            bool result = manager_->connect(address, port);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value Disconnect(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument (address)").ThrowAsJavaScriptException();
            return env.Null();
        }
        try {
            std::string address = info[0].As<Napi::String>();
            bool result = manager_->disconnect(address);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetPeerCount(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            int count = manager_->getPeerCount();
            return Napi::Number::New(env, count);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
};

Napi::Object InitNetworkManager(Napi::Env env, Napi::Object exports) {
    NetworkManagerWrapper::Init(env, exports);
    return exports;
} 