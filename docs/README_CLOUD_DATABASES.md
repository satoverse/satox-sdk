# Cloud Database Integration: Supabase & Firebase

## 🚀 Quick Start

The Satox SDK now includes comprehensive support for cloud databases, specifically **Supabase** and **Firebase**. This integration provides seamless access to modern cloud database services while maintaining the same unified API across all database types.

### Prerequisites

- C++17 compatible compiler
- CMake 3.16+
- libcurl4-openssl-dev (Ubuntu/Debian) or libcurl-devel (CentOS/RHEL)
- OpenSSL development libraries

### Installation

```bash
# Clone the repository
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk

# Build with cloud database support
./scripts/build_cloud_databases.sh --all
```

### Quick Example

```cpp
#include "satox/core/database_manager.hpp"
#include <nlohmann/json.hpp>

int main() {
    // Initialize database manager
    satox::core::DatabaseManager db_manager;
    
    // Load Supabase configuration
    std::ifstream config_file("config/cloud/supabase_config.json");
    nlohmann::json config;
    config_file >> config;
    
    // Connect to Supabase
    std::string connection_id = db_manager.connect(
        satox::core::DatabaseType::SUPABASE, 
        config
    );
    
    // Execute a query
    nlohmann::json result;
    db_manager.executeQuery(connection_id, "SELECT * FROM users", result);
    
    // Disconnect
    db_manager.disconnect(connection_id);
    
    return 0;
}
```

## 📋 Features

### ✅ Supabase Support
- **PostgreSQL-based cloud database**
- **Real-time subscriptions** with WebSocket support
- **Row Level Security (RLS)** integration
- **Built-in authentication** support
- **Auto-generated APIs** and real-time subscriptions
- **Database backups** and point-in-time recovery

### ✅ Firebase Support
- **Firestore** (NoSQL document database)
- **Realtime Database** (NoSQL JSON database)
- **Cloud Storage** integration
- **Cloud Functions** support
- **Offline support** and real-time synchronization
- **Automatic scaling** and global distribution

### ✅ Unified API
- **Same interface** for all database types
- **Connection pooling** and management
- **Transaction support** across all databases
- **Error handling** and recovery
- **Performance monitoring** and logging

### ✅ Multi-Language Support
- **C++** (core implementation)
- **TypeScript/JavaScript** (Node.js and browser)
- **Python** (async support)
- **Go** (coming soon)
- **C#** (coming soon)
- **Rust** (coming soon)

## 🏗️ Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Application   │    │  DatabaseManager │    │  Cloud Manager  │
│                 │───▶│                  │───▶│                 │
│  (C++/TS/Py/Go) │    │   (Unified API)  │    │ (Supabase/Fire) │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                │
                                ▼
                       ┌─────────────────┐
                       │   Connection    │
                       │     Pool        │
                       └─────────────────┘
```

## 🔧 Configuration

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

## 📖 Usage Examples

### C++ Examples

#### Supabase Example

```cpp
#include "satox/core/database_manager.hpp"
#include <nlohmann/json.hpp>

int main() {
    satox::core::DatabaseManager db_manager;
    
    // Load configuration
    std::ifstream config_file("config/cloud/supabase_config.json");
    nlohmann::json config;
    config_file >> config;
    
    // Connect
    std::string connection_id = db_manager.connect(
        satox::core::DatabaseType::SUPABASE, 
        config
    );
    
    // Create table
    nlohmann::json result;
    db_manager.executeQuery(connection_id, 
        "CREATE TABLE IF NOT EXISTS users (id SERIAL PRIMARY KEY, name TEXT)", 
        result
    );
    
    // Insert data
    db_manager.executeQuery(connection_id,
        "INSERT INTO users (name) VALUES ('John Doe')",
        result
    );
    
    // Query data
    db_manager.executeQuery(connection_id,
        "SELECT * FROM users",
        result
    );
    
    // Disconnect
    db_manager.disconnect(connection_id);
    
    return 0;
}
```

#### Firebase Example

```cpp
#include "satox/core/database_manager.hpp"
#include <nlohmann/json.hpp>

int main() {
    satox::core::DatabaseManager db_manager;
    
    // Load configuration
    std::ifstream config_file("config/cloud/firebase_config.json");
    nlohmann::json config;
    config_file >> config;
    
    // Connect
    std::string connection_id = db_manager.connect(
        satox::core::DatabaseType::FIREBASE, 
        config
    );
    
    // Firestore operations
    nlohmann::json result;
    
    // Create document
    nlohmann::json create_query = {
        {"operation", "set"},
        {"collection", "users"},
        {"document_id", "user123"},
        {"data", {{"name", "John Doe"}, {"email", "john@example.com"}}}
    };
    
    db_manager.executeQuery(connection_id, create_query.dump(), result);
    
    // Get document
    nlohmann::json get_query = {
        {"operation", "get"},
        {"collection", "users"},
        {"document_id", "user123"}
    };
    
    db_manager.executeQuery(connection_id, get_query.dump(), result);
    
    // Disconnect
    db_manager.disconnect(connection_id);
    
    return 0;
}
```

### TypeScript Examples

#### Supabase Example

```typescript
import { DatabaseManager, DatabaseType } from 'satox-sdk';

