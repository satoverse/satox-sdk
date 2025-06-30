# Database Isolation Guide

## Overview
This guide covers implementing database isolation for multi-tenant applications using the Satox SDK, ensuring data security and compliance.

## Multi-Tenant Architecture

### Tenant Isolation Strategies

#### 1. Database-per-Tenant
Each tenant gets their own database instance.

**Advantages:**
- Complete data isolation
- Independent scaling
- Custom configurations per tenant
- Easy backup and restore per tenant

**Disadvantages:**
- Higher resource usage
- More complex management
- Higher costs

**Implementation:**
```cpp
// config/tenant_config.h
#ifndef SATOX_TENANT_CONFIG_H
#define SATOX_TENANT_CONFIG_H

#include <string>
#include <map>

namespace satox {
namespace config {

struct TenantConfig {
    std::string tenant_id;
    std::string database_name;
    std::string connection_string;
    std::string encryption_key;
    bool is_active;
    std::map<std::string, std::string> custom_settings;
};

class TenantManager {
public:
    static TenantManager& getInstance();
    
    bool createTenant(const std::string& tenant_id, const TenantConfig& config);
    bool deleteTenant(const std::string& tenant_id);
    TenantConfig getTenantConfig(const std::string& tenant_id);
    std::vector<std::string> getAllTenants();
    
private:
    std::map<std::string, TenantConfig> tenants_;
};

} // namespace config
} // namespace satox

#endif // SATOX_TENANT_CONFIG_H
```

#### 2. Schema-per-Tenant
All tenants share the same database but have separate schemas.

**Advantages:**
- Resource efficient
- Easier management
- Shared infrastructure
- Lower costs

**Disadvantages:**
- Potential for data leakage
- More complex queries
- Schema changes affect all tenants

**Implementation:**
```sql
-- Create tenant schemas
CREATE SCHEMA tenant_001;
CREATE SCHEMA tenant_002;
CREATE SCHEMA tenant_003;

-- Grant permissions
GRANT USAGE ON SCHEMA tenant_001 TO tenant_001_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA tenant_001 TO tenant_001_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA tenant_001 TO tenant_001_user;

-- Create tables in tenant schema
CREATE TABLE tenant_001.users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE tenant_001.wallets (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES tenant_001.users(id),
    address VARCHAR(255) UNIQUE NOT NULL,
    balance DECIMAL(20,8) DEFAULT 0
);
```

#### 3. Row-Level Security (RLS)
All tenants share the same tables with row-level filtering.

**Advantages:**
- Maximum resource efficiency
- Simple architecture
- Easy to implement
- Low maintenance

**Disadvantages:**
- Potential for data leakage
- Complex security policies
- Performance impact

**Implementation:**
```sql
-- Enable RLS on tables
ALTER TABLE users ENABLE ROW LEVEL SECURITY;
ALTER TABLE wallets ENABLE ROW LEVEL SECURITY;
ALTER TABLE transactions ENABLE ROW LEVEL SECURITY;

-- Create security policies
CREATE POLICY tenant_isolation_policy ON users
    FOR ALL
    USING (tenant_id = current_setting('app.current_tenant_id'));

CREATE POLICY tenant_isolation_policy ON wallets
    FOR ALL
    USING (tenant_id = current_setting('app.current_tenant_id'));

CREATE POLICY tenant_isolation_policy ON transactions
    FOR ALL
    USING (tenant_id = current_setting('app.current_tenant_id'));

-- Set tenant context
SELECT set_config('app.current_tenant_id', 'tenant_001', false);
```

## Implementation Examples

### C++ SDK Implementation

