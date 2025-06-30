package com.satox.bindings;

import java.util.List;
import java.util.Map;
import java.util.Date;

/**
 * Represents supported database types
 */
public enum DatabaseType {
    SQLITE,
    POSTGRESQL,
    MYSQL,
    REDIS,
    MONGODB,
    SUPABASE,
    FIREBASE,
    AWS,
    AZURE,
    GOOGLE_CLOUD
}

/**
 * Represents asset types
 */
public enum AssetType {
    TOKEN,
    NFT,
    CURRENCY
}

/**
 * Represents network types
 */
public enum NetworkType {
    MAINNET,
    TESTNET,
    REGTEST
}

/**
 * Represents a database query result
 */
public class QueryResult {
    private boolean success;
    private List<Map<String, Object>> rows;
    private String error;
    private int affectedRows;
    private Integer lastInsertId;

    public QueryResult() {
        this.rows = new java.util.ArrayList<>();
    }

    public QueryResult(boolean success, List<Map<String, Object>> rows, String error) {
        this.success = success;
        this.rows = rows != null ? rows : new java.util.ArrayList<>();
        this.error = error;
    }

    public boolean isSuccess() { return success; }
    public void setSuccess(boolean success) { this.success = success; }

    public List<Map<String, Object>> getRows() { return rows; }
    public void setRows(List<Map<String, Object>> rows) { this.rows = rows; }

    public String getError() { return error; }
    public void setError(String error) { this.error = error; }

    public int getAffectedRows() { return affectedRows; }
    public void setAffectedRows(int affectedRows) { this.affectedRows = affectedRows; }

    public Integer getLastInsertId() { return lastInsertId; }
    public void setLastInsertId(Integer lastInsertId) { this.lastInsertId = lastInsertId; }
}

/**
 * Represents database configuration
 */
public class DatabaseConfig {
    private DatabaseType type;
    private String host;
    private Integer port;
    private String database;
    private String username;
    private String password;
    private String connectionString;
    private int maxConnections = 10;
    private int timeout = 30;

    public DatabaseConfig() {}

    public DatabaseConfig(DatabaseType type) {
        this.type = type;
    }

    public DatabaseType getType() { return type; }
    public void setType(DatabaseType type) { this.type = type; }

    public String getHost() { return host; }
    public void setHost(String host) { this.host = host; }

    public Integer getPort() { return port; }
    public void setPort(Integer port) { this.port = port; }

    public String getDatabase() { return database; }
    public void setDatabase(String database) { this.database = database; }

    public String getUsername() { return username; }
    public void setUsername(String username) { this.username = username; }

    public String getPassword() { return password; }
    public void setPassword(String password) { this.password = password; }

    public String getConnectionString() { return connectionString; }
    public void setConnectionString(String connectionString) { this.connectionString = connectionString; }

    public int getMaxConnections() { return maxConnections; }
    public void setMaxConnections(int maxConnections) { this.maxConnections = maxConnections; }

    public int getTimeout() { return timeout; }
    public void setTimeout(int timeout) { this.timeout = timeout; }
}

/**
 * Represents a Satox SDK error
 */
public class SatoxError extends Exception {
    private String code;

    public SatoxError(String message) {
        super(message);
        this.code = "UNKNOWN_ERROR";
    }

    public SatoxError(String message, String code) {
        super(message);
        this.code = code;
    }

    public String getCode() { return code; }
    public void setCode(String code) { this.code = code; }
} 