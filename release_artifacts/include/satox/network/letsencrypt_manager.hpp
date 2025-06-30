/**
 * @file letsencrypt_manager.hpp
 * @brief Satox satox-sdk - Letsencrypt Manager
 * @details This header provides Letsencrypt Manager functionality for the Satox satox-sdk.
 *
 * @defgroup network network Component
 * @ingroup satox-sdk
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 *
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 *
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

/**
 * @file letsencrypt_manager.hpp
 * @brief Satox satox-sdk - Letsencrypt Manager
 * @details This header provides Letsencrypt Manager functionality for the Satox satox-sdk.
 * 
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 * 
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 * 
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>

/** @ingroup network */
namespace satox {
/** @ingroup network */
namespace network {

/** @ingroup network */
class LetsEncryptManager {
public:
/** @ingroup network */
    struct Config {
        std::string domain;
        std::string email;
        std::string webroot_path;
        std::string cert_path;
        bool auto_renew = true;
        std::chrono::hours renewal_threshold{24 * 7}; // 7 days
    };

/** @ingroup network */
    struct Certificate {
        std::string cert_path;
        std::string key_path;
        std::string chain_path;
        std::string domain;
        std::chrono::system_clock::time_point expiry;
    };

    using CertificateUpdateCallback = std::function<void(const Certificate&)>;

    LetsEncryptManager();
    ~LetsEncryptManager();

    // Certificate management
    bool initialize(const Config& config);
    bool requestCertificate(const std::string& domain);
    bool renewCertificate(const std::string& domain = "");
    bool validateCertificate(const std::string& domain);
    std::string getCertificatePath(const std::string& domain) const;
    Certificate getCurrentCertificate() const;
    void registerUpdateCallback(CertificateUpdateCallback callback);
    bool isConfigured() const;
    std::string getLastError() const;
    void shutdown();

private:
/** @ingroup network */
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace network
} // namespace satox 