#### Tenant-Aware Database Connection
```cpp
// database/tenant_connection.h
#ifndef SATOX_TENANT_CONNECTION_H
#define SATOX_TENANT_CONNECTION_H

#include <memory>
#include <string>
#include "config/tenant_config.h"

namespace satox {
namespace database {

class TenantConnection {
public:
    explicit TenantConnection(const std::string& tenant_id);
    ~TenantConnection();
    
    bool connect();
    bool disconnect();
    bool isConnected() const;
    
    // Database operations with tenant context
    bool executeQuery(const std::string& query);
    std::vector<std::map<std::string, std::string>> fetchResults();
    
    // Tenant-specific operations
    bool createUser(const std::string& username, const std::string& email);
    bool createWallet(const std::string& user_id, const std::string& address);
    bool updateBalance(const std::string& wallet_id, double amount);
    
private:
    std::string tenant_id_;
    std::unique_ptr<DatabaseConnection> connection_;
    config::TenantConfig config_;
    
    void setTenantContext();
    std::string getSchemaPrefix() const;
};

} // namespace database
} // namespace satox

#endif // SATOX_TENANT_CONNECTION_H
```

#### Tenant Connection Implementation
```cpp
// database/tenant_connection.cpp
#include "database/tenant_connection.h"
#include "config/tenant_config.h"
#include "utils/logger.h"

namespace satox {
namespace database {

TenantConnection::TenantConnection(const std::string& tenant_id)
    : tenant_id_(tenant_id) {
    
    auto& tenant_manager = config::TenantManager::getInstance();
    config_ = tenant_manager.getTenantConfig(tenant_id);
    
    if (!config_.is_active) {
        throw std::runtime_error("Tenant is not active: " + tenant_id);
    }
}

bool TenantConnection::connect() {
    try {
        connection_ = std::make_unique<DatabaseConnection>(config_.connection_string);
        
        if (connection_->connect()) {
            setTenantContext();
            LOG_INFO("Connected to database for tenant: " + tenant_id_);
            return true;
        }
        
        LOG_ERROR("Failed to connect to database for tenant: " + tenant_id_);
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception connecting to database: " + std::string(e.what()));
        return false;
    }
}

void TenantConnection::setTenantContext() {
    // Set tenant context for RLS
    std::string context_query = "SELECT set_config('app.current_tenant_id', '" + 
                               tenant_id_ + "', false)";
    connection_->executeQuery(context_query);
}

std::string TenantConnection::getSchemaPrefix() const {
    // For schema-per-tenant strategy
    return "tenant_" + tenant_id_ + ".";
}

bool TenantConnection::createUser(const std::string& username, const std::string& email) {
    std::string query = "INSERT INTO " + getSchemaPrefix() + "users (username, email) VALUES (?, ?)";
    
    try {
        auto stmt = connection_->prepareStatement(query);
        stmt->setString(1, username);
        stmt->setString(2, email);
        return stmt->execute();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create user: " + std::string(e.what()));
        return false;
    }
}

bool TenantConnection::createWallet(const std::string& user_id, const std::string& address) {
    std::string query = "INSERT INTO " + getSchemaPrefix() + "wallets (user_id, address) VALUES (?, ?)";
    
    try {
        auto stmt = connection_->prepareStatement(query);
        stmt->setString(1, user_id);
        stmt->setString(2, address);
        return stmt->execute();
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create wallet: " + std::string(e.what()));
        return false;
    }
}

} // namespace database
} // namespace satox
```

### Mobile SDK Implementation

