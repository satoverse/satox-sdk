#include <napi.h>
#include <satox/ipfs/ipfs_manager.hpp>

class IPFSManagerWrapper : public Napi::ObjectWrap<IPFSManagerWrapper> {
private:
    satox::ipfs::IPFSManager* manager_;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "IPFSManager", {
            InstanceMethod("initialize", &IPFSManagerWrapper::Initialize),
            InstanceMethod("shutdown", &IPFSManagerWrapper::Shutdown),
            InstanceMethod("getLastError", &IPFSManagerWrapper::GetLastError),
            InstanceMethod("addContent", &IPFSManagerWrapper::AddContent),
            InstanceMethod("getContent", &IPFSManagerWrapper::GetContent),
            InstanceMethod("removeContent", &IPFSManagerWrapper::RemoveContent),
            InstanceMethod("listContent", &IPFSManagerWrapper::ListContent),
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
            satox::ipfs::IPFSConfig config;
            config.api_endpoint = "http://127.0.0.1:5001";
            config.gateway_endpoint = "http://127.0.0.1:8080";
            config.pinning_endpoint = "http://127.0.0.1:5001";
            config.api_key = "";
            config.secret_key = "";
            config.timeout_ms = 30000;
            config.max_retries = 3;
            config.enable_pinning = true;
            config.enable_gateway = true;
            
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

    Napi::Value AddContent(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 2) {
            Napi::Error::New(env, "Expected 2 arguments (data, content_type)").ThrowAsJavaScriptException();
            return env.Null();
        }
        try {
            Napi::Buffer<uint8_t> data = info[0].As<Napi::Buffer<uint8_t>>();
            std::string content_type = info[1].As<Napi::String>();
            
            std::vector<uint8_t> content_data(data.Data(), data.Data() + data.Length());
            std::string cid = manager_->addContent(content_data, content_type);
            return Napi::String::New(env, cid);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetContent(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument (cid)").ThrowAsJavaScriptException();
            return env.Null();
        }
        try {
            std::string cid = info[0].As<Napi::String>();
            auto content = manager_->getContent(cid);
            
            if (content) {
                Napi::Object result = Napi::Object::New(env);
                result.Set("cid", content->cid);
                result.Set("contentType", content->content_type);
                result.Set("isPinned", content->is_pinned);
                
                Napi::Buffer<uint8_t> data = Napi::Buffer<uint8_t>::Copy(env, content->data.data(), content->data.size());
                result.Set("data", data);
                
                return result;
            } else {
                return env.Null();
            }
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value RemoveContent(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument (cid)").ThrowAsJavaScriptException();
            return env.Null();
        }
        try {
            std::string cid = info[0].As<Napi::String>();
            bool result = manager_->removeContent(cid);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ListContent(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            auto content_list = manager_->listContent();
            Napi::Array result = Napi::Array::New(env, content_list.size());
            
            for (size_t i = 0; i < content_list.size(); i++) {
                result.Set(i, content_list[i]);
            }
            
            return result;
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

Napi::Object InitIPFSManager(Napi::Env env, Napi::Object exports) {
    IPFSManagerWrapper::Init(env, exports);
    return exports;
} 