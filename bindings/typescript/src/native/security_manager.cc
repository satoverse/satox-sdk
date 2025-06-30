#include <napi.h>
#include "satox/security/security_manager.hpp"

class SecurityManagerWrapper : public Napi::ObjectWrap<SecurityManagerWrapper> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports) {
        Napi::Function func = DefineClass(env, "SecurityManager", {
            InstanceMethod("initialize", &SecurityManagerWrapper::Initialize),
            InstanceMethod("shutdown", &SecurityManagerWrapper::Shutdown),
            InstanceMethod("checkRateLimit", &SecurityManagerWrapper::CheckRateLimit),
            InstanceMethod("resetRateLimit", &SecurityManagerWrapper::ResetRateLimit),
            InstanceMethod("validateInput", &SecurityManagerWrapper::ValidateInput),
            InstanceMethod("validateJson", &SecurityManagerWrapper::ValidateJson),
            InstanceMethod("validateXml", &SecurityManagerWrapper::ValidateXml),
            InstanceMethod("checkPermission", &SecurityManagerWrapper::CheckPermission),
            InstanceMethod("validateToken", &SecurityManagerWrapper::ValidateToken),
            InstanceMethod("checkIpAddress", &SecurityManagerWrapper::CheckIpAddress),
            InstanceMethod("logSecurityEvent", &SecurityManagerWrapper::LogSecurityEvent),
            InstanceMethod("clearCache", &SecurityManagerWrapper::ClearCache),
            InstanceMethod("getCacheSize", &SecurityManagerWrapper::GetCacheSize),
            InstanceMethod("getConnectionPoolSize", &SecurityManagerWrapper::GetConnectionPoolSize),
            InstanceMethod("getBatchSize", &SecurityManagerWrapper::GetBatchSize),
            InstanceMethod("processBatch", &SecurityManagerWrapper::ProcessBatch),
            InstanceMethod("optimizeMemory", &SecurityManagerWrapper::OptimizeMemory),
            InstanceMethod("recoverFromError", &SecurityManagerWrapper::RecoverFromError),
            InstanceMethod("getRecoveryHistory", &SecurityManagerWrapper::GetRecoveryHistory),
            InstanceMethod("clearRecoveryHistory", &SecurityManagerWrapper::ClearRecoveryHistory),
            InstanceMethod("isRecoveryInProgress", &SecurityManagerWrapper::IsRecoveryInProgress),
            InstanceMethod("getLastRecoveryError", &SecurityManagerWrapper::GetLastRecoveryError)
        });

        exports.Set("SecurityManager", func);
        return exports;
    }

    SecurityManagerWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<SecurityManagerWrapper>(info) {
        manager_ = std::make_unique<satox::security::SecurityManager>();
    }

private:
    std::unique_ptr<satox::security::SecurityManager> manager_;

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

    Napi::Value CheckRateLimit(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 3) {
            Napi::Error::New(env, "Expected 3 arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string key = info[0].As<Napi::String>();
            int maxRequests = info[1].As<Napi::Number>();
            int timeWindow = info[2].As<Napi::Number>();
            
            bool result = manager_->checkRateLimit(key, maxRequests, timeWindow);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ResetRateLimit(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string key = info[0].As<Napi::String>();
            manager_->resetRateLimit(key);
            return env.Undefined();
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

    Napi::Value ValidateJson(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string json = info[0].As<Napi::String>();
            bool result = manager_->validateJson(json);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ValidateXml(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string xml = info[0].As<Napi::String>();
            bool result = manager_->validateXml(xml);
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

    Napi::Value CheckIpAddress(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string ip = info[0].As<Napi::String>();
            bool result = manager_->checkIpAddress(ip);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value LogSecurityEvent(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 2) {
            Napi::Error::New(env, "Expected 2 arguments").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string event = info[0].As<Napi::String>();
            std::string details = info[1].As<Napi::String>();
            manager_->logSecurityEvent(event, details);
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ClearCache(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            manager_->clearCache();
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetCacheSize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            size_t size = manager_->getCacheSize();
            return Napi::Number::New(env, size);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetConnectionPoolSize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            size_t size = manager_->getConnectionPoolSize();
            return Napi::Number::New(env, size);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetBatchSize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            size_t size = manager_->getBatchSize();
            return Napi::Number::New(env, size);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ProcessBatch(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            bool result = manager_->processBatch();
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value OptimizeMemory(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            manager_->optimizeMemory();
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value RecoverFromError(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() < 1) {
            Napi::Error::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
            return env.Null();
        }

        try {
            std::string operationId = info[0].As<Napi::String>();
            bool result = manager_->recoverFromError(operationId);
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetRecoveryHistory(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            auto history = manager_->getRecoveryHistory();
            Napi::Array result = Napi::Array::New(env, history.size());
            
            for (size_t i = 0; i < history.size(); i++) {
                Napi::Object record = Napi::Object::New(env);
                record.Set("type", static_cast<int>(history[i].type));
                record.Set("operationId", history[i].operation_id);
                record.Set("timestamp", history[i].timestamp.time_since_epoch().count());
                record.Set("errorMessage", history[i].error_message);
                record.Set("attemptCount", history[i].attempt_count);
                record.Set("context", history[i].context);
                result.Set(i, record);
            }
            
            return result;
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value ClearRecoveryHistory(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            manager_->clearRecoveryHistory();
            return env.Undefined();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value IsRecoveryInProgress(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            bool result = manager_->isRecoveryInProgress();
            return Napi::Boolean::New(env, result);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    Napi::Value GetLastRecoveryError(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        try {
            std::string error = manager_->getLastRecoveryError();
            return Napi::String::New(env, error);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
};

Napi::Object InitSecurityManager(Napi::Env env, Napi::Object exports) {
    SecurityManagerWrapper::Init(env, exports);
    return exports;
} 