#### iOS Tenant Isolation
```swift
// TenantManager.swift
import Foundation

class TenantManager {
    static let shared = TenantManager()
    
    private var currentTenant: String?
    private var tenantConfigs: [String: TenantConfig] = [:]
    
    struct TenantConfig {
        let tenantId: String
        let databaseName: String
        let encryptionKey: String
        let isActive: Bool
    }
    
    func setCurrentTenant(_ tenantId: String) throws {
        guard let config = tenantConfigs[tenantId], config.isActive else {
            throw TenantError.invalidTenant
        }
        currentTenant = tenantId
    }
    
    func getCurrentTenant() -> String? {
        return currentTenant
    }
    
    func createTenantDatabase(_ tenantId: String) throws {
        let config = TenantConfig(
            tenantId: tenantId,
            databaseName: "tenant_\(tenantId)",
            encryptionKey: generateEncryptionKey(),
            isActive: true
        )
        
        tenantConfigs[tenantId] = config
        
        // Create local SQLite database for tenant
        try createLocalDatabase(for: config)
    }
    
    private func createLocalDatabase(for config: TenantConfig) throws {
        let dbPath = getDatabasePath(for: config.databaseName)
        let database = try Database(path: dbPath, encryptionKey: config.encryptionKey)
        
        // Create tenant-specific tables
        try database.execute("""
            CREATE TABLE IF NOT EXISTS users (
                id TEXT PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                email TEXT UNIQUE NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        try database.execute("""
            CREATE TABLE IF NOT EXISTS wallets (
                id TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                address TEXT UNIQUE NOT NULL,
                balance REAL DEFAULT 0,
                FOREIGN KEY (user_id) REFERENCES users (id)
            )
        """)
    }
}

// TenantAwareDatabase.swift
class TenantAwareDatabase {
    private let tenantManager = TenantManager.shared
    private var database: Database?
    
    func connect() throws {
        guard let tenantId = tenantManager.getCurrentTenant() else {
            throw DatabaseError.noTenantSet
        }
        
        let dbPath = getDatabasePath(for: "tenant_\(tenantId)")
        database = try Database(path: dbPath)
    }
    
    func createUser(username: String, email: String) throws {
        guard let db = database else {
            throw DatabaseError.notConnected
        }
        
        try db.execute("""
            INSERT INTO users (username, email) VALUES (?, ?)
        """, [username, email])
    }
    
    func createWallet(userId: String, address: String) throws {
        guard let db = database else {
            throw DatabaseError.notConnected
        }
        
        try db.execute("""
            INSERT INTO wallets (user_id, address) VALUES (?, ?)
        """, [userId, address])
    }
}
```

#### Android Tenant Isolation
```kotlin
// TenantManager.kt
class TenantManager private constructor() {
    companion object {
        @Volatile
        private var INSTANCE: TenantManager? = null
        
        fun getInstance(): TenantManager {
            return INSTANCE ?: synchronized(this) {
                INSTANCE ?: TenantManager().also { INSTANCE = it }
            }
        }
    }
    
    private var currentTenant: String? = null
    private val tenantConfigs = mutableMapOf<String, TenantConfig>()
    
    data class TenantConfig(
        val tenantId: String,
        val databaseName: String,
        val encryptionKey: String,
        val isActive: Boolean
    )
    
    fun setCurrentTenant(tenantId: String) {
        val config = tenantConfigs[tenantId] ?: throw IllegalArgumentException("Invalid tenant")
        if (!config.isActive) throw IllegalStateException("Tenant is not active")
        currentTenant = tenantId
    }
    
    fun getCurrentTenant(): String? = currentTenant
    
    fun createTenantDatabase(tenantId: String) {
        val config = TenantConfig(
            tenantId = tenantId,
            databaseName = "tenant_$tenantId",
            encryptionKey = generateEncryptionKey(),
            isActive = true
        )
        
        tenantConfigs[tenantId] = config
        createLocalDatabase(config)
    }
    
    private fun createLocalDatabase(config: TenantConfig) {
        val dbPath = getDatabasePath(config.databaseName)
        val database = SQLiteDatabase.openOrCreateDatabase(dbPath, null)
        
        // Create tenant-specific tables
        database.execSQL("""
            CREATE TABLE IF NOT EXISTS users (
                id TEXT PRIMARY KEY,
                username TEXT UNIQUE NOT NULL,
                email TEXT UNIQUE NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        database.execSQL("""
            CREATE TABLE IF NOT EXISTS wallets (
                id TEXT PRIMARY KEY,
                user_id TEXT NOT NULL,
                address TEXT UNIQUE NOT NULL,
                balance REAL DEFAULT 0,
                FOREIGN KEY (user_id) REFERENCES users (id)
            )
        """)
        
        database.close()
    }
}

// TenantAwareDatabase.kt
class TenantAwareDatabase {
    private val tenantManager = TenantManager.getInstance()
    private var database: SQLiteDatabase? = null
    
    fun connect() {
        val tenantId = tenantManager.getCurrentTenant() 
            ?: throw IllegalStateException("No tenant set")
        
        val dbPath = getDatabasePath("tenant_$tenantId")
        database = SQLiteDatabase.openDatabase(dbPath, null, SQLiteDatabase.OPEN_READWRITE)
    }
    
    fun createUser(username: String, email: String) {
        val db = database ?: throw IllegalStateException("Not connected")
        
        val values = ContentValues().apply {
            put("username", username)
            put("email", email)
        }
        
        db.insert("users", null, values)
    }
    
    fun createWallet(userId: String, address: String) {
        val db = database ?: throw IllegalStateException("Not connected")
        
        val values = ContentValues().apply {
            put("user_id", userId)
            put("address", address)
        }
        
        db.insert("wallets", null, values)
    }
}
```

