# PostgreSQL Configuration Guide

## Overview
This guide covers PostgreSQL configuration for the Satox SDK, including setup, optimization, and best practices for production deployments.

## Installation

### Docker Installation (Recommended)
```bash
# Pull PostgreSQL image
docker pull postgres:15-alpine

# Create PostgreSQL container
docker run -d \
  --name satox-postgres \
  --network satox-network \
  -e POSTGRES_DB=satox_sdk \
  -e POSTGRES_USER=satox_user \
  -e POSTGRES_PASSWORD=secure_password \
  -p 5432:5432 \
  -v postgres_data:/var/lib/postgresql/data \
  postgres:15-alpine
```

### System Installation (Ubuntu)
```bash
# Update package list
sudo apt update

# Install PostgreSQL
sudo apt install postgresql postgresql-contrib

# Start PostgreSQL service
sudo systemctl start postgresql
sudo systemctl enable postgresql

# Switch to postgres user
sudo -u postgres psql

# Create database and user
CREATE DATABASE satox_sdk;
CREATE USER satox_user WITH PASSWORD 'secure_password';
GRANT ALL PRIVILEGES ON DATABASE satox_sdk TO satox_user;
```

## Configuration

### Basic Configuration
```bash
# PostgreSQL configuration file location
# Docker: /var/lib/postgresql/data/postgresql.conf
# System: /etc/postgresql/15/main/postgresql.conf

# Edit configuration
sudo nano /etc/postgresql/15/main/postgresql.conf
```

### Performance Configuration
```sql
-- Memory settings
shared_buffers = 256MB                    -- 25% of RAM
effective_cache_size = 1GB                -- 75% of RAM
work_mem = 4MB                            -- Per connection
maintenance_work_mem = 64MB               -- Maintenance operations

-- Connection settings
max_connections = 100                     -- Maximum connections
max_worker_processes = 8                  -- Worker processes
max_parallel_workers_per_gather = 4       -- Parallel workers
max_parallel_workers = 8                  -- Total parallel workers

-- Write-ahead logging
wal_buffers = 16MB                        -- WAL buffers
checkpoint_completion_target = 0.9        -- Checkpoint completion
wal_writer_delay = 200ms                  -- WAL writer delay

-- Query planner
random_page_cost = 1.1                    -- SSD optimization
effective_io_concurrency = 200            -- Concurrent I/O
default_statistics_target = 100           -- Statistics accuracy
```

### Security Configuration
```sql
-- Authentication settings
password_encryption = scram-sha-256       -- Password encryption
ssl = on                                  -- Enable SSL
ssl_cert_file = '/etc/ssl/certs/server.crt'
ssl_key_file = '/etc/ssl/private/server.key'

-- Connection limits
listen_addresses = 'localhost'            -- Listen on localhost only
port = 5432                               -- Default port

-- Access control
max_connections = 100                     -- Limit connections
```

### Network Configuration
```sql
-- Network settings
listen_addresses = '*'                    -- Listen on all interfaces
port = 5432                               -- PostgreSQL port
max_connections = 200                     -- Maximum connections

-- SSL configuration
ssl = on
ssl_cert_file = '/etc/ssl/certs/postgresql.crt'
ssl_key_file = '/etc/ssl/private/postgresql.key'
ssl_ca_file = '/etc/ssl/certs/ca.crt'
```

## Database Schema

