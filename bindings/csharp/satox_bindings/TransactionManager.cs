using System;
using System.Runtime.InteropServices;

namespace Satox.Bindings.Transactions
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Transaction
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        [MarshalAs(UnmanagedType.LPStr)]
        public string From;

        [MarshalAs(UnmanagedType.LPStr)]
        public string To;

        public double Amount;
        public double Fee;
        public long Timestamp;
        public IntPtr Signature;
        public int SignatureLength;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TransactionInfo
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        [MarshalAs(UnmanagedType.LPStr)]
        public string From;

        public long Timestamp;
        public double Amount;
        public double Fee;

        [MarshalAs(UnmanagedType.LPStr)]
        public string To;

        [MarshalAs(UnmanagedType.LPStr)]
        public string BlockHash;

        public int Status;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct TransactionReceipt
    {
        [MarshalAs(UnmanagedType.LPStr)]
        public string TransactionHash;

        [MarshalAs(UnmanagedType.LPStr)]
        public string BlockHash;

        public long BlockNumber;
        public int Status;
        public long GasUsed;
        public long CumulativeGasUsed;

        [MarshalAs(UnmanagedType.LPStr)]
        public string ContractAddress;
    }

    public enum TransactionStatus
    {
        Pending = 0,
        Confirmed = 1,
        Failed = 2,
        Unknown = 3
    }

    public interface ITransactionManager : IDisposable
    {
        void Initialize();
        void Shutdown();
        Transaction CreateTransaction(string from, string to, decimal amount, decimal fee);
        Transaction SignTransaction(Transaction transaction, byte[] privateKey);
        bool ValidateTransaction(Transaction transaction);
        TransactionReceipt BroadcastTransaction(Transaction transaction);
        TransactionInfo GetTransactionByHash(string transactionHash);
        TransactionStatus GetTransactionStatus(string transactionHash);
    }

    public class TransactionManager : ThreadSafeManager, ITransactionManager
    {
        public TransactionManager() : base()
        {
        }

        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_transaction_manager_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize transaction manager");

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

                var status = NativeMethods.satox_transaction_manager_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown transaction manager");

                SetInitialized(false);
            }
        }

        public Transaction CreateTransaction(string from, string to, decimal amount, decimal fee)
        {
            EnsureInitialized();
            ValidateInput(from, nameof(from));
            ValidateInput(to, nameof(to));

            try
            {
                IntPtr transactionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_manager_create_transaction(
                    from,
                    to,
                    (double)amount,
                    (double)fee,
                    ref transactionPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to create transaction");

                var transaction = Marshal.PtrToStructure<Transaction>(transactionPtr);
                NativeMethods.satox_transaction_manager_free_transaction(transactionPtr);

                return transaction;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to create transaction: {ex.Message}");
            }
        }

        public Transaction SignTransaction(Transaction transaction, byte[] privateKey)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));
            ValidateInput(privateKey, nameof(privateKey));

            try
            {
                IntPtr signedTransactionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_manager_sign_transaction(
                    ref transaction,
                    privateKey,
                    privateKey.Length,
                    ref signedTransactionPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to sign transaction");

                var signedTransaction = Marshal.PtrToStructure<Transaction>(signedTransactionPtr);
                NativeMethods.satox_transaction_manager_free_transaction(signedTransactionPtr);

                return signedTransaction;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to sign transaction: {ex.Message}");
            }
        }

        public bool ValidateTransaction(Transaction transaction)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_transaction_manager_validate_transaction(
                    ref transaction,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate transaction");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate transaction: {ex.Message}");
            }
        }

        public TransactionReceipt BroadcastTransaction(Transaction transaction)
        {
            EnsureInitialized();
            ValidateInput(transaction, nameof(transaction));

            try
            {
                IntPtr receiptPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_manager_broadcast_transaction(
                    ref transaction,
                    ref receiptPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to broadcast transaction");

                var receipt = Marshal.PtrToStructure<TransactionReceipt>(receiptPtr);
                NativeMethods.satox_transaction_manager_free_receipt(receiptPtr);

                return receipt;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to broadcast transaction: {ex.Message}");
            }
        }

        public TransactionInfo GetTransactionByHash(string transactionHash)
        {
            EnsureInitialized();
            ValidateInput(transactionHash, nameof(transactionHash));

            try
            {
                IntPtr transactionInfoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_transaction_manager_get_transaction_by_hash(
                    transactionHash,
                    ref transactionInfoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get transaction by hash");

                var transactionInfo = Marshal.PtrToStructure<TransactionInfo>(transactionInfoPtr);
                NativeMethods.satox_transaction_manager_free_transaction_info(transactionInfoPtr);

                return transactionInfo;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get transaction by hash: {ex.Message}");
            }
        }

        public TransactionStatus GetTransactionStatus(string transactionHash)
        {
            EnsureInitialized();
            ValidateInput(transactionHash, nameof(transactionHash));

            try
            {
                int status = 0;
                var result = NativeMethods.satox_transaction_manager_get_transaction_status(
                    transactionHash,
                    ref status
                );

                if (result != 0)
                    throw new SatoxError("Failed to get transaction status");

                return (TransactionStatus)status;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get transaction status: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("Transaction manager not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException($"{paramName} cannot be null or empty", paramName);
        }

        private void ValidateInput(byte[] input, string paramName)
        {
            if (input == null || input.Length == 0)
                throw new ArgumentException($"{paramName} cannot be null or empty", paramName);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!IsInitialized)
                return;

            if (disposing)
            {
                Shutdown();
            }
        }

        ~TransactionManager()
        {
            Dispose(false);
        }

        internal static class NativeMethods
        {
            private const string DllName = "satox_transactions";

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_initialize();

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_shutdown();

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_create_transaction(
                string from,
                string to,
                double amount,
                double fee,
                ref IntPtr transaction
            );

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_sign_transaction(
                ref Transaction transaction,
                byte[] privateKey,
                int privateKeyLength,
                ref IntPtr signedTransaction
            );

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_validate_transaction(
                ref Transaction transaction,
                ref int isValid
            );

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_broadcast_transaction(
                ref Transaction transaction,
                ref IntPtr receipt
            );

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_get_transaction_by_hash(
                string transactionHash,
                ref IntPtr transactionInfo
            );

            [DllImport(DllName)]
            public static extern int satox_transaction_manager_get_transaction_status(
                string transactionHash,
                ref int status
            );

            [DllImport(DllName)]
            public static extern void satox_transaction_manager_free_transaction(IntPtr transaction);

            [DllImport(DllName)]
            public static extern void satox_transaction_manager_free_receipt(IntPtr receipt);

            [DllImport(DllName)]
            public static extern void satox_transaction_manager_free_transaction_info(IntPtr transactionInfo);
        }
    }
}