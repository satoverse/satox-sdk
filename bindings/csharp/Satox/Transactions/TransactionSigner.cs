using System;
using System.Runtime.InteropServices;

namespace Satox.Transactions
{
    /// <summary>
    /// TransactionSigner provides transaction signing functionality.
    /// </summary>
    public class TransactionSigner : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of TransactionSigner.
        /// </summary>
        public TransactionSigner()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the transaction signer.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_transaction_signer_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize transaction signer");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the transaction signer.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_transaction_signer_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown transaction signer");

            _initialized = false;
        }

        /// <summary>
        /// Signs a transaction.
        /// </summary>
        /// <param name="transaction">The transaction to sign.</param>
        /// <param name="privateKey">The private key to sign with.</param>
        /// <returns>The signed transaction.</returns>
        public Transaction SignTransaction(Transaction transaction, byte[] privateKey)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));
            ValidateInput(privateKey, nameof(privateKey));

            try
            {
                IntPtr signedTransactionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_signer_sign_transaction(
                    ref transaction,
                    privateKey,
                    privateKey.Length,
                    ref signedTransactionPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to sign transaction");

                var signedTransaction = Marshal.PtrToStructure<Transaction>(signedTransactionPtr);
                NativeMethods.satox_transaction_signer_free_transaction(signedTransactionPtr);

                return signedTransaction;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to sign transaction: {ex.Message}");
            }
        }

        /// <summary>
        /// Signs a transaction with a specific algorithm.
        /// </summary>
        /// <param name="transaction">The transaction to sign.</param>
        /// <param name="privateKey">The private key to sign with.</param>
        /// <param name="algorithm">The signing algorithm to use.</param>
        /// <returns>The signed transaction.</returns>
        public Transaction SignTransactionWithAlgorithm(Transaction transaction, byte[] privateKey, string algorithm)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));
            ValidateInput(privateKey, nameof(privateKey));
            ValidateInput(algorithm, nameof(algorithm));

            try
            {
                IntPtr signedTransactionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_signer_sign_transaction_with_algorithm(
                    ref transaction,
                    privateKey,
                    privateKey.Length,
                    algorithm,
                    ref signedTransactionPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to sign transaction with algorithm");

                var signedTransaction = Marshal.PtrToStructure<Transaction>(signedTransactionPtr);
                NativeMethods.satox_transaction_signer_free_transaction(signedTransactionPtr);

                return signedTransaction;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to sign transaction with algorithm: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the available signing algorithms.
        /// </summary>
        /// <returns>An array of available signing algorithms.</returns>
        public string[] GetAvailableAlgorithms()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_transaction_signer_get_available_algorithms(
                    ref algorithmsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get available algorithms");

                var algorithms = new string[count];
                var currentPtr = algorithmsPtr;

                for (int i = 0; i < count; i++)
                {
                    algorithms[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr));
                    currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                }

                NativeMethods.satox_transaction_signer_free_algorithms(algorithmsPtr, count);

                return algorithms;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get available algorithms: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the default signing algorithm.
        /// </summary>
        /// <returns>The default signing algorithm.</returns>
        public string GetDefaultAlgorithm()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_signer_get_default_algorithm(
                    ref algorithmPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get default algorithm");

                var algorithm = Marshal.PtrToStringAnsi(algorithmPtr);
                NativeMethods.satox_transaction_signer_free_string(algorithmPtr);

                return algorithm;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get default algorithm: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Transaction signer is not initialized");
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

        ~TransactionSigner()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_transactions")]
            public static extern int satox_transaction_signer_initialize();

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_signer_shutdown();

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_signer_sign_transaction(
                ref Transaction transaction,
                byte[] privateKey,
                int privateKeyLength,
                ref IntPtr signedTransaction
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_signer_sign_transaction_with_algorithm(
                ref Transaction transaction,
                byte[] privateKey,
                int privateKeyLength,
                string algorithm,
                ref IntPtr signedTransaction
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_signer_get_available_algorithms(
                ref IntPtr algorithms,
                ref int count
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_signer_get_default_algorithm(
                ref IntPtr algorithm
            );

            [DllImport("satox_transactions")]
            public static extern void satox_transaction_signer_free_transaction(IntPtr transaction);

            [DllImport("satox_transactions")]
            public static extern void satox_transaction_signer_free_algorithms(IntPtr algorithms, int count);

            [DllImport("satox_transactions")]
            public static extern void satox_transaction_signer_free_string(IntPtr str);
        }
    }
} 