### Core Tables
```sql
-- Enable UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Users table
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    public_key TEXT NOT NULL,
    status VARCHAR(20) DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Wallets table
CREATE TABLE wallets (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    address VARCHAR(255) UNIQUE NOT NULL,
    private_key_encrypted TEXT NOT NULL,
    balance DECIMAL(20,8) DEFAULT 0,
    status VARCHAR(20) DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Assets table
CREATE TABLE assets (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(100) NOT NULL,
    symbol VARCHAR(10) NOT NULL,
    asset_id VARCHAR(255) UNIQUE NOT NULL,
    issuer_address VARCHAR(255) NOT NULL,
    total_supply DECIMAL(20,8) NOT NULL,
    decimals INTEGER DEFAULT 8,
    metadata JSONB,
    status VARCHAR(20) DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Transactions table
CREATE TABLE transactions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    txid VARCHAR(255) UNIQUE NOT NULL,
    from_address VARCHAR(255) NOT NULL,
    to_address VARCHAR(255) NOT NULL,
    amount DECIMAL(20,8) NOT NULL,
    asset_id VARCHAR(255),
    fee DECIMAL(20,8) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    block_height INTEGER,
    confirmations INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Blocks table
CREATE TABLE blocks (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    block_hash VARCHAR(255) UNIQUE NOT NULL,
    block_height INTEGER UNIQUE NOT NULL,
    previous_block_hash VARCHAR(255),
    merkle_root VARCHAR(255) NOT NULL,
    timestamp TIMESTAMP NOT NULL,
    difficulty DECIMAL(10,8) NOT NULL,
    nonce BIGINT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### Indexes
```sql
-- Primary indexes
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_status ON users(status);

CREATE INDEX idx_wallets_user_id ON wallets(user_id);
CREATE INDEX idx_wallets_address ON wallets(address);
CREATE INDEX idx_wallets_status ON wallets(status);

CREATE INDEX idx_assets_symbol ON assets(symbol);
CREATE INDEX idx_assets_asset_id ON assets(asset_id);
CREATE INDEX idx_assets_issuer ON assets(issuer_address);

CREATE INDEX idx_transactions_txid ON transactions(txid);
CREATE INDEX idx_transactions_from_address ON transactions(from_address);
CREATE INDEX idx_transactions_to_address ON transactions(to_address);
CREATE INDEX idx_transactions_status ON transactions(status);
CREATE INDEX idx_transactions_block_height ON transactions(block_height);
CREATE INDEX idx_transactions_created_at ON transactions(created_at);

CREATE INDEX idx_blocks_hash ON blocks(block_hash);
CREATE INDEX idx_blocks_height ON blocks(block_height);
CREATE INDEX idx_blocks_timestamp ON blocks(timestamp);

-- Composite indexes
CREATE INDEX idx_transactions_from_status ON transactions(from_address, status);
CREATE INDEX idx_transactions_to_status ON transactions(to_address, status);
CREATE INDEX idx_transactions_asset_status ON transactions(asset_id, status);
```

### Constraints
```sql
-- Check constraints
ALTER TABLE users ADD CONSTRAINT chk_user_status 
    CHECK (status IN ('active', 'inactive', 'suspended'));

ALTER TABLE wallets ADD CONSTRAINT chk_wallet_status 
    CHECK (status IN ('active', 'inactive', 'frozen'));

ALTER TABLE assets ADD CONSTRAINT chk_asset_status 
    CHECK (status IN ('active', 'inactive', 'paused'));

ALTER TABLE transactions ADD CONSTRAINT chk_transaction_status 
    CHECK (status IN ('pending', 'confirmed', 'failed', 'cancelled'));

ALTER TABLE transactions ADD CONSTRAINT chk_transaction_amount 
    CHECK (amount > 0);

ALTER TABLE wallets ADD CONSTRAINT chk_wallet_balance 
    CHECK (balance >= 0);

-- Foreign key constraints
ALTER TABLE wallets ADD CONSTRAINT fk_wallets_user 
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE;

-- Unique constraints
ALTER TABLE users ADD CONSTRAINT uk_users_username_email 
    UNIQUE (username, email);
