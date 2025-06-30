#include <emscripten/bind.h>
#include <string>
#include <map>
#include <vector>

using namespace emscripten;

class DatabaseManager {
public:
    DatabaseManager() : initialized(false), connected(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("DatabaseManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void connect(const std::string& connectionString) {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (connected) throw std::runtime_error("Already connected to database");
        // TODO: Implement actual database connection
        connected = true;
    }

    void disconnect() {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        connected = false;
    }

    bool isConnected() const { return connected; }

    void createDatabase(const std::string& name) {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        if (databases.find(name) != databases.end()) throw std::runtime_error("Database already exists");
        databases[name] = std::map<std::string, std::string>();
    }

    void deleteDatabase(const std::string& name) {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        if (databases.find(name) == databases.end()) throw std::runtime_error("Database not found");
        databases.erase(name);
    }

    std::vector<std::string> listDatabases() const {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        std::vector<std::string> result;
        for (const auto& db : databases) result.push_back(db.first);
        return result;
    }

    void setValue(const std::string& database, const std::string& key, const std::string& value) {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        if (databases.find(database) == databases.end()) throw std::runtime_error("Database not found");
        databases[database][key] = value;
    }

    std::string getValue(const std::string& database, const std::string& key) {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        if (databases.find(database) == databases.end()) throw std::runtime_error("Database not found");
        if (databases[database].find(key) == databases[database].end()) throw std::runtime_error("Key not found");
        return databases[database][key];
    }

    void deleteValue(const std::string& database, const std::string& key) {
        if (!initialized) throw std::runtime_error("DatabaseManager not initialized");
        if (!connected) throw std::runtime_error("Not connected to database");
        if (databases.find(database) == databases.end()) throw std::runtime_error("Database not found");
        if (databases[database].find(key) == databases[database].end()) throw std::runtime_error("Key not found");
        databases[database].erase(key);
    }

private:
    bool initialized;
    bool connected;
    std::map<std::string, std::map<std::string, std::string>> databases;
};

EMSCRIPTEN_BINDINGS(DatabaseManager) {
    class_<DatabaseManager>("DatabaseManager")
        .constructor<>()
        .function("initialize", &DatabaseManager::initialize)
        .function("isInitialized", &DatabaseManager::isInitialized)
        .function("connect", &DatabaseManager::connect)
        .function("disconnect", &DatabaseManager::disconnect)
        .function("isConnected", &DatabaseManager::isConnected)
        .function("createDatabase", &DatabaseManager::createDatabase)
        .function("deleteDatabase", &DatabaseManager::deleteDatabase)
        .function("listDatabases", &DatabaseManager::listDatabases)
        .function("setValue", &DatabaseManager::setValue)
        .function("getValue", &DatabaseManager::getValue)
        .function("deleteValue", &DatabaseManager::deleteValue)
        ;
} 