async function main() {
    const dbManager = new DatabaseManager();
    
    // Load configuration
    const config = require('./config/cloud/supabase_config.json');
    
    // Connect
    const connectionId = await dbManager.connect(DatabaseType.SUPABASE, config);
    
    // Execute queries
    const result = await dbManager.executeQuery(connectionId, 'SELECT * FROM users');
    
    // Disconnect
    await dbManager.disconnect(connectionId);
}

main().catch(console.error);
```

#### Firebase Example

```typescript
import { DatabaseManager, DatabaseType } from 'satox-sdk';

async function main() {
    const dbManager = new DatabaseManager();
    
    // Load configuration
    const config = require('./config/cloud/firebase_config.json');
    
    // Connect
    const connectionId = await dbManager.connect(DatabaseType.FIREBASE, config);
    
    // Firestore operations
    const createQuery = JSON.stringify({
        operation: 'set',
        collection: 'users',
        document_id: 'user123',
        data: { name: 'John Doe', email: 'john@example.com' }
    });
    
    const result = await dbManager.executeQuery(connectionId, createQuery);
    
    // Disconnect
    await dbManager.disconnect(connectionId);
}

main().catch(console.error);
```

### Python Examples

#### Supabase Example

```python
import json
import asyncio
from satox_bindings.database import DatabaseManager, DatabaseType

async def main():
    db_manager = DatabaseManager()
    
    # Load configuration
    with open('config/cloud/supabase_config.json', 'r') as f:
        config = json.load(f)
    
    # Connect
    connection_id = await db_manager.connect(DatabaseType.SUPABASE, config)
    
    # Execute queries
    result = await db_manager.execute_query(connection_id, 'SELECT * FROM users')
    
    # Disconnect
    await db_manager.disconnect(connection_id)

asyncio.run(main())
```

#### Firebase Example

```python
import json
import asyncio
from satox_bindings.database import DatabaseManager, DatabaseType

async def main():
    db_manager = DatabaseManager()
    
    # Load configuration
    with open('config/cloud/firebase_config.json', 'r') as f:
        config = json.load(f)
    
    # Connect
    connection_id = await db_manager.connect(DatabaseType.FIREBASE, config)
    
    # Firestore operations
    create_query = json.dumps({
        'operation': 'set',
        'collection': 'users',
        'document_id': 'user123',
        'data': {'name': 'John Doe', 'email': 'john@example.com'}
    })
    
    result = await db_manager.execute_query(connection_id, create_query)
    
    # Disconnect
    await db_manager.disconnect(connection_id)

asyncio.run(main())
```

## 🔄 Real-time Features

### Supabase Real-time Subscriptions

```cpp
// Subscribe to table changes
std::string subscription_id = supabase_manager.subscribe(
    connection_id,
    "users",
    [](const nlohmann::json& data) {
        std::cout << "Real-time update: " << data.dump() << std::endl;
    }
);

// Unsubscribe
supabase_manager.unsubscribe(connection_id, subscription_id);
```

### Firebase Real-time Listening

```cpp
// Listen to path changes
std::string listener_id = firebase_manager.realtime().listen(
    connection_id,
    "/users/user123",
    [](const nlohmann::json& data) {
        std::cout << "Real-time update: " << data.dump() << std::endl;
    }
);

// Stop listening
firebase_manager.realtime().stopListening(connection_id, listener_id);
```

## �� Security Features

### Supabase Row Level Security (RLS)

```sql
-- Enable RLS on a table
ALTER TABLE users ENABLE ROW LEVEL SECURITY;

-- Create policy
CREATE POLICY "Users can view own data" ON users
    FOR SELECT USING (auth.uid() = user_id);
```

### Firebase Security Rules

```javascript
// Firestore security rules
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    match /users/{userId} {
      allow read, write: if request.auth != null && request.auth.uid == userId;
    }
  }
}
```

## ⚡ Performance Optimization

### Connection Pooling

```cpp
// Configure connection pool
config.max_connections = 20;
config.connection_timeout = 60;
```

### Caching

```cpp
// Example caching strategy
std::unordered_map<std::string, nlohmann::json> cache;