### Game SDK Implementation

#### Unity Tenant Isolation
```csharp
// TenantManager.cs
using UnityEngine;
using System.Collections.Generic;

public class TenantManager : MonoBehaviour
{
    private static TenantManager instance;
    public static TenantManager Instance
    {
        get
        {
            if (instance == null)
            {
                instance = FindObjectOfType<TenantManager>();
                if (instance == null)
                {
                    GameObject go = new GameObject("TenantManager");
                    instance = go.AddComponent<TenantManager>();
                }
            }
            return instance;
        }
    }
    
    [System.Serializable]
    public class TenantConfig
    {
        public string tenantId;
        public string databaseName;
        public string encryptionKey;
        public bool isActive;
    }
    
    private string currentTenant;
    private Dictionary<string, TenantConfig> tenantConfigs = new Dictionary<string, TenantConfig>();
    
    public void SetCurrentTenant(string tenantId)
    {
        if (!tenantConfigs.ContainsKey(tenantId))
            throw new System.ArgumentException("Invalid tenant");
            
        if (!tenantConfigs[tenantId].isActive)
            throw new System.InvalidOperationException("Tenant is not active");
            
        currentTenant = tenantId;
    }
    
    public string GetCurrentTenant()
    {
        return currentTenant;
    }
    
    public void CreateTenantDatabase(string tenantId)
    {
        var config = new TenantConfig
        {
            tenantId = tenantId,
            databaseName = $"tenant_{tenantId}",
            encryptionKey = GenerateEncryptionKey(),
            isActive = true
        };
        
        tenantConfigs[tenantId] = config;
        CreateLocalDatabase(config);
    }
    
    private void CreateLocalDatabase(TenantConfig config)
    {
        string dbPath = GetDatabasePath(config.databaseName);
        
        using (var connection = new SqliteConnection($"Data Source={dbPath}"))
        {
            connection.Open();
            
            using (var command = connection.CreateCommand())
            {
                command.CommandText = @"
                    CREATE TABLE IF NOT EXISTS users (
                        id TEXT PRIMARY KEY,
                        username TEXT UNIQUE NOT NULL,
                        email TEXT UNIQUE NOT NULL,
                        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
                    )";
                command.ExecuteNonQuery();
                
                command.CommandText = @"
                    CREATE TABLE IF NOT EXISTS wallets (
                        id TEXT PRIMARY KEY,
                        user_id TEXT NOT NULL,
                        address TEXT UNIQUE NOT NULL,
                        balance REAL DEFAULT 0,
                        FOREIGN KEY (user_id) REFERENCES users (id)
                    )";
                command.ExecuteNonQuery();
            }
        }
    }
}

// TenantAwareDatabase.cs
public class TenantAwareDatabase : MonoBehaviour
{
    private TenantManager tenantManager;
    private SqliteConnection database;
    
    void Awake()
    {
        tenantManager = TenantManager.Instance;
    }
    
    public void Connect()
    {
        string tenantId = tenantManager.GetCurrentTenant();
        if (string.IsNullOrEmpty(tenantId))
            throw new System.InvalidOperationException("No tenant set");
            
        string dbPath = GetDatabasePath($"tenant_{tenantId}");
        database = new SqliteConnection($"Data Source={dbPath}");
        database.Open();
    }
    
    public void CreateUser(string username, string email)
    {
        if (database == null)
            throw new System.InvalidOperationException("Not connected");
            
        using (var command = database.CreateCommand())
        {
            command.CommandText = "INSERT INTO users (username, email) VALUES (@username, @email)";
            command.Parameters.AddWithValue("@username", username);
            command.Parameters.AddWithValue("@email", email);
            command.ExecuteNonQuery();
        }
    }
    
    public void CreateWallet(string userId, string address)
    {
        if (database == null)
            throw new System.InvalidOperationException("Not connected");
            
        using (var command = database.CreateCommand())
        {
            command.CommandText = "INSERT INTO wallets (user_id, address) VALUES (@userId, @address)";
            command.Parameters.AddWithValue("@userId", userId);
            command.Parameters.AddWithValue("@address", address);
            command.ExecuteNonQuery();
        }
    }
}
```

