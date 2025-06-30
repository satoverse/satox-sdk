# Redis Caching Guide

## Overview
This guide covers Redis configuration for the Satox SDK, including setup, optimization, and best practices for caching, session storage, and real-time features.

## Installation

### Docker Installation (Recommended)
```bash
# Pull Redis image
docker pull redis:7.2-alpine

# Create Redis container
docker run -d \
  --name satox-redis \
  --network satox-network \
  -p 6379:6379 \
  -v redis_data:/data \
  redis:7.2-alpine redis-server \
  --appendonly yes \
  --requirepass redis_password \
  --maxmemory 512mb \
  --maxmemory-policy allkeys-lru
```

### System Installation (Ubuntu)
```bash
# Update package list
sudo apt update

# Install Redis
sudo apt install redis-server

# Start Redis service
sudo systemctl start redis-server
sudo systemctl enable redis-server

# Test Redis connection
redis-cli ping
```

## Configuration

### Basic Configuration
```bash
# /etc/redis/redis.conf

# Network
bind 127.0.0.1
port 6379
timeout 0
tcp-keepalive 300

# General
daemonize yes
pidfile /var/run/redis/redis-server.pid
loglevel notice
logfile /var/log/redis/redis-server.log
databases 16

# Snapshotting
save 900 1
save 300 10
save 60 10000
stop-writes-on-bgsave-error yes
rdbcompression yes
rdbchecksum yes
dbfilename dump.rdb
dir /var/lib/redis

# Replication
replica-serve-stale-data yes
replica-read-only yes

# Security
requirepass redis_password
```

### Performance Configuration
```bash
# Memory management
maxmemory 512mb
maxmemory-policy allkeys-lru
maxmemory-samples 5

# Persistence
appendonly yes
appendfilename "appendonly.aof"
appendfsync everysec
no-appendfsync-on-rewrite no
auto-aof-rewrite-percentage 100
auto-aof-rewrite-min-size 64mb

# Client output buffer limits
client-output-buffer-limit normal 0 0 0
client-output-buffer-limit replica 256mb 64mb 60
client-output-buffer-limit pubsub 32mb 8mb 60

# TCP keepalive
tcp-keepalive 300
```

### Security Configuration
```bash
# Authentication
requirepass redis_password

# Network security
bind 127.0.0.1
protected-mode yes
port 6379

# SSL/TLS (if needed)
# tls-port 6380
# tls-cert-file /etc/ssl/redis.crt
# tls-key-file /etc/ssl/redis.key
# tls-ca-cert-file /etc/ssl/ca.crt
```

## Data Structures and Usage

### String Operations
```cpp
// cache/redis_string_operations.h
#ifndef SATOX_REDIS_STRING_OPERATIONS_H
#define SATOX_REDIS_STRING_OPERATIONS_H

#include <string>
#include <optional>

namespace satox {
namespace cache {

class RedisStringOperations {
public:
    // Basic string operations
    bool set(const std::string& key, const std::string& value, int ttl_seconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    
    // Atomic operations
    bool setnx(const std::string& key, const std::string& value);
    std::optional<std::string> getset(const std::string& key, const std::string& value);
    long long incr(const std::string& key);
    long long incrby(const std::string& key, long long increment);
    
    // Expiration
    bool expire(const std::string& key, int seconds);
    bool expireat(const std::string& key, long long timestamp);
    int ttl(const std::string& key);
    
private:
    void* redis_context_; // redisContext*
};

} // namespace cache
} // namespace satox

#endif // SATOX_REDIS_STRING_OPERATIONS_H
```

