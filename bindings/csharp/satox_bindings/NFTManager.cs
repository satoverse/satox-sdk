using System;
using System.Runtime.InteropServices;

namespace Satox.Bindings.NFT
{
    /// <summary>
    /// NFTManager provides NFT management functionality.
    /// </summary>
    public interface INFTManager : IDisposable
    {
        void Initialize();
        void Shutdown();
        NFT CreateNFT(string name, string description, string uri, string owner);
        NFT GetNFT(string nftId);
        NFT UpdateNFT(NFT nft);
        void DeleteNFT(string nftId);
        NFT[] GetAllNFTs();
        NFT[] GetNFTsByOwner(string owner);
        NFT TransferNFT(string nftId, string from, string to);
    }

    public class NFTManager : ThreadSafeManager, INFTManager
    {
        /// <summary>
        /// Creates a new instance of NFTManager.
        /// </summary>
        public NFTManager() : base()
        {
        }

        /// <summary>
        /// Initializes the NFT manager.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_nft_manager_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize NFT manager");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the NFT manager.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_nft_manager_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown NFT manager");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Creates a new NFT.
        /// </summary>
        /// <param name="name">The NFT name.</param>
        /// <param name="description">The NFT description.</param>
        /// <param name="uri">The NFT URI.</param>
        /// <param name="owner">The NFT owner.</param>
        /// <returns>The created NFT.</returns>
        public NFT CreateNFT(string name, string description, string uri, string owner)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(description, nameof(description));
            ValidateInput(uri, nameof(uri));
            ValidateInput(owner, nameof(owner));

            try
            {
                IntPtr nftPtr = IntPtr.Zero;
                var status = NativeMethods.satox_nft_manager_create_nft(
                    name,
                    description,
                    uri,
                    owner,
                    ref nftPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to create NFT");

                var nft = Marshal.PtrToStructure<NFT>(nftPtr);
                NativeMethods.satox_nft_manager_free_nft(nftPtr);

                return nft;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to create NFT: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets an NFT by its ID.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <returns>The NFT information.</returns>
        public NFT GetNFT(string nftId)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));

            try
            {
                IntPtr nftPtr = IntPtr.Zero;
                var status = NativeMethods.satox_nft_manager_get_nft(
                    nftId,
                    ref nftPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get NFT");

                var nft = Marshal.PtrToStructure<NFT>(nftPtr);
                NativeMethods.satox_nft_manager_free_nft(nftPtr);

                return nft;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get NFT: {ex.Message}");
            }
        }

        /// <summary>
        /// Updates an NFT.
        /// </summary>
        /// <param name="nft">The NFT to update.</param>
        /// <returns>The updated NFT.</returns>
        public NFT UpdateNFT(NFT nft)
        {
            EnsureInitialized();
            ValidateInput(nft, nameof(nft));

            try
            {
                IntPtr updatedNftPtr = IntPtr.Zero;
                var status = NativeMethods.satox_nft_manager_update_nft(
                    ref nft,
                    ref updatedNftPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to update NFT");

                var updatedNft = Marshal.PtrToStructure<NFT>(updatedNftPtr);
                NativeMethods.satox_nft_manager_free_nft(updatedNftPtr);

                return updatedNft;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to update NFT: {ex.Message}");
            }
        }

        /// <summary>
        /// Deletes an NFT.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        public void DeleteNFT(string nftId)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));

            try
            {
                var status = NativeMethods.satox_nft_manager_delete_nft(nftId);
                if (status != 0)
                    throw new SatoxError("Failed to delete NFT");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to delete NFT: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets all NFTs.
        /// </summary>
        /// <returns>An array of NFTs.</returns>
        public NFT[] GetAllNFTs()
        {
            EnsureInitialized();

            try
            {
                IntPtr nftsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_nft_manager_get_all_nfts(
                    ref nftsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get all NFTs");

                var nfts = new NFT[count];
                var currentPtr = nftsPtr;

                for (int i = 0; i < count; i++)
                {
                    nfts[i] = Marshal.PtrToStructure<NFT>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<NFT>());
                }

                NativeMethods.satox_nft_manager_free_nfts(nftsPtr, count);

                return nfts;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get all NFTs: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets NFTs by owner.
        /// </summary>
        /// <param name="owner">The owner address.</param>
        /// <returns>An array of NFTs.</returns>
        public NFT[] GetNFTsByOwner(string owner)
        {
            EnsureInitialized();
            ValidateInput(owner, nameof(owner));

            try
            {
                IntPtr nftsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_nft_manager_get_nfts_by_owner(
                    owner,
                    ref nftsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get NFTs by owner");

                var nfts = new NFT[count];
                var currentPtr = nftsPtr;

                for (int i = 0; i < count; i++)
                {
                    nfts[i] = Marshal.PtrToStructure<NFT>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<NFT>());
                }

                NativeMethods.satox_nft_manager_free_nfts(nftsPtr, count);

                return nfts;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get NFTs by owner: {ex.Message}");
            }
        }

        /// <summary>
        /// Transfers an NFT.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <param name="from">The sender address.</param>
        /// <param name="to">The recipient address.</param>
        /// <returns>The updated NFT.</returns>
        public NFT TransferNFT(string nftId, string from, string to)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));
            ValidateInput(from, nameof(from));
            ValidateInput(to, nameof(to));

            try
            {
                IntPtr nftPtr = IntPtr.Zero;
                var status = NativeMethods.satox_nft_manager_transfer_nft(
                    nftId,
                    from,
                    to,
                    ref nftPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to transfer NFT");

                var nft = Marshal.PtrToStructure<NFT>(nftPtr);
                NativeMethods.satox_nft_manager_free_nft(nftPtr);

                return nft;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to transfer NFT: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("NFT manager is not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
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
            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_initialize();

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_shutdown();

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_create_nft(
                string name,
                string description,
                string uri,
                string owner,
                ref IntPtr nft
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_get_nft(
                string nftId,
                ref IntPtr nft
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_update_nft(
                ref NFT nft,
                ref IntPtr updatedNft
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_delete_nft(string nftId);

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_get_all_nfts(
                ref IntPtr nfts,
                ref int count
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_get_nfts_by_owner(
                string owner,
                ref IntPtr nfts,
                ref int count
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_manager_transfer_nft(
                string nftId,
                string from,
                string to,
                ref IntPtr nft
            );

            [DllImport("satox_nft")]
            public static extern void satox_nft_manager_free_nft(IntPtr nft);

            [DllImport("satox_nft")]
            public static extern void satox_nft_manager_free_nfts(IntPtr nfts, int count);
        }
    }

    /// <summary>
    /// Represents an NFT.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct NFT
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
        /// The NFT owner.
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
        /// The number of transfers.
        /// </summary>
        public int TransferCount;
    }
}