## Security Considerations

### Data Encryption
```cpp
// encryption/tenant_encryption.h
#ifndef SATOX_TENANT_ENCRYPTION_H
#define SATOX_TENANT_ENCRYPTION_H

#include <string>
#include <vector>

namespace satox {
namespace encryption {

class TenantEncryption {
public:
    static std::string encryptData(const std::string& data, const std::string& tenant_key);
    static std::string decryptData(const std::string& encrypted_data, const std::string& tenant_key);
    
    static std::string generateTenantKey();
    static bool validateTenantKey(const std::string& key);
    
private:
    static const size_t KEY_SIZE = 32;
    static const size_t IV_SIZE = 16;
};

} // namespace encryption
} // namespace satox

#endif // SATOX_TENANT_ENCRYPTION_H
```

### Access Control
```sql
-- Create tenant-specific users
CREATE USER tenant_001_user WITH PASSWORD 'secure_password_001';
CREATE USER tenant_002_user WITH PASSWORD 'secure_password_002';

-- Grant schema access
GRANT USAGE ON SCHEMA tenant_001 TO tenant_001_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA tenant_001 TO tenant_001_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA tenant_001 TO tenant_001_user;

-- Revoke access to other schemas
REVOKE ALL PRIVILEGES ON ALL TABLES IN SCHEMA tenant_002 FROM tenant_001_user;
REVOKE ALL PRIVILEGES ON ALL TABLES IN SCHEMA tenant_001 FROM tenant_002_user;
```

## Monitoring and Auditing

### Tenant Activity Monitoring
```cpp
// monitoring/tenant_monitor.h
#ifndef SATOX_TENANT_MONITOR_H
#define SATOX_TENANT_MONITOR_H

#include <string>
#include <chrono>
#include <map>

namespace satox {
namespace monitoring {

struct TenantMetrics {
    std::string tenant_id;
    int active_connections;
    int queries_per_minute;
    double average_response_time;
    int error_count;
    std::chrono::system_clock::time_point last_activity;
};

class TenantMonitor {
public:
    static TenantMonitor& getInstance();
    
    void recordQuery(const std::string& tenant_id, const std::string& query, 
                    std::chrono::milliseconds duration);
    void recordError(const std::string& tenant_id, const std::string& error);
    TenantMetrics getTenantMetrics(const std::string& tenant_id);
    std::vector<TenantMetrics> getAllTenantMetrics();
    
private:
    std::map<std::string, TenantMetrics> metrics_;
};

} // namespace monitoring
} // namespace satox

#endif // SATOX_TENANT_MONITOR_H
```

