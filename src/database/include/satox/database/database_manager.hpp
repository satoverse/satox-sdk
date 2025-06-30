/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>
#include "types.hpp"
#include "error.hpp"
#include <atomic>
#include <spdlog/spdlog.h>

namespace satox::database {

// Forward declarations
struct DatabaseData;
struct TableData;

/**
 * @brief Database Manager - Manages database operations and connections
 * 
 * This class provides a singleton interface for database operations including:
 * - Database creation, deletion, and management
 * - Table operations (create, delete, query)
 * - Transaction management
 * - Connection pooling
 * - Backup and restore operations
 * - Health monitoring and statistics
 */
class DatabaseManager {
public:
    // Callback types
    using DatabaseCallback = std::function<void(const std::string& operation, bool success, const std::string& error)>;
    using ConnectionCallback = std::function<void(bool connected, const std::string& details)>;
    using HealthCallback = std::function<void(const DatabaseHealth& health)>;

    // Singleton instance
    static DatabaseManager& getInstance();

    // Initialization and cleanup
    bool initialize(const DatabaseConfig& config = DatabaseConfig{});
    void shutdown();
    bool isInitialized() const;

    // Configuration
    void setConfig(const DatabaseConfig& config);
    DatabaseConfig getConfig() const;
    bool validateConfig(const DatabaseConfig& config) const;

    // Database operations
    bool createDatabase(const std::string& name);
    bool deleteDatabase(const std::string& name);
    bool useDatabase(const std::string& name);
    std::vector<std::string> listDatabases();
    bool databaseExists(const std::string& name);

    // Table operations
    bool createTable(const std::string& name, const nlohmann::json& schema);
    bool deleteTable(const std::string& name);
    bool tableExists(const std::string& name);
    std::vector<std::string> listTables();
    nlohmann::json getTableSchema(const std::string& name);

    // Data operations
    bool insert(const std::string& table, const nlohmann::json& data);
    bool update(const std::string& table, const std::string& id, const nlohmann::json& data);
    bool remove(const std::string& table, const std::string& id);
    nlohmann::json find(const std::string& table, const std::string& id);
    std::vector<nlohmann::json> query(const std::string& table, const nlohmann::json& query);

    // Transaction operations
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isInTransaction() const;

    // Index operations
    bool createIndex(const std::string& table, const std::string& field);
    bool dropIndex(const std::string& table, const std::string& field);
    std::vector<std::string> listIndexes(const std::string& table);

    // Backup and restore
    bool createBackup(const std::string& backupPath);
    bool restoreFromBackup(const std::string& backupPath);

    // Connection management
    bool connect(const std::string& connectionString);
    void disconnect();
    bool isConnected() const;
    bool reconnect();

    // Configuration
    void setMaxConnections(size_t max);
    size_t getMaxConnections() const;
    void setConnectionTimeout(size_t milliseconds);
    size_t getConnectionTimeout() const;

    // Callback registration
    void setDatabaseCallback(DatabaseCallback callback);
    void setConnectionCallback(ConnectionCallback callback);
    void setHealthCallback(HealthCallback callback);
    void clearCallbacks();

    // Statistics and monitoring
    DatabaseStats getStats() const;
    void resetStats();
    DatabaseHealth getHealth() const;
    bool isHealthy() const;
    bool performHealthCheck();
    nlohmann::json getHealthStatus();

    // Error handling
    DatabaseErrorCode getLastErrorCode() const;
    std::string getLastError() const;
    void clearLastError();

    // Version information
    Version getVersion() const;

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Helper methods
    bool validateDatabaseName(const std::string& name) const;
    bool validateTableName(const std::string& name) const;
    bool validateSchema(const nlohmann::json& schema) const;
    bool validateData(const std::string& table, const nlohmann::json& data) const;
    void handleError(const std::string& operation, DatabaseErrorCode code, const std::string& error);
    void invokeCallbacks(const std::string& operation, bool success, const std::string& error);
    void updateStats(bool success, std::chrono::milliseconds duration);
    void logOperation(const std::string& operation, bool success, const std::string& error);
    std::string generateUniqueId();
    bool hasRecentErrors() const;

    // Member variables
    mutable std::recursive_mutex mutex_;
    std::atomic<bool> initialized_ = false;
    DatabaseConfig config_;
    std::string currentDatabase_;
    std::string connectionString_;
    bool isConnected_ = false;
    std::unordered_map<std::string, nlohmann::json> tableSchemas_;
    std::unordered_map<std::string, std::vector<std::string>> tableIndexes_;
    size_t maxConnections_ = 10;
    size_t connectionTimeout_ = 5000;  // 5 seconds
    bool inTransaction_ = false;
    
    // Database storage
    std::map<std::string, DatabaseData> databases_;
    std::map<std::string, DatabaseData> transactionSnapshot_;
    
    // Callbacks
    DatabaseCallback databaseCallback_;
    ConnectionCallback connectionCallback_;
    HealthCallback healthCallback_;
    
    // Statistics and monitoring
    mutable DatabaseStats stats_;
    mutable DatabaseHealth health_;
    mutable DatabaseErrorCode lastErrorCode_ = DatabaseErrorCode::SUCCESS;
    mutable std::string lastError_;
    
    // Logger (now a member, not static/global)
    std::shared_ptr<spdlog::logger> logger_;
    
    // Version
    static const Version VERSION;

    // Logger initialization
    void initializeLogging(const std::string& logPath);
};

} // namespace satox::database 