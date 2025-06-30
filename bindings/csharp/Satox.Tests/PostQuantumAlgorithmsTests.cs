using System;
using System.Threading.Tasks;
using Xunit;
using Satox.Quantum;

namespace Satox.Tests
{
    public class PostQuantumAlgorithmsTests : IDisposable
    {
        private readonly PostQuantumAlgorithms _algorithms;

        public PostQuantumAlgorithmsTests()
        {
            _algorithms = new PostQuantumAlgorithms();
            _algorithms.Initialize();
        }

        public void Dispose()
        {
            _algorithms.Shutdown();
            _algorithms.Dispose();
        }

        [Fact]
        public void TestInitialization()
        {
            Assert.NotNull(_algorithms);
            Assert.True(_algorithms.IsInitialized());
        }

        [Fact]
        public void TestDoubleInitialization()
        {
            Assert.Throws<SatoxError>(() => _algorithms.Initialize());
        }

        [Fact]
        public void TestGetAvailableAlgorithms()
        {
            var algorithms = _algorithms.GetAvailableAlgorithms();
            Assert.NotNull(algorithms);
            Assert.NotEmpty(algorithms);
            Assert.Contains("CRYSTALS-Kyber", algorithms);
            Assert.Contains("NTRU", algorithms);
            Assert.Contains("SABER", algorithms);
            Assert.Contains("CRYSTALS-Dilithium", algorithms);
            Assert.Contains("FALCON", algorithms);
            Assert.Contains("SPHINCS+", algorithms);
        }

        [Fact]
        public void TestGetAlgorithmInfo()
        {
            var info = _algorithms.GetAlgorithmInfo("CRYSTALS-Kyber");
            Assert.NotNull(info);
            Assert.Equal("CRYSTALS-Kyber", info.Name);
            Assert.True(info.SecurityLevel > 0);
            Assert.True(info.KeySize > 0);
            Assert.True(info.SignatureSize > 0);
            Assert.True(info.IsRecommended);
            Assert.NotNull(info.Description);
            Assert.NotEmpty(info.Description);
        }

        [Fact]
        public void TestGetInvalidAlgorithmInfo()
        {
            Assert.Throws<SatoxError>(() => _algorithms.GetAlgorithmInfo("InvalidAlgorithm"));
        }

        [Fact]
        public async Task TestGenerateKeyPair()
        {
            var (publicKey, privateKey) = await _algorithms.GenerateKeyPair("CRYSTALS-Kyber");
            Assert.NotNull(publicKey);
            Assert.NotNull(privateKey);
            Assert.NotEmpty(publicKey);
            Assert.NotEmpty(privateKey);
        }

        [Fact]
        public async Task TestSignAndVerify()
        {
            var (publicKey, privateKey) = await _algorithms.GenerateKeyPair("CRYSTALS-Kyber");
            var data = new byte[] { 1, 2, 3, 4, 5 };

            var signature = await _algorithms.Sign(privateKey, data);
            Assert.NotNull(signature);
            Assert.NotEmpty(signature);

            var isValid = await _algorithms.Verify(publicKey, data, signature);
            Assert.True(isValid);
        }

        [Fact]
        public async Task TestEncryptAndDecrypt()
        {
            var (publicKey, privateKey) = await _algorithms.GenerateKeyPair("CRYSTALS-Kyber");
            var data = new byte[] { 1, 2, 3, 4, 5 };

            var encrypted = await _algorithms.Encrypt(publicKey, data);
            Assert.NotNull(encrypted);
            Assert.NotEmpty(encrypted);

            var decrypted = await _algorithms.Decrypt(privateKey, encrypted);
            Assert.Equal(data, decrypted);
        }

        [Fact]
        public void TestIsAlgorithmAvailable()
        {
            Assert.True(_algorithms.IsAlgorithmAvailable("CRYSTALS-Kyber"));
            Assert.False(_algorithms.IsAlgorithmAvailable("InvalidAlgorithm"));
        }

        [Fact]
        public void TestIsAlgorithmRecommended()
        {
            Assert.True(_algorithms.IsAlgorithmRecommended("CRYSTALS-Kyber"));
            Assert.False(_algorithms.IsAlgorithmRecommended("NTRU"));
        }

        [Fact]
        public void TestGetAlgorithmSecurityLevel()
        {
            var securityLevel = _algorithms.GetAlgorithmSecurityLevel("CRYSTALS-Kyber");
            Assert.True(securityLevel > 0);
        }

        [Fact]
        public void TestGetAlgorithmKeySize()
        {
            var keySize = _algorithms.GetAlgorithmKeySize("CRYSTALS-Kyber");
            Assert.True(keySize > 0);
        }

        [Fact]
        public void TestGetAlgorithmSignatureSize()
        {
            var signatureSize = _algorithms.GetAlgorithmSignatureSize("CRYSTALS-Kyber");
            Assert.True(signatureSize > 0);
        }

        [Fact]
        public void TestGetDefaultAlgorithm()
        {
            var defaultAlgorithm = _algorithms.GetDefaultAlgorithm();
            Assert.NotNull(defaultAlgorithm);
            Assert.NotEmpty(defaultAlgorithm);
            Assert.True(_algorithms.IsAlgorithmAvailable(defaultAlgorithm));
        }

        [Fact]
        public void TestSetDefaultAlgorithm()
        {
            var originalDefault = _algorithms.GetDefaultAlgorithm();
            var newDefault = "CRYSTALS-Kyber";

            Assert.True(_algorithms.SetDefaultAlgorithm(newDefault));
            Assert.Equal(newDefault, _algorithms.GetDefaultAlgorithm());

            // Restore original default
            _algorithms.SetDefaultAlgorithm(originalDefault);
        }

        [Fact]
        public void TestOperationsAfterShutdown()
        {
            _algorithms.Shutdown();
            Assert.Throws<SatoxError>(() => _algorithms.GetAvailableAlgorithms());
        }

        [Fact]
        public void TestDispose()
        {
            _algorithms.Dispose();
            Assert.Throws<ObjectDisposedException>(() => _algorithms.IsInitialized());
        }

        [Fact]
        public void TestAlgorithmSecurityLevels()
        {
            var kyberInfo = _algorithms.GetAlgorithmInfo("CRYSTALS-Kyber");
            var ntruInfo = _algorithms.GetAlgorithmInfo("NTRU");
            var dilithiumInfo = _algorithms.GetAlgorithmInfo("CRYSTALS-Dilithium");

            Assert.True(kyberInfo.SecurityLevel > ntruInfo.SecurityLevel);
            Assert.True(kyberInfo.SecurityLevel > dilithiumInfo.SecurityLevel);
        }

        [Fact]
        public void TestAlgorithmKeySizes()
        {
            var kyberInfo = _algorithms.GetAlgorithmInfo("CRYSTALS-Kyber");
            var dilithiumInfo = _algorithms.GetAlgorithmInfo("CRYSTALS-Dilithium");

            Assert.True(kyberInfo.KeySize > 0);
            Assert.True(dilithiumInfo.SignatureSize > 0);
        }

        [Fact]
        public void TestAlgorithmDescriptions()
        {
            foreach (var algorithm in _algorithms.GetAvailableAlgorithms())
            {
                var info = _algorithms.GetAlgorithmInfo(algorithm);
                Assert.NotNull(info.Description);
                Assert.NotEmpty(info.Description);
            }
        }
    }
} 