### Audit Logging
```sql
-- Create audit table
CREATE TABLE audit_log (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    tenant_id VARCHAR(50) NOT NULL,
    user_id VARCHAR(50),
    action VARCHAR(100) NOT NULL,
    table_name VARCHAR(100),
    record_id VARCHAR(50),
    old_values JSONB,
    new_values JSONB,
    ip_address INET,
    user_agent TEXT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Create audit trigger function
CREATE OR REPLACE FUNCTION audit_trigger_function()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        INSERT INTO audit_log (tenant_id, action, table_name, record_id, new_values)
        VALUES (current_setting('app.current_tenant_id'), 'INSERT', TG_TABLE_NAME, NEW.id, to_jsonb(NEW));
        RETURN NEW;
    ELSIF TG_OP = 'UPDATE' THEN
        INSERT INTO audit_log (tenant_id, action, table_name, record_id, old_values, new_values)
        VALUES (current_setting('app.current_tenant_id'), 'UPDATE', TG_TABLE_NAME, NEW.id, to_jsonb(OLD), to_jsonb(NEW));
        RETURN NEW;
    ELSIF TG_OP = 'DELETE' THEN
        INSERT INTO audit_log (tenant_id, action, table_name, record_id, old_values)
        VALUES (current_setting('app.current_tenant_id'), 'DELETE', TG_TABLE_NAME, OLD.id, to_jsonb(OLD));
        RETURN OLD;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

-- Create audit triggers
CREATE TRIGGER audit_users_trigger
    AFTER INSERT OR UPDATE OR DELETE ON users
    FOR EACH ROW EXECUTE FUNCTION audit_trigger_function();

CREATE TRIGGER audit_wallets_trigger
    AFTER INSERT OR UPDATE OR DELETE ON wallets
    FOR EACH ROW EXECUTE FUNCTION audit_trigger_function();
```

## Performance Optimization

### Connection Pooling
```cpp
// database/connection_pool.h
#ifndef SATOX_CONNECTION_POOL_H
#define SATOX_CONNECTION_POOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace satox {
namespace database {

class ConnectionPool {
public:
    static ConnectionPool& getInstance();
    
    std::shared_ptr<DatabaseConnection> getConnection(const std::string& tenant_id);
    void returnConnection(std::shared_ptr<DatabaseConnection> connection);
    
    void setMaxConnections(size_t max_connections);
    void setConnectionTimeout(std::chrono::seconds timeout);
    
private:
    std::map<std::string, std::queue<std::shared_ptr<DatabaseConnection>>> pools_;
    std::mutex pool_mutex_;
    std::condition_variable pool_cv_;
    size_t max_connections_ = 20;
    std::chrono::seconds connection_timeout_ = std::chrono::seconds(30);
};

} // namespace database
} // namespace satox

#endif // SATOX_CONNECTION_POOL_H
```

### Query Optimization
```sql
-- Create tenant-specific indexes
CREATE INDEX CONCURRENTLY idx_tenant_001_users_username ON tenant_001.users(username);
CREATE INDEX CONCURRENTLY idx_tenant_001_wallets_user_id ON tenant_001.wallets(user_id);

-- Partition tables by tenant for large datasets
CREATE TABLE users_partitioned (
    id UUID PRIMARY KEY,
    tenant_id VARCHAR(50) NOT NULL,
    username VARCHAR(50) NOT NULL,
    email VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) PARTITION BY HASH (tenant_id);

CREATE TABLE users_tenant_001 PARTITION OF users_partitioned
    FOR VALUES WITH (modulus 4, remainder 0);
CREATE TABLE users_tenant_002 PARTITION OF users_partitioned
    FOR VALUES WITH (modulus 4, remainder 1);
```

## Backup and Recovery

### Tenant-Specific Backup
```bash
#!/bin/bash
# scripts/backup_tenant.sh

TENANT_ID="$1"
BACKUP_DIR="./backups/tenants/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

echo "Creating backup for tenant: $TENANT_ID"

# Backup tenant schema
docker exec satox-postgres pg_dump -U satox_user -n "tenant_$TENANT_ID" satox_sdk > "$BACKUP_DIR/tenant_${TENANT_ID}_schema.sql"

# Backup tenant data
docker exec satox-postgres pg_dump -U satox_user -n "tenant_$TENANT_ID" --data-only satox_sdk > "$BACKUP_DIR/tenant_${TENANT_ID}_data.sql"

# Backup tenant configuration
echo "TENANT_ID=$TENANT_ID" > "$BACKUP_DIR/tenant_${TENANT_ID}_config.env"

echo "Backup completed for tenant: $TENANT_ID"
```

