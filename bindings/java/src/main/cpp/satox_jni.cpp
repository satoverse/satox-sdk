#include <jni.h>
#include <string>
#include <memory>
#include <stdexcept>
#include "satox-sdk/satox-core/include/blockchain_manager.hpp"
#include "satox-sdk/satox-core/include/transaction_manager.hpp"
#include "satox-sdk/satox-core/include/asset_manager.hpp"
#include "satox-sdk/satox-core/include/security_manager.hpp"
#include "satox-sdk/satox-core/include/database_manager.hpp"
#include "satox-sdk/satox-core/include/api_manager.hpp"
#include "satox-sdk/satox-core/include/ipfs_manager.hpp"
#include "satox-sdk/satox-core/include/wallet_manager.hpp"

extern "C" {

// Helper function to convert Java string to C++ string
std::string jstringToString(JNIEnv* env, jstring jstr) {
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    std::string str(chars);
    env->ReleaseStringUTFChars(jstr, chars);
    return str;
}

// Helper function to convert C++ string to Java string
jstring stringToJstring(JNIEnv* env, const std::string& str) {
    return env->NewStringUTF(str.c_str());
}

// Helper function to throw Java exception
void throwJavaException(JNIEnv* env, const char* className, const char* message) {
    jclass exClass = env->FindClass(className);
    if (exClass != nullptr) {
        env->ThrowNew(exClass, message);
    }
}

// BlockchainManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_blockchain_BlockchainManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::BlockchainManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_blockchain_BlockchainManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::BlockchainManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_blockchain_BlockchainManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::BlockchainManager*>(handle);
        // Convert Java config to C++ config
        satox::core::BlockchainManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// TransactionManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_transaction_TransactionManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::TransactionManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_transaction_TransactionManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::TransactionManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_transaction_TransactionManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::TransactionManager*>(handle);
        // Convert Java config to C++ config
        satox::core::TransactionManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// AssetManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_asset_AssetManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::AssetManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_asset_AssetManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::AssetManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_asset_AssetManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::AssetManager*>(handle);
        // Convert Java config to C++ config
        satox::core::AssetManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// SecurityManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_security_SecurityManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::SecurityManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_security_SecurityManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::SecurityManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_security_SecurityManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::SecurityManager*>(handle);
        // Convert Java config to C++ config
        satox::core::SecurityManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// DatabaseManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_database_DatabaseManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::DatabaseManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_database_DatabaseManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::DatabaseManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_database_DatabaseManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::DatabaseManager*>(handle);
        // Convert Java config to C++ config
        satox::core::DatabaseManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// APIManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_api_APIManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::APIManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_api_APIManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::APIManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_api_APIManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::APIManager*>(handle);
        // Convert Java config to C++ config
        satox::core::APIManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// IPFSManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_ipfs_IPFSManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::IPFSManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_ipfs_IPFSManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::IPFSManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_ipfs_IPFSManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::IPFSManager*>(handle);
        // Convert Java config to C++ config
        satox::core::IPFSManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

// WalletManager JNI methods
JNIEXPORT jlong JNICALL
Java_com_satox_sdk_wallet_WalletManager_createNative(JNIEnv* env, jobject thiz) {
    try {
        auto* manager = new satox::core::WalletManager();
        return reinterpret_cast<jlong>(manager);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_satox_sdk_wallet_WalletManager_destroyNative(JNIEnv* env, jobject thiz, jlong handle) {
    try {
        auto* manager = reinterpret_cast<satox::core::WalletManager*>(handle);
        delete manager;
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_satox_sdk_wallet_WalletManager_initializeNative(JNIEnv* env, jobject thiz, jlong handle, jobject config) {
    try {
        auto* manager = reinterpret_cast<satox::core::WalletManager*>(handle);
        // Convert Java config to C++ config
        satox::core::WalletManager::Config cpp_config;
        // TODO: Implement config conversion
        return manager->initialize(cpp_config);
    } catch (const std::exception& e) {
        throwJavaException(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

} // extern "C" 