### Hash Operations
```cpp
// cache/redis_hash_operations.h
#ifndef SATOX_REDIS_HASH_OPERATIONS_H
#define SATOX_REDIS_HASH_OPERATIONS_H

#include <string>
#include <map>
#include <vector>
#include <optional>

namespace satox {
namespace cache {

class RedisHashOperations {
public:
    // Basic hash operations
    bool hset(const std::string& key, const std::string& field, const std::string& value);
    std::optional<std::string> hget(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    bool hexists(const std::string& key, const std::string& field);
    
    // Multiple field operations
    bool hmset(const std::string& key, const std::map<std::string, std::string>& fields);
    std::vector<std::string> hmget(const std::string& key, const std::vector<std::string>& fields);
    std::map<std::string, std::string> hgetall(const std::string& key);
    
    // Hash counters
    long long hincrby(const std::string& key, const std::string& field, long long increment);
    double hincrbyfloat(const std::string& key, const std::string& field, double increment);
    
    // Hash info
    long long hlen(const std::string& key);
    std::vector<std::string> hkeys(const std::string& key);
    std::vector<std::string> hvals(const std::string& key);
    
private:
    void* redis_context_; // redisContext*
};

} // namespace cache
} // namespace satox

#endif // SATOX_REDIS_HASH_OPERATIONS_H
```

### List Operations
```cpp
// cache/redis_list_operations.h
#ifndef SATOX_REDIS_LIST_OPERATIONS_H
#define SATOX_REDIS_LIST_OPERATIONS_H

#include <string>
#include <vector>
#include <optional>

namespace satox {
namespace cache {

class RedisListOperations {
public:
    // Basic list operations
    long long lpush(const std::string& key, const std::vector<std::string>& values);
    long long rpush(const std::string& key, const std::vector<std::string>& values);
    std::optional<std::string> lpop(const std::string& key);
    std::optional<std::string> rpop(const std::string& key);
    
    // List access
    std::optional<std::string> lindex(const std::string& key, long long index);
    std::vector<std::string> lrange(const std::string& key, long long start, long long stop);
    long long llen(const std::string& key);
    
    // List modification
    bool lset(const std::string& key, long long index, const std::string& value);
    long long lrem(const std::string& key, long long count, const std::string& value);
    bool ltrim(const std::string& key, long long start, long long stop);
    
    // Blocking operations
    std::optional<std::string> blpop(const std::vector<std::string>& keys, int timeout);
    std::optional<std::string> brpop(const std::vector<std::string>& keys, int timeout);
    
private:
    void* redis_context_; // redisContext*
};

} // namespace cache
} // namespace satox

#endif // SATOX_REDIS_LIST_OPERATIONS_H
```

### Set Operations
```cpp
// cache/redis_set_operations.h
#ifndef SATOX_REDIS_SET_OPERATIONS_H
#define SATOX_REDIS_SET_OPERATIONS_H

#include <string>
#include <vector>
#include <set>

namespace satox {
namespace cache {

class RedisSetOperations {
public:
    // Basic set operations
    bool sadd(const std::string& key, const std::vector<std::string>& members);
    bool srem(const std::string& key, const std::vector<std::string>& members);
    bool sismember(const std::string& key, const std::string& member);
    
    // Set access
    std::set<std::string> smembers(const std::string& key);
    long long scard(const std::string& key);
    std::optional<std::string> spop(const std::string& key);
    std::vector<std::string> srandmember(const std::string& key, long long count);
    
    // Set operations
    std::set<std::string> sinter(const std::vector<std::string>& keys);
    std::set<std::string> sunion(const std::vector<std::string>& keys);
    std::set<std::string> sdiff(const std::vector<std::string>& keys);
    
    // Set operations with destination
    long long sinterstore(const std::string& destination, const std::vector<std::string>& keys);
    long long sunionstore(const std::string& destination, const std::vector<std::string>& keys);
    long long sdiffstore(const std::string& destination, const std::vector<std::string>& keys);
    
private:
    void* redis_context_; // redisContext*
};

} // namespace cache
} // namespace satox

#endif // SATOX_REDIS_SET_OPERATIONS_H
```

