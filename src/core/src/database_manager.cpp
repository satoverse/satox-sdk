/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "satox/core/database_manager.hpp"
#include "satox/core/cloud/supabase_manager.hpp"
#include "satox/core/cloud/firebase_manager.hpp"
#include "satox/core/cloud/aws_config.hpp"
#include "satox/core/cloud/azure_config.hpp"
#include "satox/core/cloud/google_cloud_config.hpp"
#include "satox/core/cloud/supabase_config.hpp"
#include "satox/core/cloud/firebase_config.hpp"
#include "satox/core/cloud/aws_manager.hpp"
#include "satox/core/cloud/azure_manager.hpp"
#include "satox/core/cloud/google_cloud_manager.hpp"
#include <sqlite3.h>

// Conditional includes for optional database backends
#ifdef SATOX_ENABLE_MYSQL
#include <mysql/mysql.h>
#endif

#ifdef SATOX_ENABLE_POSTGRESQL
#include <libpq-fe.h>
#endif

#ifdef SATOX_ENABLE_REDIS
#include <hiredis/hiredis.h>
#endif

#ifdef SATOX_ENABLE_MONGODB
#include <mongoc/mongoc.h>
#endif

#ifdef SATOX_ENABLE_ROCKSDB
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/table.h>
#endif

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;
using satox::core::DatabaseType;

namespace satox::core {

// Forward declarations for database-specific structures
struct DatabaseConnection {
    DatabaseType type;
    json config;
    void* handle = nullptr;
    bool connected = false;
    std::chrono::system_clock::time_point lastActivity;
    int queryCount = 0;
    int errorCount = 0;
};

enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

// Database Manager implementation
class DatabaseManager::Impl {
private:

    // Cloud database managers
    std::unique_ptr<SupabaseManager> supabase_manager_;
    std::unique_ptr<FirebaseManager> firebase_manager_;
    std::mutex mutex_;
    bool initialized_ = false;
    std::map<std::string, DatabaseConnection> connections_;
    std::string lastError_;
    bool statsEnabled_ = false;
    std::vector<std::function<void(const std::string&, bool)>> callbacks_;

public:
    bool initialize(const DatabaseConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (initialized_) {
            setLastError("Database Manager already initialized");
            return false;
        }

        try {
            // Initialize database drivers
            sqlite3_initialize();
            // Initialize optional database drivers
#ifdef ENABLE_MONGODB
            // mongoc_init();
#endif
            
            initialized_ = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to initialize Database Manager: " + std::string(e.what()));
            return false;
        }
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) return;
        initialized_ = false;

        try {
            // Close all connections
            for (auto& [id, conn] : connections_) {
                disconnect(id);
            }
            connections_.clear();
            
            // Cleanup database drivers
            sqlite3_shutdown();
            // Cleanup optional database libraries
#ifdef ENABLE_MONGODB
            // mongoc_cleanup();
#endif
            
        } catch (const std::exception& e) {
            setLastError("Failed to shutdown Database Manager: " + std::string(e.what()));
        }
    }

    std::string connect(DatabaseType type, const json& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            setLastError("Database Manager not initialized");
            return "";
        }

