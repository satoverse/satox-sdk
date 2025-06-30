using satox_bindings;

namespace Satox.Blockchain
{
    /// <summary>
    /// BlockchainManager provides blockchain connection and processing functionality.
    /// </summary>
    public class BlockchainManager : satox_bindings.BlockchainManager
    {
        // Re-export only
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