using System;
using System.Runtime.InteropServices;

namespace Satox.Blockchain
{
    /// <summary>
    /// BlockchainManager provides blockchain connection and processing functionality.
    /// </summary>
    public class BlockchainManager : ThreadSafeManager
    {
        /// <summary>
        /// Creates a new instance of BlockchainManager.
        /// </summary>
        public BlockchainManager() : base()
        {
        }

        /// <summary>
        /// Initializes the blockchain manager.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_blockchain_manager_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize blockchain manager");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the blockchain manager.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_blockchain_manager_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown blockchain manager");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Connects to the blockchain network.
        /// </summary>
        /// <param name="networkId">The network ID to connect to.</param>
        public void Connect(string networkId)
        {
            EnsureInitialized();
            ValidateInput(networkId, nameof(networkId));

            try
            {
                var status = NativeMethods.satox_blockchain_manager_connect(networkId);
                if (status != 0)
                    throw new SatoxError("Failed to connect to blockchain network");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to connect to blockchain network: {ex.Message}");
            }
        }

        /// <summary>
        /// Disconnects from the blockchain network.
        /// </summary>
        public void Disconnect()
        {
            EnsureInitialized();

            try
            {
                var status = NativeMethods.satox_blockchain_manager_disconnect();
                if (status != 0)
                    throw new SatoxError("Failed to disconnect from blockchain network");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to disconnect from blockchain network: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the current network status.
        /// </summary>
        /// <returns>The network status.</returns>
        public NetworkStatus GetNetworkStatus()
        {
            EnsureInitialized();

            try
            {
                int status = 0;
                var result = NativeMethods.satox_blockchain_manager_get_network_status(ref status);
                if (result != 0)
                    throw new SatoxError("Failed to get network status");

                return (NetworkStatus)status;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get network status: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the current block height.
        /// </summary>
        /// <returns>The current block height.</returns>
        public long GetBlockHeight()
        {
            EnsureInitialized();

            try
            {
                long height = 0;
                var status = NativeMethods.satox_blockchain_manager_get_block_height(ref height);
                if (status != 0)
                    throw new SatoxError("Failed to get block height");

                return height;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get block height: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a block by its hash.
        /// </summary>
        /// <param name="blockHash">The block hash.</param>
        /// <returns>The block information.</returns>
        public BlockInfo GetBlockByHash(string blockHash)
        {
            EnsureInitialized();
            ValidateInput(blockHash, nameof(blockHash));

            try
            {
                IntPtr blockInfoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_blockchain_manager_get_block_by_hash(blockHash, ref blockInfoPtr);
                if (status != 0)
                    throw new SatoxError("Failed to get block by hash");

                var blockInfo = Marshal.PtrToStructure<BlockInfo>(blockInfoPtr);
                NativeMethods.satox_blockchain_manager_free_block_info(blockInfoPtr);

                return blockInfo;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get block by hash: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a block by its height.
        /// </summary>
        /// <param name="height">The block height.</param>
        /// <returns>The block information.</returns>
        public BlockInfo GetBlockByHeight(long height)
        {
            EnsureInitialized();

            try
            {
                IntPtr blockInfoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_blockchain_manager_get_block_by_height(height, ref blockInfoPtr);
                if (status != 0)
                    throw new SatoxError("Failed to get block by height");

                var blockInfo = Marshal.PtrToStructure<BlockInfo>(blockInfoPtr);
                NativeMethods.satox_blockchain_manager_free_block_info(blockInfoPtr);

                return blockInfo;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get block by height: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a transaction by its hash.
        /// </summary>
        /// <param name="transactionHash">The transaction hash.</param>
        /// <returns>The transaction information.</returns>
        public TransactionInfo GetTransactionByHash(string transactionHash)
        {
            EnsureInitialized();
            ValidateInput(transactionHash, nameof(transactionHash));

            try
            {
                IntPtr transactionInfoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_blockchain_manager_get_transaction_by_hash(transactionHash, ref transactionInfoPtr);
                if (status != 0)
                    throw new SatoxError("Failed to get transaction by hash");

                var transactionInfo = Marshal.PtrToStructure<TransactionInfo>(transactionInfoPtr);
                NativeMethods.satox_blockchain_manager_free_transaction_info(transactionInfoPtr);

                return transactionInfo;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get transaction by hash: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the balance of an address.
        /// </summary>
        /// <param name="address">The address to check.</param>
        /// <returns>The balance.</returns>
        public decimal GetBalance(string address)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                double balance = 0;
                var status = NativeMethods.satox_blockchain_manager_get_balance(address, ref balance);
                if (status != 0)
                    throw new SatoxError("Failed to get balance");

                return (decimal)balance;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get balance: {ex.Message}");
            }
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrWhiteSpace(input))
                throw new ArgumentException($"{paramName} cannot be null or empty", paramName);
        }

        protected override void DisposeCore()
        {
            if (IsInitialized)
                Shutdown();
        }

        private static class NativeMethods
        {
            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_initialize();

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_shutdown();

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_connect([MarshalAs(UnmanagedType.LPStr)] string networkId);

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_disconnect();

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_get_network_status(ref int status);

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_get_block_height(ref long height);

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_get_block_by_hash([MarshalAs(UnmanagedType.LPStr)] string blockHash, ref IntPtr blockInfo);

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_get_block_by_height(long height, ref IntPtr blockInfo);

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_get_transaction_by_hash([MarshalAs(UnmanagedType.LPStr)] string transactionHash, ref IntPtr transactionInfo);

            [DllImport("satox_blockchain")]
            public static extern int satox_blockchain_manager_get_balance([MarshalAs(UnmanagedType.LPStr)] string address, ref double balance);

            [DllImport("satox_blockchain")]
            public static extern void satox_blockchain_manager_free_block_info(IntPtr blockInfo);

            [DllImport("satox_blockchain")]
            public static extern void satox_blockchain_manager_free_transaction_info(IntPtr transactionInfo);
        }
    }

    /// <summary>
    /// Represents the network status.
    /// </summary>
    public enum NetworkStatus
    {
        /// <summary>
        /// The network is disconnected.
        /// </summary>
        Disconnected = 0,

        /// <summary>
        /// The network is connecting.
        /// </summary>
        Connecting = 1,

        /// <summary>
        /// The network is connected.
        /// </summary>
        Connected = 2,

        /// <summary>
        /// The network is syncing.
        /// </summary>
        Syncing = 3,

        /// <summary>
        /// The network is synced.
        /// </summary>
        Synced = 4
    }

    /// <summary>
    /// Represents block information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct BlockInfo
    {
        /// <summary>
        /// The block hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        /// <summary>
        /// The block height.
        /// </summary>
        public long Height;

        /// <summary>
        /// The block timestamp.
        /// </summary>
        public long Timestamp;

        /// <summary>
        /// The number of transactions in the block.
        /// </summary>
        public int TransactionCount;

        /// <summary>
        /// The block size in bytes.
        /// </summary>
        public int Size;
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
    }
}