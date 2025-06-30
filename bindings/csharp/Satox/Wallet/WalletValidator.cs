using System;
using System.Runtime.InteropServices;

namespace Satox.Wallet
{
    /// <summary>
    /// WalletValidator provides wallet validation functionality.
    /// </summary>
    public class WalletValidator : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of WalletValidator.
        /// </summary>
        public WalletValidator()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the wallet validator.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_wallet_validator_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize wallet validator");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the wallet validator.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_wallet_validator_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown wallet validator");

            _initialized = false;
        }

        /// <summary>
        /// Validates a wallet.
        /// </summary>
        /// <param name="wallet">The wallet to validate.</param>
        /// <returns>The validation result.</returns>
        public ValidationResult ValidateWallet(Wallet wallet)
        {
            EnsureInitialized();
            ValidateInput(wallet, nameof(wallet));

            try
            {
                IntPtr resultPtr = IntPtr.Zero;
                var status = NativeMethods.satox_wallet_validator_validate_wallet(
                    ref wallet,
                    ref resultPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate wallet");

                var result = Marshal.PtrToStructure<ValidationResult>(resultPtr);
                NativeMethods.satox_wallet_validator_free_validation_result(resultPtr);

                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a wallet name.
        /// </summary>
        /// <param name="name">The wallet name to validate.</param>
        /// <returns>True if the name is valid, false otherwise.</returns>
        public bool ValidateName(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_wallet_validator_validate_name(
                    name,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate name");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate name: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a wallet password.
        /// </summary>
        /// <param name="password">The wallet password to validate.</param>
        /// <returns>True if the password is valid, false otherwise.</returns>
        public bool ValidatePassword(string password)
        {
            EnsureInitialized();
            ValidateInput(password, nameof(password));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_wallet_validator_validate_password(
                    password,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate password");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate password: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a wallet address.
        /// </summary>
        /// <param name="address">The wallet address to validate.</param>
        /// <returns>True if the address is valid, false otherwise.</returns>
        public bool ValidateAddress(string address)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_wallet_validator_validate_address(
                    address,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate address");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate address: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a wallet backup.
        /// </summary>
        /// <param name="backupPath">The backup path to validate.</param>
        /// <returns>True if the backup is valid, false otherwise.</returns>
        public bool ValidateBackup(string backupPath)
        {
            EnsureInitialized();
            ValidateInput(backupPath, nameof(backupPath));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_wallet_validator_validate_backup(
                    backupPath,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate backup");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate backup: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Wallet validator is not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException("Value cannot be null or empty", paramName);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    if (_initialized)
                    {
                        Shutdown();
                    }
                }

                _disposed = true;
            }
        }

        ~WalletValidator()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_initialize();

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_shutdown();

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_validate_wallet(
                ref Wallet wallet,
                ref IntPtr result
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_validate_name(
                string name,
                ref int isValid
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_validate_password(
                string password,
                ref int isValid
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_validate_address(
                string address,
                ref int isValid
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_validator_validate_backup(
                string backupPath,
                ref int isValid
            );

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_validator_free_validation_result(IntPtr result);
        }
    }

    /// <summary>
    /// Represents a validation result.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct ValidationResult
    {
        /// <summary>
        /// Whether the validation was successful.
        /// </summary>
        public int IsValid;

        /// <summary>
        /// The validation error message.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ErrorMessage;

        /// <summary>
        /// The validation error code.
        /// </summary>
        public int ErrorCode;
    }
} 