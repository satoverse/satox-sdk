using System;
using System.Runtime.InteropServices;

namespace Satox.Bindings.Database
{
    [StructLayout(LayoutKind.Sequential)]
    public struct DatabaseConfig
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Host;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Port;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Database;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Username;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Password;

        public int ConnectionTimeout;
        public int CommandTimeout;
        public bool EnableSSL;
        public bool EnablePooling;
        public int PoolSize;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DatabaseOptions
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Type;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Version;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Collation;

        public long MaxSize;
        public bool EnableEncryption;
        public bool EnableCompression;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct DatabaseInfo
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Type;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Version;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Collation;

        [MarshalAs(UnmanagedType.LPStr)]
        public string Status;

        public long CurrentSize;
        public long MaxSize;
        public int TableCount;
        public int IndexCount;
        public long CreatedAt;
        public long UpdatedAt;
        public bool IsEncrypted;
        public bool IsCompressed;
    }

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

    public class DatabaseManager : ThreadSafeManager, IDatabaseManager
    {
        public DatabaseManager() : base()
        {
        }

        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_database_manager_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize database manager");

                SetInitialized(true);
            }
        }

        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_database_manager_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown database manager");

                SetInitialized(false);
            }
        }

        public void Connect(DatabaseConfig config)
        {
            EnsureInitialized();
            ValidateInput(config, nameof(config));

            try
            {
                var status = NativeMethods.satox_database_manager_connect(ref config);
                if (status != 0)
                    throw new SatoxError("Failed to connect to database");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to connect to database: {ex.Message}");
            }
        }

        public void Disconnect()
        {
            EnsureInitialized();

            try
            {
                var status = NativeMethods.satox_database_manager_disconnect();
                if (status != 0)
                    throw new SatoxError("Failed to disconnect from database");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to disconnect from database: {ex.Message}");
            }
        }

        public void CreateDatabase(string name, DatabaseOptions options)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(options, nameof(options));

            try
            {
                var status = NativeMethods.satox_database_manager_create_database(
                    name,
                    ref options
                );

                if (status != 0)
                    throw new SatoxError("Failed to create database");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to create database: {ex.Message}");
            }
        }

        public void DeleteDatabase(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                var status = NativeMethods.satox_database_manager_delete_database(name);
                if (status != 0)
                    throw new SatoxError("Failed to delete database");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to delete database: {ex.Message}");
            }
        }

        public DatabaseInfo GetDatabaseInfo(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_database_manager_get_database_info(
                    name,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get database info");

                var info = Marshal.PtrToStructure<DatabaseInfo>(infoPtr);
                NativeMethods.satox_database_manager_free_database_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get database info: {ex.Message}");
            }
        }

        public DatabaseInfo[] GetAllDatabases()
        {
            EnsureInitialized();

            try
            {
                IntPtr infosPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_database_manager_get_all_databases(
                    ref infosPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get all databases");

                var infos = new DatabaseInfo[count];
                var currentPtr = infosPtr;

                for (int i = 0; i < count; i++)
                {
                    infos[i] = Marshal.PtrToStructure<DatabaseInfo>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<DatabaseInfo>());
                }

                NativeMethods.satox_database_manager_free_database_infos(infosPtr, count);

                return infos;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get all databases: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("Database manager not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException($"{paramName} cannot be null or empty", paramName);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (IsDisposed)
                return;

            if (disposing)
            {
                if (IsInitialized)
                    Shutdown();
            }

            SetDisposed(true);
        }

        ~DatabaseManager()
        {
            Dispose(false);
        }

        internal static class NativeMethods
        {
            private const string DllName = "satox_database";

            [DllImport(DllName)]
            public static extern int satox_database_manager_initialize();

            [DllImport(DllName)]
            public static extern int satox_database_manager_shutdown();

            [DllImport(DllName)]
            public static extern int satox_database_manager_connect(ref DatabaseConfig config);

            [DllImport(DllName)]
            public static extern int satox_database_manager_disconnect();

            [DllImport(DllName)]
            public static extern int satox_database_manager_create_database(
                string name,
                ref DatabaseOptions options
            );

            [DllImport(DllName)]
            public static extern int satox_database_manager_delete_database(string name);

            [DllImport(DllName)]
            public static extern int satox_database_manager_get_database_info(
                string name,
                ref IntPtr info
            );

            [DllImport(DllName)]
            public static extern int satox_database_manager_get_all_databases(
                ref IntPtr infos,
                ref int count
            );

            [DllImport(DllName)]
            public static extern void satox_database_manager_free_database_info(IntPtr info);

            [DllImport(DllName)]
            public static extern void satox_database_manager_free_database_infos(IntPtr infos, int count);
        }
    }
}