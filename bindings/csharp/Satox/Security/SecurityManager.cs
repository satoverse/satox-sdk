using System;
using Satox.Bindings.Security;

namespace Satox.Security
{
    public class SecurityManager : ISecurityManager
    {
        private readonly ISecurityManager _manager;

        public SecurityManager()
        {
            _manager = new Satox.Bindings.Security.SecurityManager();
        }

        public void Initialize() => _manager.Initialize();
        public void Shutdown() => _manager.Shutdown();
        public void SetSecurityPolicy(SecurityPolicy policy) => _manager.SetSecurityPolicy(policy);
        public SecurityPolicy GetSecurityPolicy() => _manager.GetSecurityPolicy();
        public void EnableFeature(SecurityFeature feature) => _manager.EnableFeature(feature);
        public void DisableFeature(SecurityFeature feature) => _manager.DisableFeature(feature);
        public bool IsFeatureEnabled(SecurityFeature feature) => _manager.IsFeatureEnabled(feature);
        public SecurityStatus GetStatus() => _manager.GetStatus();
        public void Dispose() => _manager.Dispose();
    }

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