        try {
            std::string id = generateConnectionId();
            DatabaseConnection conn;
            conn.type = type;
            conn.config = config;

            bool success = false;
            switch (type) {
                case DatabaseType::SQLITE:
                    success = connectSQLite(conn);
                    break;
#ifdef SATOX_ENABLE_POSTGRESQL
                case DatabaseType::POSTGRESQL:
                    success = connectPostgreSQL(conn);
                    break;
#endif
#ifdef SATOX_ENABLE_MYSQL
                case DatabaseType::MYSQL:
                    success = connectMySQL(conn);
                    break;
#endif
#ifdef SATOX_ENABLE_REDIS
                case DatabaseType::REDIS:
                    success = connectRedis(conn);
                    break;
#endif
#ifdef SATOX_ENABLE_MONGODB
                case DatabaseType::MONGODB:
                    success = connectMongoDB(conn);
                    break;
#endif
#ifdef SATOX_ENABLE_ROCKSDB
                case DatabaseType::ROCKSDB:
                    success = connectRocksDB(conn);
                    break;
#endif
                case DatabaseType::SUPABASE:
                    success = connectSupabase(conn);
                    break;
                case DatabaseType::FIREBASE:
                    success = connectFirebase(conn);
                    break;
                case DatabaseType::AWS: {
                    try {
                        // Parse query as JSON for AWS operations
                        json query_json;
                        try {
                            query_json = json::parse(query);
                        } catch (const std::exception& e) {
                            setLastError("Invalid AWS query format: " + std::string(e.what()));
                            return "";
                        }
                        
                        std::string service = query_json.value("service", "rds");
                        json params = query_json.value("params", json::object());
                        
                        result = satox::core::cloud::AWSManager::getInstance().executeQuery(service, params);
                        success = !result.contains("error");
                        
                        if (!success) {
                            setLastError("AWS query error: " + result.value("error", "Unknown error"));
                        }
                    } catch (const std::exception& e) {
                        setLastError("AWS query execution failed: " + std::string(e.what()));
                        success = false;
                    }
                    break;
                }
                case DatabaseType::AZURE: {
                    try {
                        // Parse query as JSON for Azure operations
                        json query_json;
                        try {
                            query_json = json::parse(query);
                        } catch (const std::exception& e) {
                            setLastError("Invalid Azure query format: " + std::string(e.what()));
                            return "";
                        }
                        
                        std::string service = query_json.value("service", "sql");
                        json params = query_json.value("params", json::object());
                        
                        result = satox::core::cloud::AzureManager::getInstance().executeQuery(service, params);
                        success = !result.contains("error");
                        
                        if (!success) {
                            setLastError("Azure query error: " + result.value("error", "Unknown error"));
                        }
                    } catch (const std::exception& e) {
                        setLastError("Azure query execution failed: " + std::string(e.what()));
                        success = false;
                    }
                    break;
                }
                case DatabaseType::GOOGLE_CLOUD: {
                    try {
                        // Parse query as JSON for Google Cloud operations
                        json query_json;
                        try {
                            query_json = json::parse(query);
                        } catch (const std::exception& e) {
                            setLastError("Invalid Google Cloud query format: " + std::string(e.what()));
                            return "";
                        }
                        
                        std::string service = query_json.value("service", "sql");
                        json params = query_json.value("params", json::object());
                        
                        result = satox::core::cloud::GoogleCloudManager::getInstance().executeQuery(service, params);
                        success = !result.contains("error");
                        
                        if (!success) {
                            setLastError("Google Cloud query error: " + result.value("error", "Unknown error"));
                        }
                    } catch (const std::exception& e) {
                        setLastError("Google Cloud query execution failed: " + std::string(e.what()));
                        success = false;
                    }
                    break;
                }
                default:
                    setLastError("Unsupported database type");
                    return "";
            }

            if (success) {
                connections_[id] = conn;
                return id;
            }
            return "";
        } catch (const std::exception& e) {
            setLastError("Failed to connect to database: " + std::string(e.what()));
            return "";
        }
    }

    bool disconnect(const std::string& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = connections_.find(id);
        if (it == connections_.end()) {
            setLastError("Connection not found");
            return false;
        }

        try {
            bool success = false;
            switch (it->second.type) {
                case DatabaseType::SQLITE:
                    success = disconnectSQLite(it->second);
                    break;
#ifdef SATOX_ENABLE_POSTGRESQL
                case DatabaseType::POSTGRESQL:
                    success = disconnectPostgreSQL(it->second);
                    break;
#endif
#ifdef SATOX_ENABLE_MYSQL
                case DatabaseType::MYSQL:
                    success = disconnectMySQL(it->second);
                    break;
#endif
#ifdef SATOX_ENABLE_REDIS
                case DatabaseType::REDIS:
                    success = disconnectRedis(it->second);
                    break;
#endif
#ifdef SATOX_ENABLE_MONGODB
                case DatabaseType::MONGODB:
                    success = disconnectMongoDB(it->second);
                    break;
#endif
#ifdef SATOX_ENABLE_ROCKSDB
                case DatabaseType::ROCKSDB:
                    success = disconnectRocksDB(it->second);
                    break;
#endif
                case DatabaseType::SUPABASE:
                    success = disconnectSupabase(it->second);
                    break;
                case DatabaseType::FIREBASE:
                    success = disconnectFirebase(it->second);
                    break;
                case DatabaseType::AWS:
                    satox::core::cloud::AWSManager::getInstance().disconnect();
                    success = true;
                    break;
                case DatabaseType::AZURE:
                    satox::core::cloud::AzureManager::getInstance().disconnect();
                    success = true;
                    break;
                case DatabaseType::GOOGLE_CLOUD:
                    satox::core::cloud::GoogleCloudManager::getInstance().disconnect();
                    success = true;
                    break;
                default:
                    setLastError("Unsupported database type");
                    return false;
            }

            if (success) {
                connections_.erase(it);
                return true;
            }
            return false;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from database: " + std::string(e.what()));
            return false;
        }
    }

    bool executeQuery(const std::string& id, const std::string& query, json& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = connections_.find(id);
        if (it == connections_.end()) {
            setLastError("Connection not found");
            return false;
        }

        try {
            bool success = false;
            switch (it->second.type) {
                case DatabaseType::SQLITE:
                    success = executeSQLiteQuery(it->second, query, result);
                    break;
#ifdef SATOX_ENABLE_POSTGRESQL
                case DatabaseType::POSTGRESQL:
                    success = executePostgreSQLQuery(it->second, query, result);
                    break;
#endif
#ifdef SATOX_ENABLE_MYSQL
                case DatabaseType::MYSQL:
                    success = executeMySQLQuery(it->second, query, result);
                    break;
#endif
#ifdef SATOX_ENABLE_REDIS
                case DatabaseType::REDIS:
                    success = executeRedisQuery(it->second, query, result);
                    break;
#endif
#ifdef SATOX_ENABLE_MONGODB
                case DatabaseType::MONGODB:
                    success = executeMongoDBQuery(it->second, query, result);
                    break;
#endif
#ifdef SATOX_ENABLE_ROCKSDB
                case DatabaseType::ROCKSDB:
                    success = executeRocksDBQuery(it->second, query, result);
                    break;
#endif
                case DatabaseType::SUPABASE:
                    success = executeSupabaseQuery(it->second, query, result);
                    break;
                case DatabaseType::FIREBASE:
                    success = executeFirebaseQuery(it->second, query, result);
                    break;
                case DatabaseType::AWS:
                    result = satox::core::cloud::AWSManager::getInstance().executeQuery("rds", query);
                    success = true; // TODO: Check actual result
                    break;
                case DatabaseType::AZURE:
                    result = satox::core::cloud::AzureManager::getInstance().executeQuery("sql", query);
                    success = true; // TODO: Check actual result
                    break;
                case DatabaseType::GOOGLE_CLOUD:
                    result = satox::core::cloud::GoogleCloudManager::getInstance().executeQuery("sql", query);
                    success = true; // TODO: Check actual result
                    break;
                default:
                    setLastError("Unsupported database type");
                    return false;
            }

            if (success) {
                updateStats(it->second, true);
                return true;
            }
            updateStats(it->second, false);
            return false;
        } catch (const std::exception& e) {
            setLastError("Failed to execute query: " + std::string(e.what()));
            updateStats(it->second, false);
            return false;
        }
    }

    bool executeTransaction(const std::string& id, const std::vector<std::string>& queries, json& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = connections_.find(id);
        if (it == connections_.end()) {
            setLastError("Connection not found");
            return false;
        }

        try {
            bool success = false;
            switch (it->second.type) {
                case DatabaseType::SQLITE:
                    success = executeSQLiteTransaction(it->second, queries, result);
                    break;
#ifdef SATOX_ENABLE_POSTGRESQL
                case DatabaseType::POSTGRESQL:
                    success = executePostgreSQLTransaction(it->second, queries, result);
                    break;
#endif
#ifdef SATOX_ENABLE_MYSQL
                case DatabaseType::MYSQL:
                    success = executeMySQLTransaction(it->second, queries, result);
                    break;
#endif
#ifdef SATOX_ENABLE_REDIS
                case DatabaseType::REDIS:
                    success = executeRedisTransaction(it->second, queries, result);
                    break;
#endif
#ifdef SATOX_ENABLE_MONGODB
                case DatabaseType::MONGODB:
                    success = executeMongoDBTransaction(it->second, queries, result);
                    break;
#endif
#ifdef SATOX_ENABLE_ROCKSDB
                case DatabaseType::ROCKSDB:
                    success = executeRocksDBTransaction(it->second, queries, result);
                    break;
#endif
                case DatabaseType::SUPABASE:
                    success = executeSupabaseTransaction(it->second, queries, result);
                    break;
                case DatabaseType::FIREBASE:
                    success = executeFirebaseTransaction(it->second, queries, result);
                    break;
                case DatabaseType::AWS:
                    result = satox::core::cloud::AWSManager::getInstance().executeTransaction("rds", queries);
                    success = true; // TODO: Check actual result
                    break;
                case DatabaseType::AZURE:
                    result = satox::core::cloud::AzureManager::getInstance().executeTransaction("sql", queries);
                    success = true; // TODO: Check actual result
                    break;
                case DatabaseType::GOOGLE_CLOUD:
                    result = satox::core::cloud::GoogleCloudManager::getInstance().executeTransaction("sql", queries);
                    success = true; // TODO: Check actual result
                    break;
                default:
                    setLastError("Unsupported database type");
                    return false;
            }

            if (success) {
                updateStats(it->second, true);
                return true;
            }
            updateStats(it->second, false);
            return false;
        } catch (const std::exception& e) {
            setLastError("Failed to execute transaction: " + std::string(e.what()));
            updateStats(it->second, false);
            return false;
        }
    }

    std::string getLastError() const {
        // Use a const_cast to access the mutex in a const method
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return lastError_;
    }

