using System;
using System.Runtime.InteropServices;

namespace Satox.IPFS
{
    /// <summary>
    /// ContentStorage provides content storage functionality.
    /// </summary>
    public class ContentStorage : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of ContentStorage.
        /// </summary>
        public ContentStorage()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the content storage.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_content_storage_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize content storage");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the content storage.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_content_storage_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown content storage");

            _initialized = false;
        }

        /// <summary>
        /// Stores content.
        /// </summary>
        /// <param name="content">The content to store.</param>
        /// <param name="options">The storage options.</param>
        /// <returns>The content hash.</returns>
        public string StoreContent(byte[] content, ContentOptions options)
        {
            EnsureInitialized();
            ValidateInput(content, nameof(content));
            ValidateInput(options, nameof(options));

            try
            {
                IntPtr hashPtr = IntPtr.Zero;
                var status = NativeMethods.satox_content_storage_store_content(
                    content,
                    content.Length,
                    ref options,
                    ref hashPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to store content");

                var hash = Marshal.PtrToStringAnsi(hashPtr);
                NativeMethods.satox_content_storage_free_string(hashPtr);

                return hash;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to store content: {ex.Message}");
            }
        }

        /// <summary>
        /// Retrieves content.
        /// </summary>
        /// <param name="hash">The content hash.</param>
        /// <returns>The content.</returns>
        public byte[] RetrieveContent(string hash)
        {
            EnsureInitialized();
            ValidateInput(hash, nameof(hash));

            try
            {
                IntPtr contentPtr = IntPtr.Zero;
                int contentLength = 0;
                var status = NativeMethods.satox_content_storage_retrieve_content(
                    hash,
                    ref contentPtr,
                    ref contentLength
                );

                if (status != 0)
                    throw new SatoxError("Failed to retrieve content");

                var content = new byte[contentLength];
                Marshal.Copy(contentPtr, content, 0, contentLength);
                NativeMethods.satox_content_storage_free_bytes(contentPtr, contentLength);

                return content;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to retrieve content: {ex.Message}");
            }
        }

        /// <summary>
        /// Removes content.
        /// </summary>
        /// <param name="hash">The content hash.</param>
        public void RemoveContent(string hash)
        {
            EnsureInitialized();
            ValidateInput(hash, nameof(hash));

            try
            {
                var status = NativeMethods.satox_content_storage_remove_content(hash);
                if (status != 0)
                    throw new SatoxError("Failed to remove content");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to remove content: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if content exists.
        /// </summary>
        /// <param name="hash">The content hash.</param>
        /// <returns>True if the content exists, false otherwise.</returns>
        public bool HasContent(string hash)
        {
            EnsureInitialized();
            ValidateInput(hash, nameof(hash));

            try
            {
                int exists = 0;
                var status = NativeMethods.satox_content_storage_has_content(
                    hash,
                    ref exists
                );

                if (status != 0)
                    throw new SatoxError("Failed to check content existence");

                return exists != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check content existence: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets content information.
        /// </summary>
        /// <param name="hash">The content hash.</param>
        /// <returns>The content information.</returns>
        public ContentInfo GetContentInfo(string hash)
        {
            EnsureInitialized();
            ValidateInput(hash, nameof(hash));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_content_storage_get_content_info(
                    hash,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get content info");

                var info = Marshal.PtrToStructure<ContentInfo>(infoPtr);
                NativeMethods.satox_content_storage_free_content_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get content info: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Content storage is not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException("Value cannot be null or empty", paramName);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    if (_initialized)
                    {
                        Shutdown();
                    }
                }

                _disposed = true;
            }
        }

        ~ContentStorage()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_initialize();

            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_shutdown();

            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_store_content(
                byte[] content,
                int contentLength,
                ref ContentOptions options,
                ref IntPtr hash
            );

            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_retrieve_content(
                string hash,
                ref IntPtr content,
                ref int contentLength
            );

            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_remove_content(string hash);

            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_has_content(
                string hash,
                ref int exists
            );

            [DllImport("satox_ipfs")]
            public static extern int satox_content_storage_get_content_info(
                string hash,
                ref IntPtr info
            );

            [DllImport("satox_ipfs")]
            public static extern void satox_content_storage_free_string(IntPtr str);

            [DllImport("satox_ipfs")]
            public static extern void satox_content_storage_free_bytes(IntPtr bytes, int length);

            [DllImport("satox_ipfs")]
            public static extern void satox_content_storage_free_content_info(IntPtr info);
        }
    }

    /// <summary>
    /// Represents content storage options.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct ContentOptions
    {
        /// <summary>
        /// Whether to pin the content.
        /// </summary>
        public bool Pin;

        /// <summary>
        /// Whether to replicate the content.
        /// </summary>
        public bool Replicate;

        /// <summary>
        /// The replication factor.
        /// </summary>
        public int ReplicationFactor;

        /// <summary>
        /// The content type.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ContentType;

        /// <summary>
        /// The content encoding.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ContentEncoding;
    }

    /// <summary>
    /// Represents content information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct ContentInfo
    {
        /// <summary>
        /// The content hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Hash;

        /// <summary>
        /// The content size in bytes.
        /// </summary>
        public long Size;

        /// <summary>
        /// The content type.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ContentType;

        /// <summary>
        /// The content encoding.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ContentEncoding;

        /// <summary>
        /// Whether the content is pinned.
        /// </summary>
        public bool IsPinned;

        /// <summary>
        /// The number of replicas.
        /// </summary>
        public int ReplicaCount;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last access timestamp.
        /// </summary>
        public long LastAccessedAt;
    }
} 