### Sorted Set Operations
```cpp
// cache/redis_sorted_set_operations.h
#ifndef SATOX_REDIS_SORTED_SET_OPERATIONS_H
#define SATOX_REDIS_SORTED_SET_OPERATIONS_H

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace satox {
namespace cache {

struct ScoreMember {
    double score;
    std::string member;
};

class RedisSortedSetOperations {
public:
    // Basic sorted set operations
    bool zadd(const std::string& key, const std::vector<ScoreMember>& score_members);
    bool zrem(const std::string& key, const std::vector<std::string>& members);
    std::optional<double> zscore(const std::string& key, const std::string& member);
    
    // Sorted set access
    std::vector<ScoreMember> zrange(const std::string& key, long long start, long long stop, bool with_scores = false);
    std::vector<ScoreMember> zrevrange(const std::string& key, long long start, long long stop, bool with_scores = false);
    std::vector<ScoreMember> zrangebyscore(const std::string& key, double min, double max, bool with_scores = false);
    std::vector<ScoreMember> zrevrangebyscore(const std::string& key, double max, double min, bool with_scores = false);
    
    // Sorted set info
    long long zcard(const std::string& key);
    long long zcount(const std::string& key, double min, double max);
    std::optional<long long> zrank(const std::string& key, const std::string& member);
    std::optional<long long> zrevrank(const std::string& key, const std::string& member);
    
    // Sorted set operations
    bool zincrby(const std::string& key, double increment, const std::string& member);
    long long zremrangebyrank(const std::string& key, long long start, long long stop);
    long long zremrangebyscore(const std::string& key, double min, double max);
    
private:
    void* redis_context_; // redisContext*
};

} // namespace cache
} // namespace satox

#endif // SATOX_REDIS_SORTED_SET_OPERATIONS_H
```

## Caching Strategies

### User Session Management
```cpp
// cache/session_manager.h
#ifndef SATOX_SESSION_MANAGER_H
#define SATOX_SESSION_MANAGER_H

#include <string>
#include <optional>
#include "cache/redis_hash_operations.h"

namespace satox {
namespace cache {

struct UserSession {
    std::string session_id;
    std::string user_id;
    std::string username;
    std::string ip_address;
    std::string user_agent;
    long long created_at;
    long long last_activity;
    bool is_active;
};

class SessionManager {
public:
    explicit SessionManager(RedisHashOperations& redis);
    
    // Session management
    std::string createSession(const std::string& user_id, const std::string& username,
                            const std::string& ip_address, const std::string& user_agent);
    std::optional<UserSession> getSession(const std::string& session_id);
    bool updateSessionActivity(const std::string& session_id);
    bool invalidateSession(const std::string& session_id);
    bool invalidateUserSessions(const std::string& user_id);
    
    // Session utilities
    std::vector<UserSession> getUserSessions(const std::string& user_id);
    void cleanupExpiredSessions();
    
private:
    RedisHashOperations& redis_;
    const std::string SESSION_PREFIX = "session:";
    const std::string USER_SESSIONS_PREFIX = "user_sessions:";
    const int SESSION_TTL = 3600; // 1 hour
    
    std::string sessionToJson(const UserSession& session);
    UserSession jsonToSession(const std::string& json);
};

} // namespace cache
} // namespace satox

#endif // SATOX_SESSION_MANAGER_H
```

