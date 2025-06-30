using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace Satox.Bindings
{
    public class APIConfig
    {
        public string Host { get; set; } = "localhost";
        public int Port { get; set; } = 7777;
        public bool EnableSSL { get; set; } = false;
        public string SSLCertPath { get; set; } = "";
        public string SSLKeyPath { get; set; } = "";
        public int MaxConnections { get; set; } = 100;
        public int Timeout { get; set; } = 30;
    }

    public class APIInfo
    {
        public string Name { get; set; } = "";
        public string Version { get; set; } = "";
        public string Host { get; set; } = "";
        public int Port { get; set; } = 0;
        public bool SSLEnabled { get; set; } = false;
        public long Uptime { get; set; } = 0;
        public long RequestCount { get; set; } = 0;
    }

    public class Endpoint
    {
        public string Path { get; set; } = "";
        public string Method { get; set; } = "";
        public string Handler { get; set; } = "";
        public string Description { get; set; } = "";
    }

    public class APIStats
    {
        public long TotalRequests { get; set; } = 0;
        public long SuccessfulRequests { get; set; } = 0;
        public long FailedRequests { get; set; } = 0;
        public double AverageResponseTime { get; set; } = 0.0;
        public int ActiveConnections { get; set; } = 0;
        public int EndpointsCount { get; set; } = 0;
    }

    public enum APIState
    {
        Stopped = 0,
        Starting = 1,
        Running = 2,
        Stopping = 3,
        Error = 4
    }

    public class APIManager : ThreadSafeManager
    {
        private bool _serverRunning;
        private APIConfig _config;
        private APIState _state;
        private readonly Dictionary<string, Endpoint> _endpoints;

        // Native method declarations
        [DllImport("satox_sdk")]
        private static extern IntPtr satox_api_manager_create();

        [DllImport("satox_sdk")]
        private static extern void satox_api_manager_destroy(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern bool satox_api_manager_initialize(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string config);

        [DllImport("satox_sdk")]
        private static extern void satox_api_manager_shutdown(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern bool satox_api_manager_start_server(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern bool satox_api_manager_stop_server(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern int satox_api_manager_get_state(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern bool satox_api_manager_register_endpoint(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string path, [MarshalAs(UnmanagedType.LPStr)] string method, [MarshalAs(UnmanagedType.LPStr)] string handler);

        [DllImport("satox_sdk")]
        private static extern bool satox_api_manager_unregister_endpoint(IntPtr manager, [MarshalAs(UnmanagedType.LPStr)] string path, [MarshalAs(UnmanagedType.LPStr)] string method);

        [DllImport("satox_sdk")]
        private static extern IntPtr satox_api_manager_get_info(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern IntPtr satox_api_manager_get_stats(IntPtr manager);

        [DllImport("satox_sdk")]
        private static extern IntPtr satox_api_manager_get_last_error(IntPtr manager);

        private IntPtr _nativeManager;

        public APIManager() : base()
        {
            _serverRunning = false;
            _state = APIState.Stopped;
            _endpoints = new Dictionary<string, Endpoint>();
            _config = new APIConfig();

            _nativeManager = satox_api_manager_create();
        }

        public bool Initialize(APIConfig config)
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return true;

                _config = config;
                var result = satox_api_manager_initialize(_nativeManager, Marshal.StringToHGlobalAnsi(JsonConvert.SerializeObject(config)));
                SetInitialized(result);
                return result;
            }
        }

        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                satox_api_manager_shutdown(_nativeManager);
                SetInitialized(false);
            }
        }

        public APIState GetState()
        {
            ValidateNotDisposed();
            return (APIState)satox_api_manager_get_state(_nativeManager);
        }

        public APIInfo GetInfo()
        {
            ValidateNotDisposed();
            IntPtr infoPtr = satox_api_manager_get_info(_nativeManager);
            return JsonConvert.DeserializeObject<APIInfo>(Marshal.PtrToStringAnsi(infoPtr));
        }

        public APIStats GetStats()
        {
            ValidateNotDisposed();
            IntPtr statsPtr = satox_api_manager_get_stats(_nativeManager);
            return JsonConvert.DeserializeObject<APIStats>(Marshal.PtrToStringAnsi(statsPtr));
        }

        public bool RegisterEndpoint(string path, string method, string handler)
        {
            ValidateNotDisposed();
            return satox_api_manager_register_endpoint(_nativeManager, Marshal.StringToHGlobalAnsi(path), Marshal.StringToHGlobalAnsi(method), Marshal.StringToHGlobalAnsi(handler));
        }

        public bool UnregisterEndpoint(string path, string method)
        {
            ValidateNotDisposed();
            return satox_api_manager_unregister_endpoint(_nativeManager, Marshal.StringToHGlobalAnsi(path), Marshal.StringToHGlobalAnsi(method));
        }

        public bool StartServer()
        {
            ValidateNotDisposed();
            return satox_api_manager_start_server(_nativeManager);
        }

        public bool StopServer()
        {
            ValidateNotDisposed();
            return satox_api_manager_stop_server(_nativeManager);
        }

        public void Dispose()
        {
            ValidateNotDisposed();
            satox_api_manager_destroy(_nativeManager);
        }
    }
}
