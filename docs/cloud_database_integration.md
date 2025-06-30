# Satox SDK Cloud Database Integration

## Overview

The Satox SDK now includes comprehensive support for cloud database services, specifically **Supabase** and **Firebase**. This integration provides a unified interface for working with both traditional and cloud-based databases, enabling developers to build scalable applications with real-time capabilities.

## Features

### 🚀 Core Features
- **Unified Database Interface**: Single API for all database types
- **Cloud Database Support**: Supabase and Firebase integration
- **Real-time Capabilities**: WebSocket-based real-time subscriptions
- **Multi-language Bindings**: C++, TypeScript, Python, Go, Java, C#
- **Transaction Support**: ACID-compliant transactions
- **Connection Pooling**: Efficient connection management
- **Error Handling**: Comprehensive error handling and recovery
- **Security**: Built-in authentication and authorization

### 🔧 Technical Features
- **HTTP/HTTPS Support**: Secure communication with cloud services
- **JSON API Integration**: Native JSON support for cloud databases
- **Authentication**: OAuth2, JWT, and API key support
- **Rate Limiting**: Built-in rate limiting and retry logic
- **Logging**: Comprehensive logging and monitoring
- **Configuration Management**: Flexible configuration system

## Architecture

### Database Manager Integration

The cloud database integration extends the existing `DatabaseManager` class to support cloud services:

```cpp
enum class DatabaseType {
    SQLITE,
    POSTGRESQL,
    MYSQL,
    REDIS,
    MONGODB,
    SUPABASE,  // Cloud database
    FIREBASE   // Cloud database
};
```

### Cloud Database Managers

Each cloud service has its own specialized manager:

- **SupabaseManager**: Handles Supabase PostgreSQL operations
- **FirebaseManager**: Handles Firebase Firestore and Realtime Database

### Connection Flow

1. **Configuration Loading**: Load service-specific configuration
2. **Authentication**: Authenticate with cloud service
3. **Connection Establishment**: Establish HTTP/WebSocket connections
4. **Session Management**: Manage connection lifecycle
5. **Query Execution**: Execute queries and handle responses

## Installation

### Prerequisites

- C++17 compatible compiler
- CMake 3.16+
- CURL library
- OpenSSL
- spdlog
- fmt
- nlohmann/json

### Quick Start

```bash
# Clone the repository
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk

# Run the build script
./scripts/build_cloud_databases.sh --all
```

### Manual Installation

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libcurl4-openssl-dev \
    libssl-dev \
    libspdlog-dev \
    libfmt-dev \
    nlohmann-json3-dev

# Build the SDK
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
make -j$(nproc)
sudo make install
```

## Configuration

### Supabase Configuration

Create `config/cloud/supabase_config.json`:

```json
{
  "url": "https://your-project.supabase.co",
  "anon_key": "your-anon-key",
  "service_role_key": "your-service-role-key",
  "database_url": "postgresql://postgres:[YOUR-PASSWORD]@db.[YOUR-PROJECT-REF].supabase.co:5432/postgres",
  "enable_realtime": true,
  "connection_timeout": 30,
  "auth_scheme": "bearer",
  "enable_ssl": true,
  "schema": "public",
  "max_connections": 10,
  "enable_logging": true,
  "log_level": "info"
}
```

### Firebase Configuration

Create `config/cloud/firebase_config.json`:

```json
{
  "project_id": "your-firebase-project-id",
  "private_key_id": "your-private-key-id",
  "private_key": "-----BEGIN PRIVATE KEY-----\nYOUR_PRIVATE_KEY_HERE\n-----END PRIVATE KEY-----\n",
  "client_email": "firebase-adminsdk-xxxxx@your-project.iam.gserviceaccount.com",
  "client_id": "your-client-id",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-xxxxx%40your-project.iam.gserviceaccount.com",
  "database_url": "https://your-project.firebaseio.com",
  "enable_auth": true,
  "enable_firestore": true,
  "enable_realtime_db": false,
  "enable_storage": false,
  "enable_functions": false,
  "connection_timeout": 30,
  "max_connections": 10,
  "enable_ssl": true,
  "enable_logging": true,
  "log_level": "info"
}
```

## Usage Examples

### C++ Examples

#### Supabase Integration

```cpp
#include "satox/core/database_manager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;
using satox::core::DatabaseManager;
using satox::core::DatabaseType;

int main() {
    // Initialize database manager
    DatabaseManager db_manager;
    
    // Load configuration
    std::ifstream config_file("config/cloud/supabase_config.json");
    json config_json;
    config_file >> config_json;
    
    // Connect to Supabase
    std::string connection_id = db_manager.connect(DatabaseType::SUPABASE, config_json);
    
    // Execute query
    std::string query = "SELECT * FROM users WHERE age >= 18";
    json result;
    if (db_manager.executeQuery(connection_id, query, result)) {
        std::cout << "Query successful: " << result.dump(2) << std::endl;
    }
    
    // Disconnect
    db_manager.disconnect(connection_id);
    return 0;
}
```

#### Firebase Integration

```cpp
#include "satox/core/database_manager.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using satox::core::DatabaseManager;
using satox::core::DatabaseType;

