/*
 * MIT License
 * Copyright(c) 2025 Satoxcoin Core Developer
 */

#include "satox/database/database_manager.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cassert>

namespace satox::database {

// Version constant
const Version DatabaseManager::VERSION{0, 1, 0, "build", "commit"};

template<typename T> using atomic_t = std::atomic<T>;

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

void DatabaseManager::initializeLogging(const std::string& logPath) {
    // if (logger_) logger_->debug("DatabaseManager::initializeLogging() - ENTRY");
    try {
        if (logger_) {
            return; // Logger already exists
        }
        std::filesystem::create_directories(logPath);
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath + "/database.log", 1024*1024*10, 5);
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        logger_ = std::make_shared<spdlog::logger>("satox-database", spdlog::sinks_init_list{file_sink, console_sink});
        logger_->set_level(spdlog::level::debug);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [satox-database] %v");
        spdlog::register_logger(logger_);
        logger_->info("Database logging system initialized");
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize database logging: " << e.what() << std::endl;
    }
}

bool DatabaseManager::initialize(const DatabaseConfig& config) {
    auto start = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::initialize() - ENTRY");
    
    if (initialized_.load()) {
        // if (logger_) logger_->warn("Database manager already initialized");
        handleError("initialize", DatabaseErrorCode::ALREADY_INITIALIZED, "Database manager already initialized");
        // if (logger_) logger_->debug("DatabaseManager::initialize() - EXIT (already initialized)");
        return false;
    }
    
    try {
        config_ = config;
        if (config_.enableLogging) {
            initializeLogging(config_.logPath);
            // if (logger_) logger_->info("Initializing DatabaseManager with config: {}", config_.name);
        }
        
        if (!validateConfig(config_)) {
            // if (logger_) logger_->error("Invalid database configuration");
            handleError("initialize", DatabaseErrorCode::INVALID_ARGUMENT, "Invalid configuration");
            // if (logger_) logger_->debug("DatabaseManager::initialize() - EXIT (invalid config)");
            return false;
        }
        
        // Clear containers with logging
        // if (logger_) logger_->debug("Clearing databases_ container");
        databases_.clear();
        // if (logger_) logger_->debug("Clearing currentDatabase_");
        currentDatabase_.clear();
        
        stats_ = DatabaseStats{};
        stats_.lastOperation = std::chrono::system_clock::now();
        health_ = DatabaseHealth{};
        health_.status = DatabaseStatus::INITIALIZED;
        health_.healthy = true;
        health_.lastCheck = std::chrono::system_clock::now();
        
        // Set connection state to true after successful initialization
        isConnected_ = true;
        
        initialized_.store(true);
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start);
        // if (logger_) logger_->info("DatabaseManager initialized successfully in {}ms", duration.count());
        logOperation("initialize", true, "Database manager initialized successfully");
        invokeCallbacks("initialize", true, "");
        
        // if (logger_) logger_->debug("DatabaseManager::initialize() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to initialize DatabaseManager: {}", e.what());
        handleError("initialize", DatabaseErrorCode::INITIALIZATION_ERROR, e.what());
        // if (logger_) logger_->debug("DatabaseManager::initialize() - EXIT (exception)");
        return false;
    }
}

void DatabaseManager::shutdown() {
    std::cerr << "[DEBUG] DatabaseManager::shutdown() called" << std::endl;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    // Mark as not initialized first to prevent new operations
    initialized_.store(false);
    
    try {
        // Disconnect first if connected
        if (isConnected_) {
            try {
                disconnect();
            } catch (const std::exception& e) {
                // Ignore disconnect errors during shutdown
            }
        }
        
        // Clear callbacks before clearing other data
        try {
            databaseCallback_ = nullptr;
            connectionCallback_ = nullptr;
            healthCallback_ = nullptr;
        } catch (const std::exception& e) {
            // Ignore callback clearing errors during shutdown
        }
        
        // Clear all data structures safely
        try {
            currentDatabase_.clear();
            tableSchemas_.clear();
            tableIndexes_.clear();
            databases_.clear();
            transactionSnapshot_.clear();
        } catch (const std::exception& e) {
            // Ignore data structure clearing errors during shutdown
        }
        
        // Update stats without logging to avoid logger issues
        try {
            stats_.totalOperations++;
            stats_.successfulOperations++;
            stats_.lastOperation = std::chrono::system_clock::now();
        } catch (const std::exception& e) {
            // Ignore stats update errors during shutdown
        }
        
        // Safely drop all loggers first
        try {
            if (logger_) {
                logger_.reset();
            }
            spdlog::drop_all();
        } catch (const std::exception& e) {
            // Ignore logger cleanup errors
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error during DatabaseManager shutdown: " << e.what() << std::endl;
        // Ensure logger is reset even if there's an error
        try {
            logger_.reset();
            spdlog::drop_all();
        } catch (...) {
            // Ignore all errors during final cleanup
        }
    }
}

void DatabaseManager::invokeCallbacks(const std::string& operation, bool success, const std::string& error) {
    if (databaseCallback_) {
        try {
            databaseCallback_(operation, success, error);
        } catch (const std::exception& e) {
            // if (logger_) logger_->error("Database callback execution failed for operation '{}': {}", operation, e.what());
        }
    }
    
    if (connectionCallback_ && (operation == "connect" || operation == "disconnect" || operation == "reconnect")) {
        try {
            connectionCallback_(operation == "connect" || operation == "reconnect", error);
        } catch (const std::exception& e) {
            // if (logger_) logger_->error("Connection callback execution failed for operation '{}': {}", operation, e.what());
        }
    }
}

bool DatabaseManager::createDatabase(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::createDatabase() - ENTRY: {}", name);
    
    if (!initialized_.load()) {
        // if (logger_) logger_->debug("DatabaseManager::createDatabase() - EXIT (not initialized)");
        handleError("createDatabase", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    // Validate name
    if (!validateDatabaseName(name)) {
        // if (logger_) logger_->debug("DatabaseManager::createDatabase() - EXIT (invalid name)");
        handleError("createDatabase", DatabaseErrorCode::INVALID_ARGUMENT, "Invalid database name");
        return false;
    }
    try {
        if (databases_.find(name) != databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::createDatabase() - EXIT (already exists)");
            handleError("createDatabase", DatabaseErrorCode::OPERATION_FAILED, "Database already exists");
            return false;
        }
        DatabaseData db;
        db.name = name;
        databases_[name] = db;
        // if (logger_) logger_->info("Database '{}' created successfully", name);
        logOperation("createDatabase", true, "Database created: " + name);
        invokeCallbacks("createDatabase", true, "");
        // if (logger_) logger_->debug("DatabaseManager::createDatabase() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to create database '{}': {}", name, e.what());
        handleError("createDatabase", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::createDatabase() - EXIT (exception)");
        return false;
    }
}


// Missing method implementations
bool DatabaseManager::isInitialized() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return initialized_.load();
}

bool DatabaseManager::isConnected() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return isConnected_;
}

size_t DatabaseManager::getMaxConnections() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return maxConnections_;
}

size_t DatabaseManager::getConnectionTimeout() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return connectionTimeout_;
}