auto getCachedData = [&](const std::string& key) -> std::optional<nlohmann::json> {
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    return std::nullopt;
};
```

## 🛠️ Building and Testing

### Build Options

```bash
# Full build with all features
./scripts/build_cloud_databases.sh --all

# Build only
./scripts/build_cloud_databases.sh --build

# Run tests only
./scripts/build_cloud_databases.sh --test

# Build examples only
./scripts/build_cloud_databases.sh --examples
```

### Testing

```bash
# Run all cloud database tests
cd build
make test

# Run specific tests
./tests/cloud/test_supabase_manager
./tests/cloud/test_firebase_manager
./tests/cloud/test_cloud_integration
```

## 📊 Monitoring and Logging

### Enable Logging

```cpp
// Enable detailed logging
config.enable_logging = true;
config.log_level = "debug";
```

### Performance Monitoring

```cpp
// Monitor query performance
auto start = std::chrono::high_resolution_clock::now();
db_manager.executeQuery(connection_id, query, result);
auto end = std::chrono::high_resolution_clock::now();

auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Query took: " << duration.count() << "ms" << std::endl;
```

## 🚨 Error Handling

### Connection Errors

```cpp
std::string connection_id = db_manager.connect(DatabaseType::SUPABASE, config);
if (connection_id.empty()) {
    std::cerr << "Connection failed: " << db_manager.getLastError() << std::endl;
    return;
}
```

### Query Errors

```cpp
nlohmann::json result;
if (!db_manager.executeQuery(connection_id, query, result)) {
    std::cerr << "Query failed: " << db_manager.getLastError() << std::endl;
    return;
}
```

### Retry Logic

```cpp
auto executeWithRetry = [&](const std::string& query, int max_retries = 3) {
    for (int i = 0; i < max_retries; ++i) {
        nlohmann::json result;
        if (db_manager.executeQuery(connection_id, query, result)) {
            return result;
        }
        
        if (i < max_retries - 1) {
            std::this_thread::sleep_for(std::chrono::seconds(1 << i)); // Exponential backoff
        }
    }
    return nlohmann::json();
};
```

## 📚 Best Practices

### 1. Connection Management
- Always disconnect connections when done
- Use connection pooling for high-traffic applications
- Implement connection health checks

### 2. Query Optimization
- Use prepared statements for repeated queries
- Implement proper indexing
- Use transactions for related operations

### 3. Security
- Never commit credentials to version control
- Use environment variables for sensitive data
- Implement proper authentication and authorization

### 4. Error Handling
- Always check for errors after operations
- Implement retry logic for transient failures
- Log errors for debugging

### 5. Performance
- Use connection pooling
- Implement caching where appropriate
- Monitor query performance

## 🔧 Troubleshooting

### Common Issues

1. **Connection Timeout**
   - Increase `connection_timeout` in configuration
   - Check network connectivity
   - Verify firewall settings

2. **Authentication Errors**
   - Verify API keys and credentials
   - Check token expiration
   - Ensure proper permissions

3. **Query Errors**
   - Verify SQL syntax
   - Check table and column names
   - Ensure proper permissions

4. **Real-time Issues**
   - Verify WebSocket connectivity
   - Check subscription permissions
   - Monitor network stability

### Debug Mode

Enable debug logging for troubleshooting:

```cpp
config.enable_logging = true;
config.log_level = "debug";
```

## 📈 Migration Guide

### From Local Databases

1. **Export Data**: Export existing data from local database
2. **Create Cloud Database**: Set up Supabase or Firebase project
3. **Import Data**: Import data to cloud database
4. **Update Configuration**: Update application configuration
5. **Test**: Verify all operations work correctly
6. **Deploy**: Deploy updated application

### From Other Cloud Providers

1. **Export Data**: Export data from existing cloud provider
2. **Transform Data**: Transform data to match new schema
3. **Import Data**: Import data to Supabase or Firebase
4. **Update Code**: Update application code to use new API
5. **Test**: Verify all operations work correctly
6. **Deploy**: Deploy updated application

## 🤝 Community

- **GitHub**: https://github.com/satoverse/
- **Discord**: Join our Discord server https://discord.com/invite/GFZYFuuHVq
- **X(Twitter)**: https://x.com/Satoverse_io
- **Website**: https://www.satoverse.io

## 📄 License

This cloud database integration is part of the Satox SDK and is licensed under the MIT License.

## 🆘 Support

For issues and questions:

- **Documentation**: Check this guide and API documentation
- **Examples**: Review example code in `examples/cloud/`
- **Issues**: Report issues on GitHub
- **Community**: Join our Discord server

---

**Happy coding with Satox SDK Cloud Database Integration! 🚀**
