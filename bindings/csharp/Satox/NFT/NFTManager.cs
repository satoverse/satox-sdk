using System;
using System.Runtime.InteropServices;
using satox_bindings;
using Satox.Bindings.NFT;

namespace Satox.NFT
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

    public class NFTManager : INFTManager
    {
        private readonly Satox.Bindings.NFT.NFTManager _manager;

        /// <summary>
        /// Creates a new instance of NFTManager.
        /// </summary>
        public NFTManager()
        {
            _manager = new Satox.Bindings.NFT.NFTManager();
        }

        /// <summary>
        /// Initializes the NFT manager.
        /// </summary>
        public void Initialize()
        {
            _manager.Initialize();
        }

        /// <summary>
        /// Shuts down the NFT manager.
        /// </summary>
        public void Shutdown()
        {
            _manager.Shutdown();
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
            return _manager.CreateNFT(name, description, uri, owner);
        }

        /// <summary>
        /// Gets an NFT by its ID.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <returns>The NFT information.</returns>
        public NFT GetNFT(string nftId)
        {
            return _manager.GetNFT(nftId);
        }

        /// <summary>
        /// Updates an NFT.
        /// </summary>
        /// <param name="nft">The NFT to update.</param>
        /// <returns>The updated NFT.</returns>
        public NFT UpdateNFT(NFT nft)
        {
            return _manager.UpdateNFT(nft);
        }

        /// <summary>
        /// Deletes an NFT.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        public void DeleteNFT(string nftId)
        {
            _manager.DeleteNFT(nftId);
        }

        /// <summary>
        /// Gets all NFTs.
        /// </summary>
        /// <returns>An array of NFTs.</returns>
        public NFT[] GetAllNFTs()
        {
            return _manager.GetAllNFTs();
        }

        /// <summary>
        /// Gets NFTs by owner.
        /// </summary>
        /// <param name="owner">The owner address.</param>
        /// <returns>An array of NFTs.</returns>
        public NFT[] GetNFTsByOwner(string owner)
        {
            return _manager.GetNFTsByOwner(owner);
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
            return _manager.TransferNFT(nftId, from, to);
        }

        public void Dispose()
        {
            _manager.Dispose();
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