int main() {
    DatabaseManager db_manager;
    
    // Load Firebase configuration
    std::ifstream config_file("config/cloud/firebase_config.json");
    json config_json;
    config_file >> config_json;
    
    // Connect to Firebase
    std::string connection_id = db_manager.connect(DatabaseType::FIREBASE, config_json);
    
    // Firestore operation
    json firestore_query = {
        {"operation", "set"},
        {"collection", "users"},
        {"document_id", "user123"},
        {"data", {
            {"name", "John Doe"},
            {"email", "john@example.com"},
            {"age", 30}
        }}
    };
    
    json result;
    if (db_manager.executeQuery(connection_id, firestore_query.dump(), result)) {
        std::cout << "Document created successfully" << std::endl;
    }
    
    db_manager.disconnect(connection_id);
    return 0;
}
```

### TypeScript Examples

```typescript
import { DatabaseManager, DatabaseType } from 'satox-sdk';

async function main() {
    const dbManager = new DatabaseManager();
    
    // Load configuration
    const config = require('./config/cloud/supabase_config.json');
    
    // Connect to Supabase
    const connectionId = await dbManager.connect(DatabaseType.SUPABASE, config);
    
    // Execute query
    const query = "SELECT * FROM users WHERE age >= 18";
    const result = await dbManager.executeQuery(connectionId, query);
    
    if (result.success) {
        console.log('Query successful:', result.rows);
    }
    
    // Disconnect
    await dbManager.disconnect(connectionId);
}

main().catch(console.error);
```

### Python Examples

```python
from satox_sdk import DatabaseManager, DatabaseType
import json

def main():
    db_manager = DatabaseManager()
    
    # Load configuration
    with open('config/cloud/supabase_config.json', 'r') as f:
        config = json.load(f)
    
    # Connect to Supabase
    connection_id = db_manager.connect(DatabaseType.SUPABASE, config)
    
    # Execute query
    query = "SELECT * FROM users WHERE age >= 18"
    result = db_manager.execute_query(connection_id, query)
    
    if result.success:
        print("Query successful:", result.rows)
    
    # Disconnect
    db_manager.disconnect(connection_id)

if __name__ == "__main__":
    main()
```

## Real-time Features

### Supabase Real-time Subscriptions

```cpp
// Subscribe to table changes
json subscription_config = {
    {"table", "users"},
    {"event", "INSERT"},
    {"schema", "public"}
};

std::string subscription_id = db_manager.subscribeToChanges(
    connection_id, 
    subscription_config,
    [](const json& data) {
        std::cout << "New user inserted: " << data.dump(2) << std::endl;
    }
);

// Unsubscribe when done
db_manager.unsubscribeFromChanges(connection_id, subscription_id);
```

### Firebase Real-time Listening

```cpp
// Listen to path changes
json listener_config = {
    {"path", "/users/user123"},
    {"event", "value"}
};

std::string listener_id = db_manager.listenToPath(
    connection_id,
    listener_config,
    [](const json& data) {
        std::cout << "Data changed: " << data.dump(2) << std::endl;
    }
);

// Stop listening when done
db_manager.stopListening(connection_id, listener_id);
```

## Security

### Authentication

The cloud database integration supports multiple authentication methods:

- **API Keys**: Simple key-based authentication
- **JWT Tokens**: JSON Web Token authentication
- **OAuth2**: OAuth2 flow for user authentication
- **Service Accounts**: Service account authentication (Firebase)

### Authorization

- **Row Level Security (RLS)**: Supabase RLS policies
- **Firestore Security Rules**: Firebase security rules
- **Role-based Access Control**: Database-level permissions

### Data Encryption

- **TLS/SSL**: All communications are encrypted
- **At-rest Encryption**: Cloud provider encryption
- **Client-side Encryption**: Optional client-side encryption

## Performance

### Connection Pooling

The SDK implements connection pooling for efficient resource management:

```cpp
// Configure connection pool
json pool_config = {
    {"max_connections", 10},
    {"min_connections", 2},
    {"connection_timeout", 30},
    {"idle_timeout", 300}
};

