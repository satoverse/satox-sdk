# Configuration Guide - Satox Core SDK

This guide explains how to configure the Satox Core SDK for different environments, use cases, and performance requirements. You'll learn about all available settings and how to optimize them for your specific needs.

## Table of Contents

- [Basic Configuration](#basic-configuration)
- [Network Configuration](#network-configuration)
- [Security Configuration](#security-configuration)
- [Performance Configuration](#performance-configuration)
- [Database Configuration](#database-configuration)
- [Logging Configuration](#logging-configuration)
- [Environment-Specific Configurations](#environment-specific-configurations)
- [Configuration Validation](#configuration-validation)

## Basic Configuration

### Core Settings

The core configuration controls the fundamental behavior of the SDK:

```cpp
#include <satox/core/config.hpp>

satox::CoreConfig config;

// Basic settings
config.data_dir = "./satox_data";           // Data storage directory
config.log_level = "info";                  // Logging level
config.environment = "production";          // Environment (development/production)
config.debug_mode = false;                  // Debug mode
config.auto_save = true;                    // Auto-save settings
config.max_memory_usage = "2GB";           // Maximum memory usage
```

### Configuration File

You can also use a JSON configuration file:

```json
{
    "core": {
        "data_dir": "./satox_data",
        "log_level": "info",
        "environment": "production",
        "debug_mode": false,
        "auto_save": true,
        "max_memory_usage": "2GB"
    }
}
```

Load the configuration:

```cpp
// Load from file
auto config = satox::CoreConfig::loadFromFile("config.json");

// Or create programmatically
satox::CoreConfig config;
config.loadFromFile("config.json");
```

## Network Configuration

### Blockchain Network Settings

Configure which blockchain network to connect to:

```cpp
// Network configuration
config.network.name = "mainnet";            // Network name
config.network.rpc_url = "https://rpc.satox.com";
config.network.rpc_username = "your_username";
config.network.rpc_password = "your_password";
config.network.port = 8332;                 // RPC port
config.network.ssl_enabled = true;          // Enable SSL
config.network.timeout = 30;                // Connection timeout (seconds)
config.network.retry_attempts = 3;          // Retry attempts
config.network.retry_delay = 5;             // Delay between retries (seconds)
```

### Network Options

| Network | RPC URL | Purpose |
|---------|---------|---------|
| **mainnet** | https://rpc.satox.com | Production network |
| **testnet** | https://testnet-rpc.satox.com | Testing network |
| **regtest** | http://localhost:18443 | Local development |
| **devnet** | https://devnet-rpc.satox.com | Development network |

### Connection Pooling

For high-performance applications, configure connection pooling:

```cpp
// Connection pool settings
config.network.connection_pool.min_connections = 5;
config.network.connection_pool.max_connections = 50;
config.network.connection_pool.connection_timeout = 30;
config.network.connection_pool.idle_timeout = 300;
config.network.connection_pool.keep_alive = true;
```

## Security Configuration

### Quantum Security Settings

Enable and configure quantum-resistant cryptography:

```cpp
// Quantum security configuration
config.quantum_security.enabled = true;
config.quantum_security.algorithm = "kyber-1024";  // Algorithm to use
config.quantum_security.hybrid_mode = true;        // Use hybrid mode
config.quantum_security.key_size = "256";          // Key size in bits
config.quantum_security.auto_key_rotation = true;  // Auto key rotation
config.quantum_security.rotation_interval = 30;    // Days between rotations
```

### Available Algorithms

| Algorithm | Type | Security Level | Performance |
|-----------|------|----------------|-------------|
| **kyber-512** | Key Exchange | 128-bit | Fast |
| **kyber-768** | Key Exchange | 192-bit | Balanced |
| **kyber-1024** | Key Exchange | 256-bit | Secure |
| **dilithium2** | Signature | 128-bit | Fast |
| **dilithium3** | Signature | 192-bit | Balanced |
| **dilithium5** | Signature | 256-bit | Secure |

### Authentication Settings

Configure user authentication:

```cpp
// Authentication configuration
config.auth.enabled = true;
config.auth.method = "jwt";                 // Authentication method
config.auth.jwt_secret = "your_jwt_secret";
config.auth.jwt_expiry = 3600;              // Token expiry (seconds)
config.auth.max_login_attempts = 5;         // Max login attempts
config.auth.lockout_duration = 300;         // Lockout duration (seconds)
config.auth.password_min_length = 8;        // Minimum password length
config.auth.require_special_chars = true;   // Require special characters
```

### Encryption Settings

Configure data encryption:

```cpp
// Encryption configuration
config.encryption.enabled = true;
config.encryption.algorithm = "aes-256-gcm"; // Encryption algorithm
config.encryption.key_derivation = "pbkdf2"; // Key derivation function
config.encryption.iterations = 100000;       // PBKDF2 iterations
config.encryption.salt_length = 32;          // Salt length in bytes
```

## Performance Configuration

### Memory Management

Configure memory usage and optimization:

```cpp
// Memory configuration
config.performance.max_memory_usage = "4GB";        // Maximum memory usage
config.performance.cache_size = "1GB";              // Cache size
config.performance.gc_interval = 300;               // Garbage collection interval
config.performance.optimize_memory = true;          // Enable memory optimization
config.performance.use_memory_mapping = true;       // Use memory mapping for large files
```

### Threading Configuration

Configure multi-threading behavior:

```cpp
// Threading configuration
config.performance.max_threads = 8;                 // Maximum number of threads
config.performance.thread_pool_size = 16;           // Thread pool size
config.performance.async_operations = true;         // Enable async operations
config.performance.parallel_processing = true;      // Enable parallel processing
config.performance.work_stealing = true;            // Enable work stealing
```

### Caching Configuration

Configure caching for better performance:

```cpp
// Caching configuration
config.cache.enabled = true;
config.cache.max_size = "2GB";                      // Maximum cache size
config.cache.ttl = 3600;                            // Time to live (seconds)
config.cache.eviction_policy = "lru";               // Eviction policy
config.cache.persistent = true;                     // Persistent cache
config.cache.compression = true;                    // Enable compression
```

## Database Configuration

### PostgreSQL Settings

Configure PostgreSQL database connection:

```cpp
// PostgreSQL configuration
config.database.postgresql.enabled = true;
config.database.postgresql.host = "localhost";
config.database.postgresql.port = 5432;
config.database.postgresql.database = "satox_sdk";
config.database.postgresql.username = "satox_user";
config.database.postgresql.password = "secure_password";
config.database.postgresql.ssl_mode = "require";
config.database.postgresql.max_connections = 100;
config.database.postgresql.connection_timeout = 30;
```

### Redis Settings

Configure Redis caching:

```cpp
// Redis configuration
config.database.redis.enabled = true;
config.database.redis.host = "localhost";
config.database.redis.port = 6379;
config.database.redis.password = "redis_password";
config.database.redis.database = 0;
config.database.redis.ssl_enabled = false;
config.database.redis.connection_timeout = 30;
config.database.redis.max_connections = 50;
```

### MongoDB Settings

Configure MongoDB document storage:

```cpp
// MongoDB configuration
config.database.mongodb.enabled = true;
config.database.mongodb.uri = "mongodb://localhost:27017";
config.database.mongodb.database = "satox_sdk";
config.database.mongodb.username = "admin";
config.database.mongodb.password = "secure_password";
config.database.mongodb.ssl_enabled = false;
config.database.mongodb.max_connections = 50;
config.database.mongodb.connection_timeout = 30;
```

### Local Storage Settings

Configure local file storage:

```cpp
// Local storage configuration
config.storage.local.enabled = true;
config.storage.local.data_dir = "./data";
config.storage.local.backup_dir = "./backups";
config.storage.local.max_file_size = "1GB";
config.storage.local.compression = true;
config.storage.local.encryption = true;
```

## Logging Configuration

### Log Levels and Output

Configure logging behavior:

```cpp
// Logging configuration
config.logging.level = "info";                      // Log level
config.logging.output = "file";                     // Output destination
config.logging.file_path = "./logs/sdk.log";        // Log file path
config.logging.max_file_size = "100MB";             // Maximum log file size
config.logging.max_files = 10;                      // Maximum number of log files
config.logging.format = "json";                     // Log format
config.logging.include_timestamp = true;            // Include timestamps
config.logging.include_thread_id = true;            // Include thread ID
config.logging.include_process_id = true;           // Include process ID
```

### Log Levels

| Level | Description | Use Case |
|-------|-------------|----------|
| **trace** | Most detailed logging | Development debugging |
| **debug** | Detailed debugging info | Development |
| **info** | General information | Production monitoring |
| **warn** | Warning messages | Production monitoring |
| **error** | Error messages | Error tracking |
| **fatal** | Fatal errors | Critical error tracking |

### Log Rotation

Configure automatic log rotation:

```cpp
// Log rotation configuration
config.logging.rotation.enabled = true;
config.logging.rotation.max_size = "100MB";         // Maximum file size
config.logging.rotation.max_files = 10;             // Maximum number of files
config.logging.rotation.interval = "daily";         // Rotation interval
config.logging.rotation.compress = true;            // Compress old logs
config.logging.rotation.backup_count = 30;          // Number of backups to keep
```

## Environment-Specific Configurations

### Development Environment

```cpp
// Development configuration
config.environment = "development";
config.debug_mode = true;
config.log_level = "debug";
config.performance.max_threads = 4;
config.cache.enabled = false;                        // Disable caching for development
config.quantum_security.enabled = false;             // Disable quantum security for faster development
```

### Production Environment

```cpp
// Production configuration
config.environment = "production";
config.debug_mode = false;
config.log_level = "warn";
config.performance.max_threads = 16;
config.cache.enabled = true;
config.quantum_security.enabled = true;
config.logging.rotation.enabled = true;
```

### Testing Environment

```cpp
// Testing configuration
config.environment = "testing";
config.debug_mode = true;
config.log_level = "error";                         // Only log errors during tests
config.performance.max_threads = 2;
config.cache.enabled = false;
config.database.postgresql.database = "satox_sdk_test";
config.database.redis.database = 1;
```

## Configuration Validation

### Validate Configuration

Before using the configuration, validate it:

```cpp
// Validate configuration
try {
    config.validate();
    std::cout << "✅ Configuration is valid" << std::endl;
} catch (const satox::ConfigError& e) {
    std::cerr << "❌ Configuration error: " << e.what() << std::endl;
    return 1;
}
```

### Configuration Schema

The SDK validates configuration against a schema:

```json
{
    "type": "object",
    "properties": {
        "core": {
            "type": "object",
            "properties": {
                "data_dir": {"type": "string", "minLength": 1},
                "log_level": {"enum": ["trace", "debug", "info", "warn", "error", "fatal"]},
                "environment": {"enum": ["development", "production", "testing"]}
            },
            "required": ["data_dir", "log_level", "environment"]
        },
        "network": {
            "type": "object",
            "properties": {
                "name": {"enum": ["mainnet", "testnet", "regtest", "devnet"]},
                "rpc_url": {"type": "string", "format": "uri"},
                "timeout": {"type": "integer", "minimum": 1, "maximum": 300}
            },
            "required": ["name", "rpc_url"]
        }
    },
    "required": ["core", "network"]
}
```

### Environment Variables

You can override configuration with environment variables:

```bash
# Set environment variables
export SATOX_DATA_DIR="/custom/data/dir"
export SATOX_LOG_LEVEL="debug"
export SATOX_NETWORK="testnet"
export SATOX_RPC_URL="https://testnet-rpc.satox.com"
export SATOX_QUANTUM_SECURITY_ENABLED="true"
export SATOX_MAX_MEMORY_USAGE="4GB"
```

### Configuration Precedence

Configuration values are loaded in this order (highest to lowest priority):

1. **Environment variables** (highest priority)
2. **Command line arguments**
3. **Configuration file**
4. **Default values** (lowest priority)

## Advanced Configuration

### Custom Plugins

Configure custom plugins and extensions:

```cpp
// Plugin configuration
config.plugins.enabled = true;
config.plugins.directory = "./plugins";
config.plugins.auto_load = true;
config.plugins.required_plugins = {"security", "monitoring"};
config.plugins.optional_plugins = {"analytics", "backup"};
```

### Monitoring Configuration

Configure monitoring and metrics:

```cpp
// Monitoring configuration
config.monitoring.enabled = true;
config.monitoring.metrics_port = 9090;
config.monitoring.health_check_interval = 30;
config.monitoring.alerting.enabled = true;
config.monitoring.alerting.webhook_url = "https://hooks.slack.com/...";
config.monitoring.alerting.email = "alerts@yourcompany.com";
```

### Backup Configuration

Configure automatic backups:

```cpp
// Backup configuration
config.backup.enabled = true;
config.backup.schedule = "0 2 * * *";               // Daily at 2 AM
config.backup.directory = "./backups";
config.backup.retention_days = 30;
config.backup.compression = true;
config.backup.encryption = true;
config.backup.include_logs = true;
config.backup.include_config = true;
```

## Configuration Examples

### Minimal Configuration

```cpp
// Minimal configuration for basic usage
satox::CoreConfig config;
config.data_dir = "./data";
config.log_level = "info";
config.network.name = "mainnet";
config.network.rpc_url = "https://rpc.satox.com";
```

### High-Performance Configuration

```cpp
// High-performance configuration for production
satox::CoreConfig config;
config.data_dir = "/opt/satox/data";
config.log_level = "warn";
config.environment = "production";
config.performance.max_threads = 16;
config.performance.max_memory_usage = "8GB";
config.cache.enabled = true;
config.cache.max_size = "4GB";
config.quantum_security.enabled = true;
config.network.connection_pool.max_connections = 100;
```

### Development Configuration

```cpp
// Development configuration for debugging
satox::CoreConfig config;
config.data_dir = "./dev_data";
config.log_level = "debug";
config.debug_mode = true;
config.environment = "development";
config.performance.max_threads = 4;
config.cache.enabled = false;
config.quantum_security.enabled = false;
```

## Troubleshooting Configuration

### Common Configuration Issues

**Problem**: "Invalid configuration"
- **Solution**: Check the configuration schema and required fields
- **Solution**: Validate configuration before use

**Problem**: "Network connection failed"
- **Solution**: Verify RPC URL and credentials
- **Solution**: Check network timeout settings

**Problem**: "Memory allocation failed"
- **Solution**: Reduce max_memory_usage setting
- **Solution**: Check available system memory

**Problem**: "Database connection failed"
- **Solution**: Verify database credentials and connection settings
- **Solution**: Check database server status

### Configuration Validation Script

Create a script to validate your configuration:

```bash
#!/bin/bash
# validate_config.sh

echo "🔍 Validating Satox SDK configuration..."

# Check configuration file
if [[ -f "config.json" ]]; then
    echo "✅ Configuration file found"
    
    # Validate JSON syntax
    if python3 -m json.tool config.json > /dev/null 2>&1; then
        echo "✅ JSON syntax is valid"
    else
        echo "❌ JSON syntax is invalid"
        exit 1
    fi
    
    # Check required fields
    required_fields=("data_dir" "log_level" "network")
    for field in "${required_fields[@]}"; do
        if jq -e ".core.$field" config.json > /dev/null 2>&1; then
            echo "✅ Required field '$field' is present"
        else
            echo "❌ Required field '$field' is missing"
            exit 1
        fi
    done
else
    echo "⚠️  No configuration file found, using defaults"
fi

echo "✅ Configuration validation complete"
```

---

*Need help with configuration? Check our [troubleshooting guide](../troubleshooting/common-issues.md) or [contact support](https://support.satox.com).*

*Last updated: $(date)* 