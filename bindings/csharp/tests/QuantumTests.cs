using System;
using System.Threading.Tasks;
using Xunit;
using SatoxSDK.Quantum;

namespace SatoxSDK.Tests
{
    public class QuantumTests
    {
        [Fact]
        public async Task TestQuantumManagerInitialization()
        {
            var manager = new QuantumManager();
            Assert.NotNull(manager);
            await manager.InitializeAsync();
            Assert.True(manager.IsInitialized);
        }

        [Fact]
        public async Task TestCRYSTALSKyberEncryption()
        {
            var manager = new QuantumManager();
            await manager.InitializeAsync();

            var message = "Test message";
            var (publicKey, privateKey) = await manager.GenerateKeyPairAsync();

            var encrypted = await manager.EncryptAsync(message, publicKey);
            var decrypted = await manager.DecryptAsync(encrypted, privateKey);

            Assert.Equal(message, decrypted);
        }

        [Fact]
        public async Task TestNTRUEncryption()
        {
            var manager = new QuantumManager();
            await manager.InitializeAsync();

            var message = "Test message";
            var (publicKey, privateKey) = await manager.GenerateNTRUKeyPairAsync();

            var encrypted = await manager.EncryptNTRUAsync(message, publicKey);
            var decrypted = await manager.DecryptNTRUAsync(encrypted, privateKey);

            Assert.Equal(message, decrypted);
        }

        [Fact]
        public async Task TestHybridEncryption()
        {
            var manager = new QuantumManager();
            await manager.InitializeAsync();

            var message = "Test message";
            var (publicKey, privateKey) = await manager.GenerateHybridKeyPairAsync();

            var encrypted = await manager.EncryptHybridAsync(message, publicKey);
            var decrypted = await manager.DecryptHybridAsync(encrypted, privateKey);

            Assert.Equal(message, decrypted);
        }

        [Fact]
        public async Task TestKeyStorage()
        {
            var manager = new QuantumManager();
            await manager.InitializeAsync();

            var (publicKey, privateKey) = await manager.GenerateKeyPairAsync();
            await manager.StoreKeyPairAsync("test-key", publicKey, privateKey);

            var storedKeys = await manager.RetrieveKeyPairAsync("test-key");
            Assert.Equal(publicKey, storedKeys.PublicKey);
            Assert.Equal(privateKey, storedKeys.PrivateKey);
        }
    }
} 