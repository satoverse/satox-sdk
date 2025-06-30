package com.satox.core;

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
    FIREBASE
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
 * Represents core configuration
 */
public class CoreConfig {
    private String name = "satox_sdk";
    private boolean enableLogging = true;
    private String logLevel = "info";
    private String logPath;
    private int maxThreads = 4;
    private int timeout = 30;

    public CoreConfig() {}

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public boolean isEnableLogging() { return enableLogging; }
    public void setEnableLogging(boolean enableLogging) { this.enableLogging = enableLogging; }

    public String getLogLevel() { return logLevel; }
    public void setLogLevel(String logLevel) { this.logLevel = logLevel; }

    public String getLogPath() { return logPath; }
    public void setLogPath(String logPath) { this.logPath = logPath; }

    public int getMaxThreads() { return maxThreads; }
    public void setMaxThreads(int maxThreads) { this.maxThreads = maxThreads; }

    public int getTimeout() { return timeout; }
    public void setTimeout(int timeout) { this.timeout = timeout; }
}

/**
 * Represents core status
 */
public class CoreStatus {
    private boolean initialized;
    private String version;
    private double uptime;
    private Map<String, Object> memoryUsage;
    private int threadCount;

    public CoreStatus() {}

    public boolean isInitialized() { return initialized; }
    public void setInitialized(boolean initialized) { this.initialized = initialized; }

    public String getVersion() { return version; }
    public void setVersion(String version) { this.version = version; }

    public double getUptime() { return uptime; }
    public void setUptime(double uptime) { this.uptime = uptime; }

    public Map<String, Object> getMemoryUsage() { return memoryUsage; }
    public void setMemoryUsage(Map<String, Object> memoryUsage) { this.memoryUsage = memoryUsage; }

    public int getThreadCount() { return threadCount; }
    public void setThreadCount(int threadCount) { this.threadCount = threadCount; }
}

/**
 * Represents an asset
 */
public class Asset {
    private String id;
    private String name;
    private String symbol;
    private AssetType type;
    private int decimals;
    private long totalSupply;
    private Map<String, Object> metadata;

    public Asset() {}

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getSymbol() { return symbol; }
    public void setSymbol(String symbol) { this.symbol = symbol; }

    public AssetType getType() { return type; }
    public void setType(AssetType type) { this.type = type; }

    public int getDecimals() { return decimals; }
    public void setDecimals(int decimals) { this.decimals = decimals; }

    public long getTotalSupply() { return totalSupply; }
    public void setTotalSupply(long totalSupply) { this.totalSupply = totalSupply; }

    public Map<String, Object> getMetadata() { return metadata; }
    public void setMetadata(Map<String, Object> metadata) { this.metadata = metadata; }
}

/**
 * Represents an NFT
 */
public class NFT {
    private String id;
    private String name;
    private String description;
    private String imageUrl;
    private Map<String, Object> metadata;
    private String owner;
    private Date createdAt;

    public NFT() {}

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getDescription() { return description; }
    public void setDescription(String description) { this.description = description; }

    public String getImageUrl() { return imageUrl; }
    public void setImageUrl(String imageUrl) { this.imageUrl = imageUrl; }

    public Map<String, Object> getMetadata() { return metadata; }
    public void setMetadata(Map<String, Object> metadata) { this.metadata = metadata; }

    public String getOwner() { return owner; }
    public void setOwner(String owner) { this.owner = owner; }

    public Date getCreatedAt() { return createdAt; }
    public void setCreatedAt(Date createdAt) { this.createdAt = createdAt; }
}

/**
 * Represents a wallet
 */
public class Wallet {
    private String address;
    private Map<String, Double> balance;
    private List<Asset> assets;
    private List<NFT> nfts;
    private Date createdAt;
    private Date lastUpdated;

    public Wallet() {
        this.balance = new java.util.HashMap<>();
        this.assets = new java.util.ArrayList<>();
        this.nfts = new java.util.ArrayList<>();
    }

    public String getAddress() { return address; }
    public void setAddress(String address) { this.address = address; }

    public Map<String, Double> getBalance() { return balance; }
    public void setBalance(Map<String, Double> balance) { this.balance = balance; }

    public List<Asset> getAssets() { return assets; }
    public void setAssets(List<Asset> assets) { this.assets = assets; }

    public List<NFT> getNfts() { return nfts; }
    public void setNfts(List<NFT> nfts) { this.nfts = nfts; }

    public Date getCreatedAt() { return createdAt; }
    public void setCreatedAt(Date createdAt) { this.createdAt = createdAt; }

    public Date getLastUpdated() { return lastUpdated; }
    public void setLastUpdated(Date lastUpdated) { this.lastUpdated = lastUpdated; }
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