private:

    // Cloud database managers
    std::unique_ptr<SupabaseManager> supabase_manager_;
    std::unique_ptr<FirebaseManager> firebase_manager_;
    // Helper methods
    std::string generateConnectionId() {
        static int counter = 0;
        return "conn_" + std::to_string(++counter) + "_" + 
               std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }

    void setLastError(const std::string& error) {
        lastError_ = error;
        spdlog::error("DatabaseManager: {}", error);
    }

    void updateStats(DatabaseConnection& conn, bool success) {
        if (!statsEnabled_) return;
        
        conn.queryCount++;
        if (!success) {
            conn.errorCount++;
        }
        conn.lastActivity = std::chrono::system_clock::now();
    }

    // SQLite methods
    bool connectSQLite(DatabaseConnection& conn) {
        try {
            std::string dbPath = conn.config.value("path", ":memory:");
            sqlite3* db;
            int rc = sqlite3_open(dbPath.c_str(), &db);
            if (rc != SQLITE_OK) {
                setLastError("Failed to open SQLite database: " + std::string(sqlite3_errmsg(db)));
                sqlite3_close(db);
                return false;
            }
            conn.handle = db;
            conn.connected = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to SQLite: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectSQLite(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                sqlite3_close(static_cast<sqlite3*>(conn.handle));
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from SQLite: " + std::string(e.what()));
            return false;
        }
    }

    bool executeSQLiteQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            sqlite3* db = static_cast<sqlite3*>(conn.handle);
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                setLastError("Failed to prepare SQLite statement: " + std::string(sqlite3_errmsg(db)));
                return false;
            }

            json rows = json::array();
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                json row;
                int cols = sqlite3_column_count(stmt);
                for (int i = 0; i < cols; i++) {
                    std::string colName = sqlite3_column_name(stmt, i);
                    int type = sqlite3_column_type(stmt, i);
                    switch (type) {
                        case SQLITE_INTEGER:
                            row[colName] = sqlite3_column_int64(stmt, i);
                            break;
                        case SQLITE_FLOAT:
                            row[colName] = sqlite3_column_double(stmt, i);
                            break;
                        case SQLITE_TEXT:
                            row[colName] = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                            break;
                        case SQLITE_BLOB:
                            row[colName] = std::string(reinterpret_cast<const char*>(sqlite3_column_blob(stmt, i)));
                            break;
                        case SQLITE_NULL:
                            row[colName] = nullptr;
                            break;
                    }
                }
                rows.push_back(row);
            }

            sqlite3_finalize(stmt);
            result["rows"] = rows;
            result["success"] = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute SQLite query: " + std::string(e.what()));
            return false;
        }
    }

    bool executeSQLiteTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            sqlite3* db = static_cast<sqlite3*>(conn.handle);
            int rc = sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
            if (rc != SQLITE_OK) {
                setLastError("Failed to begin SQLite transaction");
                return false;
            }

            bool success = true;
            for (const auto& query : queries) {
                rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
                if (rc != SQLITE_OK) {
                    setLastError("Failed to execute SQLite transaction query: " + std::string(sqlite3_errmsg(db)));
                    success = false;
                    break;
                }
            }

            if (success) {
                rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
                if (rc != SQLITE_OK) {
                    setLastError("Failed to commit SQLite transaction");
                    return false;
                }
            } else {
                sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            }

            result["success"] = success;
            return success;
        } catch (const std::exception& e) {
            setLastError("Failed to execute SQLite transaction: " + std::string(e.what()));
            return false;
        }
    }

#ifdef SATOX_ENABLE_POSTGRESQL
    // PostgreSQL methods (full implementation)
    bool connectPostgreSQL(DatabaseConnection& conn) {
        try {
            std::string host = conn.config.value("host", "localhost");
            int port = conn.config.value("port", 5432);
            std::string database = conn.config.value("database", "");
            std::string username = conn.config.value("username", "");
            std::string password = conn.config.value("password", "");
            
            // Build connection string
            std::string connInfo = "host=" + host + 
                                  " port=" + std::to_string(port) + 
                                  " dbname=" + database + 
                                  " user=" + username + 
                                  " password=" + password;
            
            // Add SSL configuration if enabled
            if (conn.config.value("enable_ssl", false)) {
                connInfo += " sslmode=require";
            } else {
                connInfo += " sslmode=disable";
            }
            
            // Add connection timeout
            int timeout = conn.config.value("timeout", 30);
            connInfo += " connect_timeout=" + std::to_string(timeout);
            
            // Connect to PostgreSQL
            PGconn* pgconn = PQconnectdb(connInfo.c_str());
            if (PQstatus(pgconn) != CONNECTION_OK) {
                setLastError("Failed to connect to PostgreSQL: " + std::string(PQerrorMessage(pgconn)));
                PQfinish(pgconn);
                return false;
            }

            // Set client encoding
            if (PQsetClientEncoding(pgconn, "UTF8") != 0) {
                spdlog::warn("Failed to set PostgreSQL client encoding: {}", PQerrorMessage(pgconn));
            }

            // Set application name for monitoring
            std::string appName = conn.config.value("application_name", "satox-sdk");
            PQexec(pgconn, ("SET application_name = '" + appName + "'").c_str());

            conn.handle = pgconn;
            conn.connected = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to PostgreSQL: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectPostgreSQL(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                PQfinish(static_cast<PGconn*>(conn.handle));
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from PostgreSQL: " + std::string(e.what()));
            return false;
        }
    }

    bool executePostgreSQLQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            PGconn* pgconn = static_cast<PGconn*>(conn.handle);
            
            // Execute query
            PGresult* res = PQexec(pgconn, query.c_str());
            if (!res) {
                setLastError("Failed to execute PostgreSQL query: " + std::string(PQerrorMessage(pgconn)));
                return false;
            }

            ExecStatusType status = PQresultStatus(res);
            if (status == PGRES_TUPLES_OK) {
                // SELECT query - process results
                int rows = PQntuples(res);
                int cols = PQnfields(res);
                
                json rows_array = json::array();
                for (int i = 0; i < rows; i++) {
                    json row_data = json::object();
                    for (int j = 0; j < cols; j++) {
                        std::string colName = PQfname(res, j);
                        std::string value = PQgetvalue(res, i, j);
                        
                        // Convert to appropriate type based on field type
                        Oid fieldType = PQftype(res, j);
                        switch (fieldType) {
                            case 20: // BIGINT
                            case 21: // SMALLINT
                            case 23: // INTEGER
                                if (!value.empty()) {
                                    row_data[colName] = std::stoll(value);
                                } else {
                                    row_data[colName] = nullptr;
                                }
                                break;
                            case 700: // FLOAT4
                            case 701: // FLOAT8
                            case 1700: // NUMERIC
                                if (!value.empty()) {
                                    row_data[colName] = std::stod(value);
                                } else {
                                    row_data[colName] = nullptr;
                                }
                                break;
                            case 16: // BOOLEAN
                                if (!value.empty()) {
                                    row_data[colName] = (value == "t" || value == "true" || value == "1");
                                } else {
                                    row_data[colName] = false;
                                }
                                break;
                            case 114: // JSON
                            case 3802: // JSONB
                                if (!value.empty()) {
                                    try {
                                        row_data[colName] = json::parse(value);
                                    } catch (const json::exception&) {
                                        row_data[colName] = value;
                                    }
                                } else {
                                    row_data[colName] = nullptr;
                                }
                                break;
                            default:
                                row_data[colName] = value;
                                break;
                        }
                    }
                    rows_array.push_back(row_data);
                }
                
                result["rows"] = rows_array;
                result["affected_rows"] = rows;
                result["success"] = true;
                
            } else if (status == PGRES_COMMAND_OK) {
                // Non-SELECT query (INSERT, UPDATE, DELETE, etc.)
                result["success"] = true;
                result["affected_rows"] = std::stoi(PQcmdTuples(res));
                result["rows"] = json::array();
                
                // Get last insert ID if available
                if (PQresultStatus(res) == PGRES_COMMAND_OK) {
                    const char* oid = PQoidValue(res);
                    if (oid && strcmp(oid, "0") != 0) {
                        result["last_insert_id"] = std::stoll(oid);
                    }
                }
                
            } else {
                // Error occurred
                result["success"] = false;
                result["error"] = PQresultErrorMessage(res);
                setLastError("PostgreSQL query failed: " + std::string(PQresultErrorMessage(res)));
                PQclear(res);
                return false;
            }

            PQclear(res);
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute PostgreSQL query: " + std::string(e.what()));
            return false;
        }
    }

    bool executePostgreSQLTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            PGconn* pgconn = static_cast<PGconn*>(conn.handle);
            
            // Start transaction
            PGresult* res = PQexec(pgconn, "BEGIN");
            if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
                setLastError("Failed to start PostgreSQL transaction: " + std::string(PQerrorMessage(pgconn)));
                if (res) PQclear(res);
                return false;
            }
            PQclear(res);

            json transaction_results = json::array();
            bool success = true;

            // Execute all queries
            for (const auto& query : queries) {
                json query_result;
                if (executePostgreSQLQuery(conn, query, query_result)) {
                    transaction_results.push_back(query_result);
                } else {
                    success = false;
                    break;
                }
            }

            if (success) {
                // Commit transaction
                res = PQexec(pgconn, "COMMIT");
                if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
                    setLastError("Failed to commit PostgreSQL transaction: " + std::string(PQerrorMessage(pgconn)));
                    if (res) PQclear(res);
                    PQexec(pgconn, "ROLLBACK");
                    return false;
                }
                PQclear(res);
                
                result["success"] = true;
                result["results"] = transaction_results;
            } else {
                // Rollback transaction
                res = PQexec(pgconn, "ROLLBACK");
                if (res) PQclear(res);
                result["success"] = false;
                result["error"] = getLastError();
            }

            return success;
        } catch (const std::exception& e) {
            setLastError("Failed to execute PostgreSQL transaction: " + std::string(e.what()));
            return false;
        }
    }