### Tenant-Specific Recovery
```bash
#!/bin/bash
# scripts/restore_tenant.sh

TENANT_ID="$1"
BACKUP_DIR="$2"

if [ -z "$TENANT_ID" ] || [ -z "$BACKUP_DIR" ]; then
    echo "Usage: $0 <tenant_id> <backup_directory>"
    exit 1
fi

echo "Restoring tenant: $TENANT_ID from $BACKUP_DIR"

# Restore tenant schema
if [ -f "$BACKUP_DIR/tenant_${TENANT_ID}_schema.sql" ]; then
    docker exec -i satox-postgres psql -U satox_user satox_sdk < "$BACKUP_DIR/tenant_${TENANT_ID}_schema.sql"
fi

# Restore tenant data
if [ -f "$BACKUP_DIR/tenant_${TENANT_ID}_data.sql" ]; then
    docker exec -i satox-postgres psql -U satox_user satox_sdk < "$BACKUP_DIR/tenant_${TENANT_ID}_data.sql"
fi

echo "Restore completed for tenant: $TENANT_ID"
```

## Testing

### Tenant Isolation Tests
```cpp
// tests/tenant_isolation_test.cpp
#include <gtest/gtest.h>
#include "database/tenant_connection.h"
#include "config/tenant_config.h"

class TenantIsolationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test tenants
        auto& tenant_manager = config::TenantManager::getInstance();
        tenant_manager.createTenant("test_tenant_1", {});
        tenant_manager.createTenant("test_tenant_2", {});
    }
    
    void TearDown() override {
        // Clean up test tenants
        auto& tenant_manager = config::TenantManager::getInstance();
        tenant_manager.deleteTenant("test_tenant_1");
        tenant_manager.deleteTenant("test_tenant_2");
    }
};

TEST_F(TenantIsolationTest, DataIsolation) {
    // Test that data is properly isolated between tenants
    database::TenantConnection conn1("test_tenant_1");
    database::TenantConnection conn2("test_tenant_2");
    
    ASSERT_TRUE(conn1.connect());
    ASSERT_TRUE(conn2.connect());
    
    // Create user in tenant 1
    ASSERT_TRUE(conn1.createUser("user1", "user1@tenant1.com"));
    
    // Verify user doesn't exist in tenant 2
    auto results = conn2.fetchResults("SELECT * FROM users WHERE username = 'user1'");
    ASSERT_TRUE(results.empty());
}

TEST_F(TenantIsolationTest, CrossTenantAccess) {
    // Test that tenants cannot access each other's data
    database::TenantConnection conn1("test_tenant_1");
    database::TenantConnection conn2("test_tenant_2");
    
    ASSERT_TRUE(conn1.connect());
    ASSERT_TRUE(conn2.connect());
    
    // Try to access tenant 2's data from tenant 1
    auto results = conn1.fetchResults("SELECT * FROM tenant_test_tenant_2.users");
    ASSERT_TRUE(results.empty());
}
```

## Best Practices

### 1. Tenant Identification
- Always validate tenant ID before database operations
- Use consistent tenant ID format across all systems
- Implement tenant ID validation and sanitization

### 2. Data Isolation
- Never allow cross-tenant data access
- Implement strict access controls
- Use database constraints to enforce isolation

### 3. Performance
- Use connection pooling for each tenant
- Implement tenant-specific caching
- Monitor performance per tenant

### 4. Security
- Encrypt tenant-specific data
- Implement audit logging for all operations
- Use separate encryption keys per tenant

### 5. Backup and Recovery
- Implement tenant-specific backup strategies
- Test recovery procedures regularly
- Maintain backup isolation

## Next Steps
- [PostgreSQL Configuration](postgresql.md)
- [MongoDB Setup](mongodb.md)
- [Redis Caching](redis.md)
- [Local Storage](local-storage.md)

---

*Last updated: $(date)*
