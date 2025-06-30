using System;
using System.Collections.Generic;
using Xunit;

namespace Satox.NFT.Tests
{
    public class NFTManagerTests
    {
        [Fact]
        public void CreateNFT_Succeeds()
        {
            var manager = new NFTManager();
            var nft = new NFT { Id = "nft1", Name = "Test NFT", Owner = "Alice" };
            manager.CreateNFT(nft);
            var retrieved = manager.GetNFT("nft1");
            Assert.Equal(nft, retrieved);
        }

        [Fact]
        public void GetNFT_ReturnsNullIfNotFound()
        {
            var manager = new NFTManager();
            var nft = manager.GetNFT("nonexistent");
            Assert.Null(nft);
        }

        [Fact]
        public void TransferNFT_ChangesOwner()
        {
            var manager = new NFTManager();
            var nft = new NFT { Id = "nft2", Name = "Transferable", Owner = "Alice" };
            manager.CreateNFT(nft);
            manager.TransferNFT("nft2", "Bob");
            var transferred = manager.GetNFT("nft2");
            Assert.Equal("Bob", transferred.Owner);
        }

        [Fact]
        public void UpdateMetadata_ChangesMetadata()
        {
            var manager = new NFTManager();
            var nft = new NFT { Id = "nft3", Name = "MetaNFT", Metadata = new Dictionary<string, object>() };
            manager.CreateNFT(nft);
            var newMetadata = new Dictionary<string, object> { { "key", "value" } };
            manager.UpdateMetadata("nft3", newMetadata);
            var updated = manager.GetNFT("nft3");
            Assert.Equal("value", updated.Metadata["key"]);
        }

        [Fact]
        public void GetNFTsByCollection_ReturnsCorrectNFTs()
        {
            var manager = new NFTManager();
            var nft1 = new NFT { Id = "nft4", Name = "NFT1", Collection = "col1" };
            var nft2 = new NFT { Id = "nft5", Name = "NFT2", Collection = "col1" };
            var nft3 = new NFT { Id = "nft6", Name = "NFT3", Collection = "col2" };
            manager.CreateNFT(nft1);
            manager.CreateNFT(nft2);
            manager.CreateNFT(nft3);
            var col1NFTs = manager.GetNFTsByCollection("col1");
            Assert.Contains(nft1, col1NFTs);
            Assert.Contains(nft2, col1NFTs);
            Assert.DoesNotContain(nft3, col1NFTs);
        }

        [Fact]
        public void GetNFTsByOwner_ReturnsCorrectNFTs()
        {
            var manager = new NFTManager();
            var nft1 = new NFT { Id = "nft7", Name = "NFT1", Owner = "Alice" };
            var nft2 = new NFT { Id = "nft8", Name = "NFT2", Owner = "Bob" };
            var nft3 = new NFT { Id = "nft9", Name = "NFT3", Owner = "Alice" };
            manager.CreateNFT(nft1);
            manager.CreateNFT(nft2);
            manager.CreateNFT(nft3);
            var aliceNFTs = manager.GetNFTsByOwner("Alice");
            Assert.Contains(nft1, aliceNFTs);
            Assert.Contains(nft3, aliceNFTs);
            Assert.DoesNotContain(nft2, aliceNFTs);
        }

        [Fact]
        public void CreateNFT_ThrowsIfDuplicate()
        {
            var manager = new NFTManager();
            var nft = new NFT { Id = "nft10", Name = "DupNFT" };
            manager.CreateNFT(nft);
            Assert.Throws<InvalidOperationException>(() => manager.CreateNFT(nft));
        }

        [Fact]
        public void UpdateMetadata_ThrowsIfNotFound()
        {
            var manager = new NFTManager();
            var newMetadata = new Dictionary<string, object> { { "key", "value" } };
            Assert.Throws<InvalidOperationException>(() => manager.UpdateMetadata("ghost", newMetadata));
        }

        [Fact]
        public void TransferNFT_ThrowsIfNotFound()
        {
            var manager = new NFTManager();
            Assert.Throws<InvalidOperationException>(() => manager.TransferNFT("ghost", "Bob"));
        }
    }
} 