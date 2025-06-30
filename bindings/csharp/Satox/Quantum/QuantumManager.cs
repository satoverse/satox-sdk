using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Satox.Bindings.Quantum;

namespace Satox.Quantum
{
    /// <summary>
    /// Manages quantum-resistant cryptographic operations.
    /// </summary>
    public class QuantumManager : IQuantumManager
    {
        private readonly IQuantumManager _manager;

        /// <summary>
        /// Initializes a new instance of the QuantumManager class.
        /// </summary>
        public QuantumManager()
        {
            _manager = new Satox.Bindings.Quantum.QuantumManager();
        }

        /// <summary>
        /// Initializes the quantum manager.
        /// </summary>
        /// <returns>True if initialization was successful, false otherwise.</returns>
        public bool Initialize() => _manager.Initialize();

        /// <summary>
        /// Shuts down the quantum manager.
        /// </summary>
        /// <returns>True if shutdown was successful, false otherwise.</returns>
        public bool Shutdown() => _manager.Shutdown();

        /// <summary>
        /// Generates a quantum-resistant key pair.
        /// </summary>
        /// <returns>A tuple containing the public and private keys.</returns>
        public Task<(byte[] PublicKey, byte[] PrivateKey)> GenerateKeyPair() => _manager.GenerateKeyPair();

        /// <summary>
        /// Encrypts data using the provided public key.
        /// </summary>
        /// <param name="publicKey">The public key to use for encryption.</param>
        /// <param name="data">The data to encrypt.</param>
        /// <returns>The encrypted data.</returns>
        public Task<byte[]> Encrypt(byte[] publicKey, byte[] data) => _manager.Encrypt(publicKey, data);

        /// <summary>
        /// Decrypts data using the provided private key.
        /// </summary>
        /// <param name="privateKey">The private key to use for decryption.</param>
        /// <param name="encryptedData">The data to decrypt.</param>
        /// <returns>The decrypted data.</returns>
        public Task<byte[]> Decrypt(byte[] privateKey, byte[] encryptedData) => _manager.Decrypt(privateKey, encryptedData);

        /// <summary>
        /// Signs data using the provided private key.
        /// </summary>
        /// <param name="privateKey">The private key to use for signing.</param>
        /// <param name="data">The data to sign.</param>
        /// <returns>The signature.</returns>
        public Task<byte[]> Sign(byte[] privateKey, byte[] data) => _manager.Sign(privateKey, data);

        /// <summary>
        /// Verifies a signature using the provided public key.
        /// </summary>
        /// <param name="publicKey">The public key to use for verification.</param>
        /// <param name="data">The data that was signed.</param>
        /// <param name="signature">The signature to verify.</param>
        /// <returns>True if the signature is valid, false otherwise.</returns>
        public Task<bool> Verify(byte[] publicKey, byte[] data, byte[] signature) => _manager.Verify(publicKey, data, signature);

        /// <summary>
        /// Generates a quantum-resistant random number within the specified range.
        /// </summary>
        /// <param name="minValue">The minimum value (inclusive).</param>
        /// <param name="maxValue">The maximum value (inclusive).</param>
        /// <returns>A random number within the specified range.</returns>
        public Task<int> GenerateRandomNumber(int minValue, int maxValue) => _manager.GenerateRandomNumber(minValue, maxValue);

        /// <summary>
        /// Generates quantum-resistant random bytes of the specified length.
        /// </summary>
        /// <param name="length">The number of random bytes to generate.</param>
        /// <returns>An array of random bytes.</returns>
        public Task<byte[]> GenerateRandomBytes(int length) => _manager.GenerateRandomBytes(length);

        /// <summary>
        /// Checks if the quantum manager is initialized.
        /// </summary>
        /// <returns>True if initialized, false otherwise.</returns>
        public bool IsInitialized() => _manager.IsInitialized();

        /// <summary>
        /// Gets the version of the quantum manager.
        /// </summary>
        /// <returns>The version string.</returns>
        public string GetVersion() => _manager.GetVersion();

        /// <summary>
        /// Gets the current quantum algorithm being used.
        /// </summary>
        /// <returns>The algorithm name.</returns>
        public string GetAlgorithm() => _manager.GetAlgorithm();

        /// <summary>
        /// Gets a list of available quantum algorithms.
        /// </summary>
        /// <returns>An array of algorithm names.</returns>
        public string[] GetAvailableAlgorithms() => _manager.GetAvailableAlgorithms();

        /// <summary>
        /// Disposes of the quantum manager.
        /// </summary>
        public void Dispose() => _manager.Dispose();
    }
} 