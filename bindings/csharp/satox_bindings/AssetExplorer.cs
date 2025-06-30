using System;
using System.Runtime.InteropServices;

namespace Satox.Assets
{
    /// <summary>
    /// AssetExplorer provides asset exploration functionality.
    /// </summary>
    public class AssetExplorer : ThreadSafeManager
    {
        /// <summary>
        /// Creates a new instance of AssetExplorer.
        /// </summary>
        public AssetExplorer() : base()
        {
        }

        /// <summary>
        /// Initializes the asset explorer.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_asset_explorer_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize asset explorer");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the asset explorer.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_asset_explorer_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown asset explorer");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Gets asset information by ID.
        /// </summary>
        /// <param name="assetId">The asset ID.</param>
        /// <returns>The asset information.</returns>
        public AssetInfo GetAssetInfo(string assetId)
        {
            EnsureInitialized();
            ValidateInput(assetId, nameof(assetId));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_asset_explorer_get_asset_info(
                    assetId,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get asset info");

                var info = Marshal.PtrToStructure<AssetInfo>(infoPtr);
                NativeMethods.satox_asset_explorer_free_asset_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get asset info: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets asset holders.
        /// </summary>
        /// <param name="assetId">The asset ID.</param>
        /// <returns>An array of asset holders.</returns>
        public AssetHolder[] GetAssetHolders(string assetId)
        {
            EnsureInitialized();
            ValidateInput(assetId, nameof(assetId));

            try
            {
                IntPtr holdersPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_asset_explorer_get_asset_holders(
                    assetId,
                    ref holdersPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get asset holders");

                var holders = new AssetHolder[count];
                var currentPtr = holdersPtr;

                for (int i = 0; i < count; i++)
                {
                    holders[i] = Marshal.PtrToStructure<AssetHolder>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<AssetHolder>());
                }

                NativeMethods.satox_asset_explorer_free_asset_holders(holdersPtr, count);

                return holders;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get asset holders: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets asset transactions.
        /// </summary>
        /// <param name="assetId">The asset ID.</param>
        /// <param name="limit">The maximum number of transactions to return.</param>
        /// <param name="offset">The offset to start from.</param>
        /// <returns>An array of asset transactions.</returns>
        public AssetTransaction[] GetAssetTransactions(string assetId, int limit, int offset)
        {
            EnsureInitialized();
            ValidateInput(assetId, nameof(assetId));

            try
            {
                IntPtr transactionsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_asset_explorer_get_asset_transactions(
                    assetId,
                    limit,
                    offset,
                    ref transactionsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get asset transactions");

                var transactions = new AssetTransaction[count];
                var currentPtr = transactionsPtr;

                for (int i = 0; i < count; i++)
                {
                    transactions[i] = Marshal.PtrToStructure<AssetTransaction>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<AssetTransaction>());
                }

                NativeMethods.satox_asset_explorer_free_asset_transactions(transactionsPtr, count);

                return transactions;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get asset transactions: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets asset statistics.
        /// </summary>
        /// <param name="assetId">The asset ID.</param>
        /// <returns>The asset statistics.</returns>
        public AssetStatistics GetAssetStatistics(string assetId)
        {
            EnsureInitialized();
            ValidateInput(assetId, nameof(assetId));

            try
            {
                IntPtr statsPtr = IntPtr.Zero;
                var status = NativeMethods.satox_asset_explorer_get_asset_statistics(
                    assetId,
                    ref statsPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get asset statistics");

                var stats = Marshal.PtrToStructure<AssetStatistics>(statsPtr);
                NativeMethods.satox_asset_explorer_free_asset_statistics(statsPtr);

                return stats;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get asset statistics: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("Asset explorer is not initialized");
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException("Value cannot be null or empty", paramName);
        }

        private static class NativeMethods
        {
            [DllImport("satox_assets")]
            public static extern int satox_asset_explorer_initialize();

            [DllImport("satox_assets")]
            public static extern int satox_asset_explorer_shutdown();

            [DllImport("satox_assets")]
            public static extern int satox_asset_explorer_get_asset_info(
                string assetId,
                ref IntPtr info
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_explorer_get_asset_holders(
                string assetId,
                ref IntPtr holders,
                ref int count
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_explorer_get_asset_transactions(
                string assetId,
                int limit,
                int offset,
                ref IntPtr transactions,
                ref int count
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_explorer_get_asset_statistics(
                string assetId,
                ref IntPtr statistics
            );

            [DllImport("satox_assets")]
            public static extern void satox_asset_explorer_free_asset_info(IntPtr info);

            [DllImport("satox_assets")]
            public static extern void satox_asset_explorer_free_asset_holders(IntPtr holders, int count);

            [DllImport("satox_assets")]
            public static extern void satox_asset_explorer_free_asset_transactions(IntPtr transactions, int count);

            [DllImport("satox_assets")]
            public static extern void satox_asset_explorer_free_asset_statistics(IntPtr statistics);
        }
    }

    /// <summary>
    /// Represents asset information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct AssetInfo
    {
        /// <summary>
        /// The asset ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Id;

        /// <summary>
        /// The asset name.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        /// <summary>
        /// The asset symbol.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Symbol;

        /// <summary>
        /// The number of decimal places.
        /// </summary>
        public int Decimals;

        /// <summary>
        /// The total supply.
        /// </summary>
        public double TotalSupply;

        /// <summary>
        /// The owner address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Owner;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long UpdatedAt;

        /// <summary>
        /// The number of holders.
        /// </summary>
        public int HolderCount;

        /// <summary>
        /// The number of transactions.
        /// </summary>
        public int TransactionCount;
    }

    /// <summary>
    /// Represents an asset holder.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct AssetHolder
    {
        /// <summary>
        /// The holder address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Address;

        /// <summary>
        /// The balance.
        /// </summary>
        public double Balance;

        /// <summary>
        /// The last transaction timestamp.
        /// </summary>
        public long LastTransactionAt;
    }

    /// <summary>
    /// Represents an asset transaction.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct AssetTransaction
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
        /// The amount.
        /// </summary>
        public double Amount;

        /// <summary>
        /// The transaction timestamp.
        /// </summary>
        public long Timestamp;

        /// <summary>
        /// The transaction type.
        /// </summary>
        public int Type;
    }

    /// <summary>
    /// Represents asset statistics.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct AssetStatistics
    {
        /// <summary>
        /// The total number of holders.
        /// </summary>
        public int TotalHolders;

        /// <summary>
        /// The total number of transactions.
        /// </summary>
        public int TotalTransactions;

        /// <summary>
        /// The total volume.
        /// </summary>
        public double TotalVolume;

        /// <summary>
        /// The average transaction size.
        /// </summary>
        public double AverageTransactionSize;

        /// <summary>
        /// The largest transaction size.
        /// </summary>
        public double LargestTransactionSize;

        /// <summary>
        /// The smallest transaction size.
        /// </summary>
        public double SmallestTransactionSize;

        /// <summary>
        /// The number of transactions in the last 24 hours.
        /// </summary>
        public int Transactions24h;

        /// <summary>
        /// The volume in the last 24 hours.
        /// </summary>
        public double Volume24h;
    }
}