#endif

#ifdef SATOX_ENABLE_MYSQL
    // MySQL methods (full implementation)
    bool connectMySQL(DatabaseConnection& conn) {
        try {
            std::string host = conn.config.value("host", "localhost");
            int port = conn.config.value("port", 3306);
            std::string database = conn.config.value("database", "");
            std::string username = conn.config.value("username", "");
            std::string password = conn.config.value("password", "");
            
            MYSQL* mysql = mysql_init(nullptr);
            if (!mysql) {
                setLastError("Failed to initialize MySQL connection");
                return false;
            }

            // Set connection options
            mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &conn.config.value("timeout", 30));
            mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &conn.config.value("read_timeout", 30));
            mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, &conn.config.value("write_timeout", 30));
            
            // Enable SSL if configured
            if (conn.config.value("enable_ssl", false)) {
                mysql_ssl_set(mysql, nullptr, nullptr, nullptr, nullptr, nullptr);
            }

            // Connect to MySQL server
            if (!mysql_real_connect(mysql, host.c_str(), username.c_str(),
                                   password.c_str(), database.c_str(),
                                   port, nullptr, 0)) {
                setLastError("Failed to connect to MySQL: " + std::string(mysql_error(mysql)));
                mysql_close(mysql);
                return false;
            }

            // Set character set
            if (mysql_set_character_set(mysql, "utf8mb4") != 0) {
                spdlog::warn("Failed to set MySQL character set: {}", mysql_error(mysql));
            }

            // Set SQL mode for better compatibility
            if (mysql_query(mysql, "SET sql_mode = 'STRICT_TRANS_TABLES,NO_ZERO_DATE,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO'") != 0) {
                spdlog::warn("Failed to set MySQL SQL mode: {}", mysql_error(mysql));
            }

            conn.handle = mysql;
            conn.connected = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to MySQL: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectMySQL(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                mysql_close(static_cast<MYSQL*>(conn.handle));
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from MySQL: " + std::string(e.what()));
            return false;
        }
    }

    bool executeMySQLQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            MYSQL* mysql = static_cast<MYSQL*>(conn.handle);
            
            // Execute query
            if (mysql_query(mysql, query.c_str()) != 0) {
                setLastError("MySQL query failed: " + std::string(mysql_error(mysql)));
                return false;
            }

            MYSQL_RES* result_set = mysql_store_result(mysql);
            if (result_set) {
                // SELECT query - process results
                MYSQL_FIELD* fields = mysql_fetch_fields(result_set);
                int num_fields = mysql_num_fields(result_set);
                int num_rows = mysql_num_rows(result_set);

                json rows = json::array();
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result_set))) {
                    json row_data = json::object();
                    unsigned long* lengths = mysql_fetch_lengths(result_set);
                    
                    for (int i = 0; i < num_fields; i++) {
                        std::string field_name = fields[i].name;
                        std::string value;
                        
                        if (row[i]) {
                            value = std::string(row[i], lengths[i]);
                        } else {
                            value = "";
                        }
                        
                        // Convert to appropriate type based on field type
                        switch (fields[i].type) {
                            case MYSQL_TYPE_TINY:
                            case MYSQL_TYPE_SHORT:
                            case MYSQL_TYPE_INT24:
                            case MYSQL_TYPE_LONG:
                            case MYSQL_TYPE_LONGLONG:
                                if (!value.empty()) {
                                    row_data[field_name] = std::stoll(value);
                                } else {
                                    row_data[field_name] = nullptr;
                                }
                                break;
                            case MYSQL_TYPE_FLOAT:
                            case MYSQL_TYPE_DOUBLE:
                            case MYSQL_TYPE_DECIMAL:
                            case MYSQL_TYPE_NEWDECIMAL:
                                if (!value.empty()) {
                                    row_data[field_name] = std::stod(value);
                                } else {
                                    row_data[field_name] = nullptr;
                                }
                                break;
                            case MYSQL_TYPE_BIT:
                                if (!value.empty()) {
                                    row_data[field_name] = (value != "0");
                                } else {
                                    row_data[field_name] = false;
                                }
                                break;
                            default:
                                row_data[field_name] = value;
                                break;
                        }
                    }
                    rows.push_back(row_data);
                }
                
                result["rows"] = rows;
                result["affected_rows"] = num_rows;
                result["success"] = true;
                
                mysql_free_result(result_set);
            } else {
                // Non-SELECT query (INSERT, UPDATE, DELETE, etc.)
                if (mysql_field_count(mysql) == 0) {
                    result["success"] = true;
                    result["affected_rows"] = mysql_affected_rows(mysql);
                    result["last_insert_id"] = mysql_insert_id(mysql);
                    result["rows"] = json::array();
                } else {
                    setLastError("MySQL query failed: " + std::string(mysql_error(mysql)));
                    return false;
                }
            }

            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute MySQL query: " + std::string(e.what()));
            return false;
        }
    }

    bool executeMySQLTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            MYSQL* mysql = static_cast<MYSQL*>(conn.handle);
            
            // Start transaction
            if (mysql_query(mysql, "START TRANSACTION") != 0) {
                setLastError("Failed to start MySQL transaction: " + std::string(mysql_error(mysql)));
                return false;
            }

            json transaction_results = json::array();
            bool success = true;

            // Execute all queries
            for (const auto& query : queries) {
                json query_result;
                if (executeMySQLQuery(conn, query, query_result)) {
                    transaction_results.push_back(query_result);
                } else {
                    success = false;
                    break;
                }
            }

            if (success) {
                // Commit transaction
                if (mysql_query(mysql, "COMMIT") != 0) {
                    setLastError("Failed to commit MySQL transaction: " + std::string(mysql_error(mysql)));
                    mysql_query(mysql, "ROLLBACK");
                    return false;
                }
                result["success"] = true;
                result["results"] = transaction_results;
            } else {
                // Rollback transaction
                mysql_query(mysql, "ROLLBACK");
                result["success"] = false;
                result["error"] = getLastError();
            }

            return success;
        } catch (const std::exception& e) {
            setLastError("Failed to execute MySQL transaction: " + std::string(e.what()));
            return false;
        }
    }
