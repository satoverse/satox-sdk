using System;
using System.Collections.Generic;

namespace Satox
{
    /// <summary>
    /// Represents supported database types
    /// </summary>
    public enum DatabaseType
    {
        SQLite,
        PostgreSQL,
        MySQL,
        Redis,
        MongoDB,
        Supabase,
        Firebase,
        AWS,
        Azure,
        GoogleCloud
    }

    /// <summary>
    /// Represents asset types
    /// </summary>
    public enum AssetType
    {
        Token,
        NFT,
        Currency
    }

    /// <summary>
    /// Represents network types
    /// </summary>
    public enum NetworkType
    {
        Mainnet,
        Testnet,
        Regtest
    }

    /// <summary>
    /// Represents a database query result
    /// </summary>
    public class QueryResult
    {
        public bool Success { get; set; }
        public List<Dictionary<string, object>> Rows { get; set; }
        public string Error { get; set; }
        public int AffectedRows { get; set; }
        public int? LastInsertId { get; set; }
    }

    /// <summary>
    /// Represents database configuration
    /// </summary>
    public class DatabaseConfig
    {
        public DatabaseType Type { get; set; }
        public string Host { get; set; }
        public int? Port { get; set; }
        public string Database { get; set; }
        public string Username { get; set; }
        public string Password { get; set; }
        public string ConnectionString { get; set; }
        public int MaxConnections { get; set; } = 10;
        public int Timeout { get; set; } = 30;
    }

    /// <summary>
    /// Represents core configuration
    /// </summary>
    public class CoreConfig
    {
        public string Name { get; set; } = "satox_sdk";
        public bool EnableLogging { get; set; } = true;
        public string LogLevel { get; set; } = "info";
        public string LogPath { get; set; }
        public int MaxThreads { get; set; } = 4;
        public int Timeout { get; set; } = 30;
    }

    /// <summary>
    /// Represents core status
    /// </summary>
    public class CoreStatus
    {
        public bool Initialized { get; set; }
        public string Version { get; set; }
        public double Uptime { get; set; }
        public Dictionary<string, object> MemoryUsage { get; set; }
        public int ThreadCount { get; set; }
    }

    /// <summary>
    /// Represents an asset
    /// </summary>
    public class Asset
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Symbol { get; set; }
        public AssetType Type { get; set; }
        public int Decimals { get; set; }
        public long TotalSupply { get; set; }
        public Dictionary<string, object> Metadata { get; set; }
    }

    /// <summary>
    /// Represents an NFT
    /// </summary>
    public class NFT
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Description { get; set; }
        public string ImageUrl { get; set; }
        public Dictionary<string, object> Metadata { get; set; }
        public string Owner { get; set; }
        public DateTime CreatedAt { get; set; }
    }

    /// <summary>
    /// Represents a wallet
    /// </summary>
    public class Wallet
    {
        public string Address { get; set; }
        public Dictionary<string, double> Balance { get; set; }
        public List<Asset> Assets { get; set; }
        public List<NFT> NFTs { get; set; }
        public DateTime CreatedAt { get; set; }
        public DateTime LastUpdated { get; set; }
    }

    /// <summary>
    /// Represents a Satox SDK error
    /// </summary>
    public class SatoxError : Exception
    {
        public string Code { get; }

        public SatoxError(string message, string code = "UNKNOWN_ERROR") : base(message)
        {
            Code = code;
        }
    }
} 