using System;
using System.Runtime.InteropServices;

namespace Satox.API
{
    public class RequestHandler : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        public RequestHandler()
        {
            _initialized = false;
            _disposed = false;
        }

        public void Initialize()
        {
            if (_initialized)
                return;

            var result = NativeMethods.satox_request_handler_initialize();
            if (result != 0)
                throw new SatoxError("Failed to initialize request handler");

            _initialized = true;
        }

        public void Shutdown()
        {
            if (!_initialized)
                return;

            var result = NativeMethods.satox_request_handler_shutdown();
            if (result != 0)
                throw new SatoxError("Failed to shutdown request handler");

            _initialized = false;
        }

        public void HandleRequest(IntPtr request, IntPtr response)
        {
            EnsureInitialized();
            ValidateInput(request, response);

            var result = NativeMethods.satox_request_handler_handle_request(request, response);
            if (result != 0)
                throw new SatoxError("Failed to handle request");
        }

        public void SetMiddleware(IntPtr middleware)
        {
            EnsureInitialized();
            ValidateInput(middleware);

            var result = NativeMethods.satox_request_handler_set_middleware(middleware);
            if (result != 0)
                throw new SatoxError("Failed to set middleware");
        }

        public void SetErrorHandler(IntPtr errorHandler)
        {
            EnsureInitialized();
            ValidateInput(errorHandler);

            var result = NativeMethods.satox_request_handler_set_error_handler(errorHandler);
            if (result != 0)
                throw new SatoxError("Failed to set error handler");
        }

        public RequestStats GetStats()
        {
            EnsureInitialized();

            var stats = new RequestStats();
            var result = NativeMethods.satox_request_handler_get_stats(out stats);
            if (result != 0)
                throw new SatoxError("Failed to get request stats");

            return stats;
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Request handler not initialized");
        }

        private void ValidateInput(IntPtr request, IntPtr response)
        {
            if (request == IntPtr.Zero)
                throw new ArgumentException("Request cannot be null", nameof(request));
            if (response == IntPtr.Zero)
                throw new ArgumentException("Response cannot be null", nameof(response));
        }

        private void ValidateInput(IntPtr handler)
        {
            if (handler == IntPtr.Zero)
                throw new ArgumentException("Handler cannot be null", nameof(handler));
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

        ~RequestHandler()
        {
            Dispose(false);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RequestStats
    {
        public int TotalRequests;
        public int SuccessfulRequests;
        public int FailedRequests;
        public int AverageResponseTime;
        public int MaxResponseTime;
        public int MinResponseTime;
        [MarshalAs(UnmanagedType.LPStr)]
        public string LastError;
    }

    internal static class NativeMethods
    {
        [DllImport("satox")]
        public static extern int satox_request_handler_initialize();

        [DllImport("satox")]
        public static extern int satox_request_handler_shutdown();

        [DllImport("satox")]
        public static extern int satox_request_handler_handle_request(IntPtr request, IntPtr response);

        [DllImport("satox")]
        public static extern int satox_request_handler_set_middleware(IntPtr middleware);

        [DllImport("satox")]
        public static extern int satox_request_handler_set_error_handler(IntPtr errorHandler);

        [DllImport("satox")]
        public static extern int satox_request_handler_get_stats(out RequestStats stats);
    }
} 