bool DatabaseManager::databaseExists(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::databaseExists() - ENTRY: {}", name);
    
    if (!initialized_.load()) {
        // if (logger_) logger_->debug("DatabaseManager::databaseExists() - EXIT (not initialized)");
        handleError("databaseExists", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    bool exists = databases_.find(name) != databases_.end();
    
    // if (logger_) logger_->debug("DatabaseManager::databaseExists() - EXIT: {}", exists);
    return exists;
}

std::vector<std::string> DatabaseManager::listDatabases() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::listDatabases() - ENTRY");
    
    if (!initialized_.load()) {
        // if (logger_) logger_->debug("DatabaseManager::listDatabases() - EXIT (not initialized)");
        handleError("listDatabases", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return {};
    }
    
    std::vector<std::string> databaseNames;
    for (const auto& [name, _] : databases_) {
        databaseNames.push_back(name);
    }
    
    // if (logger_) logger_->debug("DatabaseManager::listDatabases() - EXIT: {} databases", databaseNames.size());
    return databaseNames;
}

bool DatabaseManager::deleteDatabase(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::deleteDatabase() - ENTRY: {}", name);
    
    if (!initialized_.load()) {
        // if (logger_) logger_->debug("DatabaseManager::deleteDatabase() - EXIT (not initialized)");
        handleError("deleteDatabase", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    try {
        auto it = databases_.find(name);
        if (it == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::deleteDatabase() - EXIT (not found)");
            handleError("deleteDatabase", DatabaseErrorCode::OPERATION_FAILED, "Database not found");
            return false;
        }
        
        databases_.erase(it);
        
        // if (logger_) logger_->info("Database '{}' deleted successfully", name);
        logOperation("deleteDatabase", true, "Database deleted: " + name);
        invokeCallbacks("deleteDatabase", true, "");
        
        // if (logger_) logger_->debug("DatabaseManager::deleteDatabase() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to delete database '{}': {}", name, e.what());
        handleError("deleteDatabase", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::deleteDatabase() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::useDatabase(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::useDatabase() - ENTRY: {}", name);
    
    if (!initialized_.load()) {
        // if (logger_) logger_->debug("DatabaseManager::useDatabase() - EXIT (not initialized)");
        handleError("useDatabase", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    try {
        if (databases_.find(name) == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::useDatabase() - EXIT (not found)");
            handleError("useDatabase", DatabaseErrorCode::OPERATION_FAILED, "Database not found");
            return false;
        }
        
        currentDatabase_ = name;
        
        // if (logger_) logger_->info("Now using database '{}'", name);
        logOperation("useDatabase", true, "Using database: " + name);
        invokeCallbacks("useDatabase", true, "");
        
        // if (logger_) logger_->debug("DatabaseManager::useDatabase() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to use database '{}': {}", name, e.what());
        handleError("useDatabase", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::useDatabase() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::connect(const std::string& connectionString) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::connect() - ENTRY: {}", connectionString);
    
    if (!initialized_.load()) {
        // if (logger_) logger_->debug("DatabaseManager::connect() - EXIT (not initialized)");
        handleError("connect", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    try {
        connectionString_ = connectionString;
        isConnected_ = true;
        
        // if (logger_) logger_->info("Connected to database: {}", connectionString);
        logOperation("connect", true, "Connected to: " + connectionString);
        invokeCallbacks("connect", true, "");
        
        // if (logger_) logger_->debug("DatabaseManager::connect() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to connect: {}", e.what());
        handleError("connect", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::connect() - EXIT (exception)");
        return false;
    }
}

void DatabaseManager::disconnect() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::disconnect() - ENTRY");
    
    if (isConnected_) {
        isConnected_ = false;
        connectionString_.clear();
        
        // if (logger_) logger_->info("Disconnected from database");
        logOperation("disconnect", true, "Disconnected successfully");
        invokeCallbacks("disconnect", true, "");
    }
    
    // if (logger_) logger_->debug("DatabaseManager::disconnect() - EXIT");
}

// Table operations
bool DatabaseManager::tableExists(const std::string& tableName) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::tableExists() - ENTRY: {}", tableName);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::tableExists() - EXIT (not initialized or no current database)");
        handleError("tableExists", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    auto dbIt = databases_.find(currentDatabase_);
    if (dbIt == databases_.end()) {
        // if (logger_) logger_->debug("DatabaseManager::tableExists() - EXIT (current database not found)");
        return false;
    }
    
    bool exists = dbIt->second.tables.find(tableName) != dbIt->second.tables.end();
    
    // if (logger_) logger_->debug("DatabaseManager::tableExists() - EXIT: {}", exists);
    return exists;
}

std::vector<std::string> DatabaseManager::listTables() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::listTables() - ENTRY");
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::listTables() - EXIT (not initialized or no current database)");
        handleError("listTables", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return {};
    }
    
    auto dbIt = databases_.find(currentDatabase_);
    if (dbIt == databases_.end()) {
        // if (logger_) logger_->debug("DatabaseManager::listTables() - EXIT (current database not found)");
        return {};
    }
    
    std::vector<std::string> tableNames;
    for (const auto& [name, _] : dbIt->second.tables) {
        tableNames.push_back(name);
    }
    
    // if (logger_) logger_->debug("DatabaseManager::listTables() - EXIT: {} tables", tableNames.size());
    return tableNames;
}

bool DatabaseManager::deleteTable(const std::string& tableName) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::deleteTable() - ENTRY: {}", tableName);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::deleteTable() - EXIT (not initialized or no current database)");
        handleError("deleteTable", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::deleteTable() - EXIT (current database not found)");
            return false;
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            // if (logger_) logger_->debug("DatabaseManager::deleteTable() - EXIT (table not found)");
            handleError("deleteTable", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return false;
        }
        
        dbIt->second.tables.erase(tableIt);
        
        // if (logger_) logger_->info("Table '{}' deleted successfully from database '{}'", tableName, currentDatabase_);
        logOperation("deleteTable", true, "Table deleted: " + tableName);
        invokeCallbacks("deleteTable", true, "");
        
        // if (logger_) logger_->debug("DatabaseManager::deleteTable() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to delete table '{}': {}", tableName, e.what());
        handleError("deleteTable", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::deleteTable() - EXIT (exception)");
        return false;
    }
}

// Record operations
bool DatabaseManager::insert(const std::string& tableName, const nlohmann::json& data) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // if (logger_) logger_->debug("DatabaseManager::insert() - ENTRY: table={}, data_size={}", tableName, data.size());
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (not initialized or no current database)");
        handleError("insert", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    // Data size check: reject if too many fields or too large
    if (data.size() > 100 || data.dump().size() > 1024 * 1024) {
        // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (data too large)");
        handleError("insert", DatabaseErrorCode::INVALID_ARGUMENT, "Data too large");
        return false;
    }
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (current database not found)");
            return false;
        }
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (table not found)");
            handleError("insert", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return false;
        }
        
        // Validate data against table schema
        if (!validateData(tableName, data)) {
            // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (invalid data)");
            handleError("insert", DatabaseErrorCode::INVALID_ARGUMENT, "Data validation failed");
            return false;
        }
        
        nlohmann::json recordData = data;
        std::string id;
        if (data.contains("id")) {
            id = data["id"];
        } else {
            id = generateUniqueId();
            recordData["_id"] = id;
        }
        tableIt->second.records.push_back(recordData);
        // if (logger_) logger_->info("Record inserted successfully into table '{}' with ID '{}'", tableName, id);
        logOperation("insert", true, "Record inserted: " + id);
        invokeCallbacks("insert", true, id);
        // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to insert record: {}", e.what());
        handleError("insert", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::insert() - EXIT (exception)");
        return false;
    }
}

std::vector<nlohmann::json> DatabaseManager::query(const std::string& tableName, const nlohmann::json& query) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::query() - ENTRY: table={}", tableName);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::query() - EXIT (not initialized or no current database)");
        handleError("query", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return {};
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::query() - EXIT (current database not found)");
            return {};
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            // if (logger_) logger_->debug("DatabaseManager::query() - EXIT (table not found)");
            handleError("query", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return {};
        }
        
        std::vector<nlohmann::json> results;
        
        if (query.empty()) {
            // Return all records
            results = tableIt->second.records;
        } else {
            // Simple query matching (for demo purposes)
            for (const auto& record : tableIt->second.records) {
                bool matches = true;
                for (const auto& [key, value] : query.items()) {
                    if (!record.contains(key) || record[key] != value) {
                        matches = false;
                        break;
                    }
                }
                if (matches) {
                    results.push_back(record);
                }
            }
        }
        
        // if (logger_) logger_->info("Queried {} records from table '{}'", results.size(), tableName);
        logOperation("query", true, "Queried " + std::to_string(results.size()) + " records");
        invokeCallbacks("query", true, std::to_string(results.size()));
        
        // if (logger_) logger_->debug("DatabaseManager::query() - EXIT: {} records", results.size());
        return results;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to query records: {}", e.what());
        handleError("query", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::query() - EXIT (exception)");
        return {};
    }
}

bool DatabaseManager::update(const std::string& tableName, const std::string& id, const nlohmann::json& data) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::update() - ENTRY: table={}, id={}", tableName, id);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::update() - EXIT (not initialized or no current database)");
        handleError("update", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::update() - EXIT (current database not found)");
            return false;
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            // if (logger_) logger_->debug("DatabaseManager::update() - EXIT (table not found)");
            handleError("update", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return false;
        }
        
        // Find record by ID (check both "id" and "_id" fields)
        for (auto& record : tableIt->second.records) {
            if ((record.contains("id") && record["id"] == id) || 
                (record.contains("_id") && record["_id"] == id)) {
                // Update record
                for (const auto& [key, value] : data.items()) {
                    record[key] = value;
                }
                
                // if (logger_) logger_->info("Record '{}' updated successfully in table '{}'", id, tableName);
                logOperation("update", true, "Record updated: " + id);
                invokeCallbacks("update", true, id);
                
                // if (logger_) logger_->debug("DatabaseManager::update() - EXIT (success)");
                return true;
            }
        }
        
        // if (logger_) logger_->debug("DatabaseManager::update() - EXIT (record not found)");
        handleError("update", DatabaseErrorCode::OPERATION_FAILED, "Record not found");
        return false;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to update record: {}", e.what());
        handleError("update", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::update() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::remove(const std::string& tableName, const std::string& id) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::remove() - ENTRY: table={}, id={}", tableName, id);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::remove() - EXIT (not initialized or no current database)");
        handleError("remove", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::remove() - EXIT (current database not found)");
            return false;
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            // if (logger_) logger_->debug("DatabaseManager::remove() - EXIT (table not found)");
            handleError("remove", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return false;
        }
        
        // Find and remove record by ID (check both "id" and "_id" fields)
        auto& records = tableIt->second.records;
        for (auto it = records.begin(); it != records.end(); ++it) {
            if ((it->contains("id") && (*it)["id"] == id) || 
                (it->contains("_id") && (*it)["_id"] == id)) {
                records.erase(it);
                
                // if (logger_) logger_->info("Record '{}' removed successfully from table '{}'", id, tableName);
                logOperation("remove", true, "Record removed: " + id);
                invokeCallbacks("remove", true, id);
                
                // if (logger_) logger_->debug("DatabaseManager::remove() - EXIT (success)");
                return true;
            }
        }
        
        // if (logger_) logger_->debug("DatabaseManager::remove() - EXIT (record not found)");
        handleError("remove", DatabaseErrorCode::OPERATION_FAILED, "Record not found");
        return false;
    } catch (const std::exception& e) {
        // if (logger_) logger_->error("Failed to remove record: {}", e.what());
        handleError("remove", DatabaseErrorCode::OPERATION_FAILED, e.what());
        // if (logger_) logger_->debug("DatabaseManager::remove() - EXIT (exception)");
        return false;
    }
}

nlohmann::json DatabaseManager::find(const std::string& tableName, const std::string& id) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // if (logger_) logger_->debug("DatabaseManager::find() - ENTRY: table={}, id={}", tableName, id);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        // if (logger_) logger_->debug("DatabaseManager::find() - EXIT (not initialized or no current database)");
        handleError("find", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return nlohmann::json();
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            // if (logger_) logger_->debug("DatabaseManager::find() - EXIT (current database not found)");
            return nlohmann::json();
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            // if (logger_) logger_->debug("DatabaseManager::find() - EXIT (table not found)");
            handleError("find", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return nlohmann::json();
        }
        
        // Find record by ID (check both "id" and "_id" fields)
        for (const auto& record : tableIt->second.records) {
            if ((record.contains("id") && record["id"] == id) || 
                (record.contains("_id") && record["_id"] == id)) {
                // if (logger_) logger_->info("Record '{}' found successfully in table '{}'", id, tableName);
                logOperation("find", true, "Record found: " + id);
                invokeCallbacks("find", true, id);
                
                // if (logger_) logger_->debug("DatabaseManager::find() - EXIT (success)");
                return record;
            }
        }
        
        if (logger_) logger_->debug("DatabaseManager::find() - EXIT (record not found)");
        handleError("find", DatabaseErrorCode::OPERATION_FAILED, "Record not found");
        return nlohmann::json();
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to find record: {}", e.what());
        handleError("find", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::find() - EXIT (exception)");
        return nlohmann::json();
    }
}

// Transaction operations
bool DatabaseManager::beginTransaction() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::beginTransaction() - ENTRY");
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        if (logger_) logger_->debug("DatabaseManager::beginTransaction() - EXIT (not initialized or no current database)");
        handleError("beginTransaction", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    if (inTransaction_) {
        if (logger_) logger_->debug("DatabaseManager::beginTransaction() - EXIT (already in transaction)");
        handleError("beginTransaction", DatabaseErrorCode::OPERATION_FAILED, "Already in transaction");
        return false;
    }
    
    try {
        // Create snapshot of current database state
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt != databases_.end()) {
            transactionSnapshot_[currentDatabase_] = dbIt->second;
        }
        
        inTransaction_ = true;
        
        if (logger_) logger_->info("Transaction begun successfully");
        logOperation("beginTransaction", true, "Transaction started");
        invokeCallbacks("beginTransaction", true, "");
        
        if (logger_) logger_->debug("DatabaseManager::beginTransaction() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to begin transaction: {}", e.what());
        handleError("beginTransaction", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::beginTransaction() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::commitTransaction() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::commitTransaction() - ENTRY");
    
    if (!inTransaction_) {
        if (logger_) logger_->debug("DatabaseManager::commitTransaction() - EXIT (not in transaction)");
        handleError("commitTransaction", DatabaseErrorCode::OPERATION_FAILED, "Not in transaction");
        return false;
    }
    
    try {
        // Transaction is already committed (changes are applied immediately in this implementation)
        inTransaction_ = false;
        transactionSnapshot_.clear();
        
        if (logger_) logger_->info("Transaction committed successfully");
        logOperation("commitTransaction", true, "Transaction committed");
        invokeCallbacks("commitTransaction", true, "");
        
        if (logger_) logger_->debug("DatabaseManager::commitTransaction() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to commit transaction: {}", e.what());
        handleError("commitTransaction", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::commitTransaction() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::rollbackTransaction() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::rollbackTransaction() - ENTRY");
    
    if (!inTransaction_) {
        if (logger_) logger_->debug("DatabaseManager::rollbackTransaction() - EXIT (not in transaction)");
        handleError("rollbackTransaction", DatabaseErrorCode::OPERATION_FAILED, "Not in transaction");
        return false;
    }
    
    try {
        // Restore database state from snapshot
        if (!currentDatabase_.empty()) {
            auto snapshotIt = transactionSnapshot_.find(currentDatabase_);
            if (snapshotIt != transactionSnapshot_.end()) {
                databases_[currentDatabase_] = snapshotIt->second;
            }
        }
        
        inTransaction_ = false;
        transactionSnapshot_.clear();
        
        if (logger_) logger_->info("Transaction rolled back successfully");
        logOperation("rollbackTransaction", true, "Transaction rolled back");
        invokeCallbacks("rollbackTransaction", true, "");
        
        if (logger_) logger_->debug("DatabaseManager::rollbackTransaction() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to rollback transaction: {}", e.what());
        handleError("rollbackTransaction", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::rollbackTransaction() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::isInTransaction() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return inTransaction_;
}

// Index operations
bool DatabaseManager::createIndex(const std::string& tableName, const std::string& columnName) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::createIndex() - ENTRY: table={}, column={}", tableName, columnName);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        if (logger_) logger_->debug("DatabaseManager::createIndex() - EXIT (not initialized or no current database)");
        handleError("createIndex", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            if (logger_) logger_->debug("DatabaseManager::createIndex() - EXIT (current database not found)");
            return false;
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            if (logger_) logger_->debug("DatabaseManager::createIndex() - EXIT (table not found)");
            handleError("createIndex", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return false;
        }
        
        // Add index to table
        tableIt->second.indexes[columnName] = std::vector<std::string>();
        
        if (logger_) logger_->info("Index created successfully on column '{}' in table '{}'", columnName, tableName);
        logOperation("createIndex", true, "Index created: " + columnName);
        invokeCallbacks("createIndex", true, columnName);
        
        if (logger_) logger_->debug("DatabaseManager::createIndex() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to create index: {}", e.what());
        handleError("createIndex", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::createIndex() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::dropIndex(const std::string& tableName, const std::string& columnName) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::dropIndex() - ENTRY: table={}, column={}", tableName, columnName);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        if (logger_) logger_->debug("DatabaseManager::dropIndex() - EXIT (not initialized or no current database)");
        handleError("dropIndex", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            if (logger_) logger_->debug("DatabaseManager::dropIndex() - EXIT (current database not found)");
            return false;
        }
        
        auto tableIt = dbIt->second.tables.find(tableName);
        if (tableIt == dbIt->second.tables.end()) {
            if (logger_) logger_->debug("DatabaseManager::dropIndex() - EXIT (table not found)");
            handleError("dropIndex", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
            return false;
        }
        
        auto indexIt = tableIt->second.indexes.find(columnName);
        if (indexIt == tableIt->second.indexes.end()) {
            if (logger_) logger_->debug("DatabaseManager::dropIndex() - EXIT (index not found)");
            handleError("dropIndex", DatabaseErrorCode::OPERATION_FAILED, "Index not found");
            return false;
        }
        
        tableIt->second.indexes.erase(indexIt);
        
        if (logger_) logger_->info("Index dropped successfully from column '{}' in table '{}'", columnName, tableName);
        logOperation("dropIndex", true, "Index dropped: " + columnName);
        invokeCallbacks("dropIndex", true, columnName);
        
        if (logger_) logger_->debug("DatabaseManager::dropIndex() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to drop index: {}", e.what());
        handleError("dropIndex", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::dropIndex() - EXIT (exception)");
        return false;
    }
}

std::vector<std::string> DatabaseManager::listIndexes(const std::string& tableName) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::listIndexes() - ENTRY: table={}", tableName);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        if (logger_) logger_->debug("DatabaseManager::listIndexes() - EXIT (not initialized or no current database)");
        handleError("listIndexes", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return {};
    }
    
    auto dbIt = databases_.find(currentDatabase_);
    if (dbIt == databases_.end()) {
        if (logger_) logger_->debug("DatabaseManager::listIndexes() - EXIT (current database not found)");
        return {};
    }
    
    auto tableIt = dbIt->second.tables.find(tableName);
    if (tableIt == dbIt->second.tables.end()) {
        if (logger_) logger_->debug("DatabaseManager::listIndexes() - EXIT (table not found)");
        handleError("listIndexes", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
        return {};
    }
    
    std::vector<std::string> indexes;
    for (const auto& [name, _] : tableIt->second.indexes) {
        indexes.push_back(name);
    }
    
    if (logger_) logger_->debug("DatabaseManager::listIndexes() - EXIT: {} indexes", indexes.size());
    return indexes;
}

// Backup operations
bool DatabaseManager::createBackup(const std::string& backupPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::createBackup() - ENTRY: path={}", backupPath);
    
    if (!initialized_.load()) {
        if (logger_) logger_->debug("DatabaseManager::createBackup() - EXIT (not initialized)");
        handleError("createBackup", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    try {
        // Create backup data
        nlohmann::json backupData;
        backupData["databases"] = nlohmann::json::object();
        
        for (const auto& [name, db] : databases_) {
            backupData["databases"][name] = nlohmann::json::object();
            backupData["databases"][name]["tables"] = nlohmann::json::object();
            
            for (const auto& [tableName, table] : db.tables) {
                backupData["databases"][name]["tables"][tableName] = nlohmann::json::object();
                backupData["databases"][name]["tables"][tableName]["records"] = table.records;
                backupData["databases"][name]["tables"][tableName]["indexes"] = table.indexes;
                backupData["databases"][name]["tables"][tableName]["schema"] = table.schema;
            }
        }
        
        // Write backup to file
        std::ofstream file(backupPath);
        if (!file.is_open()) {
            if (logger_) logger_->debug("DatabaseManager::createBackup() - EXIT (cannot open file)");
            handleError("createBackup", DatabaseErrorCode::OPERATION_FAILED, "Cannot open backup file");
            return false;
        }
        
        file << backupData.dump(2);
        file.close();
        
        if (logger_) logger_->info("Backup created successfully at '{}'", backupPath);
        logOperation("createBackup", true, "Backup created: " + backupPath);
        invokeCallbacks("createBackup", true, backupPath);
        
        if (logger_) logger_->debug("DatabaseManager::createBackup() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to create backup: {}", e.what());
        handleError("createBackup", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::createBackup() - EXIT (exception)");
        return false;
    }
}

bool DatabaseManager::restoreFromBackup(const std::string& backupPath) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::restoreFromBackup() - ENTRY: path={}", backupPath);
    
    if (!initialized_.load()) {
        if (logger_) logger_->debug("DatabaseManager::restoreFromBackup() - EXIT (not initialized)");
        handleError("restoreFromBackup", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    try {
        // Read backup from file
        std::ifstream file(backupPath);
        if (!file.is_open()) {
            if (logger_) logger_->debug("DatabaseManager::restoreFromBackup() - EXIT (cannot open file)");
            handleError("restoreFromBackup", DatabaseErrorCode::OPERATION_FAILED, "Cannot open backup file");
            return false;
        }
        
        nlohmann::json backupData;
        file >> backupData;
        file.close();
        
        // Clear current databases
        databases_.clear();
        
        // Restore databases from backup
        if (backupData.contains("databases")) {
            for (const auto& [name, dbData] : backupData["databases"].items()) {
                DatabaseData db;
                db.name = name;
                
                if (dbData.contains("tables")) {
                    for (const auto& [tableName, tableData] : dbData["tables"].items()) {
                        TableData table;
                        
                        if (tableData.contains("records")) {
                            table.records = tableData["records"].get<std::vector<nlohmann::json>>();
                        }
                        
                        if (tableData.contains("indexes")) {
                            table.indexes = tableData["indexes"].get<std::map<std::string, std::vector<std::string>>>();
                        }
                        
                        if (tableData.contains("schema")) {
                            table.schema = tableData["schema"];
                        }
                        
                        db.tables[tableName] = table;
                    }
                }
                
                databases_[name] = db;
            }
        }
        
        if (logger_) logger_->info("Backup restored successfully from '{}'", backupPath);
        logOperation("restoreFromBackup", true, "Backup restored: " + backupPath);
        invokeCallbacks("restoreFromBackup", true, backupPath);
        
        if (logger_) logger_->debug("DatabaseManager::restoreFromBackup() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to restore backup: {}", e.what());
        handleError("restoreFromBackup", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::restoreFromBackup() - EXIT (exception)");
        return false;
    }
}

// Callback management
void DatabaseManager::setDatabaseCallback(DatabaseCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::setDatabaseCallback() - ENTRY");
    
    databaseCallback_ = callback;
    
    if (logger_) logger_->info("Database callback set successfully");
    if (logger_) logger_->debug("DatabaseManager::setDatabaseCallback() - EXIT");
}

void DatabaseManager::setConnectionCallback(ConnectionCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::setConnectionCallback() - ENTRY");
    
    connectionCallback_ = callback;
    
    if (logger_) logger_->info("Connection callback set successfully");
    if (logger_) logger_->debug("DatabaseManager::setConnectionCallback() - EXIT");
}

void DatabaseManager::setHealthCallback(HealthCallback callback) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::setHealthCallback() - ENTRY");
    
    healthCallback_ = callback;
    
    if (logger_) logger_->info("Health callback set successfully");
    if (logger_) logger_->debug("DatabaseManager::setHealthCallback() - EXIT");
}

void DatabaseManager::clearCallbacks() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    // Don't use logger during shutdown to prevent segfaults
    if (logger_ && initialized_.load()) {
        logger_->debug("DatabaseManager::clearCallbacks() - ENTRY");
    }
    
    databaseCallback_ = nullptr;
    connectionCallback_ = nullptr;
    healthCallback_ = nullptr;
    
    if (logger_ && initialized_.load()) {
        logger_->info("All callbacks cleared successfully");
        logger_->debug("DatabaseManager::clearCallbacks() - EXIT");
    }
}

// Statistics and monitoring
DatabaseStats DatabaseManager::getStats() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::getStats() - ENTRY");
    
    if (logger_) logger_->debug("DatabaseManager::getStats() - EXIT");
    return stats_;
}

void DatabaseManager::resetStats() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::resetStats() - ENTRY");
    
    stats_ = DatabaseStats{};
    
    if (logger_) logger_->info("Statistics reset successfully");
    if (logger_) logger_->debug("DatabaseManager::resetStats() - EXIT");
}

DatabaseHealth DatabaseManager::getHealth() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::getHealth() - ENTRY");
    
    if (logger_) logger_->debug("DatabaseManager::getHealth() - EXIT");
    return health_;
}

bool DatabaseManager::isHealthy() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::isHealthy() - ENTRY");
    
    bool healthy = initialized_.load() && health_.healthy;
    
    if (logger_) logger_->debug("DatabaseManager::isHealthy() - EXIT: {}", healthy);
    return healthy;
}

bool DatabaseManager::performHealthCheck() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::performHealthCheck() - ENTRY");
    
    bool healthy = initialized_.load() && !hasRecentErrors();
    
    health_.healthy = healthy;
    health_.status = healthy ? DatabaseStatus::RUNNING : DatabaseStatus::ERROR;
    health_.lastCheck = std::chrono::system_clock::now();
    health_.message = healthy ? "Database is healthy" : "Database has errors";
    
    if (healthCallback_) {
        try {
            healthCallback_(health_);
        } catch (const std::exception& e) {
            if (logger_) logger_->error("Health callback execution failed: {}", e.what());
        }
    }
    
    if (logger_) logger_->info("Health check completed: {}", healthy ? "healthy" : "unhealthy");
    if (logger_) logger_->debug("DatabaseManager::performHealthCheck() - EXIT: {}", healthy);
    return healthy;
}

// Error handling
DatabaseErrorCode DatabaseManager::getLastErrorCode() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return lastErrorCode_;
}

std::string DatabaseManager::getLastError() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return lastError_;
}

void DatabaseManager::clearLastError() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::clearLastError() - ENTRY");
    
    lastErrorCode_ = DatabaseErrorCode::SUCCESS;
    lastError_.clear();
    
    if (logger_) logger_->debug("DatabaseManager::clearLastError() - EXIT");
}

// Version information
Version DatabaseManager::getVersion() const {
    return VERSION;
}

// Configuration methods
void DatabaseManager::setConfig(const DatabaseConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::setConfig() - ENTRY");
    
    config_ = config;
    maxConnections_ = config.maxConnections;
    connectionTimeout_ = config.connectionTimeout;
    
    if (logger_) logger_->info("Configuration updated successfully");
    if (logger_) logger_->debug("DatabaseManager::setConfig() - EXIT");
}

DatabaseConfig DatabaseManager::getConfig() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return config_;
}

bool DatabaseManager::validateConfig(const DatabaseConfig& config) const {
    if (logger_) logger_->debug("DatabaseManager::validateConfig() - ENTRY");
    
    bool valid = !config.name.empty() && 
                 config.maxConnections > 0 && 
                 config.connectionTimeout > 0;
    
    if (logger_) logger_->debug("DatabaseManager::validateConfig() - EXIT: {}", valid);
    return valid;
}

// Connection management
bool DatabaseManager::reconnect() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::reconnect() - ENTRY");
    
    if (!initialized_.load()) {
        if (logger_) logger_->debug("DatabaseManager::reconnect() - EXIT (not initialized)");
        handleError("reconnect", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized");
        return false;
    }
    
    try {
        disconnect();
        bool success = connect(connectionString_);
        
        if (logger_) logger_->info("Reconnection attempt: {}", success ? "successful" : "failed");
        if (logger_) logger_->debug("DatabaseManager::reconnect() - EXIT: {}", success);
        return success;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to reconnect: {}", e.what());
        handleError("reconnect", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::reconnect() - EXIT (exception)");
        return false;
    }
}

void DatabaseManager::setMaxConnections(size_t max) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::setMaxConnections() - ENTRY: {}", max);
    
    maxConnections_ = max;
    
    if (logger_) logger_->info("Max connections set to {}", max);
    if (logger_) logger_->debug("DatabaseManager::setMaxConnections() - EXIT");
}

void DatabaseManager::setConnectionTimeout(size_t milliseconds) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::setConnectionTimeout() - ENTRY: {}", milliseconds);
    
    connectionTimeout_ = milliseconds;
    
    if (logger_) logger_->info("Connection timeout set to {}ms", milliseconds);
    if (logger_) logger_->debug("DatabaseManager::setConnectionTimeout() - EXIT");
}

// Table operations
bool DatabaseManager::createTable(const std::string& name, const nlohmann::json& schema) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (logger_) logger_->debug("DatabaseManager::createTable() - ENTRY: table={}", name);
    if (!initialized_.load() || currentDatabase_.empty()) {
        if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (not initialized or no current database)");
        handleError("createTable", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return false;
    }
    // Validate table name and schema
    if (!validateTableName(name)) {
        if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (invalid table name)");
        handleError("createTable", DatabaseErrorCode::INVALID_ARGUMENT, "Invalid table name");
        return false;
    }
    if (!validateSchema(schema)) {
        if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (invalid schema)");
        handleError("createTable", DatabaseErrorCode::INVALID_ARGUMENT, "Invalid table schema");
        return false;
    }
    try {
        auto dbIt = databases_.find(currentDatabase_);
        if (dbIt == databases_.end()) {
            if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (current database not found)");
            return false;
        }
        if (dbIt->second.tables.find(name) != dbIt->second.tables.end()) {
            if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (table already exists)");
            handleError("createTable", DatabaseErrorCode::OPERATION_FAILED, "Table already exists");
            return false;
        }
        TableData table;
        table.schema = schema;
        dbIt->second.tables[name] = table;
        if (logger_) logger_->info("Table '{}' created successfully in database '{}'", name, currentDatabase_);
        logOperation("createTable", true, "Table created: " + name);
        invokeCallbacks("createTable", true, name);
        if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (success)");
        return true;
    } catch (const std::exception& e) {
        if (logger_) logger_->error("Failed to create table '{}': {}", name, e.what());
        handleError("createTable", DatabaseErrorCode::OPERATION_FAILED, e.what());
        if (logger_) logger_->debug("DatabaseManager::createTable() - EXIT (exception)");
        return false;
    }
}

nlohmann::json DatabaseManager::getTableSchema(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::getTableSchema() - ENTRY: table={}", name);
    
    if (!initialized_.load() || currentDatabase_.empty()) {
        if (logger_) logger_->debug("DatabaseManager::getTableSchema() - EXIT (not initialized or no current database)");
        handleError("getTableSchema", DatabaseErrorCode::NOT_INITIALIZED, "Database manager not initialized or no current database");
        return nlohmann::json();
    }
    
    auto dbIt = databases_.find(currentDatabase_);
    if (dbIt == databases_.end()) {
        if (logger_) logger_->debug("DatabaseManager::getTableSchema() - EXIT (current database not found)");
        return nlohmann::json();
    }
    
    auto tableIt = dbIt->second.tables.find(name);
    if (tableIt == dbIt->second.tables.end()) {
        if (logger_) logger_->debug("DatabaseManager::getTableSchema() - EXIT (table not found)");
        handleError("getTableSchema", DatabaseErrorCode::OPERATION_FAILED, "Table not found");
        return nlohmann::json();
    }
    
    if (logger_) logger_->debug("DatabaseManager::getTableSchema() - EXIT (success)");
    return tableIt->second.schema;
}

// Utility methods
std::string DatabaseManager::generateUniqueId() {
    static std::atomic<uint64_t> counter{0};
    uint64_t id = counter.fetch_add(1);
    return "id_" + std::to_string(id);
}

bool DatabaseManager::hasRecentErrors() const {
    // Check if there have been errors in the last 100 operations
    return stats_.failedOperations > 0 && (stats_.totalOperations - stats_.failedOperations) < 100;
}

void DatabaseManager::logOperation(const std::string& operation, bool success, const std::string& details) {
    try {
        stats_.totalOperations++;
        if (success) {
            stats_.successfulOperations++;
        } else {
            stats_.failedOperations++;
        }
        stats_.lastOperation = std::chrono::system_clock::now();
        
        if (logger_ && initialized_.load()) {
            try {
                if (success) {
                    logger_->info("Operation '{}' completed successfully: {}", operation, details);
                } else {
                    logger_->error("Operation '{}' failed: {}", operation, details);
                }
            } catch (const std::exception& e) {
                // Ignore logging errors - logger might be in invalid state
            }
        }
    } catch (const std::exception& e) {
        // Ignore all errors in logOperation to prevent crashes
    }
}

void DatabaseManager::handleError(const std::string& operation, DatabaseErrorCode code, const std::string& message) {
    try {
        if (logger_ && initialized_.load()) {
            try {
                logger_->error("Database error in '{}': {} - {}", operation, static_cast<int>(code), message);
            } catch (const std::exception& e) {
                // Ignore logging errors - logger might be in invalid state
            }
        }
        
        lastErrorCode_ = code;
        lastError_ = message;
        
        logOperation(operation, false, message);
        invokeCallbacks(operation, false, message);
    } catch (const std::exception& e) {
        // Ignore all errors in handleError to prevent crashes
    }
}

// Helper methods
bool DatabaseManager::validateDatabaseName(const std::string& name) const {
    if (logger_) logger_->debug("DatabaseManager::validateDatabaseName() - ENTRY: {}", name);
    
    bool valid = !name.empty() && name.length() <= 64 && 
                 name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == std::string::npos;
    
    if (logger_) logger_->debug("DatabaseManager::validateDatabaseName() - EXIT: {}", valid);
    return valid;
}

bool DatabaseManager::validateTableName(const std::string& name) const {
    if (logger_) logger_->debug("DatabaseManager::validateTableName() - ENTRY: {}", name);
    
    bool valid = !name.empty() && name.length() <= 64 && 
                 name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == std::string::npos;
    
    if (logger_) logger_->debug("DatabaseManager::validateTableName() - EXIT: {}", valid);
    return valid;
}

bool DatabaseManager::validateSchema(const nlohmann::json& schema) const {
    if (logger_) logger_->debug("DatabaseManager::validateSchema() - ENTRY");
    // Schema must be an object and have a non-empty 'fields' object
    bool valid = schema.is_object() &&
                 schema.contains("fields") && schema["fields"].is_object() && !schema["fields"].empty();
    if (logger_) logger_->debug("DatabaseManager::validateSchema() - EXIT: {}", valid);
    return valid;
}

bool DatabaseManager::validateData(const std::string& table, const nlohmann::json& data) const {
    if (logger_) logger_->debug("DatabaseManager::validateData() - ENTRY: table={}", table);
    
    // Basic validation: data must be an object
    if (!data.is_object()) {
        if (logger_) logger_->debug("DatabaseManager::validateData() - EXIT: data is not an object");
        return false;
    }
    
    // Get the table schema
    auto dbIt = databases_.find(currentDatabase_);
    if (dbIt == databases_.end()) {
        if (logger_) logger_->debug("DatabaseManager::validateData() - EXIT: current database not found");
        return false;
    }
    
    auto tableIt = dbIt->second.tables.find(table);
    if (tableIt == dbIt->second.tables.end()) {
        if (logger_) logger_->debug("DatabaseManager::validateData() - EXIT: table not found");
        return false;
    }
    
    const auto& schema = tableIt->second.schema;
    const auto& fields = schema["fields"];
    
    // Check required fields
    if (schema.contains("required") && schema["required"].is_array()) {
        for (const auto& requiredField : schema["required"]) {
            if (!data.contains(requiredField.get<std::string>())) {
                if (logger_) logger_->debug("DatabaseManager::validateData() - EXIT: missing required field: {}", requiredField.get<std::string>());
                return false;
            }
        }
    }
    
    // Check that all data fields exist in schema (optional validation)
    for (const auto& [fieldName, fieldValue] : data.items()) {
        if (!fields.contains(fieldName)) {
            if (logger_) logger_->debug("DatabaseManager::validateData() - EXIT: field not in schema: {}", fieldName);
            return false;
        }
    }
    
    if (logger_) logger_->debug("DatabaseManager::validateData() - EXIT: valid");
    return true;
}

void DatabaseManager::updateStats(bool success, std::chrono::milliseconds duration) {
    stats_.totalOperations++;
    if (success) {
        stats_.successfulOperations++;
    } else {
        stats_.failedOperations++;
    }
    stats_.lastOperation = std::chrono::system_clock::now();
    
    // Update average operation time
    if (stats_.totalOperations > 1) {
        stats_.averageOperationTime = (stats_.averageOperationTime * (stats_.totalOperations - 1) + duration.count()) / stats_.totalOperations;
    } else {
        stats_.averageOperationTime = duration.count();
    }
}

nlohmann::json DatabaseManager::getHealthStatus() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    
    if (logger_) logger_->debug("DatabaseManager::getHealthStatus() - ENTRY");
    
    nlohmann::json status;
    status["healthy"] = isHealthy();
    status["initialized"] = initialized_.load();
    status["connected"] = isConnected_;
    status["current_database"] = currentDatabase_;
    status["database_count"] = databases_.size();
    status["max_connections"] = maxConnections_;
    status["connection_timeout"] = connectionTimeout_;
    status["last_error"] = lastError_;
    status["last_error_code"] = static_cast<int>(lastErrorCode_);
    
    // Add health details
    status["health"]["status"] = static_cast<int>(health_.status);
    status["health"]["message"] = health_.message;
    status["health"]["last_check"] = std::chrono::duration_cast<std::chrono::seconds>(
        health_.lastCheck.time_since_epoch()).count();
    
    // Add statistics
    status["stats"]["total_operations"] = stats_.totalOperations;
    status["stats"]["successful_operations"] = stats_.successfulOperations;
    status["stats"]["failed_operations"] = stats_.failedOperations;
    status["stats"]["average_operation_time"] = stats_.averageOperationTime;
    status["stats"]["last_operation"] = std::chrono::duration_cast<std::chrono::seconds>(
        stats_.lastOperation.time_since_epoch()).count();
    
    if (logger_) logger_->debug("DatabaseManager::getHealthStatus() - EXIT");
    return status;
}

DatabaseManager::~DatabaseManager() {
    std::cerr << "[DEBUG] DatabaseManager destructor called" << std::endl;
    // Make sure shutdown is only called once and is safe
    if (initialized_.load()) {
        shutdown();
    }
}

} // namespace satox::database 