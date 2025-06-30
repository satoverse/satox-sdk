using System;
using System.Runtime.InteropServices;

namespace Satox.Security
{
    public class EncryptionManager : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        public EncryptionManager()
        {
            _initialized = false;
            _disposed = false;
        }

        public void Initialize()
        {
            if (_initialized)
                return;

            var result = NativeMethods.satox_encryption_manager_initialize();
            if (result != 0)
                throw new SatoxError("Failed to initialize encryption manager");

            _initialized = true;
        }

        public void Shutdown()
        {
            if (!_initialized)
                return;

            var result = NativeMethods.satox_encryption_manager_shutdown();
            if (result != 0)
                throw new SatoxError("Failed to shutdown encryption manager");

            _initialized = false;
        }

        public byte[] Encrypt(byte[] data, EncryptionConfig config)
        {
            EnsureInitialized();
            ValidateInput(data, config);

            IntPtr encryptedData;
            int encryptedLength;
            var result = NativeMethods.satox_encryption_manager_encrypt(data, data.Length, ref config, out encryptedData, out encryptedLength);
            if (result != 0)
                throw new SatoxError("Failed to encrypt data");

            var encrypted = new byte[encryptedLength];
            Marshal.Copy(encryptedData, encrypted, 0, encryptedLength);
            NativeMethods.satox_encryption_manager_free_buffer(encryptedData);

            return encrypted;
        }

        public byte[] Decrypt(byte[] encryptedData, EncryptionConfig config)
        {
            EnsureInitialized();
            ValidateInput(encryptedData, config);

            IntPtr decryptedData;
            int decryptedLength;
            var result = NativeMethods.satox_encryption_manager_decrypt(encryptedData, encryptedData.Length, ref config, out decryptedData, out decryptedLength);
            if (result != 0)
                throw new SatoxError("Failed to decrypt data");

            var decrypted = new byte[decryptedLength];
            Marshal.Copy(decryptedData, decrypted, 0, decryptedLength);
            NativeMethods.satox_encryption_manager_free_buffer(decryptedData);

            return decrypted;
        }

        public void GenerateKeyPair(out byte[] publicKey, out byte[] privateKey, KeyConfig config)
        {
            EnsureInitialized();
            ValidateInput(config);

            IntPtr pubKey, privKey;
            int pubKeyLength, privKeyLength;
            var result = NativeMethods.satox_encryption_manager_generate_key_pair(ref config, out pubKey, out pubKeyLength, out privKey, out privKeyLength);
            if (result != 0)
                throw new SatoxError("Failed to generate key pair");

            publicKey = new byte[pubKeyLength];
            privateKey = new byte[privKeyLength];
            Marshal.Copy(pubKey, publicKey, 0, pubKeyLength);
            Marshal.Copy(privKey, privateKey, 0, privKeyLength);
            NativeMethods.satox_encryption_manager_free_buffer(pubKey);
            NativeMethods.satox_encryption_manager_free_buffer(privKey);
        }

        public byte[] Sign(byte[] data, byte[] privateKey)
        {
            EnsureInitialized();
            ValidateInput(data, privateKey);

            IntPtr signature;
            int signatureLength;
            var result = NativeMethods.satox_encryption_manager_sign(data, data.Length, privateKey, privateKey.Length, out signature, out signatureLength);
            if (result != 0)
                throw new SatoxError("Failed to sign data");

            var signed = new byte[signatureLength];
            Marshal.Copy(signature, signed, 0, signatureLength);
            NativeMethods.satox_encryption_manager_free_buffer(signature);

            return signed;
        }

        public bool Verify(byte[] data, byte[] signature, byte[] publicKey)
        {
            EnsureInitialized();
            ValidateInput(data, signature, publicKey);

            bool isValid;
            var result = NativeMethods.satox_encryption_manager_verify(data, data.Length, signature, signature.Length, publicKey, publicKey.Length, out isValid);
            if (result != 0)
                throw new SatoxError("Failed to verify signature");

            return isValid;
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Encryption manager not initialized");
        }

        private void ValidateInput(byte[] data, EncryptionConfig config)
        {
            if (data == null)
                throw new ArgumentNullException(nameof(data));
            if (config == null)
                throw new ArgumentNullException(nameof(config));
        }

        private void ValidateInput(KeyConfig config)
        {
            if (config == null)
                throw new ArgumentNullException(nameof(config));
        }

        private void ValidateInput(byte[] data, byte[] key)
        {
            if (data == null)
                throw new ArgumentNullException(nameof(data));
            if (key == null)
                throw new ArgumentNullException(nameof(key));
        }

        private void ValidateInput(byte[] data, byte[] signature, byte[] publicKey)
        {
            if (data == null)
                throw new ArgumentNullException(nameof(data));
            if (signature == null)
                throw new ArgumentNullException(nameof(signature));
            if (publicKey == null)
                throw new ArgumentNullException(nameof(publicKey));
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_disposed)
                return;

            if (disposing)
            {
                if (_initialized)
                    Shutdown();
            }

            _disposed = true;
        }

        ~EncryptionManager()
        {
            Dispose(false);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct EncryptionConfig
    {
        public EncryptionAlgorithm Algorithm;
        public int KeySize;
        public bool UsePadding;
        public bool UseIV;
        [MarshalAs(UnmanagedType.LPStr)]
        public string Mode;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct KeyConfig
    {
        public KeyAlgorithm Algorithm;
        public int KeySize;
        public bool UseSalt;
        public int Iterations;
    }

    public enum EncryptionAlgorithm
    {
        AES = 1,
        ChaCha20 = 2,
        Twofish = 3,
        Serpent = 4
    }

    public enum KeyAlgorithm
    {
        RSA = 1,
        ECC = 2,
        Ed25519 = 3,
        X25519 = 4
    }

    internal static class NativeMethods
    {
        [DllImport("satox")]
        public static extern int satox_encryption_manager_initialize();

        [DllImport("satox")]
        public static extern int satox_encryption_manager_shutdown();

        [DllImport("satox")]
        public static extern int satox_encryption_manager_encrypt(byte[] data, int dataLength, ref EncryptionConfig config, out IntPtr encryptedData, out int encryptedLength);

        [DllImport("satox")]
        public static extern int satox_encryption_manager_decrypt(byte[] encryptedData, int encryptedLength, ref EncryptionConfig config, out IntPtr decryptedData, out int decryptedLength);

        [DllImport("satox")]
        public static extern int satox_encryption_manager_generate_key_pair(ref KeyConfig config, out IntPtr publicKey, out int publicKeyLength, out IntPtr privateKey, out int privateKeyLength);

        [DllImport("satox")]
        public static extern int satox_encryption_manager_sign(byte[] data, int dataLength, byte[] privateKey, int privateKeyLength, out IntPtr signature, out int signatureLength);

        [DllImport("satox")]
        public static extern int satox_encryption_manager_verify(byte[] data, int dataLength, byte[] signature, int signatureLength, byte[] publicKey, int publicKeyLength, out bool isValid);

        [DllImport("satox")]
        public static extern void satox_encryption_manager_free_buffer(IntPtr buffer);
    }
} 