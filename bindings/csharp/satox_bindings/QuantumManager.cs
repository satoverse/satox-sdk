using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Satox.Bindings.Quantum
{
    public interface IQuantumManager : IDisposable
    {
        bool Initialize();
        bool Shutdown();
        Task<(byte[] PublicKey, byte[] PrivateKey)> GenerateKeyPair();
        Task<byte[]> Encrypt(byte[] publicKey, byte[] data);
        Task<byte[]> Decrypt(byte[] privateKey, byte[] encryptedData);
        Task<byte[]> Sign(byte[] privateKey, byte[] data);
        Task<bool> Verify(byte[] publicKey, byte[] data, byte[] signature);
        Task<int> GenerateRandomNumber(int minValue, int maxValue);
        Task<byte[]> GenerateRandomBytes(int length);
        bool IsInitialized();
        string GetVersion();
        string GetAlgorithm();
        string[] GetAvailableAlgorithms();
    }

    public class QuantumManager : ThreadSafeManager, IQuantumManager
    {
        private bool _isInitialized;
        private bool _disposed;

        public QuantumManager() : base()
        {
            _isInitialized = false;
            _disposed = false;
        }

        public bool Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return true;

                try
                {
                    var result = NativeMethods.satox_quantum_manager_initialize();
                    SetInitialized(result);
                    return result;
                }
                catch (Exception ex)
                {
                    throw new SatoxError($"Failed to initialize quantum manager: {ex.Message}");
                }
            }
        }

        public bool Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return true;

                try
                {
                    var result = NativeMethods.satox_quantum_manager_shutdown();
                    SetInitialized(!result);
                    return result;
                }
                catch (Exception ex)
                {
                    throw new SatoxError($"Failed to shutdown quantum manager: {ex.Message}");
                }
            }
        }

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

                    var status = NativeMethods.satox_quantum_manager_generate_key_pair(
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

                    NativeMethods.satox_quantum_manager_free_buffer(publicKeyPtr);
                    NativeMethods.satox_quantum_manager_free_buffer(privateKeyPtr);

                    return (publicKey, privateKey);
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to generate key pair: {ex.Message}");
            }
        }

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

                    var status = NativeMethods.satox_quantum_manager_encrypt(
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
                    NativeMethods.satox_quantum_manager_free_buffer(encryptedDataPtr);

                    return encryptedData;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to encrypt data: {ex.Message}");
            }
        }

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

                    var status = NativeMethods.satox_quantum_manager_decrypt(
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
                    NativeMethods.satox_quantum_manager_free_buffer(decryptedDataPtr);

                    return decryptedData;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to decrypt data: {ex.Message}");
            }
        }

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

                    var status = NativeMethods.satox_quantum_manager_sign(
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
                    NativeMethods.satox_quantum_manager_free_buffer(signaturePtr);

                    return signature;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to sign data: {ex.Message}");
            }
        }

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
                    var status = NativeMethods.satox_quantum_manager_verify(
                        publicKey,
                        publicKey.Length,
                        data,
                        data.Length,
                        signature,
                        signature.Length
                    );

                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to verify signature: {ex.Message}");
            }
        }

        public async Task<int> GenerateRandomNumber(int minValue, int maxValue)
        {
            EnsureInitialized();

            if (minValue >= maxValue)
                throw new ArgumentException("minValue must be less than maxValue");

            try
            {
                return await Task.Run(() =>
                {
                    var result = NativeMethods.satox_quantum_manager_generate_random_number(minValue, maxValue);
                    if (result < minValue || result > maxValue)
                        throw new SatoxError("Failed to generate random number within specified range");
                    return result;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to generate random number: {ex.Message}");
            }
        }

        public async Task<byte[]> GenerateRandomBytes(int length)
        {
            EnsureInitialized();

            if (length <= 0)
                throw new ArgumentException("length must be greater than 0");

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr randomBytesPtr = IntPtr.Zero;
                    var status = NativeMethods.satox_quantum_manager_generate_random_bytes(length, ref randomBytesPtr);

                    if (status != 0)
                        throw new SatoxError("Failed to generate random bytes");

                    var randomBytes = new byte[length];
                    Marshal.Copy(randomBytesPtr, randomBytes, 0, length);
                    NativeMethods.satox_quantum_manager_free_buffer(randomBytesPtr);

                    return randomBytes;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to generate random bytes: {ex.Message}");
            }
        }

        public bool IsInitialized()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(QuantumManager));

            return _isInitialized;
        }

        public string GetVersion()
        {
            EnsureInitialized();

            try
            {
                IntPtr versionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_quantum_manager_get_version(ref versionPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get version");

                var version = Marshal.PtrToStringAnsi(versionPtr);
                NativeMethods.satox_quantum_manager_free_buffer(versionPtr);

                return version;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get version: {ex.Message}");
            }
        }

        public string GetAlgorithm()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmPtr = IntPtr.Zero;
                var status = NativeMethods.satox_quantum_manager_get_algorithm(ref algorithmPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get algorithm");

                var algorithm = Marshal.PtrToStringAnsi(algorithmPtr);
                NativeMethods.satox_quantum_manager_free_buffer(algorithmPtr);

                return algorithm;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get algorithm: {ex.Message}");
            }
        }

        public string[] GetAvailableAlgorithms()
        {
            EnsureInitialized();

            try
            {
                IntPtr algorithmsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_quantum_manager_get_available_algorithms(ref algorithmsPtr, ref count);

                if (status != 0)
                    throw new SatoxError("Failed to get available algorithms");

                var algorithms = new string[count];
                var currentPtr = algorithmsPtr;

                for (int i = 0; i < count; i++)
                {
                    algorithms[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr));
                    currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                }

                NativeMethods.satox_quantum_manager_free_buffer(algorithmsPtr);

                return algorithms;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get available algorithms: {ex.Message}");
            }
        }

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
                throw new ObjectDisposedException(nameof(QuantumManager));

            if (!_isInitialized)
                throw new InvalidOperationException("Quantum manager is not initialized");
        }

        private void ValidateInput(byte[] input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);

            if (input.Length == 0)
                throw new ArgumentException($"{paramName} cannot be empty", paramName);
        }

        private static class NativeMethods
        {
            private const string DllName = "satox_quantum";

            [DllImport(DllName)]
            public static extern bool satox_quantum_manager_initialize();

            [DllImport(DllName)]
            public static extern bool satox_quantum_manager_shutdown();

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_generate_key_pair(
                ref IntPtr publicKey,
                ref int publicKeyLength,
                ref IntPtr privateKey,
                ref int privateKeyLength
            );

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_encrypt(
                byte[] publicKey,
                int publicKeyLength,
                byte[] data,
                int dataLength,
                ref IntPtr encryptedData,
                ref int encryptedDataLength
            );

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_decrypt(
                byte[] privateKey,
                int privateKeyLength,
                byte[] encryptedData,
                int encryptedDataLength,
                ref IntPtr decryptedData,
                ref int decryptedDataLength
            );

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_sign(
                byte[] privateKey,
                int privateKeyLength,
                byte[] data,
                int dataLength,
                ref IntPtr signature,
                ref int signatureLength
            );

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_verify(
                byte[] publicKey,
                int publicKeyLength,
                byte[] data,
                int dataLength,
                byte[] signature,
                int signatureLength
            );

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_generate_random_number(int minValue, int maxValue);

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_generate_random_bytes(int length, ref IntPtr randomBytes);

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_get_version(ref IntPtr version);

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_get_algorithm(ref IntPtr algorithm);

            [DllImport(DllName)]
            public static extern int satox_quantum_manager_get_available_algorithms(ref IntPtr algorithms, ref int count);

            [DllImport(DllName)]
            public static extern void satox_quantum_manager_free_buffer(IntPtr buffer);
        }
    }
}