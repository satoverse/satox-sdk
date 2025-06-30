using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Satox.Quantum
{
    /// <summary>
    /// Represents metadata for a stored key.
    /// </summary>
    public class KeyMetadata
    {
        /// <summary>
        /// Gets or sets the algorithm used for the key.
        /// </summary>
        public string Algorithm { get; set; }

        /// <summary>
        /// Gets or sets the creation time of the key.
        /// </summary>
        public long CreationTime { get; set; }

        /// <summary>
        /// Gets or sets the expiration time of the key.
        /// </summary>
        public long Expiration { get; set; }

        /// <summary>
        /// Gets or sets the access levels for the key.
        /// </summary>
        public List<string> AccessLevels { get; set; }

        /// <summary>
        /// Gets or sets the tags associated with the key.
        /// </summary>
        public List<string> Tags { get; set; }
    }

    /// <summary>
    /// Provides secure storage for cryptographic keys.
    /// </summary>
    public class KeyStorage : IDisposable
    {
        private bool _isInitialized;
        private bool _disposed;

        /// <summary>
        /// Initializes a new instance of the KeyStorage class.
        /// </summary>
        public KeyStorage()
        {
            _isInitialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the key storage system.
        /// </summary>
        /// <returns>True if initialization was successful, false otherwise.</returns>
        public bool Initialize()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(KeyStorage));

            if (_isInitialized)
                return true;

            try
            {
                var result = NativeMethods.satox_key_storage_initialize();
                _isInitialized = result;
                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to initialize key storage: {ex.Message}");
            }
        }

        /// <summary>
        /// Shuts down the key storage system.
        /// </summary>
        /// <returns>True if shutdown was successful, false otherwise.</returns>
        public bool Shutdown()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(KeyStorage));

            if (!_isInitialized)
                return true;

            try
            {
                var result = NativeMethods.satox_key_storage_shutdown();
                _isInitialized = !result;
                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to shutdown key storage: {ex.Message}");
            }
        }

        /// <summary>
        /// Stores a key with associated metadata.
        /// </summary>
        /// <param name="key">The key to store.</param>
        /// <param name="metadata">The metadata for the key.</param>
        /// <returns>The ID of the stored key.</returns>
        public async Task<string> StoreKey(byte[] key, KeyMetadata metadata)
        {
            EnsureInitialized();
            ValidateInput(key, nameof(key));
            ValidateInput(metadata, nameof(metadata));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr keyIdPtr = IntPtr.Zero;
                    var status = NativeMethods.satox_key_storage_store_key(
                        key,
                        key.Length,
                        metadata.Algorithm,
                        metadata.CreationTime,
                        metadata.Expiration,
                        metadata.AccessLevels.ToArray(),
                        metadata.AccessLevels.Count,
                        metadata.Tags.ToArray(),
                        metadata.Tags.Count,
                        ref keyIdPtr
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to store key");

                    var keyId = Marshal.PtrToStringAnsi(keyIdPtr);
                    NativeMethods.satox_key_storage_free_buffer(keyIdPtr);

                    return keyId;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to store key: {ex.Message}");
            }
        }

        /// <summary>
        /// Retrieves a key by its ID.
        /// </summary>
        /// <param name="keyId">The ID of the key to retrieve.</param>
        /// <returns>The retrieved key.</returns>
        public async Task<byte[]> GetKey(string keyId)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr keyPtr = IntPtr.Zero;
                    int keyLength = 0;

                    var status = NativeMethods.satox_key_storage_get_key(
                        keyId,
                        ref keyPtr,
                        ref keyLength
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to get key");

                    var key = new byte[keyLength];
                    Marshal.Copy(keyPtr, key, 0, keyLength);
                    NativeMethods.satox_key_storage_free_buffer(keyPtr);

                    return key;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get key: {ex.Message}");
            }
        }

        /// <summary>
        /// Deletes a key by its ID.
        /// </summary>
        /// <param name="keyId">The ID of the key to delete.</param>
        /// <returns>True if deletion was successful, false otherwise.</returns>
        public async Task<bool> DeleteKey(string keyId)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_delete_key(keyId);
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to delete key: {ex.Message}");
            }
        }

        /// <summary>
        /// Updates a key with new data.
        /// </summary>
        /// <param name="keyId">The ID of the key to update.</param>
        /// <param name="newKey">The new key data.</param>
        /// <returns>True if update was successful, false otherwise.</returns>
        public async Task<bool> UpdateKey(string keyId, byte[] newKey)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));
            ValidateInput(newKey, nameof(newKey));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_update_key(
                        keyId,
                        newKey,
                        newKey.Length
                    );
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to update key: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the metadata for a key.
        /// </summary>
        /// <param name="keyId">The ID of the key.</param>
        /// <returns>The key metadata.</returns>
        public async Task<KeyMetadata> GetKeyMetadata(string keyId)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr algorithmPtr = IntPtr.Zero;
                    long creationTime = 0;
                    long expiration = 0;
                    IntPtr accessLevelsPtr = IntPtr.Zero;
                    int accessLevelsCount = 0;
                    IntPtr tagsPtr = IntPtr.Zero;
                    int tagsCount = 0;

                    var status = NativeMethods.satox_key_storage_get_key_metadata(
                        keyId,
                        ref algorithmPtr,
                        ref creationTime,
                        ref expiration,
                        ref accessLevelsPtr,
                        ref accessLevelsCount,
                        ref tagsPtr,
                        ref tagsCount
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to get key metadata");

                    var metadata = new KeyMetadata
                    {
                        Algorithm = Marshal.PtrToStringAnsi(algorithmPtr),
                        CreationTime = creationTime,
                        Expiration = expiration,
                        AccessLevels = new List<string>(),
                        Tags = new List<string>()
                    };

                    // Copy access levels
                    var currentPtr = accessLevelsPtr;
                    for (int i = 0; i < accessLevelsCount; i++)
                    {
                        metadata.AccessLevels.Add(Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr)));
                        currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                    }

                    // Copy tags
                    currentPtr = tagsPtr;
                    for (int i = 0; i < tagsCount; i++)
                    {
                        metadata.Tags.Add(Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr)));
                        currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                    }

                    NativeMethods.satox_key_storage_free_buffer(algorithmPtr);
                    NativeMethods.satox_key_storage_free_buffer(accessLevelsPtr);
                    NativeMethods.satox_key_storage_free_buffer(tagsPtr);

                    return metadata;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get key metadata: {ex.Message}");
            }
        }

        /// <summary>
        /// Updates the metadata for a key.
        /// </summary>
        /// <param name="keyId">The ID of the key.</param>
        /// <param name="metadata">The new metadata.</param>
        /// <returns>True if update was successful, false otherwise.</returns>
        public async Task<bool> UpdateKeyMetadata(string keyId, KeyMetadata metadata)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));
            ValidateInput(metadata, nameof(metadata));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_update_key_metadata(
                        keyId,
                        metadata.Algorithm,
                        metadata.CreationTime,
                        metadata.Expiration,
                        metadata.AccessLevels.ToArray(),
                        metadata.AccessLevels.Count,
                        metadata.Tags.ToArray(),
                        metadata.Tags.Count
                    );
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to update key metadata: {ex.Message}");
            }
        }

        /// <summary>
        /// Rotates a key with new data.
        /// </summary>
        /// <param name="keyId">The ID of the key to rotate.</param>
        /// <param name="newKey">The new key data.</param>
        /// <returns>True if rotation was successful, false otherwise.</returns>
        public async Task<bool> RotateKey(string keyId, byte[] newKey)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));
            ValidateInput(newKey, nameof(newKey));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_rotate_key(
                        keyId,
                        newKey,
                        newKey.Length
                    );
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to rotate key: {ex.Message}");
            }
        }

        /// <summary>
        /// Reencrypts a key.
        /// </summary>
        /// <param name="keyId">The ID of the key to reencrypt.</param>
        /// <returns>True if reencryption was successful, false otherwise.</returns>
        public async Task<bool> ReencryptKey(string keyId)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_reencrypt_key(keyId);
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to reencrypt key: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates a key.
        /// </summary>
        /// <param name="keyId">The ID of the key to validate.</param>
        /// <returns>True if the key is valid, false otherwise.</returns>
        public async Task<bool> ValidateKey(string keyId)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));

            try
            {
                return await Task.Run(() =>
                {
                    int isValid = 0;
                    var status = NativeMethods.satox_key_storage_validate_key(keyId, ref isValid);
                    return status == 0 && isValid != 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate key: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if a key has expired.
        /// </summary>
        /// <param name="keyId">The ID of the key to check.</param>
        /// <returns>True if the key has expired, false otherwise.</returns>
        public async Task<bool> IsKeyExpired(string keyId)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));

            try
            {
                return await Task.Run(() =>
                {
                    int isExpired = 0;
                    var status = NativeMethods.satox_key_storage_is_key_expired(keyId, ref isExpired);
                    return status == 0 && isExpired != 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check key expiration: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if a key has a specific access level.
        /// </summary>
        /// <param name="keyId">The ID of the key to check.</param>
        /// <param name="accessLevel">The access level to check for.</param>
        /// <returns>True if the key has the access level, false otherwise.</returns>
        public async Task<bool> HasAccess(string keyId, string accessLevel)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));
            ValidateInput(accessLevel, nameof(accessLevel));

            try
            {
                return await Task.Run(() =>
                {
                    int hasAccess = 0;
                    var status = NativeMethods.satox_key_storage_has_access(
                        keyId,
                        accessLevel,
                        ref hasAccess
                    );
                    return status == 0 && hasAccess != 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check access level: {ex.Message}");
            }
        }

        /// <summary>
        /// Adds an access level to a key.
        /// </summary>
        /// <param name="keyId">The ID of the key.</param>
        /// <param name="accessLevel">The access level to add.</param>
        /// <returns>True if the access level was added successfully, false otherwise.</returns>
        public async Task<bool> AddAccessLevel(string keyId, string accessLevel)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));
            ValidateInput(accessLevel, nameof(accessLevel));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_add_access_level(keyId, accessLevel);
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to add access level: {ex.Message}");
            }
        }

        /// <summary>
        /// Removes an access level from a key.
        /// </summary>
        /// <param name="keyId">The ID of the key.</param>
        /// <param name="accessLevel">The access level to remove.</param>
        /// <returns>True if the access level was removed successfully, false otherwise.</returns>
        public async Task<bool> RemoveAccessLevel(string keyId, string accessLevel)
        {
            EnsureInitialized();
            ValidateInput(keyId, nameof(keyId));
            ValidateInput(accessLevel, nameof(accessLevel));

            try
            {
                return await Task.Run(() =>
                {
                    var status = NativeMethods.satox_key_storage_remove_access_level(keyId, accessLevel);
                    return status == 0;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to remove access level: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the number of stored keys.
        /// </summary>
        /// <returns>The number of stored keys.</returns>
        public async Task<int> GetKeyCount()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    int count = 0;
                    var status = NativeMethods.satox_key_storage_get_key_count(ref count);
                    if (status != 0)
                        throw new SatoxError("Failed to get key count");
                    return count;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get key count: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets all stored key IDs.
        /// </summary>
        /// <returns>An array of key IDs.</returns>
        public async Task<string[]> GetAllKeyIds()
        {
            EnsureInitialized();

            try
            {
                return await Task.Run(() =>
                {
                    IntPtr keyIdsPtr = IntPtr.Zero;
                    int count = 0;

                    var status = NativeMethods.satox_key_storage_get_all_key_ids(
                        ref keyIdsPtr,
                        ref count
                    );

                    if (status != 0)
                        throw new SatoxError("Failed to get key IDs");

                    var keyIds = new string[count];
                    var currentPtr = keyIdsPtr;

                    for (int i = 0; i < count; i++)
                    {
                        keyIds[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr));
                        currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                    }

                    NativeMethods.satox_key_storage_free_buffer(keyIdsPtr);
                    return keyIds;
                });
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get key IDs: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if the key storage system is initialized.
        /// </summary>
        /// <returns>True if initialized, false otherwise.</returns>
        public bool IsInitialized()
        {
            if (_disposed)
                throw new ObjectDisposedException(nameof(KeyStorage));

            return _isInitialized;
        }

        /// <summary>
        /// Disposes of the key storage system.
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
                throw new ObjectDisposedException(nameof(KeyStorage));

            if (!_isInitialized)
                throw new SatoxError("Key storage system is not initialized");
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
            public static extern int satox_key_storage_initialize();

            [DllImport(DllName)]
            public static extern int satox_key_storage_shutdown();

            [DllImport(DllName)]
            public static extern int satox_key_storage_store_key(
                byte[] key,
                int keyLength,
                string algorithm,
                long creationTime,
                long expiration,
                string[] accessLevels,
                int accessLevelsCount,
                string[] tags,
                int tagsCount,
                ref IntPtr keyId
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_get_key(
                string keyId,
                ref IntPtr key,
                ref int keyLength
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_delete_key(string keyId);

            [DllImport(DllName)]
            public static extern int satox_key_storage_update_key(
                string keyId,
                byte[] newKey,
                int newKeyLength
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_get_key_metadata(
                string keyId,
                ref IntPtr algorithm,
                ref long creationTime,
                ref long expiration,
                ref IntPtr accessLevels,
                ref int accessLevelsCount,
                ref IntPtr tags,
                ref int tagsCount
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_update_key_metadata(
                string keyId,
                string algorithm,
                long creationTime,
                long expiration,
                string[] accessLevels,
                int accessLevelsCount,
                string[] tags,
                int tagsCount
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_rotate_key(
                string keyId,
                byte[] newKey,
                int newKeyLength
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_reencrypt_key(string keyId);

            [DllImport(DllName)]
            public static extern int satox_key_storage_validate_key(
                string keyId,
                ref int isValid
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_is_key_expired(
                string keyId,
                ref int isExpired
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_has_access(
                string keyId,
                string accessLevel,
                ref int hasAccess
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_add_access_level(
                string keyId,
                string accessLevel
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_remove_access_level(
                string keyId,
                string accessLevel
            );

            [DllImport(DllName)]
            public static extern int satox_key_storage_get_key_count(ref int count);

            [DllImport(DllName)]
            public static extern int satox_key_storage_get_all_key_ids(
                ref IntPtr keyIds,
                ref int count
            );

            [DllImport(DllName)]
            public static extern void satox_key_storage_free_buffer(IntPtr buffer);
        }
    }
} 