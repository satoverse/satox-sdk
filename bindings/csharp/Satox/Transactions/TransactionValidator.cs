using System;
using System.Runtime.InteropServices;

namespace Satox.Transactions
{
    /// <summary>
    /// TransactionValidator provides transaction validation functionality.
    /// </summary>
    public class TransactionValidator : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of TransactionValidator.
        /// </summary>
        public TransactionValidator()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the transaction validator.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_transaction_validator_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize transaction validator");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the transaction validator.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_transaction_validator_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown transaction validator");

            _initialized = false;
        }

        /// <summary>
        /// Validates a transaction.
        /// </summary>
        /// <param name="transaction">The transaction to validate.</param>
        /// <returns>The validation result.</returns>
        public ValidationResult ValidateTransaction(Transaction transaction)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));

            try
            {
                IntPtr resultPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_validator_validate_transaction(
                    ref transaction,
                    ref resultPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate transaction");

                var result = Marshal.PtrToStructure<ValidationResult>(resultPtr);
                NativeMethods.satox_transaction_validator_free_validation_result(resultPtr);

                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate transaction: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a transaction signature.
        /// </summary>
        /// <param name="transaction">The transaction to validate.</param>
        /// <param name="publicKey">The public key to validate against.</param>
        /// <returns>True if the signature is valid, false otherwise.</returns>
        public bool ValidateSignature(Transaction transaction, byte[] publicKey)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));
            ValidateInput(publicKey, nameof(publicKey));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_transaction_validator_validate_signature(
                    ref transaction,
                    publicKey,
                    publicKey.Length,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate signature");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate signature: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a transaction amount.
        /// </summary>
        /// <param name="amount">The amount to validate.</param>
        /// <returns>True if the amount is valid, false otherwise.</returns>
        public bool ValidateAmount(decimal amount)
        {
            EnsureInitialized();

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_transaction_validator_validate_amount(
                    (double)amount,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate amount");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate amount: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a transaction fee.
        /// </summary>
        /// <param name="fee">The fee to validate.</param>
        /// <returns>True if the fee is valid, false otherwise.</returns>
        public bool ValidateFee(decimal fee)
        {
            EnsureInitialized();

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_transaction_validator_validate_fee(
                    (double)fee,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate fee");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate fee: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a transaction address.
        /// </summary>
        /// <param name="address">The address to validate.</param>
        /// <returns>True if the address is valid, false otherwise.</returns>
        public bool ValidateAddress(string address)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_transaction_validator_validate_address(
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

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Transaction validator is not initialized");
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

        private void ValidateInput(byte[] input, string paramName)
        {
            if (input == null || input.Length == 0)
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

        ~TransactionValidator()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_initialize();

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_shutdown();

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_validate_transaction(
                ref Transaction transaction,
                ref IntPtr result
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_validate_signature(
                ref Transaction transaction,
                byte[] publicKey,
                int publicKeyLength,
                ref int isValid
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_validate_amount(
                double amount,
                ref int isValid
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_validate_fee(
                double fee,
                ref int isValid
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_validator_validate_address(
                string address,
                ref int isValid
            );

            [DllImport("satox_transactions")]
            public static extern void satox_transaction_validator_free_validation_result(IntPtr result);
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