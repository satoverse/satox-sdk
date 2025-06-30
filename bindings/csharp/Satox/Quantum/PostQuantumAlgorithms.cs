using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Satox.Quantum
{
    /// <summary>
    /// Represents information about a post-quantum algorithm.
    /// </summary>
    public class AlgorithmInfo
    {
        /// <summary>
        /// Gets or sets the name of the algorithm.
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Gets or sets the security level of the algorithm.
        /// </summary>
        public int SecurityLevel { get; set; }

        /// <summary>
        /// Gets or sets the key size of the algorithm.
        /// </summary>
        public int KeySize { get; set; }

        /// <summary>
        /// Gets or sets the signature size of the algorithm.
        /// </summary>
        public int SignatureSize { get; set; }

        /// <summary>
        /// Gets or sets whether the algorithm is recommended.
        /// </summary>
        public bool IsRecommended { get; set; }

        /// <summary>
        /// Gets or sets the description of the algorithm.
        /// </summary>
        public string Description { get; set; }
    }

    /// <summary>
    /// Provides access to post-quantum cryptographic algorithms.
    /// </summary>
    public class PostQuantumAlgorithms : IDisposable
    {
        private bool _isInitialized;
        private bool _disposed;

        /// <summary>
        /// Initializes a new instance of the PostQuantumAlgorithms class.
        /// </summary>
        public PostQuantumAlgorithms()
        {
            _isInitialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the post-quantum algorithms system.
        /// </summary>
        /// <returns>True if initialization was successful, false otherwise.</returns>
        public bool Initialize()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(PostQuantumAlgorithms));

            if (_isInitialized)
                return true;

            try
            {
                var result = NativeMethods.satox_post_quantum_algorithms_initialize();
                _isInitialized = result;
                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to initialize post-quantum algorithms: {ex.Message}");
            }
        }

        /// <summary>
        /// Shuts down the post-quantum algorithms system.
        /// </summary>
        /// <returns>True if shutdown was successful, false otherwise.</returns>
        public bool Shutdown()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(PostQuantumAlgorithms));

            if (!_isInitialized)
                return true;

            try
            {
                var result = NativeMethods.satox_post_quantum_algorithms_shutdown();
                _isInitialized = !result;
                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to shutdown post-quantum algorithms: {ex.Message}");
            }
        }

        /// <summary>
        /// Generates a key pair for the specified algorithm.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm to use.</param>
        /// <returns>A tuple containing the public and private keys.</returns>
        public async Task<(byte[] PublicKey, byte[] PrivateKey)> GenerateKeyPair(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr publicKeyPtr = IntPtr.Zero;
                    IntPtr privateKeyPtr = IntPtr.Zero;
                    int publicKeyLength = 0;
                    int privateKeyLength = 0;

                    var status = NativeMethods.satox_post_quantum_algorithms_generate_key_pair(
                        algorithmName,
                        ref publicKeyPtr,
                        ref publicKeyLength,
                        ref privateKeyPtr,
                        ref privateKeyLength
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to generate key pair");

                    var publicKey = new byte[publicKeyLength];
                    var privateKey = new byte[privateKeyLength];

                    Marshal.Copy(publicKeyPtr, publicKey, 0, publicKeyLength);
                    Marshal.Copy(privateKeyPtr, privateKey, 0, privateKeyLength);

                    NativeMethods.satox_post_quantum_algorithms_free_buffer(publicKeyPtr);
                    NativeMethods.satox_post_quantum_algorithms_free_buffer(privateKeyPtr);

                    return (publicKey, privateKey);
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to generate key pair: {ex.Message}");
            }
        }

        /// <summary>
        /// Signs data using the provided private key.
        /// </summary>
        /// <param name="privateKey">The private key to use for signing.</param>
        /// <param name="data">The data to sign.</param>
        /// <returns>The signature.</returns>
        public async Task<byte[]> Sign(byte[] privateKey, byte[] data)
        {
            EnsureInitialized();
            ValidateInput(privateKey, nameof(privateKey));
            ValidateInput(data, nameof(data));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr signaturePtr = IntPtr.Zero;
                    int signatureLength = 0;

                    var status = NativeMethods.satox_post_quantum_algorithms_sign(
                        privateKey,
                        privateKey.Length,
                        data,
                        data.Length,
                        ref signaturePtr,
                        ref signatureLength
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to sign data");

                    var signature = new byte[signatureLength];
                    Marshal.Copy(signaturePtr, signature, 0, signatureLength);
                    NativeMethods.satox_post_quantum_algorithms_free_buffer(signaturePtr);

                    return signature;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to sign data: {ex.Message}");
            }
        }

        /// <summary>
        /// Verifies a signature using the provided public key.
        /// </summary>
        /// <param name="publicKey">The public key to use for verification.</param>
        /// <param name="data">The data that was signed.</param>
        /// <param name="signature">The signature to verify.</param>
        /// <returns>True if the signature is valid, false otherwise.</returns>
        public async Task<bool> Verify(byte[] publicKey, byte[] data, byte[] signature)
        {
            EnsureInitialized();
            ValidateInput(publicKey, nameof(publicKey));
            ValidateInput(data, nameof(data));
            ValidateInput(signature, nameof(signature));

            try
            {
                return await Task.Run(() =>
                {
                    int isValid = 0;
                    var status = NativeMethods.satox_post_quantum_algorithms_verify(
                        publicKey,
                        publicKey.Length,
                        data,
                        data.Length,
                        signature,
                        signature.Length,
                        ref isValid
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to verify signature");

                    return isValid != 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to verify signature: {ex.Message}");
            }
        }

        /// <summary>
        /// Encrypts data using the provided public key.
        /// </summary>
        /// <param name="publicKey">The public key to use for encryption.</param>
        /// <param name="data">The data to encrypt.</param>
        /// <returns>The encrypted data.</returns>
        public async Task<byte[]> Encrypt(byte[] publicKey, byte[] data)
        {
            EnsureInitialized();
            ValidateInput(publicKey, nameof(publicKey));
            ValidateInput(data, nameof(data));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr encryptedDataPtr = IntPtr.Zero;
                    int encryptedDataLength = 0;

                    var status = NativeMethods.satox_post_quantum_algorithms_encrypt(
                        publicKey,
                        publicKey.Length,
                        data,
                        data.Length,
                        ref encryptedDataPtr,
                        ref encryptedDataLength
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to encrypt data");

                    var encryptedData = new byte[encryptedDataLength];
                    Marshal.Copy(encryptedDataPtr, encryptedData, 0, encryptedDataLength);
                    NativeMethods.satox_post_quantum_algorithms_free_buffer(encryptedDataPtr);

                    return encryptedData;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to encrypt data: {ex.Message}");
            }
        }

        /// <summary>
        /// Decrypts data using the provided private key.
        /// </summary>
        /// <param name="privateKey">The private key to use for decryption.</param>
        /// <param name="encryptedData">The data to decrypt.</param>
        /// <returns>The decrypted data.</returns>
        public async Task<byte[]> Decrypt(byte[] privateKey, byte[] encryptedData)
        {
            EnsureInitialized();
            ValidateInput(privateKey, nameof(privateKey));
            ValidateInput(encryptedData, nameof(encryptedData));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr decryptedDataPtr = IntPtr.Zero;
                    int decryptedDataLength = 0;

                    var status = NativeMethods.satox_post_quantum_algorithms_decrypt(
                        privateKey,
                        privateKey.Length,
                        encryptedData,
                        encryptedData.Length,
                        ref decryptedDataPtr,
                        ref decryptedDataLength
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to decrypt data");

                    var decryptedData = new byte[decryptedDataLength];
                    Marshal.Copy(decryptedDataPtr, decryptedData, 0, decryptedDataLength);
                    NativeMethods.satox_post_quantum_algorithms_free_buffer(decryptedDataPtr);

                    return decryptedData;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to decrypt data: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets information about a specific algorithm.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm.</param>
        /// <returns>Information about the algorithm.</returns>
        public AlgorithmInfo GetAlgorithmInfo(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                IntPtr namePtr = IntPtr.Zero;
                int securityLevel = 0;
                int keySize = 0;
                int signatureSize = 0;
                int isRecommended = 0;
                IntPtr descriptionPtr = IntPtr.Zero;

                var status = NativeMethods.satox_post_quantum_algorithms_get_algorithm_info(
                    algorithmName,
                    ref namePtr,
                    ref securityLevel,
                    ref keySize,
                    ref signatureSize,
                    ref isRecommended,
                    ref descriptionPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get algorithm info");

                var info = new AlgorithmInfo
                {
                    Name = Marshal.PtrToStringAnsi(namePtr),
                    SecurityLevel = securityLevel,
                    KeySize = keySize,
                    SignatureSize = signatureSize,
                    IsRecommended = isRecommended != 0,
                    Description = Marshal.PtrToStringAnsi(descriptionPtr)
                };

                NativeMethods.satox_post_quantum_algorithms_free_buffer(namePtr);
                NativeMethods.satox_post_quantum_algorithms_free_buffer(descriptionPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get algorithm info: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a list of available algorithms.
        /// </summary>
        /// <returns>An array of algorithm names.</returns>
        public string[] GetAvailableAlgorithms()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_get_available_algorithms(
                    ref algorithmsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get available algorithms");

                var algorithms = new string[count];
                var currentPtr = algorithmsPtr;

                for (int i = 0; i < count; i++)
                {
                    algorithms[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr));
                    currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                }

                NativeMethods.satox_post_quantum_algorithms_free_buffer(algorithmsPtr);
                return algorithms;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get available algorithms: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a list of recommended algorithms.
        /// </summary>
        /// <returns>An array of algorithm names.</returns>
        public string[] GetRecommendedAlgorithms()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_get_recommended_algorithms(
                    ref algorithmsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get recommended algorithms");

                var algorithms = new string[count];
                var currentPtr = algorithmsPtr;

                for (int i = 0; i < count; i++)
                {
                    algorithms[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr));
                    currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                }

                NativeMethods.satox_post_quantum_algorithms_free_buffer(algorithmsPtr);
                return algorithms;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get recommended algorithms: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if an algorithm is available.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm to check.</param>
        /// <returns>True if the algorithm is available, false otherwise.</returns>
        public bool IsAlgorithmAvailable(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                int isAvailable = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_is_algorithm_available(
                    algorithmName,
                    ref isAvailable
                );
                return status == 0 && isAvailable != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check algorithm availability: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if an algorithm is recommended.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm to check.</param>
        /// <returns>True if the algorithm is recommended, false otherwise.</returns>
        public bool IsAlgorithmRecommended(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                int isRecommended = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_is_algorithm_recommended(
                    algorithmName,
                    ref isRecommended
                );
                return status == 0 && isRecommended != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check algorithm recommendation: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the security level of an algorithm.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm.</param>
        /// <returns>The security level.</returns>
        public int GetAlgorithmSecurityLevel(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                int securityLevel = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_get_algorithm_security_level(
                    algorithmName,
                    ref securityLevel
                );

                if (status != 0)
                    throw new SatoxError("Failed to get algorithm security level");

                return securityLevel;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get algorithm security level: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the key size of an algorithm.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm.</param>
        /// <returns>The key size in bits.</returns>
        public int GetAlgorithmKeySize(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                int keySize = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_get_algorithm_key_size(
                    algorithmName,
                    ref keySize
                );

                if (status != 0)
                    throw new SatoxError("Failed to get algorithm key size");

                return keySize;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get algorithm key size: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the signature size of an algorithm.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm.</param>
        /// <returns>The signature size in bytes.</returns>
        public int GetAlgorithmSignatureSize(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                int signatureSize = 0;
                var status = NativeMethods.satox_post_quantum_algorithms_get_algorithm_signature_size(
                    algorithmName,
                    ref signatureSize
                );

                if (status != 0)
                    throw new SatoxError("Failed to get algorithm signature size");

                return signatureSize;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get algorithm signature size: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the default algorithm.
        /// </summary>
        /// <returns>The name of the default algorithm.</returns>
        public string GetDefaultAlgorithm()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmPtr = IntPtr.Zero;
                var status = NativeMethods.satox_post_quantum_algorithms_get_default_algorithm(
                    ref algorithmPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get default algorithm");

                var algorithm = Marshal.PtrToStringAnsi(algorithmPtr);
                NativeMethods.satox_post_quantum_algorithms_free_buffer(algorithmPtr);

                return algorithm;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get default algorithm: {ex.Message}");
            }
        }

        /// <summary>
        /// Sets the default algorithm.
        /// </summary>
        /// <param name="algorithmName">The name of the algorithm to set as default.</param>
        /// <returns>True if the default algorithm was set successfully, false otherwise.</returns>
        public bool SetDefaultAlgorithm(string algorithmName)
        {
            EnsureInitialized();
            ValidateInput(algorithmName, nameof(algorithmName));

            try
            {
                var status = NativeMethods.satox_post_quantum_algorithms_set_default_algorithm(
                    algorithmName
                );
                return status == 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to set default algorithm: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if the post-quantum algorithms system is initialized.
        /// </summary>
        /// <returns>True if initialized, false otherwise.</returns>
        public bool IsInitialized()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(PostQuantumAlgorithms));

            return _isInitialized;
        }

        /// <summary>
        /// Disposes of the post-quantum algorithms system.
        /// </summary>
        public void Dispose()
        {
            if (!_disposed)
            {
                if (_isInitialized)
                    Shutdown();
                _disposed = true;
            }
        }

        private void EnsureInitialized()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(PostQuantumAlgorithms));

            if (!_isInitialized)
                throw new SatoxError("Post-quantum algorithms system is not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private static class NativeMethods
        {
            private const string DllName = "satox_quantum";

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_initialize();

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_shutdown();

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_generate_key_pair(
                string algorithmName,
                ref IntPtr publicKey,
                ref int publicKeyLength,
                ref IntPtr privateKey,
                ref int privateKeyLength
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_sign(
                byte[] privateKey,
                int privateKeyLength,
                byte[] data,
                int dataLength,
                ref IntPtr signature,
                ref int signatureLength
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_verify(
                byte[] publicKey,
                int publicKeyLength,
                byte[] data,
                int dataLength,
                byte[] signature,
                int signatureLength,
                ref int isValid
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_encrypt(
                byte[] publicKey,
                int publicKeyLength,
                byte[] data,
                int dataLength,
                ref IntPtr encryptedData,
                ref int encryptedDataLength
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_decrypt(
                byte[] privateKey,
                int privateKeyLength,
                byte[] encryptedData,
                int encryptedDataLength,
                ref IntPtr decryptedData,
                ref int decryptedDataLength
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_algorithm_info(
                string algorithmName,
                ref IntPtr name,
                ref int securityLevel,
                ref int keySize,
                ref int signatureSize,
                ref int isRecommended,
                ref IntPtr description
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_available_algorithms(
                ref IntPtr algorithms,
                ref int count
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_recommended_algorithms(
                ref IntPtr algorithms,
                ref int count
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_is_algorithm_available(
                string algorithmName,
                ref int isAvailable
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_is_algorithm_recommended(
                string algorithmName,
                ref int isRecommended
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_algorithm_security_level(
                string algorithmName,
                ref int securityLevel
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_algorithm_key_size(
                string algorithmName,
                ref int keySize
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_algorithm_signature_size(
                string algorithmName,
                ref int signatureSize
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_get_default_algorithm(
                ref IntPtr algorithm
            );

            [DllImport(DllName)]
            public static extern int satox_post_quantum_algorithms_set_default_algorithm(
                string algorithmName
            );

            [DllImport(DllName)]
            public static extern void satox_post_quantum_algorithms_free_buffer(IntPtr buffer);
        }
    }
} 