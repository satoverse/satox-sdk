# Database Setup Guide

## Overview
This guide covers setting up databases for the Satox SDK, including PostgreSQL, MongoDB, Redis, and local storage options.

## Prerequisites
- Docker and Docker Compose installed
- At least 4GB RAM available
- 10GB free disk space
- Network access for database downloads

## Quick Setup
```bash
# Clone the repository
git clone https://github.com/satox/satox-sdk.git
cd satox-sdk

# Start all databases with Docker Compose
docker-compose -f docker/database/docker-compose.yml up -d

# Verify databases are running
docker-compose -f docker/database/docker-compose.yml ps
```

## Database Options

### PostgreSQL (Recommended for Production)
PostgreSQL is the primary database for production deployments.

**Features:**
- ACID compliance
- Advanced indexing
- JSON support
- Full-text search
- Multi-tenant isolation

**Setup:**
```bash
# Start PostgreSQL
docker-compose -f docker/database/postgresql.yml up -d

# Create database and user
docker exec -it satox-postgres psql -U postgres
CREATE DATABASE satox_sdk;
CREATE USER satox_user WITH PASSWORD 'secure_password';
GRANT ALL PRIVILEGES ON DATABASE satox_sdk TO satox_user;
```

**Configuration:**
```yaml
# docker/database/postgresql.yml
version: '3.8'
services:
  postgres:
    image: postgres:15-alpine
    container_name: satox-postgres
    environment:
      POSTGRES_DB: satox_sdk
      POSTGRES_USER: satox_user
      POSTGRES_PASSWORD: secure_password
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data
      - ./init:/docker-entrypoint-initdb.d
    networks:
      - satox-network

volumes:
  postgres_data:

networks:
  satox-network:
    driver: bridge
```

### MongoDB (Document Storage)
MongoDB is used for document storage and flexible schema requirements.

**Features:**
- Document-oriented storage
- Flexible schema
- Horizontal scaling
- Aggregation pipeline
- GridFS for large files

**Setup:**
```bash
# Start MongoDB
docker-compose -f docker/database/mongodb.yml up -d

# Connect to MongoDB
docker exec -it satox-mongodb mongosh
use satox_sdk
db.createUser({
  user: "satox_user",
  pwd: "secure_password",
  roles: ["readWrite"]
})
```

**Configuration:**
```yaml
# docker/database/mongodb.yml
version: '3.8'
services:
  mongodb:
    image: mongo:7.0
    container_name: satox-mongodb
    environment:
      MONGO_INITDB_ROOT_USERNAME: admin
      MONGO_INITDB_ROOT_PASSWORD: admin_password
      MONGO_INITDB_DATABASE: satox_sdk
    ports:
      - "27017:27017"
    volumes:
      - mongodb_data:/data/db
      - ./init:/docker-entrypoint-initdb.d
    networks:
      - satox-network

volumes:
  mongodb_data:

networks:
  satox-network:
    driver: bridge
```

### Redis (Caching)
Redis is used for caching, session storage, and real-time features.

**Features:**
- In-memory storage
- Pub/Sub messaging
- Data structures (lists, sets, hashes)
- Persistence options
- Cluster support

**Setup:**
```bash
# Start Redis
docker-compose -f docker/database/redis.yml up -d

# Test Redis connection
docker exec -it satox-redis redis-cli ping
```

**Configuration:**
```yaml
# docker/database/redis.yml
version: '3.8'
services:
  redis:
    image: redis:7.2-alpine
    container_name: satox-redis
    command: redis-server --appendonly yes --requirepass redis_password
    ports:
      - "6379:6379"
    volumes:
      - redis_data:/data
    networks:
      - satox-network

volumes:
  redis_data:

networks:
  satox-network:
    driver: bridge
```

### Local Storage (Development)
Local storage is used for development and testing.

**Features:**
- SQLite for relational data
- File-based storage
- No external dependencies
- Easy backup and restore

**Setup:**
```bash
# Create local storage directory
mkdir -p data/local
chmod 755 data/local

# Initialize SQLite database
sqlite3 data/local/satox_sdk.db < scripts/init_sqlite.sql
```

## Multi-Database Setup

### Combined Docker Compose
```yaml
# docker/database/docker-compose.yml
version: '3.8'
services:
  postgres:
    image: postgres:15-alpine
    container_name: satox-postgres
    environment:
      POSTGRES_DB: satox_sdk
      POSTGRES_USER: satox_user
      POSTGRES_PASSWORD: secure_password
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data
    networks:
      - satox-network

  mongodb:
    image: mongo:7.0
    container_name: satox-mongodb
    environment:
      MONGO_INITDB_ROOT_USERNAME: admin
      MONGO_INITDB_ROOT_PASSWORD: admin_password
      MONGO_INITDB_DATABASE: satox_sdk
    ports:
      - "27017:27017"
    volumes:
      - mongodb_data:/data/db
    networks:
      - satox-network

  redis:
    image: redis:7.2-alpine
    container_name: satox-redis
    command: redis-server --appendonly yes --requirepass redis_password
    ports:
      - "6379:6379"
    volumes:
      - redis_data:/data
    networks:
      - satox-network

volumes:
  postgres_data:
  mongodb_data:
  redis_data:

networks:
  satox-network:
    driver: bridge
```

