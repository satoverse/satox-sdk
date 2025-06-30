using System;
using System.Runtime.InteropServices;
using Satox.Bindings.Transactions;

namespace Satox.Transactions
{
    /// <summary>
    /// TransactionManager provides transaction handling and validation functionality.
    /// </summary>
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

    public class TransactionManager : ITransactionManager
    {
        private readonly Satox.Bindings.Transactions.TransactionManager _manager;

        /// <summary>
        /// Creates a new instance of TransactionManager.
        /// </summary>
        public TransactionManager()
        {
            _manager = new Satox.Bindings.Transactions.TransactionManager();
        }

        /// <summary>
        /// Initializes the transaction manager.
        /// </summary>
        public void Initialize()
        {
            _manager.Initialize();
        }

        /// <summary>
        /// Shuts down the transaction manager.
        /// </summary>
        public void Shutdown()
        {
            _manager.Shutdown();
        }

        /// <summary>
        /// Creates a new transaction.
        /// </summary>
        /// <param name="from">The sender address.</param>
        /// <param name="to">The recipient address.</param>
        /// <param name="amount">The transaction amount.</param>
        /// <param name="fee">The transaction fee.</param>
        /// <returns>The created transaction.</returns>
        public Transaction CreateTransaction(string from, string to, decimal amount, decimal fee)
        {
            return _manager.CreateTransaction(from, to, amount, fee);
        }

        /// <summary>
        /// Signs a transaction.
        /// </summary>
        /// <param name="transaction">The transaction to sign.</param>
        /// <param name="privateKey">The private key to sign with.</param>
        /// <returns>The signed transaction.</returns>
        public Transaction SignTransaction(Transaction transaction, byte[] privateKey)
        {
            return _manager.SignTransaction(transaction, privateKey);
        }

        /// <summary>
        /// Validates a transaction.
        /// </summary>
        /// <param name="transaction">The transaction to validate.</param>
        /// <returns>True if the transaction is valid, false otherwise.</returns>
        public bool ValidateTransaction(Transaction transaction)
        {
            return _manager.ValidateTransaction(transaction);
        }

        /// <summary>
        /// Broadcasts a transaction to the network.
        /// </summary>
        /// <param name="transaction">The transaction to broadcast.</param>
        /// <returns>The transaction receipt.</returns>
        public TransactionReceipt BroadcastTransaction(Transaction transaction)
        {
            return _manager.BroadcastTransaction(transaction);
        }

        /// <summary>
        /// Gets a transaction by its hash.
        /// </summary>
        /// <param name="transactionHash">The transaction hash.</param>
        /// <returns>The transaction information.</returns>
        public TransactionInfo GetTransactionByHash(string transactionHash)
        {
            return _manager.GetTransactionByHash(transactionHash);
        }

        /// <summary>
        /// Gets the transaction status.
        /// </summary>
        /// <param name="transactionHash">The transaction hash.</param>
        /// <returns>The transaction status.</returns>
        public TransactionStatus GetTransactionStatus(string transactionHash)
        {
            return _manager.GetTransactionStatus(transactionHash);
        }

        public void Dispose()
        {
            _manager.Dispose();
        }
    }

    /// <summary>
    /// Represents a transaction.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Transaction
    {
        /// <summary>
        /// The transaction hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        /// <summary>
        /// The sender address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string From;

        /// <summary>
        /// The recipient address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string To;

        /// <summary>
        /// The transaction amount.
        /// </summary>
        public double Amount;

        /// <summary>
        /// The transaction fee.
        /// </summary>
        public double Fee;

        /// <summary>
        /// The transaction timestamp.
        /// </summary>
        public long Timestamp;

        /// <summary>
        /// The transaction signature.
        /// </summary>
        public IntPtr Signature;

        /// <summary>
        /// The length of the signature.
        /// </summary>
        public int SignatureLength;
    }

    /// <summary>
    /// Represents transaction information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct TransactionInfo
    {
        /// <summary>
        /// The transaction hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        /// <summary>
        /// The block hash containing this transaction.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string BlockHash;

        /// <summary>
        /// The transaction timestamp.
        /// </summary>
        public long Timestamp;

        /// <summary>
        /// The transaction amount.
        /// </summary>
        public double Amount;

        /// <summary>
        /// The transaction fee.
        /// </summary>
        public double Fee;

        /// <summary>
        /// The sender address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string From;

        /// <summary>
        /// The recipient address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string To;

        /// <summary>
        /// The transaction status.
        /// </summary>
        public int Status;
    }

    /// <summary>
    /// Represents a transaction receipt.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct TransactionReceipt
    {
        /// <summary>
        /// The transaction hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        /// <summary>
        /// The block hash containing this transaction.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string BlockHash;

        /// <summary>
        /// The block number.
        /// </summary>
        public long BlockNumber;

        /// <summary>
        /// The transaction status.
        /// </summary>
        public int Status;

        /// <summary>
        /// The gas used.
        /// </summary>
        public long GasUsed;

        /// <summary>
        /// The cumulative gas used.
        /// </summary>
        public long CumulativeGasUsed;

        /// <summary>
        /// The contract address (if any).
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ContractAddress;
    }

    /// <summary>
    /// Represents the transaction status.
    /// </summary>
    public enum TransactionStatus
    {
        /// <summary>
        /// The transaction is pending.
        /// </summary>
        Pending = 0,

        /// <summary>
        /// The transaction is confirmed.
        /// </summary>
        Confirmed = 1,

        /// <summary>
        /// The transaction failed.
        /// </summary>
        Failed = 2,

        /// <summary>
        /// The transaction is unknown.
        /// </summary>
        Unknown = 3
    }
} 