### Wallet Balance Caching
```cpp
// cache/wallet_cache.h
#ifndef SATOX_WALLET_CACHE_H
#define SATOX_WALLET_CACHE_H

#include <string>
#include <optional>
#include "cache/redis_hash_operations.h"

namespace satox {
namespace cache {

struct WalletBalance {
    std::string wallet_id;
    std::string address;
    double balance;
    std::string asset_id;
    long long last_updated;
    int confirmations;
};

class WalletCache {
public:
    explicit WalletCache(RedisHashOperations& redis);
    
    // Balance caching
    void cacheBalance(const WalletBalance& balance);
    std::optional<WalletBalance> getCachedBalance(const std::string& wallet_id);
    void invalidateBalance(const std::string& wallet_id);
    void invalidateUserBalances(const std::string& user_id);
    
    // Batch operations
    void cacheBalances(const std::vector<WalletBalance>& balances);
    std::vector<WalletBalance> getCachedBalances(const std::vector<std::string>& wallet_ids);
    
    // Cache utilities
    bool isBalanceStale(const WalletBalance& balance, int max_age_seconds = 300);
    void cleanupStaleBalances();
    
private:
    RedisHashOperations& redis_;
    const std::string BALANCE_PREFIX = "wallet_balance:";
    const std::string USER_BALANCES_PREFIX = "user_balances:";
    const int BALANCE_TTL = 600; // 10 minutes
    
    std::string balanceToJson(const WalletBalance& balance);
    WalletBalance jsonToBalance(const std::string& json);
};

} // namespace cache
} // namespace satox

#endif // SATOX_WALLET_CACHE_H
```

### Transaction Cache
```cpp
// cache/transaction_cache.h
#ifndef SATOX_TRANSACTION_CACHE_H
#define SATOX_TRANSACTION_CACHE_H

#include <string>
#include <vector>
#include <optional>
#include "cache/redis_list_operations.h"
#include "cache/redis_hash_operations.h"

namespace satox {
namespace cache {

struct CachedTransaction {
    std::string txid;
    std::string from_address;
    std::string to_address;
    double amount;
    std::string asset_id;
    double fee;
    std::string status;
    int block_height;
    int confirmations;
    long long created_at;
};

class TransactionCache {
public:
    explicit TransactionCache(RedisListOperations& list_redis, RedisHashOperations& hash_redis);
    
    // Transaction caching
    void cacheTransaction(const CachedTransaction& transaction);
    std::optional<CachedTransaction> getCachedTransaction(const std::string& txid);
    void invalidateTransaction(const std::string& txid);
    
    // Transaction history
    void addToHistory(const std::string& address, const CachedTransaction& transaction);
    std::vector<CachedTransaction> getTransactionHistory(const std::string& address, int limit = 50);
    
    // Pending transactions
    void addToPending(const CachedTransaction& transaction);
    std::vector<CachedTransaction> getPendingTransactions(const std::string& address);
    void removeFromPending(const std::string& txid);
    
    // Cache utilities
    void cleanupOldTransactions(int max_age_hours = 24);
    
private:
    RedisListOperations& list_redis_;
    RedisHashOperations& hash_redis_;
    const std::string TRANSACTION_PREFIX = "transaction:";
    const std::string HISTORY_PREFIX = "history:";
    const std::string PENDING_PREFIX = "pending:";
    const int TRANSACTION_TTL = 3600; // 1 hour
    const int HISTORY_TTL = 86400; // 24 hours
    
    std::string transactionToJson(const CachedTransaction& transaction);
    CachedTransaction jsonToTransaction(const std::string& json);
};

} // namespace cache
} // namespace satox

#endif // SATOX_TRANSACTION_CACHE_H
```

## Pub/Sub Messaging