## Database Initialization

### PostgreSQL Schema
```sql
-- scripts/init_postgresql.sql
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Users table
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    public_key TEXT NOT NULL,
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
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_wallets_user_id ON wallets(user_id);
CREATE INDEX idx_wallets_address ON wallets(address);
CREATE INDEX idx_assets_symbol ON assets(symbol);
CREATE INDEX idx_assets_asset_id ON assets(asset_id);
CREATE INDEX idx_transactions_txid ON transactions(txid);
CREATE INDEX idx_transactions_from_address ON transactions(from_address);
CREATE INDEX idx_transactions_to_address ON transactions(to_address);
CREATE INDEX idx_transactions_status ON transactions(status);
```

### MongoDB Collections
```javascript
// scripts/init_mongodb.js
db = db.getSiblingDB('satox_sdk');

// Create collections
db.createCollection('user_profiles');
db.createCollection('asset_metadata');
db.createCollection('transaction_logs');
db.createCollection('system_config');

// Create indexes
db.user_profiles.createIndex({ "user_id": 1 }, { unique: true });
db.user_profiles.createIndex({ "username": 1 }, { unique: true });
db.asset_metadata.createIndex({ "asset_id": 1 }, { unique: true });
db.transaction_logs.createIndex({ "txid": 1 }, { unique: true });
db.transaction_logs.createIndex({ "timestamp": -1 });
db.system_config.createIndex({ "key": 1 }, { unique: true });

// Insert initial system configuration
db.system_config.insertOne({
    key: "sdk_version",
    value: "1.0.0",
    description: "Current SDK version",
    updated_at: new Date()
});

db.system_config.insertOne({
    key: "blockchain_network",
    value: "mainnet",
    description: "Target blockchain network",
    updated_at: new Date()
});
```

## Connection Configuration

### SDK Configuration
```cpp
// config/database_config.h
#ifndef SATOX_DATABASE_CONFIG_H
#define SATOX_DATABASE_CONFIG_H

#include <string>

namespace satox {
namespace config {

struct DatabaseConfig {
    // PostgreSQL
    std::string postgres_host = "localhost";
    int postgres_port = 5432;
    std::string postgres_db = "satox_sdk";
    std::string postgres_user = "satox_user";
    std::string postgres_password = "secure_password";
    
    // MongoDB
    std::string mongodb_uri = "mongodb://admin:admin_password@localhost:27017/satox_sdk";
    std::string mongodb_db = "satox_sdk";
    
    // Redis
    std::string redis_host = "localhost";
    int redis_port = 6379;
    std::string redis_password = "redis_password";
    
    // Local Storage
    std::string local_storage_path = "./data/local";
    
    // Connection pooling
    int max_connections = 20;
    int connection_timeout = 30;
    int query_timeout = 60;
};

} // namespace config
} // namespace satox

#endif // SATOX_DATABASE_CONFIG_H
```

### Environment Variables
```bash
# .env
# PostgreSQL
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=satox_sdk
POSTGRES_USER=satox_user
POSTGRES_PASSWORD=secure_password

# MongoDB
MONGODB_URI=mongodb://admin:admin_password@localhost:27017/satox_sdk
MONGODB_DB=satox_sdk

# Redis
REDIS_HOST=localhost
REDIS_PORT=6379
REDIS_PASSWORD=redis_password

# Local Storage
LOCAL_STORAGE_PATH=./data/local
```

## Health Checks

### Database Health Check Script
```bash
#!/bin/bash
# scripts/check_database_health.sh

echo "Checking database health..."

# Check PostgreSQL
echo "Checking PostgreSQL..."
if pg_isready -h localhost -p 5432 -U satox_user; then
    echo "✅ PostgreSQL is healthy"
else
    echo "❌ PostgreSQL is not responding"
    exit 1
fi

# Check MongoDB
echo "Checking MongoDB..."
if docker exec satox-mongodb mongosh --eval "db.adminCommand('ping')" > /dev/null 2>&1; then
    echo "✅ MongoDB is healthy"
else
    echo "❌ MongoDB is not responding"
    exit 1
fi

# Check Redis
echo "Checking Redis..."
if docker exec satox-redis redis-cli -a redis_password ping | grep -q "PONG"; then
    echo "✅ Redis is healthy"
else
    echo "❌ Redis is not responding"
    exit 1
fi

echo "All databases are healthy!"
```

## Backup and Recovery