```

## Connection Management

### Connection Pooling
```cpp
// database/postgresql_connection_pool.h
#ifndef SATOX_POSTGRESQL_CONNECTION_POOL_H
#define SATOX_POSTGRESQL_CONNECTION_POOL_H

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace satox {
namespace database {

struct PostgreSQLConfig {
    std::string host = "localhost";
    int port = 5432;
    std::string database = "satox_sdk";
    std::string username = "satox_user";
    std::string password = "secure_password";
    int max_connections = 20;
    int connection_timeout = 30;
    bool ssl_enabled = true;
};

class PostgreSQLConnection {
public:
    explicit PostgreSQLConnection(const PostgreSQLConfig& config);
    ~PostgreSQLConnection();
    
    bool connect();
    bool disconnect();
    bool isConnected() const;
    
    bool executeQuery(const std::string& query);
    std::vector<std::map<std::string, std::string>> fetchResults();
    
private:
    PostgreSQLConfig config_;
    void* connection_; // libpq connection handle
    bool connected_;
};

class PostgreSQLConnectionPool {
public:
    static PostgreSQLConnectionPool& getInstance();
    
    std::shared_ptr<PostgreSQLConnection> getConnection();
    void returnConnection(std::shared_ptr<PostgreSQLConnection> connection);
    
    void setConfig(const PostgreSQLConfig& config);
    void setMaxConnections(size_t max_connections);
    
private:
    PostgreSQLConnectionPool() = default;
    
    std::queue<std::shared_ptr<PostgreSQLConnection>> available_connections_;
    std::vector<std::shared_ptr<PostgreSQLConnection>> active_connections_;
    std::mutex pool_mutex_;
    std::condition_variable pool_cv_;
    PostgreSQLConfig config_;
    size_t max_connections_ = 20;
};

} // namespace database
} // namespace satox

#endif // SATOX_POSTGRESQL_CONNECTION_POOL_H
```

### Connection Implementation
```cpp
// database/postgresql_connection_pool.cpp
#include "database/postgresql_connection_pool.h"
#include <libpq-fe.h>
#include <sstream>
#include "utils/logger.h"

namespace satox {
namespace database {

PostgreSQLConnection::PostgreSQLConnection(const PostgreSQLConfig& config)
    : config_(config), connection_(nullptr), connected_(false) {
}

PostgreSQLConnection::~PostgreSQLConnection() {
    disconnect();
}

bool PostgreSQLConnection::connect() {
    if (connected_) {
        return true;
    }
    
    std::stringstream conninfo;
    conninfo << "host=" << config_.host
             << " port=" << config_.port
             << " dbname=" << config_.database
             << " user=" << config_.username
             << " password=" << config_.password;
    
    if (config_.ssl_enabled) {
        conninfo << " sslmode=require";
    }
    
    connection_ = PQconnectdb(conninfo.str().c_str());
    
    if (PQstatus(connection_) != CONNECTION_OK) {
        LOG_ERROR("PostgreSQL connection failed: " + std::string(PQerrorMessage(connection_)));
        PQfinish(connection_);
        connection_ = nullptr;
        return false;
    }
    
    connected_ = true;
    LOG_INFO("PostgreSQL connection established");
    return true;
}

bool PostgreSQLConnection::disconnect() {
    if (connection_) {
        PQfinish(connection_);
        connection_ = nullptr;
        connected_ = false;
        LOG_INFO("PostgreSQL connection closed");
    }
    return true;
}

bool PostgreSQLConnection::executeQuery(const std::string& query) {
    if (!connected_) {
        LOG_ERROR("Not connected to PostgreSQL");
        return false;
    }
    
    PGresult* result = PQexec(connection_, query.c_str());
    
    if (PQresultStatus(result) != PGRES_COMMAND_OK && 
        PQresultStatus(result) != PGRES_TUPLES_OK) {
        LOG_ERROR("PostgreSQL query failed: " + std::string(PQerrorMessage(connection_)));
        PQclear(result);
        return false;
    }
    
    PQclear(result);
    return true;
}

std::vector<std::map<std::string, std::string>> PostgreSQLConnection::fetchResults() {
    std::vector<std::map<std::string, std::string>> results;
    
    if (!connected_) {
        LOG_ERROR("Not connected to PostgreSQL");
        return results;
    }
    
    PGresult* result = PQgetResult(connection_);
    
    if (PQresultStatus(result) == PGRES_TUPLES_OK) {
        int rows = PQntuples(result);
        int cols = PQnfields(result);
        
        for (int i = 0; i < rows; i++) {
            std::map<std::string, std::string> row;
            for (int j = 0; j < cols; j++) {
                std::string field_name = PQfname(result, j);
                std::string field_value = PQgetvalue(result, i, j);
                row[field_name] = field_value;
            }
            results.push_back(row);
        }
    }
    
    PQclear(result);
    return results;
}

// Connection Pool Implementation
PostgreSQLConnectionPool& PostgreSQLConnectionPool::getInstance() {
    static PostgreSQLConnectionPool instance;
    return instance;
}

std::shared_ptr<PostgreSQLConnection> PostgreSQLConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(pool_mutex_);
    