db_manager.configureConnectionPool(connection_id, pool_config);
```

### Query Optimization

- **Prepared Statements**: Reusable query templates
- **Batch Operations**: Bulk insert/update operations
- **Connection Reuse**: Efficient connection management
- **Caching**: Query result caching

### Monitoring

```cpp
// Get performance metrics
json metrics = db_manager.getPerformanceMetrics(connection_id);
std::cout << "Active connections: " << metrics["active_connections"] << std::endl;
std::cout << "Query count: " << metrics["query_count"] << std::endl;
std::cout << "Average response time: " << metrics["avg_response_time"] << "ms" << std::endl;
```

## Error Handling

### Comprehensive Error Handling

```cpp
try {
    json result = db_manager.executeQuery(connection_id, query);
    if (!result["success"]) {
        std::cerr << "Query failed: " << result["error"] << std::endl;
        std::cerr << "Error code: " << result["error_code"] << std::endl;
        std::cerr << "Error details: " << result["error_details"] << std::endl;
    }
} catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
}
```

### Error Recovery

- **Automatic Retry**: Configurable retry logic
- **Circuit Breaker**: Prevents cascading failures
- **Fallback Mechanisms**: Alternative data sources
- **Graceful Degradation**: Reduced functionality mode

## Testing

### Unit Tests

```bash
# Run unit tests
cd build
./tests/unit/test_cloud_integration
```

### Integration Tests

```bash
# Run integration tests
cd build
./tests/integration/test_cloud_integration
```

### Performance Tests

```bash
# Run performance benchmarks
cd build
./tests/performance/test_cloud_performance
```

## Monitoring and Logging

### Logging Configuration

```cpp
// Configure logging
json log_config = {
    {"level", "info"},
    {"file", "logs/cloud_database.log"},
    {"max_size", "10MB"},
    {"max_files", 5},
    {"enable_console", true}
};

db_manager.configureLogging(log_config);
```

### Metrics Collection

- **Query Performance**: Response times and throughput
- **Connection Metrics**: Connection pool statistics
- **Error Rates**: Error frequency and types
- **Resource Usage**: Memory and CPU utilization

## Best Practices

### Configuration Management

1. **Environment Variables**: Use environment variables for sensitive data
2. **Configuration Validation**: Validate configuration before use
3. **Default Values**: Provide sensible defaults
4. **Documentation**: Document all configuration options

### Connection Management

1. **Connection Pooling**: Use connection pooling for efficiency
2. **Connection Limits**: Set appropriate connection limits
3. **Timeout Configuration**: Configure appropriate timeouts
4. **Connection Monitoring**: Monitor connection health

### Query Optimization

1. **Prepared Statements**: Use prepared statements for repeated queries
2. **Batch Operations**: Use batch operations for bulk data
3. **Indexing**: Ensure proper database indexing
4. **Query Analysis**: Analyze query performance

### Security

1. **Authentication**: Use strong authentication methods
2. **Authorization**: Implement proper access controls
3. **Data Validation**: Validate all input data
4. **Encryption**: Use encryption for sensitive data

## Troubleshooting

### Common Issues

#### Connection Failures

```bash
# Check network connectivity
curl -I https://your-project.supabase.co

# Check SSL certificates
openssl s_client -connect your-project.supabase.co:443
```

#### Authentication Errors

```cpp
// Verify credentials
json auth_test = {
    {"operation", "auth_test"},
    {"credentials", config}
};

json result = db_manager.executeQuery(connection_id, auth_test.dump());
if (!result["success"]) {
    std::cerr << "Authentication failed: " << result["error"] << std::endl;
}
```

#### Performance Issues

```cpp
// Enable detailed logging
json debug_config = {
    {"log_level", "debug"},
    {"enable_query_logging", true},
    {"enable_performance_logging", true}
};

db_manager.configureLogging(debug_config);
```

### Debug Mode

```bash
# Build with debug information
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSATOX_ENABLE_DEBUG=ON
make -j$(nproc)

# Run with debug output
./examples/cloud/supabase_example --debug
```

## Migration Guide

### From Traditional Databases

1. **Schema Migration**: Migrate existing schemas to cloud format
2. **Data Migration**: Transfer data to cloud databases
3. **Application Updates**: Update application code to use cloud APIs
4. **Testing**: Thoroughly test migrated applications

### From Other Cloud Services

1. **API Mapping**: Map existing APIs to Satox SDK APIs
2. **Configuration Updates**: Update configuration files
3. **Feature Parity**: Ensure feature parity with existing solutions
4. **Performance Testing**: Test performance with new implementation

## Community and Support

### Getting Help

- **Documentation**: Comprehensive documentation available
- **Examples**: Extensive example code provided
- **Tests**: Unit and integration tests for reference
- **Community**: Active community support

### Contributing

1. **Fork the Repository**: Fork the Satox SDK repository
2. **Create Feature Branch**: Create a branch for your feature
3. **Implement Changes**: Implement your changes
4. **Add Tests**: Add tests for your changes
5. **Submit Pull Request**: Submit a pull request

### Reporting Issues

When reporting issues, please include:

- **SDK Version**: Version of the SDK being used
- **Platform**: Operating system and architecture
- **Configuration**: Relevant configuration details
- **Error Messages**: Complete error messages and logs
- **Steps to Reproduce**: Detailed steps to reproduce the issue

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Supabase Team**: For the excellent Supabase platform
- **Firebase Team**: For the comprehensive Firebase platform
- **Open Source Community**: For the libraries and tools used in this project

---

**Last Updated**: $(date '+%Y-%m-%d %H:%M:%S')
**SDK Version**: 1.0.0
**Cloud Database Support**: Supabase, Firebase