### Real-time Notifications
```cpp
// cache/pubsub_manager.h
#ifndef SATOX_PUBSUB_MANAGER_H
#define SATOX_PUBSUB_MANAGER_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <map>

namespace satox {
namespace cache {

enum class MessageType {
    TRANSACTION_CONFIRMED,
    BALANCE_UPDATED,
    ASSET_CREATED,
    USER_ACTIVITY,
    SYSTEM_ALERT
};

struct PubSubMessage {
    MessageType type;
    std::string channel;
    std::string data;
    long long timestamp;
};

class PubSubManager {
public:
    explicit PubSubManager(const std::string& redis_host, int redis_port, const std::string& password);
    ~PubSubManager();
    
    // Publishing
    bool publish(const std::string& channel, const std::string& message);
    bool publishTransaction(const std::string& txid, const std::string& status);
    bool publishBalanceUpdate(const std::string& wallet_id, double new_balance);
    
    // Subscribing
    bool subscribe(const std::string& channel, std::function<void(const PubSubMessage&)> callback);
    bool unsubscribe(const std::string& channel);
    
    // Pattern subscriptions
    bool psubscribe(const std::string& pattern, std::function<void(const PubSubMessage&)> callback);
    bool punsubscribe(const std::string& pattern);
    
    // Management
    void start();
    void stop();
    bool isRunning() const;
    
private:
    std::string redis_host_;
    int redis_port_;
    std::string password_;
    void* redis_context_;
    void* pubsub_context_;
    std::thread listener_thread_;
    std::atomic<bool> running_;
    std::map<std::string, std::function<void(const PubSubMessage&)>> callbacks_;
    
    void listenerLoop();
    PubSubMessage parseMessage(const std::string& channel, const std::string& message);
};

} // namespace cache
} // namespace satox

#endif // SATOX_PUBSUB_MANAGER_H
```

## Performance Optimization

### Connection Pooling
```cpp
// cache/redis_connection_pool.h
#ifndef SATOX_REDIS_CONNECTION_POOL_H
#define SATOX_REDIS_CONNECTION_POOL_H

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

namespace satox {
namespace cache {

struct RedisConfig {
    std::string host = "localhost";
    int port = 6379;
    std::string password = "redis_password";
    int database = 0;
    int timeout = 5000;
    int max_connections = 20;
    bool ssl_enabled = false;
};

class RedisConnection {
public:
    explicit RedisConnection(const RedisConfig& config);
    ~RedisConnection();
    
    bool connect();
    bool disconnect();
    bool isConnected() const;
    
    // Execute Redis commands
    void* executeCommand(const std::string& command);
    void* executeCommand(const std::string& command, const std::vector<std::string>& args);
    
private:
    RedisConfig config_;
    void* context_; // redisContext*
    bool connected_;
};

class RedisConnectionPool {
public:
    static RedisConnectionPool& getInstance();
    
    std::shared_ptr<RedisConnection> getConnection();
    void returnConnection(std::shared_ptr<RedisConnection> connection);
    
    void setConfig(const RedisConfig& config);
    void setMaxConnections(size_t max_connections);
    
private:
    RedisConnectionPool() = default;
    
    std::queue<std::shared_ptr<RedisConnection>> available_connections_;
    std::vector<std::shared_ptr<RedisConnection>> active_connections_;
    std::mutex pool_mutex_;
    std::condition_variable pool_cv_;
    RedisConfig config_;
    size_t max_connections_ = 20;
};

} // namespace cache
} // namespace satox

#endif // SATOX_REDIS_CONNECTION_POOL_H
```

### Caching Strategies
```cpp
// cache/caching_strategies.h
#ifndef SATOX_CACHING_STRATEGIES_H
#define SATOX_CACHING_STRATEGIES_H

#include <string>
#include <optional>
#include <functional>
#include "cache/redis_string_operations.h"

namespace satox {
namespace cache {

enum class CacheStrategy {
    CACHE_FIRST,      // Check cache first, fallback to source
    SOURCE_FIRST,     // Check source first, update cache
    WRITE_THROUGH,    // Write to both cache and source
    WRITE_BEHIND,     // Write to cache, sync to source later
    REFRESH_AHEAD     // Refresh cache before expiration
};

template<typename T>
class CacheManager {
public:
    explicit CacheManager(RedisStringOperations& redis);
    
    // Cache operations with strategy
    std::optional<T> get(const std::string& key, 
                        std::function<std::optional<T>()> source_fetch,
                        CacheStrategy strategy = CacheStrategy::CACHE_FIRST);
    
    bool set(const std::string& key, const T& value, int ttl_seconds = 0,
             CacheStrategy strategy = CacheStrategy::WRITE_THROUGH);
    
    bool invalidate(const std::string& key);
    
    // Batch operations
    std::vector<std::optional<T>> mget(const std::vector<std::string>& keys,
                                      std::function<std::vector<T>(const std::vector<std::string>&)> source_fetch);
    
    bool mset(const std::map<std::string, T>& key_values, int ttl_seconds = 0);
    
private:
    RedisStringOperations& redis_;
    
    std::string serialize(const T& value);
    T deserialize(const std::string& data);
};

} // namespace cache
} // namespace satox

#endif // SATOX_CACHING_STRATEGIES_H
```

