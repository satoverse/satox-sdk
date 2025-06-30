using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Satox.Quantum
{
    /// <summary>
    /// Provides hybrid encryption functionality combining classical and quantum-resistant algorithms.
    /// </summary>
    public class HybridEncryption : IDisposable
    {
        private bool _isInitialized;
        private bool _disposed;

        /// <summary>
        /// Initializes a new instance of the HybridEncryption class.
        /// </summary>
        public HybridEncryption()
        {
            _isInitialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the hybrid encryption system.
        /// </summary>
        /// <returns>True if initialization was successful, false otherwise.</returns>
        public bool Initialize()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(HybridEncryption));

            if (_isInitialized)
                return true;

            try
            {
                var result = NativeMethods.satox_hybrid_encryption_initialize();
                _isInitialized = result;
                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to initialize hybrid encryption: {ex.Message}");
            }
        }

        /// <summary>
        /// Shuts down the hybrid encryption system.
        /// </summary>
        /// <returns>True if shutdown was successful, false otherwise.</returns>
        public bool Shutdown()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(HybridEncryption));

            if (!_isInitialized)
                return true;

            try
            {
                var result = NativeMethods.satox_hybrid_encryption_shutdown();
                _isInitialized = !result;
                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to shutdown hybrid encryption: {ex.Message}");
            }
        }

        /// <summary>
        /// Generates a new key pair for hybrid encryption.
        /// </summary>
        /// <returns>A tuple containing the public and private keys.</returns>
        public async Task<(byte[] PublicKey, byte[] PrivateKey)> GenerateKeyPair()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr publicKeyPtr = IntPtr.Zero;
                    IntPtr privateKeyPtr = IntPtr.Zero;
                    int publicKeyLength = 0;
                    int privateKeyLength = 0;

                    var status = NativeMethods.satox_hybrid_encryption_generate_key_pair(
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

                    NativeMethods.satox_hybrid_encryption_free_buffer(publicKeyPtr);
                    NativeMethods.satox_hybrid_encryption_free_buffer(privateKeyPtr);

                    return (publicKey, privateKey);
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to generate key pair: {ex.Message}");
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

                    var status = NativeMethods.satox_hybrid_encryption_encrypt(
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
                    NativeMethods.satox_hybrid_encryption_free_buffer(encryptedDataPtr);

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

                    var status = NativeMethods.satox_hybrid_encryption_decrypt(
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
                    NativeMethods.satox_hybrid_encryption_free_buffer(decryptedDataPtr);

                    return decryptedData;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to decrypt data: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the current session key.
        /// </summary>
        /// <returns>The current session key.</returns>
        public async Task<byte[]> GetSessionKey()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr sessionKeyPtr = IntPtr.Zero;
                    int sessionKeyLength = 0;

                    var status = NativeMethods.satox_hybrid_encryption_get_session_key(
                        ref sessionKeyPtr,
                        ref sessionKeyLength
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to get session key");

                    var sessionKey = new byte[sessionKeyLength];
                    Marshal.Copy(sessionKeyPtr, sessionKey, 0, sessionKeyLength);
                    NativeMethods.satox_hybrid_encryption_free_buffer(sessionKeyPtr);

                    return sessionKey;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get session key: {ex.Message}");
            }
        }

        /// <summary>
        /// Rotates the session key.
        /// </summary>
        /// <returns>True if rotation was successful, false otherwise.</returns>
        public async Task<bool> RotateSessionKey()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_hybrid_encryption_rotate_session_key();
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to rotate session key: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if the hybrid encryption system is initialized.
        /// </summary>
        /// <returns>True if initialized, false otherwise.</returns>
        public bool IsInitialized()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(HybridEncryption));

            return _isInitialized;
        }

        /// <summary>
        /// Gets the version of the hybrid encryption system.
        /// </summary>
        /// <returns>The version string.</returns>
        public string GetVersion()
        {
            EnsureInitialized();

            try
            {
                IntPtr versionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_hybrid_encryption_get_version(ref versionPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get version");

                var version = Marshal.PtrToStringAnsi(versionPtr);
                NativeMethods.satox_hybrid_encryption_free_buffer(versionPtr);

                return version;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get version: {ex.Message}");
            }
        }

        /// <summary>
        /// Disposes of the hybrid encryption system.
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
                throw new ObjectDisposedException(nameof(HybridEncryption));

            if (!_isInitialized)
                throw new SatoxError("Hybrid encryption system is not initialized");
        }

        private void ValidateInput(byte[] input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
            if (input.Length == 0)
                throw new ArgumentException("Input cannot be empty", paramName);
        }

        private static class NativeMethods
        {
            private const string DllName = "satox_quantum";

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_initialize();

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_shutdown();

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_generate_key_pair(
                ref IntPtr publicKey,
                ref int publicKeyLength,
                ref IntPtr privateKey,
                ref int privateKeyLength
            );

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_encrypt(
                byte[] publicKey,
                int publicKeyLength,
                byte[] data,
                int dataLength,
                ref IntPtr encryptedData,
                ref int encryptedDataLength
            );

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_decrypt(
                byte[] privateKey,
                int privateKeyLength,
                byte[] encryptedData,
                int encryptedDataLength,
                ref IntPtr decryptedData,
                ref int decryptedDataLength
            );

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_get_session_key(
                ref IntPtr sessionKey,
                ref int sessionKeyLength
            );

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_rotate_session_key();

            [DllImport(DllName)]
            public static extern int satox_hybrid_encryption_get_version(ref IntPtr version);

            [DllImport(DllName)]
            public static extern void satox_hybrid_encryption_free_buffer(IntPtr buffer);
        }
    }
} 