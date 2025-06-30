using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Moq;
using Xunit;

namespace Satox.Assets.Tests
{
    public class AssetManagerTests
    {
        [Fact]
        public void CreateAsset_Succeeds()
        {
            var manager = new AssetManager();
            var asset = new Asset { Id = "asset1", Name = "Test Asset", Supply = 1000 };
            manager.CreateAsset(asset);
            var retrieved = manager.GetAsset("asset1");
            Assert.Equal(asset, retrieved);
        }

        [Fact]
        public void GetAsset_ReturnsNullIfNotFound()
        {
            var manager = new AssetManager();
            var asset = manager.GetAsset("nonexistent");
            Assert.Null(asset);
        }

        [Fact]
        public void UpdateAsset_UpdatesFields()
        {
            var manager = new AssetManager();
            var asset = new Asset { Id = "asset2", Name = "Old Name", Supply = 500 };
            manager.CreateAsset(asset);
            asset.Name = "New Name";
            asset.Supply = 2000;
            manager.UpdateAsset(asset);
            var updated = manager.GetAsset("asset2");
            Assert.Equal("New Name", updated.Name);
            Assert.Equal(2000, updated.Supply);
        }

        [Fact]
        public void TransferAsset_ChangesOwner()
        {
            var manager = new AssetManager();
            var asset = new Asset { Id = "asset3", Name = "Transferable", Owner = "Alice" };
            manager.CreateAsset(asset);
            manager.TransferAsset("asset3", "Bob");
            var transferred = manager.GetAsset("asset3");
            Assert.Equal("Bob", transferred.Owner);
        }

        [Fact]
        public void UpdateMetadata_ChangesMetadata()
        {
            var manager = new AssetManager();
            var asset = new Asset { Id = "asset4", Name = "MetaAsset", Metadata = new Dictionary<string, object>() };
            manager.CreateAsset(asset);
            var newMetadata = new Dictionary<string, object> { { "key", "value" } };
            manager.UpdateMetadata("asset4", newMetadata);
            var updated = manager.GetAsset("asset4");
            Assert.Equal("value", updated.Metadata["key"]);
        }

        [Fact]
        public void CreateAsset_ThrowsIfDuplicate()
        {
            var manager = new AssetManager();
            var asset = new Asset { Id = "asset5", Name = "DupAsset" };
            manager.CreateAsset(asset);
            Assert.Throws<InvalidOperationException>(() => manager.CreateAsset(asset));
        }

        [Fact]
        public void UpdateAsset_ThrowsIfNotFound()
        {
            var manager = new AssetManager();
            var asset = new Asset { Id = "nonexistent", Name = "Ghost" };
            Assert.Throws<InvalidOperationException>(() => manager.UpdateAsset(asset));
        }

        [Fact]
        public void TransferAsset_ThrowsIfNotFound()
        {
            var manager = new AssetManager();
            Assert.Throws<InvalidOperationException>(() => manager.TransferAsset("ghost", "Bob"));
        }
    }
} 