#endif

#ifdef SATOX_ENABLE_REDIS
    // Redis methods
    bool connectRedis(DatabaseConnection& conn) {
        try {
            std::string host = conn.config.value("host", "localhost");
            int port = conn.config.value("port", 6379);
            
            redisContext* context = redisConnect(host.c_str(), port);
            if (context == nullptr || context->err) {
                setLastError("Failed to connect to Redis: " + 
                           (context ? context->errstr : "Unknown error"));
                if (context) redisFree(context);
                return false;
            }
            
            conn.handle = context;
            conn.connected = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to Redis: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectRedis(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                redisFree(static_cast<redisContext*>(conn.handle));
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from Redis: " + std::string(e.what()));
            return false;
        }
    }

    bool executeRedisQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            redisContext* context = static_cast<redisContext*>(conn.handle);
            redisReply* reply = static_cast<redisReply*>(redisCommand(context, query.c_str()));
            
            if (reply == nullptr) {
                setLastError("Failed to execute Redis command");
                return false;
            }

            result = parseRedisReply(reply);
            freeReplyObject(reply);
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute Redis query: " + std::string(e.what()));
            return false;
        }
    }

    json parseRedisReply(redisReply* reply) {
        json result;
        switch (reply->type) {
            case REDIS_REPLY_STRING:
                result = std::string(reply->str, reply->len);
                break;
            case REDIS_REPLY_INTEGER:
                result = reply->integer;
                break;
            case REDIS_REPLY_ARRAY: {
                json array = json::array();
                for (size_t i = 0; i < reply->elements; i++) {
                    array.push_back(parseRedisReply(reply->element[i]));
                }
                result = array;
                break;
            }
            case REDIS_REPLY_NIL:
                result = nullptr;
                break;
            case REDIS_REPLY_STATUS:
                result = std::string(reply->str, reply->len);
                break;
            case REDIS_REPLY_ERROR:
                result = std::string(reply->str, reply->len);
                break;
        }
        return result;
    }

    bool executeRedisTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            redisContext* context = static_cast<redisContext*>(conn.handle);
            
            // Start transaction
            redisReply* reply = static_cast<redisReply*>(redisCommand(context, "MULTI"));
            if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                setLastError("Failed to start Redis transaction");
                if (reply) freeReplyObject(reply);
                return false;
            }
            freeReplyObject(reply);

            // Execute all queries
            for (const auto& query : queries) {
                reply = static_cast<redisReply*>(redisCommand(context, query.c_str()));
                if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                    setLastError("Failed to execute Redis transaction query");
                    if (reply) freeReplyObject(reply);
                    redisCommand(context, "DISCARD");
                    return false;
                }
                freeReplyObject(reply);
            }

            // Commit transaction
            reply = static_cast<redisReply*>(redisCommand(context, "EXEC"));
            if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
                setLastError("Failed to commit Redis transaction");
                if (reply) freeReplyObject(reply);
                return false;
            }

            result = parseRedisReply(reply);
            freeReplyObject(reply);
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute Redis transaction: " + std::string(e.what()));
            return false;
        }
    }
#endif

#ifdef SATOX_ENABLE_MONGODB
    // MongoDB methods
    bool connectMongoDB(DatabaseConnection& conn) {
        try {
            std::string host = conn.config.value("host", "localhost");
            int port = conn.config.value("port", 27017);
            std::string database = conn.config.value("database", "satox");
            
            std::string uri = "mongodb://" + host + ":" + std::to_string(port);
            mongoc_client_t* client = mongoc_client_new(uri.c_str());
            if (!client) {
                setLastError("Failed to create MongoDB client");
                return false;
            }

            // Test connection
            bson_error_t error;
            if (!mongoc_client_command_simple(client, database.c_str(), 
                BCON_NEW("ping", BCON_INT32(1)), nullptr, nullptr, &error)) {
                setLastError("Failed to connect to MongoDB: " + std::string(error.message));
                mongoc_client_destroy(client);
                return false;
            }

            conn.handle = client;
            conn.connected = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to MongoDB: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectMongoDB(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                mongoc_client_destroy(static_cast<mongoc_client_t*>(conn.handle));
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from MongoDB: " + std::string(e.what()));
            return false;
        }
    }

    bool executeMongoDBQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            mongoc_client_t* client = static_cast<mongoc_client_t*>(conn.handle);
            std::string database = conn.config.value("database", "satox");
            std::string collection = conn.config.value("collection", "default");

            // Parse query as JSON
            json queryJson;
            try {
                queryJson = json::parse(query);
            } catch (const json::exception& e) {
                setLastError("Invalid MongoDB query format");
                return false;
            }

            mongoc_collection_t* coll = mongoc_client_get_collection(client, database.c_str(), collection.c_str());
            mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(coll, queryJson.dump().c_str(), nullptr, nullptr);

            json rows = json::array();
            const bson_t* doc;
            while (mongoc_cursor_next(cursor, &doc)) {
                char* str = bson_as_json(doc, nullptr);
                json row = json::parse(str);
                rows.push_back(row);
                bson_free(str);
            }

            mongoc_cursor_destroy(cursor);
            mongoc_collection_destroy(coll);

            result["rows"] = rows;
            result["success"] = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute MongoDB query: " + std::string(e.what()));
            return false;
        }
    }

    bool executeMongoDBTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            mongoc_client_t* client = static_cast<mongoc_client_t*>(conn.handle);
            std::string database = conn.config.value("database", "satox");
            
            mongoc_session_opt_t* session_opts = mongoc_session_opts_new();
            mongoc_client_session_t* session = mongoc_client_start_session(client, session_opts, nullptr);
            mongoc_session_opts_destroy(session_opts);

            if (!session) {
                setLastError("Failed to start MongoDB session");
                return false;
            }

            bson_error_t error;
            if (!mongoc_client_session_start_transaction(session, nullptr, &error)) {
                setLastError("Failed to start MongoDB transaction: " + std::string(error.message));
                mongoc_client_session_destroy(session);
                return false;
            }

            bool success = true;
            for (const auto& query : queries) {
                // Execute query in transaction
                // This is a simplified implementation
                if (!executeMongoDBQuery(conn, query, result)) {
                    success = false;
                    break;
                }
            }

            if (success) {
                if (!mongoc_client_session_commit_transaction(session, nullptr, &error)) {
                    setLastError("Failed to commit MongoDB transaction: " + std::string(error.message));
                    success = false;
                }
            } else {
                mongoc_client_session_abort_transaction(session, nullptr, &error);
            }

            mongoc_client_session_destroy(session);
            return success;
        } catch (const std::exception& e) {
            setLastError("Failed to execute MongoDB transaction: " + std::string(e.what()));
            return false;
        }
    }
