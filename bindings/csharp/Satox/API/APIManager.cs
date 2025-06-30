using System;
using System.Runtime.InteropServices;
using Satox.Bindings.API;

namespace Satox.API
{
    public interface IAPIManager : IDisposable
    {
        void Initialize();
        void Shutdown();
        void StartServer(APIConfig config);
        void StopServer();
        APIStatus GetStatus();
        void RegisterEndpoint(string path, string method, IntPtr handler);
        void UnregisterEndpoint(string path, string method);
        void SetMiddleware(IntPtr middleware);
    }

    public class APIManager : IAPIManager
    {
        private readonly Satox.Bindings.API.APIManager _manager;

        public APIManager()
        {
            _manager = new Satox.Bindings.API.APIManager();
        }

        public void Initialize()
        {
            _manager.Initialize();
        }

        public void Shutdown()
        {
            _manager.Shutdown();
        }

        public void StartServer(APIConfig config)
        {
            _manager.StartServer(config);
        }

        public void StopServer()
        {
            _manager.StopServer();
        }

        public APIStatus GetStatus()
        {
            return _manager.GetStatus();
        }

        public void RegisterEndpoint(string path, string method, IntPtr handler)
        {
            _manager.RegisterEndpoint(path, method, handler);
        }

        public void UnregisterEndpoint(string path, string method)
        {
            _manager.UnregisterEndpoint(path, method);
        }

        public void SetMiddleware(IntPtr middleware)
        {
            _manager.SetMiddleware(middleware);
        }

        public void Dispose()
        {
            _manager.Dispose();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct APIConfig
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Host;
        public ushort Port;
        public bool EnableSSL;
        [MarshalAs(UnmanagedType.LPStr)]
        public string SSLCertPath;
        [MarshalAs(UnmanagedType.LPStr)]
        public string SSLKeyPath;
        public int MaxConnections;
        public int Timeout;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct APIStatus
    {
        public bool IsRunning;
        public int ActiveConnections;
        public int TotalRequests;
        public int FailedRequests;
        [MarshalAs(UnmanagedType.LPStr)]
        public string LastError;
    }

    internal static class NativeMethods
    {
        [DllImport("satox")]
        public static extern int satox_api_manager_initialize();

        [DllImport("satox")]
        public static extern int satox_api_manager_shutdown();

        [DllImport("satox")]
        public static extern int satox_api_manager_start_server(ref APIConfig config);

        [DllImport("satox")]
        public static extern int satox_api_manager_stop_server();

        [DllImport("satox")]
        public static extern int satox_api_manager_get_status(out APIStatus status);

        [DllImport("satox")]
        public static extern int satox_api_manager_register_endpoint(string path, string method, IntPtr handler);

        [DllImport("satox")]
        public static extern int satox_api_manager_unregister_endpoint(string path, string method);

        [DllImport("satox")]
        public static extern int satox_api_manager_set_middleware(IntPtr middleware);
    }
} 