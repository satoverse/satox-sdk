/**
 * @file satox_sdk_python.cpp
 * @brief Python bindings for the Satox SDK using pybind11
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/cast.h>
#include <pybind11/attr.h>

#include "satox/satox.hpp"
#include <nlohmann/json.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace py = pybind11;

// Forward declarations for wrapper classes
class PyWalletManager;
class PySecurityManager;
class PyAssetManager;
class PyNFTManager;
class PyDatabaseManager;
class PyBlockchainManager;
class PyIPFSManager;
class PyNetworkManager;
class PyCoreManager;

// Custom exception classes
class SatoxError : public std::exception {
public:
    explicit SatoxError(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }
private:
    std::string message_;
};

class SatoxConfigError : public SatoxError {
public:
    explicit SatoxConfigError(const std::string& message) : SatoxError(message) {}
};

class SatoxNetworkError : public SatoxError {
public:
    explicit SatoxNetworkError(const std::string& message) : SatoxError(message) {}
};

class SatoxDatabaseError : public SatoxError {
public:
    explicit SatoxDatabaseError(const std::string& message) : SatoxError(message) {}
};

// Main SDK wrapper class
class PySDK {
private:
    satox::SDK& sdk_;

public:
    PySDK() : sdk_(satox::SDK::getInstance()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                } else if (py::isinstance<py::dict>(item.second)) {
                    json_config[key] = nlohmann::json::parse(py::cast<std::string>(py::str(item.second)));
                }
            }
            return sdk_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    void shutdown() {
        sdk_.shutdown();
    }

    std::string getVersion() const {
        return sdk_.getVersion();
    }

    bool isInitialized() const {
        return sdk_.isInitialized();
    }

    // Manager getters - return None for now since we need to implement the wrapper classes
    py::object get_wallet_manager() {
        return py::none();
    }

    py::object get_security_manager() {
        return py::none();
    }

    py::object get_asset_manager() {
        return py::none();
    }

    py::object get_nft_manager() {
        return py::none();
    }

    py::object get_database_manager() {
        return py::none();
    }

    py::object get_blockchain_manager() {
        return py::none();
    }

    py::object get_ipfs_manager() {
        return py::none();
    }

    py::object get_network_manager() {
        return py::none();
    }

    py::object get_core_manager() {
        return py::none();
    }
};

// Security Manager wrapper
class PySecurityManager {
private:
    satox::core::SecurityManager& manager_;

public:
    PySecurityManager() : manager_(satox::SDK::getInstance().getSecurityManager()) {}

    bool initialize(const py::dict& config) {
        try {
            satox::core::SecurityManager::SecurityConfig security_config;
            if (config.contains("network")) security_config.network = py::cast<std::string>(config["network"]);
            if (config.contains("key_length")) security_config.key_length = py::cast<int>(config["key_length"]);
            if (config.contains("sessionTimeout")) security_config.sessionTimeout = py::cast<int>(config["sessionTimeout"]);
            return manager_.initialize(security_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    void shutdown() {
        manager_.shutdown();
    }

    bool generate_key_pair(const std::string& key_id) {
        return manager_.generateKeyPair(key_id);
    }

    bool validate_key(const std::string& key) {
        return manager_.validateKey(key);
    }

    bool store_key(const std::string& key_id, const std::string& key) {
        return manager_.storeKey(key_id, key);
    }

    std::string get_key(const std::string& key_id) {
        return manager_.getKey(key_id);
    }

    std::string generate_key(int length) {
        return manager_.generateKey(length);
    }

    py::object sign(const std::string& key_id, const std::string& data) {
        auto result = manager_.sign(key_id, data);
        if (result.has_value()) {
            return py::str(result.value());
        }
        return py::none();
    }

    bool verify(const std::string& key_id, const std::string& data, const std::string& signature) {
        return manager_.verify(key_id, data, signature);
    }

    py::object encrypt(const std::string& key_id, const std::string& data) {
        auto result = manager_.encrypt(key_id, data);
        if (result.has_value()) {
            return py::str(result.value());
        }
        return py::none();
    }

    py::object decrypt(const std::string& key_id, const std::string& encrypted_data) {
        auto result = manager_.decrypt(key_id, encrypted_data);
        if (result.has_value()) {
            return py::str(result.value());
        }
        return py::none();
    }

    std::vector<uint8_t> encrypt_bytes(const std::vector<uint8_t>& data, const std::string& key) {
        return manager_.encrypt(data, key);
    }

    std::vector<uint8_t> decrypt_bytes(const std::vector<uint8_t>& data, const std::string& key) {
        return manager_.decrypt(data, key);
    }

    std::string encrypt_string(const std::string& data, const std::string& key) {
        return manager_.encryptString(data, key);
    }

    std::string decrypt_string(const std::string& data, const std::string& key) {
        return manager_.decryptString(data, key);
    }

    bool authenticate(const std::string& username, const std::string& password) {
        return manager_.authenticate(username, password);
    }

    bool validate_session(const std::string& session_id) {
        return manager_.validateSession(session_id);
    }

    std::string create_session(const std::string& username) {
        return manager_.createSession(username);
    }

    void invalidate_session(const std::string& session_id) {
        manager_.invalidateSession(session_id);
    }

    bool is_session_expired(const std::string& session_id) {
        return manager_.isSessionExpired(session_id);
    }

    void cleanup_expired_sessions() {
        manager_.cleanupExpiredSessions();
    }

    bool validate_input(const std::string& input) {
        return manager_.validateInput(input);
    }

    bool check_permission(const std::string& user, const std::string& resource) {
        return manager_.checkPermission(user, resource);
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void set_last_error(const std::string& error) {
        manager_.setLastError(error);
    }
};

// Database Manager wrapper
class PyDatabaseManager {
private:
    satox::core::DatabaseManager& manager_;

public:
    PyDatabaseManager() : manager_(satox::SDK::getInstance().getDatabaseManager()) {}

    bool initialize(const py::dict& config) {
        try {
            satox::core::DatabaseManager::DatabaseConfig db_config;
            if (config.contains("host")) db_config.host = py::cast<std::string>(config["host"]);
            if (config.contains("port")) db_config.port = py::cast<int>(config["port"]);
            if (config.contains("database")) db_config.database = py::cast<std::string>(config["database"]);
            if (config.contains("username")) db_config.username = py::cast<std::string>(config["username"]);
            if (config.contains("password")) db_config.password = py::cast<std::string>(config["password"]);
            if (config.contains("max_connections")) db_config.max_connections = py::cast<int>(config["max_connections"]);
            if (config.contains("connection_timeout")) db_config.connection_timeout = py::cast<int>(config["connection_timeout"]);
            if (config.contains("enable_ssl")) db_config.enable_ssl = py::cast<bool>(config["enable_ssl"]);
            if (config.contains("ssl_cert")) db_config.ssl_cert = py::cast<std::string>(config["ssl_cert"]);
            if (config.contains("ssl_key")) db_config.ssl_key = py::cast<std::string>(config["ssl_key"]);
            if (config.contains("ssl_ca")) db_config.ssl_ca = py::cast<std::string>(config["ssl_ca"]);
            return manager_.initialize(db_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    std::string connect(const std::string& db_type, const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            
            satox::core::DatabaseType type;
            if (db_type == "SQLITE") type = satox::core::DatabaseType::SQLITE;
            else if (db_type == "POSTGRESQL") type = satox::core::DatabaseType::POSTGRESQL;
            else if (db_type == "MYSQL") type = satox::core::DatabaseType::MYSQL;
            else if (db_type == "REDIS") type = satox::core::DatabaseType::REDIS;
            else if (db_type == "MONGODB") type = satox::core::DatabaseType::MONGODB;
            else if (db_type == "ROCKSDB") type = satox::core::DatabaseType::ROCKSDB;
            else if (db_type == "SUPABASE") type = satox::core::DatabaseType::SUPABASE;
            else if (db_type == "FIREBASE") type = satox::core::DatabaseType::FIREBASE;
            else if (db_type == "AWS") type = satox::core::DatabaseType::AWS;
            else if (db_type == "AZURE") type = satox::core::DatabaseType::AZURE;
            else if (db_type == "GOOGLE_CLOUD") type = satox::core::DatabaseType::GOOGLE_CLOUD;
            else throw SatoxConfigError("Unsupported database type: " + db_type);
            
            return manager_.connect(type, json_config);
        } catch (const std::exception& e) {
            throw SatoxDatabaseError(e.what());
        }
    }

    bool disconnect(const std::string& connection_id) {
        return manager_.disconnect(connection_id);
    }

    bool execute_query(const std::string& connection_id, const std::string& query) {
        nlohmann::json result;
        return manager_.executeQuery(connection_id, query, result);
    }

    bool execute_transaction(const std::string& connection_id, const std::vector<std::string>& queries) {
        nlohmann::json result;
        return manager_.executeTransaction(connection_id, queries, result);
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// Blockchain Manager wrapper
class PyBlockchainManager {
private:
    satox::core::BlockchainManager& manager_;

public:
    PyBlockchainManager() : manager_(satox::SDK::getInstance().getBlockchainManager()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    bool connect() {
        return manager_.connect();
    }

    bool disconnect() {
        return manager_.disconnect();
    }

    bool is_connected() const {
        return manager_.isConnected();
    }

    py::object send_transaction(const std::string& hex_data) {
        auto result = manager_.sendTransaction(hex_data);
        if (result.has_value()) {
            return py::cast(result.value());
        }
        return py::none();
    }

    py::object get_transaction(const std::string& txid) {
        auto result = manager_.getTransaction(txid);
        if (result.has_value()) {
            return py::cast(result.value());
        }
        return py::none();
    }

    py::object get_block(const std::string& hash) {
        auto result = manager_.getBlock(hash);
        if (result.has_value()) {
            return py::cast(result.value());
        }
        return py::none();
    }

    py::object get_block_by_height(uint64_t height) {
        auto result = manager_.getBlockByHeight(height);
        if (result.has_value()) {
            return py::cast(result.value());
        }
        return py::none();
    }

    uint64_t get_best_block_height() const {
        return manager_.getBestBlockHeight();
    }

    std::string get_best_block_hash() const {
        return manager_.getBestBlockHash();
    }

    bool validate_transaction(const std::string& hex) {
        return manager_.validateTransaction(hex);
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// IPFS Manager wrapper
class PyIPFSManager {
private:
    satox::core::IPFSManager& manager_;

public:
    PyIPFSManager() : manager_(satox::SDK::getInstance().getIPFSManager()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    bool add_content_from_file(const std::string& file_path) {
        return manager_.addContentFromFile(file_path);
    }

    bool get_content_to_file(const std::string& hash, const std::string& output_path) {
        return manager_.getContentToFile(hash, output_path);
    }

    py::object get_content_info(const std::string& hash) {
        auto info = manager_.getContentInfo(hash);
        py::dict result;
        result["hash"] = info.hash;
        result["name"] = info.name;
        result["size"] = info.size;
        result["mimeType"] = info.mimeType;
        return result;
    }

    py::object get_node_info(const std::string& address) {
        auto info = manager_.getNodeInfo(address);
        py::dict result;
        result["id"] = info.id;
        result["address"] = info.address;
        result["version"] = info.version;
        result["agent"] = info.agent;
        return result;
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// Network Manager wrapper
class PyNetworkManager {
private:
    satox::core::NetworkManager& manager_;

public:
    PyNetworkManager() : manager_(satox::SDK::getInstance().getNetworkManager()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    bool connect(const std::string& address, uint16_t port) {
        return manager_.connect(address, port);
    }

    bool disconnect(const std::string& address) {
        return manager_.disconnect(address);
    }

    bool is_connected(const std::string& address) const {
        return manager_.isConnected(address);
    }

    bool send_message(const std::string& address, const std::string& type, const py::dict& data) {
        try {
            nlohmann::json json_data;
            for (const auto& item : data) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_data[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_data[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_data[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_data[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.sendMessage(address, type, json_data);
        } catch (const std::exception& e) {
            throw SatoxNetworkError(e.what());
        }
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// Core Manager wrapper
class PyCoreManager {
private:
    satox::core::CoreManager& manager_;

public:
    PyCoreManager() : manager_(satox::SDK::getInstance().getCoreManager()) {}

    bool initialize(const py::dict& config) {
        try {
            satox::core::CoreConfig core_config;
            if (config.contains("data_dir")) core_config.data_dir = py::cast<std::string>(config["data_dir"]);
            if (config.contains("network")) core_config.network = py::cast<std::string>(config["network"]);
            if (config.contains("enable_mining")) core_config.enable_mining = py::cast<bool>(config["enable_mining"]);
            if (config.contains("enable_sync")) core_config.enable_sync = py::cast<bool>(config["enable_sync"]);
            if (config.contains("sync_interval_ms")) core_config.sync_interval_ms = py::cast<uint32_t>(config["sync_interval_ms"]);
            if (config.contains("mining_threads")) core_config.mining_threads = py::cast<uint32_t>(config["mining_threads"]);
            if (config.contains("rpc_endpoint")) core_config.rpc_endpoint = py::cast<std::string>(config["rpc_endpoint"]);
            if (config.contains("rpc_username")) core_config.rpc_username = py::cast<std::string>(config["rpc_username"]);
            if (config.contains("rpc_password")) core_config.rpc_password = py::cast<std::string>(config["rpc_password"]);
            if (config.contains("timeout_ms")) core_config.timeout_ms = py::cast<uint32_t>(config["timeout_ms"]);
            
            // Handle component-specific configurations
            if (config.contains("database")) {
                core_config.database = nlohmann::json::parse(py::cast<std::string>(py::str(config["database"])));
            }
            if (config.contains("network_config")) {
                core_config.network_config = nlohmann::json::parse(py::cast<std::string>(py::str(config["network_config"])));
            }
            if (config.contains("blockchain")) {
                core_config.blockchain = nlohmann::json::parse(py::cast<std::string>(py::str(config["blockchain"])));
            }
            if (config.contains("wallet")) {
                core_config.wallet = nlohmann::json::parse(py::cast<std::string>(py::str(config["wallet"])));
            }
            if (config.contains("asset")) {
                core_config.asset = nlohmann::json::parse(py::cast<std::string>(py::str(config["asset"])));
            }
            if (config.contains("ipfs")) {
                core_config.ipfs = nlohmann::json::parse(py::cast<std::string>(py::str(config["ipfs"])));
            }
            
            return manager_.initialize(core_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// Asset Manager wrapper
class PyAssetManager {
private:
    satox::core::AssetManager& manager_;

public:
    PyAssetManager() : manager_(satox::SDK::getInstance().getAssetManager()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// NFT Manager wrapper
class PyNFTManager {
private:
    satox::core::NFTManager& manager_;

public:
    PyNFTManager() : manager_(satox::SDK::getInstance().getNFTManager()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

// Wallet Manager wrapper
class PyWalletManager {
private:
    satox::core::WalletManager& manager_;

public:
    PyWalletManager() : manager_(satox::SDK::getInstance().getWalletManager()) {}

    bool initialize(const py::dict& config) {
        try {
            nlohmann::json json_config;
            for (const auto& item : config) {
                std::string key = py::cast<std::string>(item.first);
                if (py::isinstance<py::str>(item.second)) {
                    json_config[key] = py::cast<std::string>(item.second);
                } else if (py::isinstance<py::int_>(item.second)) {
                    json_config[key] = py::cast<int>(item.second);
                } else if (py::isinstance<py::float_>(item.second)) {
                    json_config[key] = py::cast<double>(item.second);
                } else if (py::isinstance<py::bool_>(item.second)) {
                    json_config[key] = py::cast<bool>(item.second);
                }
            }
            return manager_.initialize(json_config);
        } catch (const std::exception& e) {
            throw SatoxConfigError(e.what());
        }
    }

    std::string get_last_error() const {
        return manager_.getLastError();
    }

    void shutdown() {
        manager_.shutdown();
    }
};

PYBIND11_MODULE(_satox_sdk_python, m) {
    m.doc() = "Satox SDK Python Bindings";

    // Register custom exception classes
    py::register_exception<SatoxError>(m, "SatoxError");
    py::register_exception<SatoxConfigError>(m, "SatoxConfigError");
    py::register_exception<SatoxNetworkError>(m, "SatoxNetworkError");
    py::register_exception<SatoxDatabaseError>(m, "SatoxDatabaseError");

    // Main SDK class
    py::class_<PySDK>(m, "SDK")
        .def(py::init<>())
        .def("initialize", &PySDK::initialize)
        .def("shutdown", &PySDK::shutdown)
        .def("get_version", &PySDK::getVersion)
        .def("is_initialized", &PySDK::isInitialized)
        .def("get_wallet_manager", &PySDK::get_wallet_manager)
        .def("get_security_manager", &PySDK::get_security_manager)
        .def("get_asset_manager", &PySDK::get_asset_manager)
        .def("get_nft_manager", &PySDK::get_nft_manager)
        .def("get_database_manager", &PySDK::get_database_manager)
        .def("get_blockchain_manager", &PySDK::get_blockchain_manager)
        .def("get_ipfs_manager", &PySDK::get_ipfs_manager)
        .def("get_network_manager", &PySDK::get_network_manager)
        .def("get_core_manager", &PySDK::get_core_manager);

    // Security Manager class
    py::class_<PySecurityManager>(m, "SecurityManager")
        .def(py::init<>())
        .def("initialize", &PySecurityManager::initialize)
        .def("shutdown", &PySecurityManager::shutdown)
        .def("generate_key_pair", &PySecurityManager::generate_key_pair)
        .def("validate_key", &PySecurityManager::validate_key)
        .def("store_key", &PySecurityManager::store_key)
        .def("get_key", &PySecurityManager::get_key)
        .def("generate_key", &PySecurityManager::generate_key)
        .def("sign", &PySecurityManager::sign)
        .def("verify", &PySecurityManager::verify)
        .def("encrypt", &PySecurityManager::encrypt)
        .def("decrypt", &PySecurityManager::decrypt)
        .def("encrypt_bytes", &PySecurityManager::encrypt_bytes)
        .def("decrypt_bytes", &PySecurityManager::decrypt_bytes)
        .def("encrypt_string", &PySecurityManager::encrypt_string)
        .def("decrypt_string", &PySecurityManager::decrypt_string)
        .def("authenticate", &PySecurityManager::authenticate)
        .def("validate_session", &PySecurityManager::validate_session)
        .def("create_session", &PySecurityManager::create_session)
        .def("invalidate_session", &PySecurityManager::invalidate_session)
        .def("is_session_expired", &PySecurityManager::is_session_expired)
        .def("cleanup_expired_sessions", &PySecurityManager::cleanup_expired_sessions)
        .def("validate_input", &PySecurityManager::validate_input)
        .def("check_permission", &PySecurityManager::check_permission)
        .def("get_last_error", &PySecurityManager::get_last_error)
        .def("set_last_error", &PySecurityManager::set_last_error);

    // Database Manager class
    py::class_<PyDatabaseManager>(m, "DatabaseManager")
        .def(py::init<>())
        .def("initialize", &PyDatabaseManager::initialize)
        .def("connect", &PyDatabaseManager::connect)
        .def("disconnect", &PyDatabaseManager::disconnect)
        .def("execute_query", &PyDatabaseManager::execute_query)
        .def("execute_transaction", &PyDatabaseManager::execute_transaction)
        .def("get_last_error", &PyDatabaseManager::get_last_error)
        .def("shutdown", &PyDatabaseManager::shutdown);

    // Blockchain Manager class
    py::class_<PyBlockchainManager>(m, "BlockchainManager")
        .def(py::init<>())
        .def("initialize", &PyBlockchainManager::initialize)
        .def("connect", &PyBlockchainManager::connect)
        .def("disconnect", &PyBlockchainManager::disconnect)
        .def("is_connected", &PyBlockchainManager::is_connected)
        .def("send_transaction", &PyBlockchainManager::send_transaction)
        .def("get_transaction", &PyBlockchainManager::get_transaction)
        .def("get_block", &PyBlockchainManager::get_block)
        .def("get_block_by_height", &PyBlockchainManager::get_block_by_height)
        .def("get_best_block_height", &PyBlockchainManager::get_best_block_height)
        .def("get_best_block_hash", &PyBlockchainManager::get_best_block_hash)
        .def("validate_transaction", &PyBlockchainManager::validate_transaction)
        .def("get_last_error", &PyBlockchainManager::get_last_error)
        .def("shutdown", &PyBlockchainManager::shutdown);

    // IPFS Manager class
    py::class_<PyIPFSManager>(m, "IPFSManager")
        .def(py::init<>())
        .def("initialize", &PyIPFSManager::initialize)
        .def("add_content_from_file", &PyIPFSManager::add_content_from_file)
        .def("get_content_to_file", &PyIPFSManager::get_content_to_file)
        .def("get_content_info", &PyIPFSManager::get_content_info)
        .def("get_node_info", &PyIPFSManager::get_node_info)
        .def("get_last_error", &PyIPFSManager::get_last_error)
        .def("shutdown", &PyIPFSManager::shutdown);

    // Network Manager class
    py::class_<PyNetworkManager>(m, "NetworkManager")
        .def(py::init<>())
        .def("initialize", &PyNetworkManager::initialize)
        .def("connect", &PyNetworkManager::connect)
        .def("disconnect", &PyNetworkManager::disconnect)
        .def("is_connected", &PyNetworkManager::is_connected)
        .def("send_message", &PyNetworkManager::send_message)
        .def("get_last_error", &PyNetworkManager::get_last_error)
        .def("shutdown", &PyNetworkManager::shutdown);

    // Core Manager class
    py::class_<PyCoreManager>(m, "CoreManager")
        .def(py::init<>())
        .def("initialize", &PyCoreManager::initialize)
        .def("get_last_error", &PyCoreManager::get_last_error)
        .def("shutdown", &PyCoreManager::shutdown);

    // Asset Manager class
    py::class_<PyAssetManager>(m, "AssetManager")
        .def(py::init<>())
        .def("initialize", &PyAssetManager::initialize)
        .def("get_last_error", &PyAssetManager::get_last_error)
        .def("shutdown", &PyAssetManager::shutdown);

    // NFT Manager class
    py::class_<PyNFTManager>(m, "NFTManager")
        .def(py::init<>())
        .def("initialize", &PyNFTManager::initialize)
        .def("get_last_error", &PyNFTManager::get_last_error)
        .def("shutdown", &PyNFTManager::shutdown);

    // Wallet Manager class
    py::class_<PyWalletManager>(m, "WalletManager")
        .def(py::init<>())
        .def("initialize", &PyWalletManager::initialize)
        .def("get_last_error", &PyWalletManager::get_last_error)
        .def("shutdown", &PyWalletManager::shutdown);

    // Database types enum
    py::enum_<satox::core::DatabaseType>(m, "DatabaseType")
        .value("SQLITE", satox::core::DatabaseType::SQLITE)
        .value("POSTGRESQL", satox::core::DatabaseType::POSTGRESQL)
        .value("MYSQL", satox::core::DatabaseType::MYSQL)
        .value("REDIS", satox::core::DatabaseType::REDIS)
        .value("MONGODB", satox::core::DatabaseType::MONGODB)
        .value("ROCKSDB", satox::core::DatabaseType::ROCKSDB)
        .value("MEMORY", satox::core::DatabaseType::MEMORY)
        .value("SUPABASE", satox::core::DatabaseType::SUPABASE)
        .value("FIREBASE", satox::core::DatabaseType::FIREBASE)
        .value("AWS", satox::core::DatabaseType::AWS)
        .value("AZURE", satox::core::DatabaseType::AZURE)
        .value("GOOGLE_CLOUD", satox::core::DatabaseType::GOOGLE_CLOUD);

    // Security level enum
    py::enum_<satox::core::SecurityManager::SecurityLevel>(m, "SecurityLevel")
        .value("LOW", satox::core::SecurityManager::SecurityLevel::LOW)
        .value("MEDIUM", satox::core::SecurityManager::SecurityLevel::MEDIUM)
        .value("HIGH", satox::core::SecurityManager::SecurityLevel::HIGH);

    // Blockchain state enum
    py::enum_<satox::core::BlockchainManager::BlockchainState>(m, "BlockchainState")
        .value("UNINITIALIZED", satox::core::BlockchainManager::BlockchainState::UNINITIALIZED)
        .value("INITIALIZING", satox::core::BlockchainManager::BlockchainState::INITIALIZING)
        .value("INITIALIZED", satox::core::BlockchainManager::BlockchainState::INITIALIZED)
        .value("CONNECTING", satox::core::BlockchainManager::BlockchainState::CONNECTING)
        .value("CONNECTED", satox::core::BlockchainManager::BlockchainState::CONNECTED)
        .value("DISCONNECTED", satox::core::BlockchainManager::BlockchainState::DISCONNECTED)
        .value("SYNCING", satox::core::BlockchainManager::BlockchainState::SYNCING)
        .value("SYNCED", satox::core::BlockchainManager::BlockchainState::SYNCED)
        .value("ERROR", satox::core::BlockchainManager::BlockchainState::ERROR)
        .value("SHUTDOWN", satox::core::BlockchainManager::BlockchainState::SHUTDOWN);

    // Network type enum
    py::enum_<satox::core::NetworkManager::NetworkType>(m, "NetworkType")
        .value("MAINNET", satox::core::NetworkManager::NetworkType::MAINNET)
        .value("TESTNET", satox::core::NetworkManager::NetworkType::TESTNET)
        .value("REGTEST", satox::core::NetworkManager::NetworkType::REGTEST);

    // IPFS node state enum
    py::enum_<satox::core::IPFSManager::NodeState>(m, "IPFSNodeState")
        .value("UNINITIALIZED", satox::core::IPFSManager::NodeState::UNINITIALIZED)
        .value("INITIALIZING", satox::core::IPFSManager::NodeState::INITIALIZING)
        .value("CONNECTING", satox::core::IPFSManager::NodeState::CONNECTING)
        .value("CONNECTED", satox::core::IPFSManager::NodeState::CONNECTED)
        .value("DISCONNECTED", satox::core::IPFSManager::NodeState::DISCONNECTED)
        .value("ERROR", satox::core::IPFSManager::NodeState::ERROR);

    // IPFS content type enum
    py::enum_<satox::core::IPFSManager::ContentType>(m, "IPFSContentType")
        .value("FILE", satox::core::IPFSManager::ContentType::FILE)
        .value("DIRECTORY", satox::core::IPFSManager::ContentType::DIRECTORY)
        .value("SYMLINK", satox::core::IPFSManager::ContentType::SYMLINK)
        .value("RAW", satox::core::IPFSManager::ContentType::RAW)
        .value("JSON", satox::core::IPFSManager::ContentType::JSON)
        .value("TEXT", satox::core::IPFSManager::ContentType::TEXT)
        .value("BINARY", satox::core::IPFSManager::ContentType::BINARY);
} 