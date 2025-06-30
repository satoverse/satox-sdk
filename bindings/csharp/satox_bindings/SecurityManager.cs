using System;
using System.Runtime.InteropServices;

namespace Satox.Bindings.Security
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SecurityPolicy
    {
        public bool RequireAuthentication;
        public bool RequireAuthorization;
        public bool EnableEncryption;
        public bool EnableRateLimiting;
        public int MaxRequestsPerSecond;
        public int MaxConcurrentConnections;
        public int SessionTimeout;
        public int PasswordMinLength;
        public bool RequireSpecialChars;
        public bool RequireNumbers;
        public bool RequireUppercase;
        public bool RequireLowercase;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SecurityStatus
    {
        public bool IsSecure;
        public int ActiveConnections;
        public int BlockedConnections;
        public int FailedAttempts;
        public int SecurityViolations;
        [MarshalAs(UnmanagedType.LPStr)]
        public string LastViolation;
    }

    public enum SecurityFeature
    {
        Authentication = 1,
        Authorization = 2,
        Encryption = 3,
        RateLimiting = 4,
        InputValidation = 5,
        OutputSanitization = 6,
        SessionManagement = 7,
        AuditLogging = 8
    }

    public interface ISecurityManager : IDisposable
    {
        void Initialize();
        void Shutdown();
        void SetSecurityPolicy(SecurityPolicy policy);
        SecurityPolicy GetSecurityPolicy();
        void EnableFeature(SecurityFeature feature);
        void DisableFeature(SecurityFeature feature);
        bool IsFeatureEnabled(SecurityFeature feature);
        SecurityStatus GetStatus();
    }

    public class SecurityManager : ThreadSafeManager, ISecurityManager
    {
        public SecurityManager() : base()
        {
        }

        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var result = NativeMethods.satox_security_manager_initialize();
                if (result != 0)
                    throw new SatoxError("Failed to initialize security manager");

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

                var result = NativeMethods.satox_security_manager_shutdown();
                if (result != 0)
                    throw new SatoxError("Failed to shutdown security manager");

                SetInitialized(false);
            }
        }

        public void SetSecurityPolicy(SecurityPolicy policy)
        {
            EnsureInitialized();
            ValidateInput(policy);

            var result = NativeMethods.satox_security_manager_set_policy(ref policy);
            if (result != 0)
                throw new SatoxError("Failed to set security policy");
        }

        public SecurityPolicy GetSecurityPolicy()
        {
            EnsureInitialized();

            var policy = new SecurityPolicy();
            var result = NativeMethods.satox_security_manager_get_policy(out policy);
            if (result != 0)
                throw new SatoxError("Failed to get security policy");

            return policy;
        }

        public void EnableFeature(SecurityFeature feature)
        {
            EnsureInitialized();

            var result = NativeMethods.satox_security_manager_enable_feature(feature);
            if (result != 0)
                throw new SatoxError("Failed to enable security feature");
        }

        public void DisableFeature(SecurityFeature feature)
        {
            EnsureInitialized();

            var result = NativeMethods.satox_security_manager_disable_feature(feature);
            if (result != 0)
                throw new SatoxError("Failed to disable security feature");
        }

        public bool IsFeatureEnabled(SecurityFeature feature)
        {
            EnsureInitialized();

            bool enabled;
            var result = NativeMethods.satox_security_manager_is_feature_enabled(feature, out enabled);
            if (result != 0)
                throw new SatoxError("Failed to check security feature status");

            return enabled;
        }

        public SecurityStatus GetStatus()
        {
            EnsureInitialized();

            var status = new SecurityStatus();
            var result = NativeMethods.satox_security_manager_get_status(out status);
            if (result != 0)
                throw new SatoxError("Failed to get security status");

            return status;
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("Security manager not initialized");
        }

        private void ValidateInput(SecurityPolicy policy)
        {
            if (policy == null)
                throw new ArgumentNullException(nameof(policy));
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

        ~SecurityManager()
        {
            Dispose(false);
        }

        internal static class NativeMethods
        {
            [DllImport("satox")]
            public static extern int satox_security_manager_initialize();

            [DllImport("satox")]
            public static extern int satox_security_manager_shutdown();

            [DllImport("satox")]
            public static extern int satox_security_manager_set_policy(ref SecurityPolicy policy);

            [DllImport("satox")]
            public static extern int satox_security_manager_get_policy(out SecurityPolicy policy);

            [DllImport("satox")]
            public static extern int satox_security_manager_enable_feature(SecurityFeature feature);

            [DllImport("satox")]
            public static extern int satox_security_manager_disable_feature(SecurityFeature feature);

            [DllImport("satox")]
            public static extern int satox_security_manager_is_feature_enabled(SecurityFeature feature, out bool enabled);

            [DllImport("satox")]
            public static extern int satox_security_manager_get_status(out SecurityStatus status);
        }
    }
}