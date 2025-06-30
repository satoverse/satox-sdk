using System;
using System.Runtime.InteropServices;

namespace Satox.Wallet
{
    /// <summary>
    /// WalletExplorer provides wallet exploration functionality.
    /// </summary>
    public class WalletExplorer : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of WalletExplorer.
        /// </summary>
        public WalletExplorer()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the wallet explorer.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_wallet_explorer_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize wallet explorer");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the wallet explorer.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_wallet_explorer_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown wallet explorer");

            _initialized = false;
        }

        /// <summary>
        /// Gets wallet information by address.
        /// </summary>
        /// <param name="address">The wallet address.</param>
        /// <returns>The wallet information.</returns>
        public WalletInfo GetWalletInfo(string address)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_wallet_explorer_get_wallet_info(
                    address,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get wallet info");

                var info = Marshal.PtrToStructure<WalletInfo>(infoPtr);
                NativeMethods.satox_wallet_explorer_free_wallet_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get wallet info: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets wallet transactions.
        /// </summary>
        /// <param name="address">The wallet address.</param>
        /// <param name="limit">The maximum number of transactions to return.</param>
        /// <param name="offset">The offset to start from.</param>
        /// <returns>An array of wallet transactions.</returns>
        public WalletTransaction[] GetWalletTransactions(string address, int limit, int offset)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                IntPtr transactionsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_wallet_explorer_get_wallet_transactions(
                    address,
                    limit,
                    offset,
                    ref transactionsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get wallet transactions");

                var transactions = new WalletTransaction[count];
                var currentPtr = transactionsPtr;

                for (int i = 0; i < count; i++)
                {
                    transactions[i] = Marshal.PtrToStructure<WalletTransaction>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<WalletTransaction>());
                }

                NativeMethods.satox_wallet_explorer_free_wallet_transactions(transactionsPtr, count);

                return transactions;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get wallet transactions: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets wallet assets.
        /// </summary>
        /// <param name="address">The wallet address.</param>
        /// <returns>An array of wallet assets.</returns>
        public WalletAsset[] GetWalletAssets(string address)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                IntPtr assetsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_wallet_explorer_get_wallet_assets(
                    address,
                    ref assetsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get wallet assets");

                var assets = new WalletAsset[count];
                var currentPtr = assetsPtr;

                for (int i = 0; i < count; i++)
                {
                    assets[i] = Marshal.PtrToStructure<WalletAsset>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<WalletAsset>());
                }

                NativeMethods.satox_wallet_explorer_free_wallet_assets(assetsPtr, count);

                return assets;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get wallet assets: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets wallet statistics.
        /// </summary>
        /// <param name="address">The wallet address.</param>
        /// <returns>The wallet statistics.</returns>
        public WalletStatistics GetWalletStatistics(string address)
        {
            EnsureInitialized();
            ValidateInput(address, nameof(address));

            try
            {
                IntPtr statsPtr = IntPtr.Zero;
                var status = NativeMethods.satox_wallet_explorer_get_wallet_statistics(
                    address,
                    ref statsPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get wallet statistics");

                var stats = Marshal.PtrToStructure<WalletStatistics>(statsPtr);
                NativeMethods.satox_wallet_explorer_free_wallet_statistics(statsPtr);

                return stats;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get wallet statistics: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Wallet explorer is not initialized");
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

        ~WalletExplorer()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_wallet")]
            public static extern int satox_wallet_explorer_initialize();

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_explorer_shutdown();

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_explorer_get_wallet_info(
                string address,
                ref IntPtr info
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_explorer_get_wallet_transactions(
                string address,
                int limit,
                int offset,
                ref IntPtr transactions,
                ref int count
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_explorer_get_wallet_assets(
                string address,
                ref IntPtr assets,
                ref int count
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_explorer_get_wallet_statistics(
                string address,
                ref IntPtr statistics
            );

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_explorer_free_wallet_info(IntPtr info);

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_explorer_free_wallet_transactions(IntPtr transactions, int count);

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_explorer_free_wallet_assets(IntPtr assets, int count);

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_explorer_free_wallet_statistics(IntPtr statistics);
        }
    }

    /// <summary>
    /// Represents wallet information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct WalletInfo
    {
        /// <summary>
        /// The wallet address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Address;

        /// <summary>
        /// The wallet balance.
        /// </summary>
        public double Balance;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long UpdatedAt;

        /// <summary>
        /// The number of transactions.
        /// </summary>
        public int TransactionCount;

        /// <summary>
        /// The number of assets.
        /// </summary>
        public int AssetCount;
    }

    /// <summary>
    /// Represents a wallet transaction.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct WalletTransaction
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
    /// Represents a wallet asset.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct WalletAsset
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
        /// The balance.
        /// </summary>
        public double Balance;

        /// <summary>
        /// The last transaction timestamp.
        /// </summary>
        public long LastTransactionAt;
    }

    /// <summary>
    /// Represents wallet statistics.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct WalletStatistics
    {
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