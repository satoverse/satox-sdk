using System;
using System.Runtime.InteropServices;

namespace Satox.NFT
{
    /// <summary>
    /// NFTExplorer provides NFT exploration functionality.
    /// </summary>
    public class NFTExplorer : ThreadSafeManager
    {
        /// <summary>
        /// Creates a new instance of NFTExplorer.
        /// </summary>
        public NFTExplorer() : base()
        {
        }

        /// <summary>
        /// Initializes the NFT explorer.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_nft_explorer_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize NFT explorer");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the NFT explorer.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_nft_explorer_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown NFT explorer");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Gets NFT information.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <returns>The NFT information.</returns>
        public NFTInfo GetNFTInfo(string nftId)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_nft_explorer_get_nft_info(
                    nftId,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get NFT info");

                var info = Marshal.PtrToStructure<NFTInfo>(infoPtr);
                NativeMethods.satox_nft_explorer_free_nft_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get NFT info: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets NFT transactions.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <returns>An array of NFT transactions.</returns>
        public NFTTransaction[] GetNFTTransactions(string nftId)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));

            try
            {
                IntPtr transactionsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_nft_explorer_get_nft_transactions(
                    nftId,
                    ref transactionsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get NFT transactions");

                var transactions = new NFTTransaction[count];
                var currentPtr = transactionsPtr;

                for (int i = 0; i < count; i++)
                {
                    transactions[i] = Marshal.PtrToStructure<NFTTransaction>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<NFTTransaction>());
                }

                NativeMethods.satox_nft_explorer_free_nft_transactions(transactionsPtr, count);

                return transactions;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get NFT transactions: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets NFT owners.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <returns>An array of NFT owners.</returns>
        public NFTOwner[] GetNFTOwners(string nftId)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));

            try
            {
                IntPtr ownersPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_nft_explorer_get_nft_owners(
                    nftId,
                    ref ownersPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get NFT owners");

                var owners = new NFTOwner[count];
                var currentPtr = ownersPtr;

                for (int i = 0; i < count; i++)
                {
                    owners[i] = Marshal.PtrToStructure<NFTOwner>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<NFTOwner>());
                }

                NativeMethods.satox_nft_explorer_free_nft_owners(ownersPtr, count);

                return owners;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get NFT owners: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets NFT statistics.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <returns>The NFT statistics.</returns>
        public NFTStatistics GetNFTStatistics(string nftId)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));

            try
            {
                IntPtr statsPtr = IntPtr.Zero;
                var status = NativeMethods.satox_nft_explorer_get_nft_statistics(
                    nftId,
                    ref statsPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get NFT statistics");

                var stats = Marshal.PtrToStructure<NFTStatistics>(statsPtr);
                NativeMethods.satox_nft_explorer_free_nft_statistics(statsPtr);

                return stats;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get NFT statistics: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("NFT explorer is not initialized");
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException("Value cannot be null or empty", paramName);
        }

        private static class NativeMethods
        {
            [DllImport("satox_nft")]
            public static extern int satox_nft_explorer_initialize();

            [DllImport("satox_nft")]
            public static extern int satox_nft_explorer_shutdown();

            [DllImport("satox_nft")]
            public static extern int satox_nft_explorer_get_nft_info(
                string nftId,
                ref IntPtr info
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_explorer_get_nft_transactions(
                string nftId,
                ref IntPtr transactions,
                ref int count
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_explorer_get_nft_owners(
                string nftId,
                ref IntPtr owners,
                ref int count
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_explorer_get_nft_statistics(
                string nftId,
                ref IntPtr statistics
            );

            [DllImport("satox_nft")]
            public static extern void satox_nft_explorer_free_nft_info(IntPtr info);

            [DllImport("satox_nft")]
            public static extern void satox_nft_explorer_free_nft_transactions(IntPtr transactions, int count);

            [DllImport("satox_nft")]
            public static extern void satox_nft_explorer_free_nft_owners(IntPtr owners, int count);

            [DllImport("satox_nft")]
            public static extern void satox_nft_explorer_free_nft_statistics(IntPtr statistics);
        }
    }

    /// <summary>
    /// Represents NFT information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct NFTInfo
    {
        /// <summary>
        /// The NFT ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Id;

        /// <summary>
        /// The NFT name.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        /// <summary>
        /// The NFT description.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Description;

        /// <summary>
        /// The NFT URI.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Uri;

        /// <summary>
        /// The current owner.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string CurrentOwner;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long UpdatedAt;

        /// <summary>
        /// The number of transfers.
        /// </summary>
        public int TransferCount;

        /// <summary>
        /// The number of owners.
        /// </summary>
        public int OwnerCount;
    }

    /// <summary>
    /// Represents an NFT transaction.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct NFTTransaction
    {
        /// <summary>
        /// The transaction hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        /// <summary>
        /// The NFT ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string NFTId;

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
        /// The transaction timestamp.
        /// </summary>
        public long Timestamp;

        /// <summary>
        /// The transaction type.
        /// </summary>
        public int Type;
    }

    /// <summary>
    /// Represents an NFT owner.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct NFTOwner
    {
        /// <summary>
        /// The owner address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Address;

        /// <summary>
        /// The acquisition timestamp.
        /// </summary>
        public long AcquiredAt;

        /// <summary>
        /// The number of NFTs owned.
        /// </summary>
        public int NFTCount;
    }

    /// <summary>
    /// Represents NFT statistics.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct NFTStatistics
    {
        /// <summary>
        /// The total number of transfers.
        /// </summary>
        public int TotalTransfers;

        /// <summary>
        /// The total number of owners.
        /// </summary>
        public int TotalOwners;

        /// <summary>
        /// The average holding time in seconds.
        /// </summary>
        public long AverageHoldingTime;

        /// <summary>
        /// The last transfer timestamp.
        /// </summary>
        public long LastTransferAt;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;
    }
}