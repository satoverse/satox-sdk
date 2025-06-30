using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Satox.Bindings.Core
{
    public interface ICoreManager : IDisposable
    {
        bool Initialize();
        bool Shutdown();
        string GetVersion();
        string GetBuildInfo();
        Task<bool> ValidateConfiguration();
        Task<bool> UpdateConfiguration();
        Task<bool> BackupData();
        Task<bool> RestoreData();
        Task<bool> PerformMaintenance();
        Task<bool> CheckHealth();
        Task<bool> ResetToDefaults();
        bool IsInitialized { get; }
    }

    public class CoreManager : ThreadSafeManager, ICoreManager
    {
        public bool Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return true;

                try
                {
                    var result = NativeMethods.satox_core_manager_initialize();
                    SetInitialized(result);
                    return result;
                }
                catch (Exception ex)
                {
                    throw new SatoxError($"Failed to initialize core manager: {ex.Message}");
                }
            }
        }

        public bool Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return true;

                try
                {
                    var result = NativeMethods.satox_core_manager_shutdown();
                    SetInitialized(!result);
                    return result;
                }
                catch (Exception ex)
                {
                    throw new SatoxError($"Failed to shutdown core manager: {ex.Message}");
                }
            }
        }

        public string GetVersion()
        {
            EnsureInitialized();

            try
            {
                IntPtr versionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_core_manager_get_version(ref versionPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get version");

                var version = Marshal.PtrToStringAnsi(versionPtr);
                NativeMethods.satox_core_manager_free_buffer(versionPtr);

                return version;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get version: {ex.Message}");
            }
        }

        public string GetBuildInfo()
        {
            EnsureInitialized();

            try
            {
                IntPtr buildInfoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_core_manager_get_build_info(ref buildInfoPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get build info");

                var buildInfo = Marshal.PtrToStringAnsi(buildInfoPtr);
                NativeMethods.satox_core_manager_free_buffer(buildInfoPtr);

                return buildInfo;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get build info: {ex.Message}");
            }
        }

        public async Task<bool> ValidateConfiguration()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_validate_configuration();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate configuration: {ex.Message}");
            }
        }

        public async Task<bool> UpdateConfiguration()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_update_configuration();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to update configuration: {ex.Message}");
            }
        }

        public async Task<bool> BackupData()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_backup_data();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to backup data: {ex.Message}");
            }
        }

        public async Task<bool> RestoreData()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_restore_data();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to restore data: {ex.Message}");
            }
        }

        public async Task<bool> PerformMaintenance()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_perform_maintenance();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to perform maintenance: {ex.Message}");
            }
        }

        public async Task<bool> CheckHealth()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_check_health();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check health: {ex.Message}");
            }
        }

        public async Task<bool> ResetToDefaults()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_core_manager_reset_to_defaults();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to reset to defaults: {ex.Message}");
            }
        }

        protected override void DisposeCore()
        {
            if (IsInitialized)
                Shutdown();
        }

        internal static class NativeMethods
        {
            private const string DllName = "satox_core";

            [DllImport(DllName)]
            public static extern bool satox_core_manager_initialize();

            [DllImport(DllName)]
            public static extern bool satox_core_manager_shutdown();

            [DllImport(DllName)]
            public static extern int satox_core_manager_get_version(ref IntPtr version);

            [DllImport(DllName)]
            public static extern int satox_core_manager_get_build_info(ref IntPtr buildInfo);

            [DllImport(DllName)]
            public static extern int satox_core_manager_validate_configuration();

            [DllImport(DllName)]
            public static extern int satox_core_manager_update_configuration();

            [DllImport(DllName)]
            public static extern int satox_core_manager_backup_data();

            [DllImport(DllName)]
            public static extern int satox_core_manager_restore_data();

            [DllImport(DllName)]
            public static extern int satox_core_manager_perform_maintenance();

            [DllImport(DllName)]
            public static extern int satox_core_manager_check_health();

            [DllImport(DllName)]
            public static extern int satox_core_manager_reset_to_defaults();

            [DllImport(DllName)]
            public static extern void satox_core_manager_free_buffer(IntPtr buffer);
        }
    }
}