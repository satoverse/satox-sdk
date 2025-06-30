# Satox Database Component

## Overview
The Satox Database Component provides a unified database interface for the Satox SDK, supporting multiple database backends with a consistent API. This component follows the Satox SDK development templates and provides comprehensive database management capabilities.

## Features
- **Multi-Database Support**: In-memory database with extensible backend support
- **Transaction Management**: ACID-compliant transaction support
- **Connection Pooling**: Efficient connection management
- **Health Monitoring**: Comprehensive health checks and status reporting
- **Structured Logging**: Detailed logging with spdlog integration
- **Error Handling**: Standardized error codes and exception handling
- **Statistics**: Performance metrics and operation tracking
- **Backup & Restore**: Database backup and restoration capabilities
- **Index Management**: Automatic and manual index creation
- **Callback System**: Event-driven architecture with callback support

## Architecture
The Database Component follows the unified architecture principle:
- **C++ Core**: All database logic implemented in C++
- **Singleton Pattern**: Single instance per application
- **Thread-Safe**: All operations are thread-safe with proper mutex protection
- **Template Compliance**: Follows all established development templates

## Usage

### Basic Initialization
```cpp
#include "satox/database/database_manager.hpp"

// Get the singleton instance
auto& manager = satox::database::DatabaseManager::getInstance();

// Configure the database
satox::database::DatabaseConfig config;
config.name = "my_database";
config.enableLogging = true;
config.logPath = "logs/components/database/";
config.maxConnections = 10;
config.connectionTimeout = 5000;

// Initialize the manager
if (!manager.initialize(config)) {
    std::cerr << "Failed to initialize: " << manager.getLastError() << std::endl;
    return 1;
}
```

### Database Operations
```cpp
// Create a database
manager.createDatabase("my_db");

// Use a database
manager.useDatabase("my_db");

// List databases
auto databases = manager.listDatabases();
```

### Table Operations
```cpp
// Create a table with schema
nlohmann::json schema = {
    {"id", "string"},
    {"name", "string"},
    {"age", "integer"},
    {"active", "boolean"}
};

manager.createTable("users", schema);

// Insert data
nlohmann::json user = {
    {"id", "user1"},
    {"name", "John Doe"},
    {"age", 30},
    {"active", true}
};

manager.insert("users", user);
```

### Transaction Management
```cpp
// Begin transaction
manager.beginTransaction();

try {
    // Perform operations
    manager.insert("users", user1);
    manager.insert("users", user2);
    
    // Commit transaction
    manager.commitTransaction();
} catch (...) {
    // Rollback on error
    manager.rollbackTransaction();
}
```

### Health Monitoring
```cpp
// Perform health check
if (manager.performHealthCheck()) {
    std::cout << "Database is healthy" << std::endl;
} else {
    std::cout << "Database health check failed" << std::endl;
}

// Get health status
auto health = manager.getHealth();
std::cout << "Health: " << health.message << std::endl;
```

### Callback Registration
```cpp
// Register database operation callback
manager.setDatabaseCallback([](const std::string& operation, bool success, const std::string& error) {
    if (success) {
        std::cout << "Operation " << operation << " succeeded" << std::endl;
    } else {
        std::cerr << "Operation " << operation << " failed: " << error << std::endl;
    }
});

// Register health callback
manager.setHealthCallback([](const satox::database::DatabaseHealth& health) {
    std::cout << "Health status: " << health.message << std::endl;
});
```

## Building

### Prerequisites
- C++17 compatible compiler
- CMake 3.14 or higher
- nlohmann/json library
- spdlog library

### Build Instructions
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the component
make

# Run tests
make test

# Install (optional)
make install
```

### Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install nlohmann-json3-dev libspdlog-dev

# macOS
brew install nlohmann-json spdlog

# CentOS/RHEL
sudo yum install nlohmann-json-devel spdlog-devel
```

## Testing
The component includes comprehensive test coverage:
- Unit tests for all public APIs
- Integration tests for database operations
- Performance tests for benchmarking
- Concurrency tests for thread safety

```bash
# Run all tests
make test

# Run specific test
./tests/database_manager_test
./tests/database_manager_comprehensive_test
```

## Logging
The component uses structured logging with spdlog:
- **Log Location**: `logs/components/database/`
- **Log Files**: 
  - `database.log` - Main application log
  - `database_operations.log` - Operation audit trail
- **Log Levels**: DEBUG, INFO, WARN, ERROR
- **Log Format**: `[timestamp] [level] [thread] [component] message`

## Error Handling
The component uses standardized error codes:
- `SUCCESS` - Operation completed successfully
- `INVALID_ARGUMENT` - Invalid input parameters
- `NOT_INITIALIZED` - Component not initialized
- `OPERATION_FAILED` - General operation failure
- `VALIDATION_ERROR` - Data validation failed
- `TIMEOUT_ERROR` - Operation timed out

## Performance
The component includes performance monitoring:
- Operation timing statistics
- Average operation time tracking
- Slow operation detection (>1 second)
- Memory usage monitoring
- Connection pool statistics

## Security
- Input validation for all operations
- SQL injection prevention (when SQL backends are implemented)
- Secure connection handling
- Audit logging for all operations

## ⚠️ Limitations

### **Blockchain Compatibility**

- **No Smart Contract Support:** Satox is based on Ravencoin and does **not** support smart contracts or EVM compatibility
- **Asset-Based:** Focuses on asset creation, management, and transfer rather than programmable contracts
- **UTXO Model:** Uses Unspent Transaction Output (UTXO) model like Bitcoin/Ravencoin, not account-based like Ethereum

### **Platform Support**

- **Desktop Focus:** Primarily designed for desktop/server environments
- **Mobile Limitations:** Mobile SDK provides limited functionality compared to core SDK
- **Browser Support:** WebAssembly bindings have performance limitations for complex operations

### **Network Requirements**

- **Internet Connection:** Requires active internet connection for blockchain operations
- **Node Synchronization:** Full node operations require significant storage and bandwidth
- **API Dependencies:** Some features depend on external API services

### **Development Considerations**

- **Learning Curve:** Ravencoin/UTXO model differs from Ethereum/smart contract development
- **Testing Environment:** Testnet required for development and testing
- **Asset Management:** Asset creation and management have specific requirements and limitations


## Contributing
When contributing to this component:
1. Follow the established development templates
2. Ensure all tests pass
3. Add appropriate logging
4. Update documentation
5. Follow the error handling patterns
6. Maintain thread safety

## License
MIT License - Copyright(c) 2025 Satoxcoin Core Developer
