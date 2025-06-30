using System;
using System.Runtime.InteropServices;

namespace Satox.API
{
    public class EndpointManager : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        public EndpointManager()
        {
            _initialized = false;
            _disposed = false;
        }

        public void Initialize()
        {
            if (_initialized)
                return;

            var result = NativeMethods.satox_endpoint_manager_initialize();
            if (result != 0)
                throw new SatoxError("Failed to initialize endpoint manager");

            _initialized = true;
        }

        public void Shutdown()
        {
            if (!_initialized)
                return;

            var result = NativeMethods.satox_endpoint_manager_shutdown();
            if (result != 0)
                throw new SatoxError("Failed to shutdown endpoint manager");

            _initialized = false;
        }

        public void RegisterEndpoint(string path, string method, EndpointHandler handler)
        {
            EnsureInitialized();
            ValidateInput(path, method, handler);

            var result = NativeMethods.satox_endpoint_manager_register_endpoint(path, method, handler);
            if (result != 0)
                throw new SatoxError("Failed to register endpoint");
        }

        public void UnregisterEndpoint(string path, string method)
        {
            EnsureInitialized();
            ValidateInput(path, method);

            var result = NativeMethods.satox_endpoint_manager_unregister_endpoint(path, method);
            if (result != 0)
                throw new SatoxError("Failed to unregister endpoint");
        }

        public EndpointInfo GetEndpointInfo(string path, string method)
        {
            EnsureInitialized();
            ValidateInput(path, method);

            var info = new EndpointInfo();
            var result = NativeMethods.satox_endpoint_manager_get_endpoint_info(path, method, out info);
            if (result != 0)
                throw new SatoxError("Failed to get endpoint info");

            return info;
        }

        public EndpointInfo[] GetAllEndpoints()
        {
            EnsureInitialized();

            var count = 0;
            var result = NativeMethods.satox_endpoint_manager_get_endpoint_count(out count);
            if (result != 0)
                throw new SatoxError("Failed to get endpoint count");

            var endpoints = new EndpointInfo[count];
            result = NativeMethods.satox_endpoint_manager_get_all_endpoints(endpoints, count);
            if (result != 0)
                throw new SatoxError("Failed to get all endpoints");

            return endpoints;
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Endpoint manager not initialized");
        }

        private void ValidateInput(string path, string method, EndpointHandler handler)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentException("Path cannot be null or empty", nameof(path));
            if (string.IsNullOrEmpty(method))
                throw new ArgumentException("Method cannot be null or empty", nameof(method));
            if (handler == null)
                throw new ArgumentNullException(nameof(handler));
        }

        private void ValidateInput(string path, string method)
        {
            if (string.IsNullOrEmpty(path))
                throw new ArgumentException("Path cannot be null or empty", nameof(path));
            if (string.IsNullOrEmpty(method))
                throw new ArgumentException("Method cannot be null or empty", nameof(method));
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_disposed)
                return;

            if (disposing)
            {
                if (_initialized)
                    Shutdown();
            }

            _disposed = true;
        }

        ~EndpointManager()
        {
            Dispose(false);
        }
    }

    public delegate int EndpointHandler(IntPtr request, IntPtr response);

    [StructLayout(LayoutKind.Sequential)]
    public struct EndpointInfo
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Path;
        [MarshalAs(UnmanagedType.LPStr)]
        public string Method;
        public bool IsActive;
        public int RequestCount;
        public int ErrorCount;
        [MarshalAs(UnmanagedType.LPStr)]
        public string LastError;
    }

    internal static class NativeMethods
    {
        [DllImport("satox")]
        public static extern int satox_endpoint_manager_initialize();

        [DllImport("satox")]
        public static extern int satox_endpoint_manager_shutdown();

        [DllImport("satox")]
        public static extern int satox_endpoint_manager_register_endpoint(string path, string method, EndpointHandler handler);

        [DllImport("satox")]
        public static extern int satox_endpoint_manager_unregister_endpoint(string path, string method);

        [DllImport("satox")]
        public static extern int satox_endpoint_manager_get_endpoint_info(string path, string method, out EndpointInfo info);

        [DllImport("satox")]
        public static extern int satox_endpoint_manager_get_endpoint_count(out int count);

        [DllImport("satox")]
        public static extern int satox_endpoint_manager_get_all_endpoints([Out] EndpointInfo[] endpoints, int count);
    }
} 