    // Wait for available connection
    pool_cv_.wait(lock, [this] {
        return !available_connections_.empty() || active_connections_.size() < max_connections_;
    });
    
    if (!available_connections_.empty()) {
        auto connection = available_connections_.front();
        available_connections_.pop();
        active_connections_.push_back(connection);
        return connection;
    }
    
    // Create new connection
    auto connection = std::make_shared<PostgreSQLConnection>(config_);
    if (connection->connect()) {
        active_connections_.push_back(connection);
        return connection;
    }
    
    return nullptr;
}

void PostgreSQLConnectionPool::returnConnection(std::shared_ptr<PostgreSQLConnection> connection) {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    
    // Remove from active connections
    auto it = std::find(active_connections_.begin(), active_connections_.end(), connection);
    if (it != active_connections_.end()) {
        active_connections_.erase(it);
    }
    
    // Return to available pool
    available_connections_.push(connection);
    pool_cv_.notify_one();
}

} // namespace database
} // namespace satox
```

## Performance Optimization

### Query Optimization
```sql
-- Analyze table statistics
ANALYZE users;
ANALYZE wallets;
ANALYZE transactions;
ANALYZE assets;
ANALYZE blocks;

-- Update statistics
ALTER TABLE users ALTER COLUMN username SET STATISTICS 1000;
ALTER TABLE wallets ALTER COLUMN address SET STATISTICS 1000;
ALTER TABLE transactions ALTER COLUMN txid SET STATISTICS 1000;

-- Create partial indexes for common queries
CREATE INDEX idx_active_users ON users(username) WHERE status = 'active';
CREATE INDEX idx_active_wallets ON wallets(address) WHERE status = 'active';
CREATE INDEX idx_pending_transactions ON transactions(txid) WHERE status = 'pending';

-- Create covering indexes
CREATE INDEX idx_wallet_balance_covering ON wallets(user_id, address, balance) 
    INCLUDE (status, created_at);

CREATE INDEX idx_transaction_status_covering ON transactions(status, created_at) 
    INCLUDE (txid, from_address, to_address, amount);
```

### Partitioning
```sql
-- Partition transactions table by date
CREATE TABLE transactions_partitioned (
    id UUID NOT NULL,
    txid VARCHAR(255) NOT NULL,
    from_address VARCHAR(255) NOT NULL,
    to_address VARCHAR(255) NOT NULL,
    amount DECIMAL(20,8) NOT NULL,
    asset_id VARCHAR(255),
    fee DECIMAL(20,8) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    block_height INTEGER,
    confirmations INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) PARTITION BY RANGE (created_at);

-- Create monthly partitions
CREATE TABLE transactions_2024_01 PARTITION OF transactions_partitioned
    FOR VALUES FROM ('2024-01-01') TO ('2024-02-01');
CREATE TABLE transactions_2024_02 PARTITION OF transactions_partitioned
    FOR VALUES FROM ('2024-02-01') TO ('2024-03-01');
CREATE TABLE transactions_2024_03 PARTITION OF transactions_partitioned
    FOR VALUES FROM ('2024-03-01') TO ('2024-04-01');

-- Create indexes on partitions
CREATE INDEX idx_transactions_2024_01_created_at ON transactions_2024_01(created_at);
CREATE INDEX idx_transactions_2024_02_created_at ON transactions_2024_02(created_at);
CREATE INDEX idx_transactions_2024_03_created_at ON transactions_2024_03(created_at);
```

### Materialized Views
```sql
-- Create materialized view for wallet balances
CREATE MATERIALIZED VIEW wallet_balances AS
SELECT 
    w.user_id,
    u.username,
    w.address,
    w.balance,
    COUNT(t.id) as transaction_count,
    MAX(t.created_at) as last_transaction
