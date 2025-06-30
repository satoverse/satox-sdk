using System;
using System.Runtime.InteropServices;

namespace Satox.Security
{
    public class AccessControl : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        public AccessControl()
        {
            _initialized = false;
            _disposed = false;
        }

        public void Initialize()
        {
            if (_initialized)
                return;

            var result = NativeMethods.satox_access_control_initialize();
            if (result != 0)
                throw new SatoxError("Failed to initialize access control");

            _initialized = true;
        }

        public void Shutdown()
        {
            if (!_initialized)
                return;

            var result = NativeMethods.satox_access_control_shutdown();
            if (result != 0)
                throw new SatoxError("Failed to shutdown access control");

            _initialized = false;
        }

        public void AddRole(string roleName, RolePermissions permissions)
        {
            EnsureInitialized();
            ValidateInput(roleName);

            var result = NativeMethods.satox_access_control_add_role(roleName, ref permissions);
            if (result != 0)
                throw new SatoxError("Failed to add role");
        }

        public void RemoveRole(string roleName)
        {
            EnsureInitialized();
            ValidateInput(roleName);

            var result = NativeMethods.satox_access_control_remove_role(roleName);
            if (result != 0)
                throw new SatoxError("Failed to remove role");
        }

        public void AssignRole(string userId, string roleName)
        {
            EnsureInitialized();
            ValidateInput(userId, roleName);

            var result = NativeMethods.satox_access_control_assign_role(userId, roleName);
            if (result != 0)
                throw new SatoxError("Failed to assign role");
        }

        public void RevokeRole(string userId, string roleName)
        {
            EnsureInitialized();
            ValidateInput(userId, roleName);

            var result = NativeMethods.satox_access_control_revoke_role(userId, roleName);
            if (result != 0)
                throw new SatoxError("Failed to revoke role");
        }

        public bool HasPermission(string userId, string permission)
        {
            EnsureInitialized();
            ValidateInput(userId, permission);

            bool hasPermission;
            var result = NativeMethods.satox_access_control_has_permission(userId, permission, out hasPermission);
            if (result != 0)
                throw new SatoxError("Failed to check permission");

            return hasPermission;
        }

        public string[] GetUserRoles(string userId)
        {
            EnsureInitialized();
            ValidateInput(userId);

            int count;
            var result = NativeMethods.satox_access_control_get_user_role_count(userId, out count);
            if (result != 0)
                throw new SatoxError("Failed to get user role count");

            var roles = new string[count];
            result = NativeMethods.satox_access_control_get_user_roles(userId, roles, count);
            if (result != 0)
                throw new SatoxError("Failed to get user roles");

            return roles;
        }

        public RolePermissions GetRolePermissions(string roleName)
        {
            EnsureInitialized();
            ValidateInput(roleName);

            var permissions = new RolePermissions();
            var result = NativeMethods.satox_access_control_get_role_permissions(roleName, out permissions);
            if (result != 0)
                throw new SatoxError("Failed to get role permissions");

            return permissions;
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Access control not initialized");
        }

        private void ValidateInput(string userId, string roleName)
        {
            if (string.IsNullOrEmpty(userId))
                throw new ArgumentException("User ID cannot be null or empty", nameof(userId));
            if (string.IsNullOrEmpty(roleName))
                throw new ArgumentException("Role name cannot be null or empty", nameof(roleName));
        }

        private void ValidateInput(string userId, string permission)
        {
            if (string.IsNullOrEmpty(userId))
                throw new ArgumentException("User ID cannot be null or empty", nameof(userId));
            if (string.IsNullOrEmpty(permission))
                throw new ArgumentException("Permission cannot be null or empty", nameof(permission));
        }

        private void ValidateInput(string roleName)
        {
            if (string.IsNullOrEmpty(roleName))
                throw new ArgumentException("Role name cannot be null or empty", nameof(roleName));
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

        ~AccessControl()
        {
            Dispose(false);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RolePermissions
    {
        public bool CanRead;
        public bool CanWrite;
        public bool CanDelete;
        public bool CanExecute;
        public bool CanAdmin;
        public bool CanModerate;
        public bool CanCreate;
        public bool CanUpdate;
    }

    internal static class NativeMethods
    {
        [DllImport("satox")]
        public static extern int satox_access_control_initialize();

        [DllImport("satox")]
        public static extern int satox_access_control_shutdown();

        [DllImport("satox")]
        public static extern int satox_access_control_add_role(string roleName, ref RolePermissions permissions);

        [DllImport("satox")]
        public static extern int satox_access_control_remove_role(string roleName);

        [DllImport("satox")]
        public static extern int satox_access_control_assign_role(string userId, string roleName);

        [DllImport("satox")]
        public static extern int satox_access_control_revoke_role(string userId, string roleName);

        [DllImport("satox")]
        public static extern int satox_access_control_has_permission(string userId, string permission, out bool hasPermission);

        [DllImport("satox")]
        public static extern int satox_access_control_get_user_role_count(string userId, out int count);

        [DllImport("satox")]
        public static extern int satox_access_control_get_user_roles(string userId, [Out] string[] roles, int count);

        [DllImport("satox")]
        public static extern int satox_access_control_get_role_permissions(string roleName, out RolePermissions permissions);
    }
} 