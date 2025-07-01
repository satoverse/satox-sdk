package com.satox.core;

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