FROM wallets w
JOIN users u ON w.user_id = u.id
LEFT JOIN transactions t ON w.address = t.from_address OR w.address = t.to_address
WHERE w.status = 'active'
GROUP BY w.user_id, u.username, w.address, w.balance;

-- Create index on materialized view
CREATE INDEX idx_wallet_balances_user_id ON wallet_balances(user_id);
CREATE INDEX idx_wallet_balances_address ON wallet_balances(address);

-- Refresh materialized view
REFRESH MATERIALIZED VIEW wallet_balances;
```

## Backup and Recovery

### Automated Backup Script
```bash
#!/bin/bash
# scripts/backup_postgresql.sh

# Configuration
DB_NAME="satox_sdk"
DB_USER="satox_user"
BACKUP_DIR="./backups/postgresql"
RETENTION_DAYS=30

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Generate backup filename
BACKUP_FILE="$BACKUP_DIR/satox_sdk_$(date +%Y%m%d_%H%M%S).sql"

echo "Starting PostgreSQL backup..."

# Create backup
docker exec satox-postgres pg_dump \
    -U "$DB_USER" \
    -d "$DB_NAME" \
    --verbose \
    --clean \
    --if-exists \
    --create \
    --no-owner \
    --no-privileges \
    > "$BACKUP_FILE"

if [ $? -eq 0 ]; then
    echo "Backup completed successfully: $BACKUP_FILE"
    
    # Compress backup
    gzip "$BACKUP_FILE"
    echo "Backup compressed: ${BACKUP_FILE}.gz"
    
    # Remove old backups
    find "$BACKUP_DIR" -name "*.sql.gz" -mtime +$RETENTION_DAYS -delete
    echo "Old backups cleaned up"
else
    echo "Backup failed!"
    exit 1
fi
```

### Recovery Script
```bash
#!/bin/bash
# scripts/restore_postgresql.sh

