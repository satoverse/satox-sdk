using System;
using System.Runtime.InteropServices;
using Satox.Bindings.Database;

namespace Satox.Database
{
    /// <summary>
    /// DatabaseManager provides database management functionality.
    /// </summary>
    public interface IDatabaseManager : IDisposable
    {
        void Initialize();
        void Shutdown();
        void Connect(DatabaseConfig config);
        void Disconnect();
        void CreateDatabase(string name, DatabaseOptions options);
        void DeleteDatabase(string name);
        DatabaseInfo GetDatabaseInfo(string name);
        DatabaseInfo[] GetAllDatabases();
    }

    public class DatabaseManager : IDatabaseManager
    {
        private readonly Satox.Bindings.Database.DatabaseManager _manager;

        /// <summary>
        /// Creates a new instance of DatabaseManager.
        /// </summary>
        public DatabaseManager()
        {
            _manager = new Satox.Bindings.Database.DatabaseManager();
        }

        /// <summary>
        /// Initializes the database manager.
        /// </summary>
        public void Initialize()
        {
            _manager.Initialize();
        }

        /// <summary>
        /// Shuts down the database manager.
        /// </summary>
        public void Shutdown()
        {
            _manager.Shutdown();
        }

        /// <summary>
        /// Connects to a database.
        /// </summary>
        /// <param name="config">The connection configuration.</param>
        public void Connect(DatabaseConfig config)
        {
            _manager.Connect(config);
        }

        /// <summary>
        /// Disconnects from the database.
        /// </summary>
        public void Disconnect()
        {
            _manager.Disconnect();
        }

        /// <summary>
        /// Creates a database.
        /// </summary>
        /// <param name="name">The database name.</param>
        /// <param name="options">The creation options.</param>
        public void CreateDatabase(string name, DatabaseOptions options)
        {
            _manager.CreateDatabase(name, options);
        }

        /// <summary>
        /// Deletes a database.
        /// </summary>
        /// <param name="name">The database name.</param>
        public void DeleteDatabase(string name)
        {
            _manager.DeleteDatabase(name);
        }

        /// <summary>
        /// Gets database information.
        /// </summary>
        /// <param name="name">The database name.</param>
        /// <returns>The database information.</returns>
        public DatabaseInfo GetDatabaseInfo(string name)
        {
            return _manager.GetDatabaseInfo(name);
        }

        /// <summary>
        /// Gets all databases.
        /// </summary>
        /// <returns>An array of database information.</returns>
        public DatabaseInfo[] GetAllDatabases()
        {
            return _manager.GetAllDatabases();
        }

        public void Dispose()
        {
            _manager.Dispose();
        }
    }

    /// <summary>
    /// Represents database configuration.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct DatabaseConfig
    {
        /// <summary>
        /// The connection string.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ConnectionString;

        /// <summary>
        /// The username.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Username;

        /// <summary>
        /// The password.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Password;

        /// <summary>
        /// The connection timeout in seconds.
        /// </summary>
        public int ConnectionTimeout;

        /// <summary>
        /// The command timeout in seconds.
        /// </summary>
        public int CommandTimeout;

        /// <summary>
        /// Whether to enable SSL.
        /// </summary>
        public bool EnableSSL;

        /// <summary>
        /// Whether to enable pooling.
        /// </summary>
        public bool EnablePooling;

        /// <summary>
        /// The pool size.
        /// </summary>
        public int PoolSize;
    }

    /// <summary>
    /// Represents database options.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct DatabaseOptions
    {
        /// <summary>
        /// The database type.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Type;

        /// <summary>
        /// The database version.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Version;

        /// <summary>
        /// The character set.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string CharacterSet;

        /// <summary>
        /// The collation.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Collation;

        /// <summary>
        /// The maximum size in bytes.
        /// </summary>
        public long MaxSize;

        /// <summary>
        /// Whether to enable encryption.
        /// </summary>
        public bool EnableEncryption;

        /// <summary>
        /// Whether to enable compression.
        /// </summary>
        public bool EnableCompression;
    }

    /// <summary>
    /// Represents database information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct DatabaseInfo
    {
        /// <summary>
        /// The database name.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        /// <summary>
        /// The database type.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Type;

        /// <summary>
        /// The database version.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Version;

        /// <summary>
        /// The character set.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string CharacterSet;

        /// <summary>
        /// The collation.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Collation;

        /// <summary>
        /// The current size in bytes.
        /// </summary>
        public long CurrentSize;

        /// <summary>
        /// The maximum size in bytes.
        /// </summary>
        public long MaxSize;

        /// <summary>
        /// The number of tables.
        /// </summary>
        public int TableCount;

        /// <summary>
        /// The number of indexes.
        /// </summary>
        public int IndexCount;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long UpdatedAt;

        /// <summary>
        /// Whether encryption is enabled.
        /// </summary>
        public bool IsEncrypted;

        /// <summary>
        /// Whether compression is enabled.
        /// </summary>
        public bool IsCompressed;
    }
} 