### Backup Script
```bash
#!/bin/bash
# scripts/backup_databases.sh

BACKUP_DIR="./backups/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

echo "Creating database backups in $BACKUP_DIR"

# Backup PostgreSQL
echo "Backing up PostgreSQL..."
docker exec satox-postgres pg_dump -U satox_user satox_sdk > "$BACKUP_DIR/postgresql_backup.sql"

# Backup MongoDB
echo "Backing up MongoDB..."
docker exec satox-mongodb mongodump --db satox_sdk --out "$BACKUP_DIR/mongodb_backup"

# Backup Redis
echo "Backing up Redis..."
docker exec satox-redis redis-cli -a redis_password BGSAVE
sleep 5
docker cp satox-redis:/data/dump.rdb "$BACKUP_DIR/redis_backup.rdb"

echo "Backup completed successfully!"
```

### Recovery Script
```bash
#!/bin/bash
# scripts/restore_databases.sh

if [ -z "$1" ]; then
    echo "Usage: $0 <backup_directory>"
    exit 1
fi

BACKUP_DIR="$1"

if [ ! -d "$BACKUP_DIR" ]; then
    echo "Backup directory $BACKUP_DIR does not exist"
    exit 1
fi

echo "Restoring databases from $BACKUP_DIR"

# Restore PostgreSQL
if [ -f "$BACKUP_DIR/postgresql_backup.sql" ]; then
    echo "Restoring PostgreSQL..."
    docker exec -i satox-postgres psql -U satox_user satox_sdk < "$BACKUP_DIR/postgresql_backup.sql"
fi

# Restore MongoDB
if [ -d "$BACKUP_DIR/mongodb_backup" ]; then
    echo "Restoring MongoDB..."
    docker exec -i satox-mongodb mongorestore --db satox_sdk "$BACKUP_DIR/mongodb_backup/satox_sdk"
fi

# Restore Redis
if [ -f "$BACKUP_DIR/redis_backup.rdb" ]; then
    echo "Restoring Redis..."
    docker cp "$BACKUP_DIR/redis_backup.rdb" satox-redis:/data/dump.rdb
    docker exec satox-redis redis-cli -a redis_password BGREWRITEAOF
fi

echo "Database restoration completed!"
```

## Performance Optimization

### PostgreSQL Optimization
```sql
-- Performance tuning
ALTER SYSTEM SET shared_buffers = '256MB';
ALTER SYSTEM SET effective_cache_size = '1GB';
ALTER SYSTEM SET maintenance_work_mem = '64MB';
ALTER SYSTEM SET checkpoint_completion_target = 0.9;
ALTER SYSTEM SET wal_buffers = '16MB';
ALTER SYSTEM SET default_statistics_target = 100;

-- Reload configuration
SELECT pg_reload_conf();
```

### MongoDB Optimization
```javascript
// Create compound indexes for common queries
db.transaction_logs.createIndex({ "from_address": 1, "timestamp": -1 });
db.transaction_logs.createIndex({ "to_address": 1, "timestamp": -1 });
db.transaction_logs.createIndex({ "asset_id": 1, "timestamp": -1 });

// Enable WiredTiger compression
db.adminCommand({
    setParameter: 1,
    wiredTigerConcurrentReadTransactions: 128
});
```

### Redis Optimization
```bash
# Redis configuration optimization
# redis.conf
maxmemory 512mb
maxmemory-policy allkeys-lru
save 900 1
save 300 10
save 60 10000
```

## Security Considerations

### Network Security
- Use internal Docker networks
- Restrict database access to application containers
- Use strong passwords and encryption
- Enable SSL/TLS for database connections

### Access Control
- Create dedicated database users with minimal privileges
- Use connection pooling to limit concurrent connections
- Implement rate limiting for database access
- Monitor and log all database access

### Data Encryption
- Encrypt sensitive data at rest
- Use SSL/TLS for data in transit
- Implement key rotation procedures
- Secure backup storage

## Troubleshooting

### Common Issues

**PostgreSQL Connection Refused**
```bash
# Check if PostgreSQL is running
docker ps | grep postgres

# Check PostgreSQL logs
docker logs satox-postgres

# Restart PostgreSQL
docker restart satox-postgres
```

**MongoDB Authentication Failed**
```bash
# Check MongoDB logs
docker logs satox-mongodb

# Reset MongoDB password
docker exec -it satox-mongodb mongosh
use admin
db.changeUserPassword("admin", "new_password")
```

**Redis Memory Issues**
```bash
# Check Redis memory usage
docker exec satox-redis redis-cli -a redis_password info memory

# Clear Redis cache
docker exec satox-redis redis-cli -a redis_password FLUSHALL
```

### Performance Monitoring
```bash
# Monitor database performance
docker stats satox-postgres satox-mongodb satox-redis

# Check slow queries
docker exec satox-postgres psql -U satox_user satox_sdk -c "SELECT * FROM pg_stat_statements ORDER BY mean_time DESC LIMIT 10;"
```

## Next Steps
- [Database Isolation Guide](isolation.md)
- [PostgreSQL Configuration](postgresql.md)
- [MongoDB Setup](mongodb.md)
- [Redis Caching](redis.md)
- [Local Storage](local-storage.md)

---

*Last updated: $(date)* 