## Backup and Recovery

### Automated Backup Script
```bash
#!/bin/bash
# scripts/backup_redis.sh

# Configuration
BACKUP_DIR="./backups/redis"
RETENTION_DAYS=30

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Generate backup filename
BACKUP_FILE="$BACKUP_DIR/redis_$(date +%Y%m%d_%H%M%S).rdb"

echo "Starting Redis backup..."

# Create backup using BGSAVE
docker exec satox-redis redis-cli -a redis_password BGSAVE

# Wait for backup to complete
sleep 10

# Copy backup file
docker cp satox-redis:/data/dump.rdb "$BACKUP_FILE"

if [ $? -eq 0 ]; then
    echo "Backup completed successfully: $BACKUP_FILE"
    
    # Compress backup
    gzip "$BACKUP_FILE"
    echo "Backup compressed: ${BACKUP_FILE}.gz"
    
    # Remove old backups
    find "$BACKUP_DIR" -name "*.rdb.gz" -mtime +$RETENTION_DAYS -delete
    echo "Old backups cleaned up"
else
    echo "Backup failed!"
    exit 1
fi
```

### Recovery Script
```bash
#!/bin/bash
# scripts/restore_redis.sh

if [ -z "$1" ]; then
    echo "Usage: $0 <backup_file>"
    exit 1
fi

BACKUP_FILE="$1"

if [ ! -f "$BACKUP_FILE" ]; then
    echo "Backup file not found: $BACKUP_FILE"
    exit 1
fi

echo "Starting Redis restore from: $BACKUP_FILE"

# Stop Redis
docker stop satox-redis

# Decompress if needed
if [[ "$BACKUP_FILE" == *.gz ]]; then
    echo "Decompressing backup file..."
    gunzip -c "$BACKUP_FILE" > /tmp/dump.rdb
    BACKUP_PATH="/tmp/dump.rdb"
else
    BACKUP_PATH="$BACKUP_FILE"
fi

# Copy backup to container
docker cp "$BACKUP_PATH" satox-redis:/data/dump.rdb

# Start Redis
docker start satox-redis

# Verify restore
sleep 5
if docker exec satox-redis redis-cli -a redis_password ping | grep -q "PONG"; then
    echo "Redis restore completed successfully"
else
    echo "Redis restore failed!"
    exit 1
fi
```

## Monitoring

### Health Check Script
```bash
#!/bin/bash
# scripts/check_redis_health.sh

echo "Checking Redis health..."

# Check if Redis is running
if ! docker ps | grep -q satox-redis; then
    echo "❌ Redis container is not running"
    exit 1
fi

# Check connection
if ! docker exec satox-redis redis-cli -a redis_password ping | grep -q "PONG"; then
    echo "❌ Redis is not responding"
    exit 1
fi

# Check memory usage
MEMORY_INFO=$(docker exec satox-redis redis-cli -a redis_password info memory | grep -E "used_memory_human|maxmemory_human")

echo "✅ Redis is healthy"
echo "📊 Memory usage: $MEMORY_INFO"

# Check connected clients
CLIENT_COUNT=$(docker exec satox-redis redis-cli -a redis_password info clients | grep "connected_clients" | cut -d: -f2)
echo "🔗 Connected clients: $CLIENT_COUNT"

# Check database size
DB_SIZE=$(docker exec satox-redis redis-cli -a redis_password dbsize)
echo "📋 Database size: $DB_SIZE keys"
```