#endif

#ifdef SATOX_ENABLE_ROCKSDB
    // RocksDB methods
    bool connectRocksDB(DatabaseConnection& conn) {
        try {
            std::string path = conn.config.value("path", "./rocksdb_data");
            bool createIfMissing = conn.config.value("create_if_missing", true);
            int maxBackgroundJobs = conn.config.value("max_background_jobs", 4);
            int writeBufferSize = conn.config.value("write_buffer_size", 64 * 1024 * 1024); // 64MB
            int maxWriteBufferNumber = conn.config.value("max_write_buffer_number", 2);
            int targetFileSizeBase = conn.config.value("target_file_size_base", 64 * 1024 * 1024); // 64MB
            int maxBytesForLevelBase = conn.config.value("max_bytes_for_level_base", 256 * 1024 * 1024); // 256MB
            bool enableCompression = conn.config.value("enable_compression", true);
            std::string compressionType = conn.config.value("compression_type", "snappy");
            
            rocksdb::DB* db;
            rocksdb::Options options;
            
            // Basic options
            options.create_if_missing = createIfMissing;
            options.max_background_jobs = maxBackgroundJobs;
            options.write_buffer_size = writeBufferSize;
            options.max_write_buffer_number = maxWriteBufferNumber;
            options.target_file_size_base = targetFileSizeBase;
            options.max_bytes_for_level_base = maxBytesForLevelBase;
            
            // Compression options
            if (enableCompression) {
                if (compressionType == "snappy") {
                    options.compression = rocksdb::kSnappyCompression;
                } else if (compressionType == "zlib") {
                    options.compression = rocksdb::kZlibCompression;
                } else if (compressionType == "lz4") {
                    options.compression = rocksdb::kLZ4Compression;
                } else if (compressionType == "zstd") {
                    options.compression = rocksdb::kZSTD;
                } else {
                    options.compression = rocksdb::kSnappyCompression; // Default
                }
            } else {
                options.compression = rocksdb::kNoCompression;
            }
            
            // Performance optimizations
            options.allow_mmap_reads = true;
            options.allow_mmap_writes = true;
            options.use_fsync = false; // Use fdatasync for better performance
            options.bytes_per_sync = 1024 * 1024; // 1MB
            
            // Configure table options
            rocksdb::BlockBasedTableOptions tableOptions;
            // tableOptions.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10));
            // Use default filter policy for now
            tableOptions.block_cache = rocksdb::NewLRUCache(8 * 1024 * 1024); // 8MB cache
            options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(tableOptions));
            
            rocksdb::Status status = rocksdb::DB::Open(options, path, &db);
            if (!status.ok()) {
                setLastError("Failed to open RocksDB: " + status.ToString());
                return false;
            }

            conn.handle = db;
            conn.connected = true;
            spdlog::info("RocksDB connected successfully to: {}", path);
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to RocksDB: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectRocksDB(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                rocksdb::DB* db = static_cast<rocksdb::DB*>(conn.handle);
                delete db;
                conn.handle = nullptr;
            }
            conn.connected = false;
            spdlog::info("RocksDB disconnected successfully");
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from RocksDB: " + std::string(e.what()));
            return false;
        }
    }

    bool executeRocksDBQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            rocksdb::DB* db = static_cast<rocksdb::DB*>(conn.handle);
            
            // Parse query as JSON for key-value operations
            json queryJson;
            try {
                queryJson = json::parse(query);
            } catch (const json::exception& e) {
                setLastError("Invalid RocksDB query format");
                return false;
            }

            std::string operation = queryJson.value("operation", "");
            std::string key = queryJson.value("key", "");
            
            if (operation == "get") {
                std::string value;
                rocksdb::Status status = db->Get(rocksdb::ReadOptions(), key, &value);
                if (status.ok()) {
                    json row;
                    row["key"] = key;
                    row["value"] = value;
                    result["rows"] = json::array({row});
                    result["success"] = true;
                } else if (status.IsNotFound()) {
                    result["success"] = false;
                    result["error"] = "Key not found";
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
            } else if (operation == "put") {
                std::string value = queryJson.value("value", "");
                rocksdb::Status status = db->Put(rocksdb::WriteOptions(), key, value);
                if (status.ok()) {
                    result["success"] = true;
                    result["affected_rows"] = 1;
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
            } else if (operation == "delete") {
                rocksdb::Status status = db->Delete(rocksdb::WriteOptions(), key);
                if (status.ok()) {
                    result["success"] = true;
                    result["affected_rows"] = 1;
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
            } else if (operation == "scan") {
                // Scan operation for range queries
                std::string startKey = queryJson.value("start_key", "");
                std::string endKey = queryJson.value("end_key", "");
                int limit = queryJson.value("limit", 1000);
                
                rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
                json rows = json::array();
                int count = 0;
                
                if (startKey.empty()) {
                    it->SeekToFirst();
                } else {
                    it->Seek(startKey);
                }
                
                while (it->Valid() && count < limit) {
                    if (!endKey.empty() && it->key().ToString() >= endKey) {
                        break;
                    }
                    
                    json row;
                    row["key"] = it->key().ToString();
                    row["value"] = it->value().ToString();
                    rows.push_back(row);
                    count++;
                    it->Next();
                }
                
                delete it;
                result["rows"] = rows;
                result["success"] = true;
                result["count"] = count;
                
            } else if (operation == "exists") {
                std::string value;
                rocksdb::Status status = db->Get(rocksdb::ReadOptions(), key, &value);
                result["success"] = true;
                result["exists"] = status.ok();
                
            } else if (operation == "get_multi") {
                // Get multiple keys
                std::vector<std::string> keys = queryJson.value("keys", std::vector<std::string>());
                std::vector<rocksdb::Slice> keySlices;
                std::vector<std::string> values;
                std::vector<rocksdb::Status> statuses;
                
                for (const auto& k : keys) {
                    keySlices.emplace_back(k);
                }
                
                statuses = db->MultiGet(rocksdb::ReadOptions(), keySlices, &values);
                
                json rows = json::array();
                for (size_t i = 0; i < keys.size(); i++) {
                    json row;
                    row["key"] = keys[i];
                    if (statuses[i].ok()) {
                        row["value"] = values[i];
                        row["found"] = true;
                    } else {
                        row["value"] = "";
                        row["found"] = false;
                    }
                    rows.push_back(row);
                }
                
                result["rows"] = rows;
                result["success"] = true;
                
            } else if (operation == "put_multi") {
                // Put multiple key-value pairs
                json keyValues = queryJson.value("key_values", json::object());
                rocksdb::WriteBatch batch;
                
                for (const auto& [k, v] : keyValues.items()) {
                    batch.Put(k, v.dump());
                }
                
                rocksdb::Status status = db->Write(rocksdb::WriteOptions(), &batch);
                if (status.ok()) {
                    result["success"] = true;
                    result["affected_rows"] = keyValues.size();
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
                
            } else if (operation == "delete_multi") {
                // Delete multiple keys
                std::vector<std::string> keys = queryJson.value("keys", std::vector<std::string>());
                rocksdb::WriteBatch batch;
                
                for (const auto& key : keys) {
                    batch.Delete(key);
                }
                
                rocksdb::Status status = db->Write(rocksdb::WriteOptions(), &batch);
                if (status.ok()) {
                    result["success"] = true;
                    result["affected_rows"] = keys.size();
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
                
            } else if (operation == "compact") {
                // Compact the database
                rocksdb::Status status = db->CompactRange(rocksdb::CompactRangeOptions(), nullptr, nullptr);
                if (status.ok()) {
                    result["success"] = true;
                    result["message"] = "Database compaction completed";
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
                
            } else if (operation == "flush") {
                // Flush memtable to disk
                rocksdb::FlushOptions flushOptions;
                rocksdb::Status status = db->Flush(flushOptions);
                if (status.ok()) {
                    result["success"] = true;
                    result["message"] = "Database flush completed";
                } else {
                    result["success"] = false;
                    result["error"] = status.ToString();
                }
                
            } else if (operation == "get_property") {
                // Get database properties
                std::string property = queryJson.value("property", "rocksdb.stats");
                std::string value;
                bool success = db->GetProperty(property, &value);
                if (success) {
                    result["success"] = true;
                    result["property"] = property;
                    result["value"] = value;
                } else {
                    result["success"] = false;
                    result["error"] = "Property not found or not supported";
                }
                
            } else {
                setLastError("Unsupported RocksDB operation: " + operation);
                return false;
            }

            return result["success"].get<bool>();
        } catch (const std::exception& e) {
            setLastError("Failed to execute RocksDB query: " + std::string(e.what()));
            return false;
        }
    }

    bool executeRocksDBTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            rocksdb::DB* db = static_cast<rocksdb::DB*>(conn.handle);
            rocksdb::WriteBatch batch;
            int affectedRows = 0;

            for (const auto& query : queries) {
                json queryJson = json::parse(query);
                std::string operation = queryJson.value("operation", "");
                std::string key = queryJson.value("key", "");

                if (operation == "put") {
                    std::string value = queryJson.value("value", "");
                    batch.Put(key, value);
                    affectedRows++;
                } else if (operation == "delete") {
                    batch.Delete(key);
                    affectedRows++;
                } else if (operation == "put_multi") {
                    json keyValues = queryJson.value("key_values", json::object());
                    for (const auto& [k, v] : keyValues.items()) {
                        batch.Put(k, v.dump());
                        affectedRows++;
                    }
                } else if (operation == "delete_multi") {
                    std::vector<std::string> keys = queryJson.value("keys", std::vector<std::string>());
                    for (const auto& k : keys) {
                        batch.Delete(k);
                        affectedRows++;
                    }
                } else {
                    setLastError("Unsupported RocksDB transaction operation: " + operation);
                    return false;
                }
            }

            rocksdb::Status status = db->Write(rocksdb::WriteOptions(), &batch);
            if (status.ok()) {
                result["success"] = true;
                result["affected_rows"] = affectedRows;
                return true;
            } else {
                result["success"] = false;
                result["error"] = status.ToString();
                return false;
            }
        } catch (const std::exception& e) {
            setLastError("Failed to execute RocksDB transaction: " + std::string(e.what()));
            return false;
        }
    }
#endif

    // Memory database methods (in-memory storage)
    bool connectMemory(DatabaseConnection& conn) {
        try {
            // Create in-memory storage
            auto* storage = new std::map<std::string, std::string>();
            conn.handle = storage;
            conn.connected = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to connect to memory database: " + std::string(e.what()));
            return false;
        }
    }

    bool disconnectMemory(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                delete static_cast<std::map<std::string, std::string>*>(conn.handle);
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to disconnect from memory database: " + std::string(e.what()));
            return false;
        }
    }

    bool executeMemoryQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            auto* storage = static_cast<std::map<std::string, std::string>*>(conn.handle);
            
            json queryJson = json::parse(query);
            std::string operation = queryJson.value("operation", "");
            std::string key = queryJson.value("key", "");

            if (operation == "get") {
                auto it = storage->find(key);
                if (it != storage->end()) {
                    json row;
                    row["key"] = key;
                    row["value"] = it->second;
                    result["rows"] = json::array({row});
                    result["success"] = true;
                } else {
                    result["success"] = false;
                    result["error"] = "Key not found";
                }
            } else if (operation == "put") {
                std::string value = queryJson.value("value", "");
                (*storage)[key] = value;
                result["success"] = true;
            } else if (operation == "delete") {
                auto it = storage->find(key);
                if (it != storage->end()) {
                    storage->erase(it);
                    result["success"] = true;
                } else {
                    result["success"] = false;
                    result["error"] = "Key not found";
                }
            } else {
                setLastError("Unsupported memory database operation: " + operation);
                return false;
            }

            return result["success"].get<bool>();
        } catch (const std::exception& e) {
            setLastError("Failed to execute memory database query: " + std::string(e.what()));
            return false;
        }
    }

    bool executeMemoryTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            auto* storage = static_cast<std::map<std::string, std::string>*>(conn.handle);
            
            for (const auto& query : queries) {
                json queryJson = json::parse(query);
                std::string operation = queryJson.value("operation", "");
                std::string key = queryJson.value("key", "");

                if (operation == "put") {
                    std::string value = queryJson.value("value", "");
                    (*storage)[key] = value;
                } else if (operation == "delete") {
                    storage->erase(key);
                } else {
                    setLastError("Unsupported memory database transaction operation: " + operation);
                    return false;
                }
            }

            result["success"] = true;
            return true;
        } catch (const std::exception& e) {
            setLastError("Failed to execute memory database transaction: " + std::string(e.what()));
            return false;
        }
    }

    // Cloud database connection methods
    bool connectSupabase(DatabaseConnection& conn) {
        try {
            if (!supabase_manager_) {
                supabase_manager_ = std::make_unique<SupabaseManager>();
            }
            
            SupabaseConfig config = SupabaseConfig::fromJson(conn.config);
            if (!supabase_manager_->initialize(config)) {
                setLastError("Failed to initialize Supabase manager: " + supabase_manager_->getLastError());
                return false;
            }
            
            std::string connection_id = supabase_manager_->connect();
            if (connection_id.empty()) {
                setLastError("Failed to connect to Supabase: " + supabase_manager_->getLastError());
                return false;
            }
            
            conn.handle = new std::string(connection_id);
            conn.connected = true;
            conn.lastActivity = std::chrono::system_clock::now();
            
            return true;
        } catch (const std::exception& e) {
            setLastError("Supabase connection error: " + std::string(e.what()));
            return false;
        }
    }
    
    bool disconnectSupabase(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                std::string* connection_id = static_cast<std::string*>(conn.handle);
                supabase_manager_->disconnect(*connection_id);
                delete connection_id;
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Supabase disconnection error: " + std::string(e.what()));
            return false;
        }
    }
    
    bool executeSupabaseQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            if (!conn.handle) {
                setLastError("No active Supabase connection");
                return false;
            }
            
            std::string* connection_id = static_cast<std::string*>(conn.handle);
            json params = json::object();
            
            bool success = supabase_manager_->executeQuery(*connection_id, query, params, result);
            if (success) {
                conn.lastActivity = std::chrono::system_clock::now();
                updateStats(conn, true);
            } else {
                setLastError("Supabase query error: " + supabase_manager_->getLastError());
                updateStats(conn, false);
            }
            
            return success;
        } catch (const std::exception& e) {
            setLastError("Supabase query error: " + std::string(e.what()));
            updateStats(conn, false);
            return false;
        }
    }
    
    bool executeSupabaseTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            if (!conn.handle) {
                setLastError("No active Supabase connection");
                return false;
            }
            
            std::string* connection_id = static_cast<std::string*>(conn.handle);
            
            bool success = supabase_manager_->executeTransaction(*connection_id, queries, result);
            if (success) {
                conn.lastActivity = std::chrono::system_clock::now();
                updateStats(conn, true);
            } else {
                setLastError("Supabase transaction error: " + supabase_manager_->getLastError());
                updateStats(conn, false);
            }
            
            return success;
        } catch (const std::exception& e) {
            setLastError("Supabase transaction error: " + std::string(e.what()));
            updateStats(conn, false);
            return false;
        }
    }
    
    bool connectFirebase(DatabaseConnection& conn) {
        try {
            if (!firebase_manager_) {
                firebase_manager_ = std::make_unique<FirebaseManager>();
            }
            
            FirebaseConfig config = FirebaseConfig::fromJson(conn.config);
            if (!firebase_manager_->initialize(config)) {
                setLastError("Failed to initialize Firebase manager: " + firebase_manager_->getLastError());
                return false;
            }
            
            std::string connection_id = firebase_manager_->connect();
            if (connection_id.empty()) {
                setLastError("Failed to connect to Firebase: " + firebase_manager_->getLastError());
                return false;
            }
            
            conn.handle = new std::string(connection_id);
            conn.connected = true;
            conn.lastActivity = std::chrono::system_clock::now();
            
            return true;
        } catch (const std::exception& e) {
            setLastError("Firebase connection error: " + std::string(e.what()));
            return false;
        }
    }
    
    bool disconnectFirebase(DatabaseConnection& conn) {
        try {
            if (conn.handle) {
                std::string* connection_id = static_cast<std::string*>(conn.handle);
                firebase_manager_->disconnect(*connection_id);
                delete connection_id;
                conn.handle = nullptr;
            }
            conn.connected = false;
            return true;
        } catch (const std::exception& e) {
            setLastError("Firebase disconnection error: " + std::string(e.what()));
            return false;
        }
    }
    
    bool executeFirebaseQuery(DatabaseConnection& conn, const std::string& query, json& result) {
        try {
            if (!conn.handle) {
                setLastError("No active Firebase connection");
                return false;
            }
            
            std::string* connection_id = static_cast<std::string*>(conn.handle);
            
            // Parse query as JSON for Firebase operations
            json query_json;
            try {
                query_json = json::parse(query);
            } catch (const std::exception& e) {
                setLastError("Invalid Firebase query format: " + std::string(e.what()));
                return false;
            }
            
            // Determine operation type from query
            std::string operation = query_json.value("operation", "");
            std::string collection = query_json.value("collection", "");
            std::string document_id = query_json.value("document_id", "");
            json data = query_json.value("data", json::object());
            
            bool success = false;
            if (operation == "get") {
                success = firebase_manager_->firestore().getDocument(*connection_id, collection, document_id, result);
            } else if (operation == "set") {
                success = firebase_manager_->firestore().setDocument(*connection_id, collection, document_id, data, result);
            } else if (operation == "update") {
                success = firebase_manager_->firestore().updateDocument(*connection_id, collection, document_id, data, result);
            } else if (operation == "delete") {
                success = firebase_manager_->firestore().deleteDocument(*connection_id, collection, document_id, result);
            } else if (operation == "query") {
                success = firebase_manager_->firestore().queryDocuments(*connection_id, collection, data, result);
            } else {
                setLastError("Unsupported Firebase operation: " + operation);
                return false;
            }
            
            if (success) {
                conn.lastActivity = std::chrono::system_clock::now();
                updateStats(conn, true);
            } else {
                setLastError("Firebase query error: " + firebase_manager_->getLastError());
                updateStats(conn, false);
            }
            
            return success;
        } catch (const std::exception& e) {
            setLastError("Firebase query error: " + std::string(e.what()));
            updateStats(conn, false);
            return false;
        }
    }
    
    bool executeFirebaseTransaction(DatabaseConnection& conn, const std::vector<std::string>& queries, json& result) {
        try {
            if (!conn.handle) {
                setLastError("No active Firebase connection");
                return false;
            }
            
            std::string* connection_id = static_cast<std::string*>(conn.handle);
            
            // Execute queries sequentially
            json transaction_result = json::array();
            bool all_success = true;
            
            for (const auto& query : queries) {
                json query_result;
                bool success = executeFirebaseQuery(conn, query, query_result);
                transaction_result.push_back(query_result);
                if (!success) {
                    all_success = false;
                }
            }
            
            result = {
                {"success", all_success},
                {"results", transaction_result}
            };
            
            if (all_success) {
                conn.lastActivity = std::chrono::system_clock::now();
                updateStats(conn, true);
            } else {
                updateStats(conn, false);
            }
            
            return all_success;
        } catch (const std::exception& e) {
            setLastError("Firebase transaction error: " + std::string(e.what()));
            updateStats(conn, false);
            return false;
        }
    }
};

// DatabaseManager implementation using the internal class
// Remove the static global instance - use PIMPL idiom correctly
// static std::unique_ptr<DatabaseManager::Impl> g_dbManager = std::make_unique<DatabaseManager::Impl>();

DatabaseManager::DatabaseManager() : impl_(std::make_unique<Impl>()) {
}

DatabaseManager::~DatabaseManager() {
    if (impl_) {
        impl_->shutdown();
    }
}

bool DatabaseManager::initialize(const DatabaseConfig& config) {
    return impl_->initialize(config);
}

void DatabaseManager::shutdown() {
    impl_->shutdown();
}

std::string DatabaseManager::connect(DatabaseType type, const json& config) {
    return impl_->connect(type, config);
}

bool DatabaseManager::disconnect(const std::string& id) {
    return impl_->disconnect(id);
}

bool DatabaseManager::executeQuery(const std::string& id, const std::string& query, json& result) {
    return impl_->executeQuery(id, query, result);
}

bool DatabaseManager::executeTransaction(const std::string& id, const std::vector<std::string>& queries, json& result) {
    return impl_->executeTransaction(id, queries, result);
}

std::string DatabaseManager::getLastError() const {
    return impl_->getLastError();
}

} // namespace satox::core 
