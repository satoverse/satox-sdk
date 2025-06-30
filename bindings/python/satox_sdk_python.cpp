/**
 * @file satox_sdk_python.cpp
 * @brief Python bindings for Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/eigen.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/stl_bind.h>
#include <pybind11/cast.h>
#include <pybind11/attr.h>
#include <pybind11/buffer_info.h>
#include <pybind11/cast.h>
#include <pybind11/chrono.h>
#include <pybind11/complex.h>
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "satox/satox.hpp"
#include "satox/core/core_manager.hpp"
#include "satox/database/database_manager.hpp"
#include "satox/security/security_manager.hpp"
#include "satox/nft/nft_manager.hpp"

namespace py = pybind11;
using namespace satox;

// Helper function to convert nlohmann::json to Python dict
py::object json_to_python(const nlohmann::json& j) {
    if (j.is_null()) {
        return py::none();
    } else if (j.is_boolean()) {
        return py::bool_(j.get<bool>());
    } else if (j.is_number_integer()) {
        return py::int_(j.get<int64_t>());
    } else if (j.is_number_float()) {
        return py::float_(j.get<double>());
    } else if (j.is_string()) {
        return py::str(j.get<std::string>());
    } else if (j.is_array()) {
        py::list result;
        for (const auto& item : j) {
            result.append(json_to_python(item));
        }
        return result;
    } else if (j.is_object()) {
        py::dict result;
        for (auto it = j.begin(); it != j.end(); ++it) {
            result[py::str(it.key())] = json_to_python(it.value());
        }
        return result;
    }
    return py::none();
}

// Helper function to convert Python object to nlohmann::json
nlohmann::json python_to_json(py::object obj) {
    if (obj.is_none()) {
        return nlohmann::json();
    } else if (py::isinstance<py::bool_>(obj)) {
        return nlohmann::json(obj.cast<bool>());
    } else if (py::isinstance<py::int_>(obj)) {
        return nlohmann::json(obj.cast<int64_t>());
    } else if (py::isinstance<py::float_>(obj)) {
        return nlohmann::json(obj.cast<double>());
    } else if (py::isinstance<py::str>(obj)) {
        return nlohmann::json(obj.cast<std::string>());
    } else if (py::isinstance<py::list>(obj)) {
        nlohmann::json result = nlohmann::json::array();
        py::list list = obj.cast<py::list>();
        for (const auto& item : list) {
            result.push_back(python_to_json(item.cast<py::object>()));
        }
        return result;
    } else if (py::isinstance<py::dict>(obj)) {
        nlohmann::json result = nlohmann::json::object();
        py::dict dict = obj.cast<py::dict>();
        for (const auto& item : dict) {
            std::string key = item.first.cast<std::string>();
            result[key] = python_to_json(item.second.cast<py::object>());
        }
        return result;
    }
    return nlohmann::json();
}

// Wrapper classes for better Python integration
class PySDK {
public:
    static PySDK& getInstance() {
        static PySDK instance;
        return instance;
    }

    bool initialize(py::object config) {
        nlohmann::json json_config = python_to_json(config);
        return SDK::getInstance().initialize(json_config);
    }

    void shutdown() {
        SDK::getInstance().shutdown();
    }

    bool isInitialized() const {
        return SDK::getInstance().isInitialized();
    }

    py::object getHealthStatus() const {
        auto status = SDK::getInstance().getHealthStatus();
        return json_to_python(status);
    }

    py::object getStats() const {
        auto stats = SDK::getInstance().getStats();
        return json_to_python(stats);
    }

    std::string getVersion() const {
        return SDK::getInstance().getVersion();
    }

    // Component getters
    core::CoreManager& getCoreManager() {
        return SDK::getInstance().getCoreManager();
    }

    database::DatabaseManager& getDatabaseManager() {
        return SDK::getInstance().getDatabaseManager();
    }

    security::SecurityManager& getSecurityManager() {
        return SDK::getInstance().getSecurityManager();
    }

    nft::NFTManager& getNFTManager() {
        return SDK::getInstance().getNFTManager();
    }

private:
    PySDK() = default;
    PySDK(const PySDK&) = delete;
    PySDK& operator=(const PySDK&) = delete;
};

// Wrapper for CoreManager
class PyCoreManager {
public:
    static PyCoreManager& getInstance() {
        static PyCoreManager instance;
        return instance;
    }

    bool initialize(py::object config) {
        core::CoreConfig core_config;
        auto json_config = python_to_json(config);
        
        if (json_config.contains("data_dir")) core_config.data_dir = json_config["data_dir"];
        if (json_config.contains("network")) core_config.network = json_config["network"];
        if (json_config.contains("enable_mining")) core_config.enable_mining = json_config["enable_mining"];
        if (json_config.contains("enable_sync")) core_config.enable_sync = json_config["enable_sync"];
        if (json_config.contains("sync_interval_ms")) core_config.sync_interval_ms = json_config["sync_interval_ms"];
        if (json_config.contains("mining_threads")) core_config.mining_threads = json_config["mining_threads"];
        if (json_config.contains("rpc_endpoint")) core_config.rpc_endpoint = json_config["rpc_endpoint"];
        if (json_config.contains("rpc_username")) core_config.rpc_username = json_config["rpc_username"];
        if (json_config.contains("rpc_password")) core_config.rpc_password = json_config["rpc_password"];
        if (json_config.contains("timeout_ms")) core_config.timeout_ms = json_config["timeout_ms"];
        
        return core::CoreManager::getInstance().initialize(core_config);
    }

    void shutdown() {
        core::CoreManager::getInstance().shutdown();
    }

    bool isInitialized() const {
        return core::CoreManager::getInstance().isInitialized();
    }

    bool isHealthy() const {
        return core::CoreManager::getInstance().isHealthy();
    }

    bool isRunning() const {
        return core::CoreManager::getInstance().isRunning();
    }

    std::string getLastError() const {
        return core::CoreManager::getInstance().getLastError();
    }

    py::object getStats() const {
        auto stats = core::CoreManager::getInstance().getStats();
        return json_to_python(nlohmann::json{
            {"start_time", stats.start_time.time_since_epoch().count()},
            {"last_activity", stats.last_activity.time_since_epoch().count()},
            {"total_components", stats.total_components},
            {"active_components", stats.active_components},
            {"failed_components", stats.failed_components}
        });
    }

private:
    PyCoreManager() = default;
    PyCoreManager(const PyCoreManager&) = delete;
    PyCoreManager& operator=(const PyCoreManager&) = delete;
};

// Wrapper for DatabaseManager
class PyDatabaseManager {
public:
    static PyDatabaseManager& getInstance() {
        static PyDatabaseManager instance;
        return instance;
    }

    bool initialize(py::object config) {
        database::DatabaseConfig db_config;
        auto json_config = python_to_json(config);
        
        if (json_config.contains("name")) db_config.name = json_config["name"];
        if (json_config.contains("enableLogging")) db_config.enableLogging = json_config["enableLogging"];
        if (json_config.contains("logPath")) db_config.logPath = json_config["logPath"];
        if (json_config.contains("maxConnections")) db_config.maxConnections = json_config["maxConnections"];
        if (json_config.contains("connectionTimeout")) db_config.connectionTimeout = json_config["connectionTimeout"];
        
        return database::DatabaseManager::getInstance().initialize(db_config);
    }

    void shutdown() {
        database::DatabaseManager::getInstance().shutdown();
    }

    bool isInitialized() const {
        return database::DatabaseManager::getInstance().isInitialized();
    }

    bool isHealthy() const {
        return database::DatabaseManager::getInstance().isHealthy();
    }

    py::object getHealthStatus() const {
        auto status = database::DatabaseManager::getInstance().getHealthStatus();
        return json_to_python(status);
    }

    py::object getStats() const {
        auto stats = database::DatabaseManager::getInstance().getStats();
        return json_to_python(nlohmann::json{
            {"totalOperations", stats.totalOperations},
            {"successfulOperations", stats.successfulOperations},
            {"failedOperations", stats.failedOperations}
        });
    }

    // Database operations
    bool createDatabase(const std::string& name) {
        return database::DatabaseManager::getInstance().createDatabase(name);
    }

    bool deleteDatabase(const std::string& name) {
        return database::DatabaseManager::getInstance().deleteDatabase(name);
    }

    bool useDatabase(const std::string& name) {
        return database::DatabaseManager::getInstance().useDatabase(name);
    }

    py::list listDatabases() {
        auto databases = database::DatabaseManager::getInstance().listDatabases();
        py::list result;
        for (const auto& db : databases) {
            result.append(py::str(db));
        }
        return result;
    }

    bool databaseExists(const std::string& name) {
        return database::DatabaseManager::getInstance().databaseExists(name);
    }

    // Table operations
    bool createTable(const std::string& name, py::object schema) {
        auto json_schema = python_to_json(schema);
        return database::DatabaseManager::getInstance().createTable(name, json_schema);
    }

    bool deleteTable(const std::string& name) {
        return database::DatabaseManager::getInstance().deleteTable(name);
    }

    bool tableExists(const std::string& name) {
        return database::DatabaseManager::getInstance().tableExists(name);
    }

    py::list listTables() {
        auto tables = database::DatabaseManager::getInstance().listTables();
        py::list result;
        for (const auto& table : tables) {
            result.append(py::str(table));
        }
        return result;
    }

    py::object getTableSchema(const std::string& name) {
        auto schema = database::DatabaseManager::getInstance().getTableSchema(name);
        return json_to_python(schema);
    }

    // Data operations
    bool insert(const std::string& table, py::object data) {
        auto json_data = python_to_json(data);
        return database::DatabaseManager::getInstance().insert(table, json_data);
    }

    bool update(const std::string& table, const std::string& id, py::object data) {
        auto json_data = python_to_json(data);
        return database::DatabaseManager::getInstance().update(table, id, json_data);
    }

    bool remove(const std::string& table, const std::string& id) {
        return database::DatabaseManager::getInstance().remove(table, id);
    }

    py::object find(const std::string& table, const std::string& id) {
        auto result = database::DatabaseManager::getInstance().find(table, id);
        return json_to_python(result);
    }

    py::list query(const std::string& table, py::object query_obj) {
        auto json_query = python_to_json(query_obj);
        auto results = database::DatabaseManager::getInstance().query(table, json_query);
        py::list result;
        for (const auto& item : results) {
            result.append(json_to_python(item));
        }
        return result;
    }

    // Transaction operations
    bool beginTransaction() {
        return database::DatabaseManager::getInstance().beginTransaction();
    }

    bool commitTransaction() {
        return database::DatabaseManager::getInstance().commitTransaction();
    }

    bool rollbackTransaction() {
        return database::DatabaseManager::getInstance().rollbackTransaction();
    }

    bool isInTransaction() const {
        return database::DatabaseManager::getInstance().isInTransaction();
    }

private:
    PyDatabaseManager() = default;
    PyDatabaseManager(const PyDatabaseManager&) = delete;
    PyDatabaseManager& operator=(const PyDatabaseManager&) = delete;
};

// Wrapper for SecurityManager
class PySecurityManager {
public:
    static PySecurityManager& getInstance() {
        static PySecurityManager instance;
        return instance;
    }

    bool initialize(py::object config) {
        security::SecurityConfig sec_config;
        auto json_config = python_to_json(config);
        
        if (json_config.contains("name")) sec_config.name = json_config["name"];
        if (json_config.contains("enablePQC")) sec_config.enablePQC = json_config["enablePQC"];
        if (json_config.contains("enableInputValidation")) sec_config.enableInputValidation = json_config["enableInputValidation"];
        if (json_config.contains("enableRateLimiting")) sec_config.enableRateLimiting = json_config["enableRateLimiting"];
        if (json_config.contains("enableLogging")) sec_config.enableLogging = json_config["enableLogging"];
        if (json_config.contains("logPath")) sec_config.logPath = json_config["logPath"];
        
        return security::SecurityManager::getInstance().initialize(sec_config);
    }

    void shutdown() {
        security::SecurityManager::getInstance().shutdown();
    }

    bool isInitialized() const {
        return security::SecurityManager::getInstance().isInitialized();
    }

    bool isHealthy() const {
        return security::SecurityManager::getInstance().isHealthy();
    }

    bool isRunning() const {
        return security::SecurityManager::getInstance().isRunning();
    }

    py::object getHealthStatus() const {
        auto status = security::SecurityManager::getInstance().getHealthStatus();
        return json_to_python(status);
    }

    py::object getStats() const {
        auto stats = security::SecurityManager::getInstance().getStats();
        return json_to_python(nlohmann::json{
            {"totalOperations", stats.totalOperations},
            {"successfulOperations", stats.successfulOperations},
            {"failedOperations", stats.failedOperations},
            {"securityViolations", stats.securityViolations},
            {"pqcOperations", stats.pqcOperations},
            {"validationOperations", stats.validationOperations}
        });
    }

    // Security operations
    bool performOperation(const std::string& operation, py::object params) {
        auto json_params = python_to_json(params);
        return security::SecurityManager::getInstance().performOperation(operation, json_params);
    }

    bool validateInput(const std::string& input, const std::string& type) {
        return security::SecurityManager::getInstance().validateInput(input, type);
    }

    bool validateEmail(const std::string& email) {
        return security::SecurityManager::getInstance().validateEmail(email);
    }

    bool validateURL(const std::string& url) {
        return security::SecurityManager::getInstance().validateURL(url);
    }

    bool validateIPAddress(const std::string& ip) {
        return security::SecurityManager::getInstance().validateIPAddress(ip);
    }

    bool validateJson(const std::string& json_str) {
        return security::SecurityManager::getInstance().validateJson(json_str);
    }

    bool validateToken(const std::string& token) {
        return security::SecurityManager::getInstance().validateToken(token);
    }

    bool checkRateLimit(const std::string& identifier, const std::string& operation) {
        return security::SecurityManager::getInstance().checkRateLimit(identifier, operation);
    }

    bool setRateLimit(const std::string& identifier, int maxRequests, int timeWindow) {
        return security::SecurityManager::getInstance().setRateLimit(identifier, maxRequests, timeWindow);
    }

    bool resetRateLimit(const std::string& identifier) {
        return security::SecurityManager::getInstance().resetRateLimit(identifier);
    }

    std::string getLastError() const {
        return security::SecurityManager::getInstance().getLastError();
    }

private:
    PySecurityManager() = default;
    PySecurityManager(const PySecurityManager&) = delete;
    PySecurityManager& operator=(const PySecurityManager&) = delete;
};

// Wrapper for NFTManager
class PyNFTManager {
public:
    static PyNFTManager& getInstance() {
        static PyNFTManager instance;
        return instance;
    }

    bool initialize() {
        return nft::NFTManager::getInstance().initialize();
    }

    void shutdown() {
        nft::NFTManager::getInstance().shutdown();
    }

    bool isInitialized() const {
        return nft::NFTManager::getInstance().isInitialized();
    }

    bool isHealthy() const {
        return nft::NFTManager::getInstance().isHealthy();
    }

    // NFT Creation
    std::string createNFT(const std::string& contractAddress, py::object metadata, 
                         const std::string& creator, bool isTransferable = true, 
                         bool isBurnable = true, const std::string& royaltyRecipient = "",
                         uint32_t royaltyBasisPoints = 0) {
        nft::NFTManager::NFTMetadata nft_metadata;
        auto json_metadata = python_to_json(metadata);
        
        if (json_metadata.contains("name")) nft_metadata.name = json_metadata["name"];
        if (json_metadata.contains("description")) nft_metadata.description = json_metadata["description"];
        if (json_metadata.contains("image")) nft_metadata.image = json_metadata["image"];
        if (json_metadata.contains("externalUrl")) nft_metadata.externalUrl = json_metadata["externalUrl"];
        if (json_metadata.contains("attributes")) {
            for (auto it = json_metadata["attributes"].begin(); it != json_metadata["attributes"].end(); ++it) {
                nft_metadata.attributes[it.key()] = it.value();
            }
        }
        if (json_metadata.contains("additionalData")) nft_metadata.additionalData = json_metadata["additionalData"];
        
        return nft::NFTManager::getInstance().createNFT(contractAddress, nft_metadata, creator, 
                                                       isTransferable, isBurnable, royaltyRecipient, 
                                                       royaltyBasisPoints);
    }

    // NFT Operations
    bool transferNFT(const std::string& nftId, const std::string& from, const std::string& to) {
        return nft::NFTManager::getInstance().transferNFT(nftId, from, to);
    }

    bool burnNFT(const std::string& nftId, const std::string& owner) {
        return nft::NFTManager::getInstance().burnNFT(nftId, owner);
    }

    bool updateNFTMetadata(const std::string& nftId, py::object newMetadata, const std::string& updater) {
        nft::NFTManager::NFTMetadata nft_metadata;
        auto json_metadata = python_to_json(newMetadata);
        
        if (json_metadata.contains("name")) nft_metadata.name = json_metadata["name"];
        if (json_metadata.contains("description")) nft_metadata.description = json_metadata["description"];
        if (json_metadata.contains("image")) nft_metadata.image = json_metadata["image"];
        if (json_metadata.contains("externalUrl")) nft_metadata.externalUrl = json_metadata["externalUrl"];
        if (json_metadata.contains("attributes")) {
            for (auto it = json_metadata["attributes"].begin(); it != json_metadata["attributes"].end(); ++it) {
                nft_metadata.attributes[it.key()] = it.value();
            }
        }
        if (json_metadata.contains("additionalData")) nft_metadata.additionalData = json_metadata["additionalData"];
        
        return nft::NFTManager::getInstance().updateNFTMetadata(nftId, nft_metadata, updater);
    }

    bool addNFTAttribute(const std::string& nftId, const std::string& key, const std::string& value, const std::string& updater) {
        return nft::NFTManager::getInstance().addNFTAttribute(nftId, key, value, updater);
    }

    bool removeNFTAttribute(const std::string& nftId, const std::string& key, const std::string& updater) {
        return nft::NFTManager::getInstance().removeNFTAttribute(nftId, key, updater);
    }

    bool setNFTTransferable(const std::string& nftId, bool isTransferable) {
        return nft::NFTManager::getInstance().setNFTTransferable(nftId, isTransferable);
    }

    bool setNFTBurnable(const std::string& nftId, bool isBurnable) {
        return nft::NFTManager::getInstance().setNFTBurnable(nftId, isBurnable);
    }

    bool setNFTRoyalty(const std::string& nftId, const std::string& recipient, uint32_t basisPoints) {
        return nft::NFTManager::getInstance().setNFTRoyalty(nftId, recipient, basisPoints);
    }

    // NFT Queries
    py::object getNFT(const std::string& nftId) {
        auto nft = nft::NFTManager::getInstance().getNFT(nftId);
        return json_to_python(nlohmann::json{
            {"id", nft.id},
            {"contractAddress", nft.contractAddress},
            {"owner", nft.owner},
            {"creator", nft.creator},
            {"tokenId", nft.tokenId},
            {"tokenURI", nft.tokenURI},
            {"isTransferable", nft.isTransferable},
            {"isBurnable", nft.isBurnable},
            {"royaltyRecipient", nft.royaltyRecipient},
            {"royaltyBasisPoints", nft.royaltyBasisPoints}
        });
    }

    py::list getNFTsByOwner(const std::string& owner) {
        auto nfts = nft::NFTManager::getInstance().getNFTsByOwner(owner);
        py::list result;
        for (const auto& nft : nfts) {
            result.append(json_to_python(nlohmann::json{
                {"id", nft.id},
                {"contractAddress", nft.contractAddress},
                {"owner", nft.owner},
                {"creator", nft.creator},
                {"tokenId", nft.tokenId},
                {"tokenURI", nft.tokenURI},
                {"isTransferable", nft.isTransferable},
                {"isBurnable", nft.isBurnable},
                {"royaltyRecipient", nft.royaltyRecipient},
                {"royaltyBasisPoints", nft.royaltyBasisPoints}
            }));
        }
        return result;
    }

    py::list getNFTsByContract(const std::string& contractAddress) {
        auto nfts = nft::NFTManager::getInstance().getNFTsByContract(contractAddress);
        py::list result;
        for (const auto& nft : nfts) {
            result.append(json_to_python(nlohmann::json{
                {"id", nft.id},
                {"contractAddress", nft.contractAddress},
                {"owner", nft.owner},
                {"creator", nft.creator},
                {"tokenId", nft.tokenId},
                {"tokenURI", nft.tokenURI},
                {"isTransferable", nft.isTransferable},
                {"isBurnable", nft.isBurnable},
                {"royaltyRecipient", nft.royaltyRecipient},
                {"royaltyBasisPoints", nft.royaltyBasisPoints}
            }));
        }
        return result;
    }

    py::list getNFTsByCreator(const std::string& creator) {
        auto nfts = nft::NFTManager::getInstance().getNFTsByCreator(creator);
        py::list result;
        for (const auto& nft : nfts) {
            result.append(json_to_python(nlohmann::json{
                {"id", nft.id},
                {"contractAddress", nft.contractAddress},
                {"owner", nft.owner},
                {"creator", nft.creator},
                {"tokenId", nft.tokenId},
                {"tokenURI", nft.tokenURI},
                {"isTransferable", nft.isTransferable},
                {"isBurnable", nft.isBurnable},
                {"royaltyRecipient", nft.royaltyRecipient},
                {"royaltyBasisPoints", nft.royaltyBasisPoints}
            }));
        }
        return result;
    }

    bool isNFTTransferable(const std::string& nftId) {
        return nft::NFTManager::getInstance().isNFTTransferable(nftId);
    }

    bool isNFTBurnable(const std::string& nftId) {
        return nft::NFTManager::getInstance().isNFTBurnable(nftId);
    }

    std::string getNFTOwner(const std::string& nftId) {
        return nft::NFTManager::getInstance().getNFTOwner(nftId);
    }

    py::object getNFTMetadata(const std::string& nftId) {
        auto metadata = nft::NFTManager::getInstance().getNFTMetadata(nftId);
        return json_to_python(nlohmann::json{
            {"name", metadata.name},
            {"description", metadata.description},
            {"image", metadata.image},
            {"externalUrl", metadata.externalUrl},
            {"attributes", metadata.attributes},
            {"additionalData", metadata.additionalData}
        });
    }

    // Batch Operations
    py::list batchCreateNFTs(const std::string& contractAddress, py::list metadataList, const std::string& creator) {
        std::vector<nft::NFTManager::NFTMetadata> nft_metadata_list;
        
        for (const auto& item : metadataList) {
            nft::NFTManager::NFTMetadata nft_metadata;
            auto json_metadata = python_to_json(item.cast<py::object>());
            
            if (json_metadata.contains("name")) nft_metadata.name = json_metadata["name"];
            if (json_metadata.contains("description")) nft_metadata.description = json_metadata["description"];
            if (json_metadata.contains("image")) nft_metadata.image = json_metadata["image"];
            if (json_metadata.contains("externalUrl")) nft_metadata.externalUrl = json_metadata["externalUrl"];
            if (json_metadata.contains("attributes")) {
                for (auto it = json_metadata["attributes"].begin(); it != json_metadata["attributes"].end(); ++it) {
                    nft_metadata.attributes[it.key()] = it.value();
                }
            }
            if (json_metadata.contains("additionalData")) nft_metadata.additionalData = json_metadata["additionalData"];
            
            nft_metadata_list.push_back(nft_metadata);
        }
        
        auto results = nft::NFTManager::getInstance().batchCreateNFTs(contractAddress, nft_metadata_list, creator);
        py::list result;
        for (const auto& id : results) {
            result.append(py::str(id));
        }
        return result;
    }

    bool batchTransferNFTs(py::list nftIds, const std::string& from, const std::string& to) {
        std::vector<std::string> nft_id_list;
        for (const auto& item : nftIds) {
            nft_id_list.push_back(item.cast<std::string>());
        }
        return nft::NFTManager::getInstance().batchTransferNFTs(nft_id_list, from, to);
    }

    bool batchBurnNFTs(py::list nftIds, const std::string& owner) {
        std::vector<std::string> nft_id_list;
        for (const auto& item : nftIds) {
            nft_id_list.push_back(item.cast<std::string>());
        }
        return nft::NFTManager::getInstance().batchBurnNFTs(nft_id_list, owner);
    }

    // Validation
    bool validateNFT(py::object nft_obj) {
        // This would require creating an NFT object from the Python dict
        // For now, return true as a placeholder
        return true;
    }

    bool validateTransfer(const std::string& nftId, const std::string& from, const std::string& to) {
        return nft::NFTManager::getInstance().validateTransfer(nftId, from, to);
    }

    // Error handling
    py::object getLastError() const {
        auto error = nft::NFTManager::getInstance().getLastError();
        return json_to_python(nlohmann::json{
            {"code", error.code},
            {"message", error.message}
        });
    }

    void clearLastError() {
        nft::NFTManager::getInstance().clearLastError();
    }

    // Ownership history
    py::list getOwnershipHistory(const std::string& nftId) {
        auto history = nft::NFTManager::getInstance().getOwnershipHistory(nftId);
        py::list result;
        for (const auto& record : history) {
            result.append(json_to_python(nlohmann::json{
                {"owner", record.owner},
                {"previousOwner", record.previousOwner},
                {"timestamp", record.timestamp.time_since_epoch().count()},
                {"transactionId", record.transactionId},
                {"reason", record.reason}
            }));
        }
        return result;
    }

    std::string getCurrentOwner(const std::string& nftId) {
        return nft::NFTManager::getInstance().getCurrentOwner(nftId);
    }

    bool verifyOwnership(const std::string& nftId, const std::string& owner) {
        return nft::NFTManager::getInstance().verifyOwnership(nftId, owner);
    }

private:
    PyNFTManager() = default;
    PyNFTManager(const PyNFTManager&) = delete;
    PyNFTManager& operator=(const PyNFTManager&) = delete;
};

PYBIND11_MODULE(satox_sdk, m) {
    m.doc() = "Satox SDK Python Bindings"; // Optional module docstring

    // Main SDK class
    py::class_<PySDK>(m, "SDK")
        .def(py::init<>())
        .def_static("getInstance", &PySDK::getInstance, py::return_value_policy::reference)
        .def("initialize", &PySDK::initialize)
        .def("shutdown", &PySDK::shutdown)
        .def("isInitialized", &PySDK::isInitialized)
        .def("getHealthStatus", &PySDK::getHealthStatus)
        .def("getStats", &PySDK::getStats)
        .def("getVersion", &PySDK::getVersion)
        .def("getCoreManager", &PySDK::getCoreManager, py::return_value_policy::reference)
        .def("getDatabaseManager", &PySDK::getDatabaseManager, py::return_value_policy::reference)
        .def("getSecurityManager", &PySDK::getSecurityManager, py::return_value_policy::reference)
        .def("getNFTManager", &PySDK::getNFTManager, py::return_value_policy::reference);

    // Core Manager
    py::class_<PyCoreManager>(m, "CoreManager")
        .def(py::init<>())
        .def_static("getInstance", &PyCoreManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &PyCoreManager::initialize)
        .def("shutdown", &PyCoreManager::shutdown)
        .def("isInitialized", &PyCoreManager::isInitialized)
        .def("isHealthy", &PyCoreManager::isHealthy)
        .def("isRunning", &PyCoreManager::isRunning)
        .def("getLastError", &PyCoreManager::getLastError)
        .def("getStats", &PyCoreManager::getStats);

    // Database Manager
    py::class_<PyDatabaseManager>(m, "DatabaseManager")
        .def(py::init<>())
        .def_static("getInstance", &PyDatabaseManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &PyDatabaseManager::initialize)
        .def("shutdown", &PyDatabaseManager::shutdown)
        .def("isInitialized", &PyDatabaseManager::isInitialized)
        .def("isHealthy", &PyDatabaseManager::isHealthy)
        .def("getHealthStatus", &PyDatabaseManager::getHealthStatus)
        .def("getStats", &PyDatabaseManager::getStats)
        .def("createDatabase", &PyDatabaseManager::createDatabase)
        .def("deleteDatabase", &PyDatabaseManager::deleteDatabase)
        .def("useDatabase", &PyDatabaseManager::useDatabase)
        .def("listDatabases", &PyDatabaseManager::listDatabases)
        .def("databaseExists", &PyDatabaseManager::databaseExists)
        .def("createTable", &PyDatabaseManager::createTable)
        .def("deleteTable", &PyDatabaseManager::deleteTable)
        .def("tableExists", &PyDatabaseManager::tableExists)
        .def("listTables", &PyDatabaseManager::listTables)
        .def("getTableSchema", &PyDatabaseManager::getTableSchema)
        .def("insert", &PyDatabaseManager::insert)
        .def("update", &PyDatabaseManager::update)
        .def("remove", &PyDatabaseManager::remove)
        .def("find", &PyDatabaseManager::find)
        .def("query", &PyDatabaseManager::query)
        .def("beginTransaction", &PyDatabaseManager::beginTransaction)
        .def("commitTransaction", &PyDatabaseManager::commitTransaction)
        .def("rollbackTransaction", &PyDatabaseManager::rollbackTransaction)
        .def("isInTransaction", &PyDatabaseManager::isInTransaction);

    // Security Manager
    py::class_<PySecurityManager>(m, "SecurityManager")
        .def(py::init<>())
        .def_static("getInstance", &PySecurityManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &PySecurityManager::initialize)
        .def("shutdown", &PySecurityManager::shutdown)
        .def("isInitialized", &PySecurityManager::isInitialized)
        .def("isHealthy", &PySecurityManager::isHealthy)
        .def("isRunning", &PySecurityManager::isRunning)
        .def("getHealthStatus", &PySecurityManager::getHealthStatus)
        .def("getStats", &PySecurityManager::getStats)
        .def("performOperation", &PySecurityManager::performOperation)
        .def("validateInput", &PySecurityManager::validateInput)
        .def("validateEmail", &PySecurityManager::validateEmail)
        .def("validateURL", &PySecurityManager::validateURL)
        .def("validateIPAddress", &PySecurityManager::validateIPAddress)
        .def("validateJson", &PySecurityManager::validateJson)
        .def("validateToken", &PySecurityManager::validateToken)
        .def("checkRateLimit", &PySecurityManager::checkRateLimit)
        .def("setRateLimit", &PySecurityManager::setRateLimit)
        .def("resetRateLimit", &PySecurityManager::resetRateLimit)
        .def("getLastError", &PySecurityManager::getLastError);

    // NFT Manager
    py::class_<PyNFTManager>(m, "NFTManager")
        .def(py::init<>())
        .def_static("getInstance", &PyNFTManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &PyNFTManager::initialize)
        .def("shutdown", &PyNFTManager::shutdown)
        .def("isInitialized", &PyNFTManager::isInitialized)
        .def("isHealthy", &PyNFTManager::isHealthy)
        .def("createNFT", &PyNFTManager::createNFT)
        .def("transferNFT", &PyNFTManager::transferNFT)
        .def("burnNFT", &PyNFTManager::burnNFT)
        .def("updateNFTMetadata", &PyNFTManager::updateNFTMetadata)
        .def("addNFTAttribute", &PyNFTManager::addNFTAttribute)
        .def("removeNFTAttribute", &PyNFTManager::removeNFTAttribute)
        .def("setNFTTransferable", &PyNFTManager::setNFTTransferable)
        .def("setNFTBurnable", &PyNFTManager::setNFTBurnable)
        .def("setNFTRoyalty", &PyNFTManager::setNFTRoyalty)
        .def("getNFT", &PyNFTManager::getNFT)
        .def("getNFTsByOwner", &PyNFTManager::getNFTsByOwner)
        .def("getNFTsByContract", &PyNFTManager::getNFTsByContract)
        .def("getNFTsByCreator", &PyNFTManager::getNFTsByCreator)
        .def("isNFTTransferable", &PyNFTManager::isNFTTransferable)
        .def("isNFTBurnable", &PyNFTManager::isNFTBurnable)
        .def("getNFTOwner", &PyNFTManager::getNFTOwner)
        .def("getNFTMetadata", &PyNFTManager::getNFTMetadata)
        .def("batchCreateNFTs", &PyNFTManager::batchCreateNFTs)
        .def("batchTransferNFTs", &PyNFTManager::batchTransferNFTs)
        .def("batchBurnNFTs", &PyNFTManager::batchBurnNFTs)
        .def("validateNFT", &PyNFTManager::validateNFT)
        .def("validateTransfer", &PyNFTManager::validateTransfer)
        .def("getLastError", &PyNFTManager::getLastError)
        .def("clearLastError", &PyNFTManager::clearLastError)
        .def("getOwnershipHistory", &PyNFTManager::getOwnershipHistory)
        .def("getCurrentOwner", &PyNFTManager::getCurrentOwner)
        .def("verifyOwnership", &PyNFTManager::verifyOwnership);

    // Enums
    py::enum_<core::ComponentState>(m, "ComponentState")
        .value("UNINITIALIZED", core::ComponentState::UNINITIALIZED)
        .value("INITIALIZING", core::ComponentState::INITIALIZING)
        .value("RUNNING", core::ComponentState::RUNNING)
        .value("RESTARTING", core::ComponentState::RESTARTING)
        .value("ERROR", core::ComponentState::ERROR)
        .value("SHUTDOWN", core::ComponentState::SHUTDOWN);

    py::enum_<security::SecurityState>(m, "SecurityState")
        .value("UNINITIALIZED", security::SecurityState::UNINITIALIZED)
        .value("INITIALIZING", security::SecurityState::INITIALIZING)
        .value("INITIALIZED", security::SecurityState::INITIALIZED)
        .value("RUNNING", security::SecurityState::RUNNING)
        .value("STOPPED", security::SecurityState::STOPPED)
        .value("ERROR", security::SecurityState::ERROR)
        .value("SHUTDOWN", security::SecurityState::SHUTDOWN);

    py::enum_<nft::NFTErrorCode>(m, "NFTErrorCode")
        .value("NONE", nft::NFTErrorCode::NONE)
        .value("INVALID_METADATA", nft::NFTErrorCode::INVALID_METADATA)
        .value("INVALID_OWNER", nft::NFTErrorCode::INVALID_OWNER)
        .value("TRANSFER_FAILED", nft::NFTErrorCode::TRANSFER_FAILED)
        .value("BURN_FAILED", nft::NFTErrorCode::BURN_FAILED)
        .value("DUPLICATE_ID", nft::NFTErrorCode::DUPLICATE_ID)
        .value("INVALID_ID", nft::NFTErrorCode::INVALID_ID)
        .value("INVALID_AMOUNT", nft::NFTErrorCode::INVALID_AMOUNT)
        .value("INSUFFICIENT_BALANCE", nft::NFTErrorCode::INSUFFICIENT_BALANCE)
        .value("INVALID_OPERATION", nft::NFTErrorCode::INVALID_OPERATION)
        .value("STORAGE_ERROR", nft::NFTErrorCode::STORAGE_ERROR)
        .value("NETWORK_ERROR", nft::NFTErrorCode::NETWORK_ERROR)
        .value("VALIDATION_ERROR", nft::NFTErrorCode::VALIDATION_ERROR)
        .value("UNKNOWN_ERROR", nft::NFTErrorCode::UNKNOWN_ERROR)
        .value("NOT_INITIALIZED", nft::NFTErrorCode::NOT_INITIALIZED)
        .value("CACHE_ERROR", nft::NFTErrorCode::CACHE_ERROR)
        .value("CONFIG_ERROR", nft::NFTErrorCode::CONFIG_ERROR)
        .value("QUERY_ERROR", nft::NFTErrorCode::QUERY_ERROR)
        .value("CLEAR_ERROR", nft::NFTErrorCode::CLEAR_ERROR);

    py::enum_<nft::NFTManager::NFTEventType>(m, "NFTEventType")
        .value("CREATED", nft::NFTManager::NFTEventType::CREATED)
        .value("TRANSFERRED", nft::NFTManager::NFTEventType::TRANSFERRED)
        .value("BURNED", nft::NFTManager::NFTEventType::BURNED)
        .value("METADATA_UPDATED", nft::NFTManager::NFTEventType::METADATA_UPDATED)
        .value("TRANSFERABLE_CHANGED", nft::NFTManager::NFTEventType::TRANSFERABLE_CHANGED)
        .value("BURNABLE_CHANGED", nft::NFTManager::NFTEventType::BURNABLE_CHANGED)
        .value("ROYALTY_CHANGED", nft::NFTManager::NFTEventType::ROYALTY_CHANGED)
        .value("ATTRIBUTE_ADDED", nft::NFTManager::NFTEventType::ATTRIBUTE_ADDED)
        .value("ATTRIBUTE_REMOVED", nft::NFTManager::NFTEventType::ATTRIBUTE_REMOVED);

    // Module-level functions
    m.def("get_version", []() { return SATOX_SDK_VERSION; });
    m.def("get_sdk_instance", []() { return PySDK::getInstance(); }, py::return_value_policy::reference);
    m.def("get_core_manager", []() { return PyCoreManager::getInstance(); }, py::return_value_policy::reference);
    m.def("get_database_manager", []() { return PyDatabaseManager::getInstance(); }, py::return_value_policy::reference);
    m.def("get_security_manager", []() { return PySecurityManager::getInstance(); }, py::return_value_policy::reference);
    m.def("get_nft_manager", []() { return PyNFTManager::getInstance(); }, py::return_value_policy::reference);
} 