### Performance Monitoring
```bash
# Monitor Redis performance
docker exec satox-redis redis-cli -a redis_password info

# Monitor memory usage
docker exec satox-redis redis-cli -a redis_password info memory

# Monitor client connections
docker exec satox-redis redis-cli -a redis_password info clients

# Monitor command statistics
docker exec satox-redis redis-cli -a redis_password info stats

# Monitor slow queries
docker exec satox-redis redis-cli -a redis_password slowlog get 10
```

## Security

### Authentication Setup
```bash
# Set Redis password
docker exec satox-redis redis-cli CONFIG SET requirepass "secure_password"

# Verify authentication
docker exec satox-redis redis-cli -a secure_password ping
```

### Network Security
```bash
# Bind to localhost only
docker exec satox-redis redis-cli CONFIG SET bind "127.0.0.1"

# Enable protected mode
docker exec satox-redis redis-cli CONFIG SET protected-mode "yes"

# Set connection limits
docker exec satox-redis redis-cli CONFIG SET maxclients "10000"
```

### SSL Configuration
```bash
# Generate SSL certificate
openssl req -new -x509 -days 365 -nodes -text -out redis.crt \
    -keyout redis.key -subj "/CN=localhost"

# Combine certificate and key
cat redis.key redis.crt > redis.pem
chmod 600 redis.pem

# Copy to container
docker cp redis.pem satox-redis:/etc/ssl/
docker exec satox-redis chown redis:redis /etc/ssl/redis.pem

# Configure SSL
docker exec satox-redis redis-cli CONFIG SET tls-port "6380"
docker exec satox-redis redis-cli CONFIG SET tls-cert-file "/etc/ssl/redis.pem"
docker exec satox-redis redis-cli CONFIG SET tls-key-file "/etc/ssl/redis.pem"
```

## Troubleshooting

### Common Issues

**Connection Refused**
```bash
# Check if Redis is running
docker ps | grep redis

# Check Redis logs
docker logs satox-redis

# Check port binding
netstat -tlnp | grep 6379
```

**Authentication Failed**
```bash
# Check Redis configuration
docker exec satox-redis redis-cli CONFIG GET requirepass

# Reset password
docker exec satox-redis redis-cli CONFIG SET requirepass "new_password"
```

**Memory Issues**
```bash
# Check memory usage
docker exec satox-redis redis-cli -a redis_password info memory

# Clear all data
docker exec satox-redis redis-cli -a redis_password FLUSHALL

# Check memory policy
docker exec satox-redis redis-cli -a redis_password CONFIG GET maxmemory-policy
```

**Performance Issues**
```bash
# Check slow queries
docker exec satox-redis redis-cli -a redis_password slowlog get 10

# Check command statistics
docker exec satox-redis redis-cli -a redis_password info stats

# Monitor real-time commands
docker exec satox-redis redis-cli -a redis_password monitor
```

## Best Practices

### 1. Memory Management
- Set appropriate maxmemory limits
- Use LRU eviction policies
- Monitor memory usage regularly
- Implement memory-efficient data structures

### 2. Performance
- Use pipelining for batch operations
- Implement connection pooling
- Use appropriate data structures
- Monitor slow queries

### 3. Security
- Enable authentication
- Use strong passwords
- Bind to localhost when possible
- Implement SSL/TLS for sensitive data

### 4. Caching
- Set appropriate TTL values
- Implement cache invalidation strategies
- Use cache warming for critical data
- Monitor cache hit rates

### 5. Backup
- Perform regular backups
- Test backup restoration
- Store backups securely
- Monitor backup success/failure

## Next Steps
- [Local Storage](local-storage.md)
- [Database Isolation](isolation.md)
- [PostgreSQL Configuration](postgresql.md)
- [MongoDB Setup](mongodb.md)

---

*Last updated: $(date)* 