if [ -z "$1" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

BACKUP_FILE="$1"

if [ ! -f "$BACKUP_FILE" ]; then
    echo "Backup file not found: $BACKUP_FILE"
    exit 1
fi

echo "Starting PostgreSQL restore from: $BACKUP_FILE"

# Decompress if needed
if [[ "$BACKUP_FILE" == *.gz ]]; then
    echo "Decompressing backup file..."
    gunzip -c "$BACKUP_FILE" | docker exec -i satox-postgres psql -U satox_user satox_sdk
else
    docker exec -i satox-postgres psql -U satox_user satox_sdk < "$BACKUP_FILE"
fi

if [ $? -eq 0 ]; then
    echo "PostgreSQL restore completed successfully"
else
    echo "PostgreSQL restore failed!"
    exit 1
fi
```

## Monitoring

### Health Check Script
```bash
#!/bin/bash
# scripts/check_postgresql_health.sh

echo "Checking PostgreSQL health..."

# Check if PostgreSQL is running
if ! docker ps | grep -q satox-postgres; then
    echo "❌ PostgreSQL container is not running"
    exit 1
fi

# Check connection
if ! docker exec satox-postgres pg_isready -U satox_user -d satox_sdk; then
    echo "❌ PostgreSQL is not responding"
    exit 1
fi

# Check database size
DB_SIZE=$(docker exec satox-postgres psql -U satox_user -d satox_sdk -t -c "
    SELECT pg_size_pretty(pg_database_size('satox_sdk'));
" | xargs)

echo "✅ PostgreSQL is healthy"
echo "📊 Database size: $DB_SIZE"

# Check active connections
ACTIVE_CONNECTIONS=$(docker exec satox-postgres psql -U satox_user -d satox_sdk -t -c "
    SELECT count(*) FROM pg_stat_activity WHERE state = 'active';
" | xargs)

echo "🔗 Active connections: $ACTIVE_CONNECTIONS"
```

### Performance Monitoring
```sql
-- Check slow queries
SELECT 
    query,
    calls,
    total_time,
    mean_time,
    rows
FROM pg_stat_statements 
ORDER BY mean_time DESC 
LIMIT 10;

-- Check table sizes
SELECT 
    schemaname,
    tablename,
    pg_size_pretty(pg_total_relation_size(schemaname||'.'||tablename)) as size
FROM pg_tables 
WHERE schemaname = 'public'
ORDER BY pg_total_relation_size(schemaname||'.'||tablename) DESC;

-- Check index usage
SELECT 
    schemaname,
    tablename,
    indexname,
    idx_scan,
    idx_tup_read,
    idx_tup_fetch
FROM pg_stat_user_indexes 
ORDER BY idx_scan DESC;

-- Check connection statistics
SELECT 
    state,
    count(*) as connections
FROM pg_stat_activity 
GROUP BY state 
ORDER BY connections DESC;
```

## Security

### SSL Configuration
```bash
# Generate SSL certificate
openssl req -new -x509 -days 365 -nodes -text -out server.crt \
    -keyout server.key -subj "/CN=localhost"

# Copy certificates to container
docker cp server.crt satox-postgres:/var/lib/postgresql/data/
docker cp server.key satox-postgres:/var/lib/postgresql/data/

# Set permissions
docker exec satox-postgres chown postgres:postgres /var/lib/postgresql/data/server.crt
docker exec satox-postgres chown postgres:postgres /var/lib/postgresql/data/server.key
docker exec satox-postgres chmod 600 /var/lib/postgresql/data/server.key
```

### Access Control
```sql
-- Create application user with limited privileges
CREATE USER satox_app WITH PASSWORD 'app_password';

-- Grant specific privileges
GRANT CONNECT ON DATABASE satox_sdk TO satox_app;
GRANT USAGE ON SCHEMA public TO satox_app;
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO satox_app;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO satox_app;

-- Revoke unnecessary privileges
REVOKE CREATE ON SCHEMA public FROM satox_app;
REVOKE ALL ON ALL TABLES IN SCHEMA public FROM PUBLIC;
```

## Troubleshooting

### Common Issues

**Connection Refused**
```bash
# Check if PostgreSQL is running
docker ps | grep postgres

# Check PostgreSQL logs
docker logs satox-postgres

# Check port binding
netstat -tlnp | grep 5432
```

**Authentication Failed**
```bash
# Check user permissions
docker exec -it satox-postgres psql -U postgres -c "\du"

# Reset user password
docker exec -it satox-postgres psql -U postgres -c "
    ALTER USER satox_user WITH PASSWORD 'new_password';
"
```

**Performance Issues**
```sql
-- Check for long-running queries
SELECT 
    pid,
    now() - pg_stat_activity.query_start AS duration,
    query
FROM pg_stat_activity 
WHERE state = 'active' 
    AND now() - pg_stat_activity.query_start > interval '5 minutes';

-- Kill long-running queries
SELECT pg_terminate_backend(pid) 
FROM pg_stat_activity 
WHERE state = 'active' 
    AND now() - pg_stat_activity.query_start > interval '10 minutes';
```

## Best Practices

### 1. Configuration
- Use connection pooling to manage connections efficiently
- Set appropriate memory parameters based on available RAM
- Enable SSL for all connections
- Use prepared statements to prevent SQL injection

### 2. Performance
- Create indexes on frequently queried columns
- Use partitioning for large tables
- Monitor query performance regularly
- Update table statistics periodically

### 3. Security
- Use strong passwords for database users
- Limit database user privileges
- Enable SSL/TLS encryption
- Implement connection rate limiting

### 4. Backup
- Perform regular backups
- Test backup restoration procedures
- Store backups securely
- Monitor backup success/failure

### 5. Monitoring
- Monitor database performance metrics
- Set up alerts for critical issues
- Track slow queries
- Monitor disk space usage

## Next Steps
- [MongoDB Setup](mongodb.md)
- [Redis Caching](redis.md)
- [Local Storage](local-storage.md)
- [Database Isolation](isolation.md)

---

*Last updated: $(date)* 