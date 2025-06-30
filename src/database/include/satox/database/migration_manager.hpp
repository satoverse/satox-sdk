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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include "satox/database/connection_pool.hpp"
#include "satox/database/transaction_manager.hpp"

namespace satox {
namespace database {

class MigrationManager {
public:
    struct MigrationConfig {
        std::string migrations_table;
        std::string migrations_path;
        bool auto_migrate;
        bool validate_migrations;
        std::chrono::seconds timeout;
    };

    struct MigrationInfo {
        std::string version;
        std::string name;
        std::string description;
        std::chrono::system_clock::time_point applied_at;
        std::string checksum;
        bool success;
        std::string error;
    };

    struct MigrationStats {
        size_t total_migrations;
        size_t applied_migrations;
        size_t pending_migrations;
        size_t failed_migrations;
        std::chrono::milliseconds avg_migration_time;
        std::string current_version;
        std::string latest_version;
    };

    static MigrationManager& getInstance();

    bool initialize(const MigrationConfig& config);
    void shutdown();

    // Migration management
    bool createMigration(const std::string& name, const std::string& description);
    bool applyMigration(const std::string& version);
    bool rollbackMigration(const std::string& version);
    bool validateMigrations();

    // Migration status
    std::vector<MigrationInfo> getMigrationHistory() const;
    std::vector<MigrationInfo> getPendingMigrations() const;
    MigrationStats getStats() const;
    std::string getCurrentVersion() const;
    bool isUpToDate() const;

    // Migration operations
    bool migrateUp();
    bool migrateDown();
    bool migrateToVersion(const std::string& version);
    bool migrateToLatest();

    // Configuration
    void setAutoMigrate(bool auto_migrate);
    void setValidateMigrations(bool validate);
    void setTimeout(std::chrono::seconds timeout);

    // Health check
    bool isHealthy() const;
    std::string getLastError() const;

private:
    MigrationManager() = default;
    ~MigrationManager();
    MigrationManager(const MigrationManager&) = delete;
    MigrationManager& operator=(const MigrationManager&) = delete;

    // Internal migration state
    struct Migration {
        std::string version;
        std::string name;
        std::string description;
        std::string up_sql;
        std::string down_sql;
        std::string checksum;
    };

    // Helper methods
    bool loadMigrations();
    bool saveMigration(const MigrationInfo& info);
    bool executeMigration(const Migration& migration, bool up);
    bool validateMigration(const Migration& migration);
    std::string calculateChecksum(const std::string& content);
    bool createMigrationsTable();
    bool updateMigrationsTable(const MigrationInfo& info);
    std::vector<Migration> parseMigrationFile(const std::string& path);
    bool backupDatabase();
    bool restoreDatabase();

    MigrationConfig config_;
    std::map<std::string, Migration> migrations_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_;
    std::atomic<size_t> total_migrations_;
    std::atomic<size_t> applied_migrations_;
    std::atomic<size_t> failed_migrations_;
    std::string last_error_;
};

} // namespace database
} // namespace satox 