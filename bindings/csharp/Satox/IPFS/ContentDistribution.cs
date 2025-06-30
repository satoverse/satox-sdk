using System;
using System.Runtime.InteropServices;

namespace Satox.IPFS
{
    /// <summary>
    /// ContentDistribution provides content distribution functionality.
    /// </summary>
    public class ContentDistribution : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of ContentDistribution.
        /// </summary>
        public ContentDistribution()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the content distribution.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_content_distribution_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize content distribution");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the content distribution.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_content_distribution_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown content distribution");

            _initialized = false;
        }

        /// <summary>
        /// Publishes content.
        /// </summary>
        /// <param name="hash">The content hash.</param>
        /// <param name="options">The distribution options.</param>
        /// <returns>The publication ID.</returns>
        public string PublishContent(string hash, DistributionOptions options)
        {
            EnsureInitialized();
            ValidateInput(hash, nameof(hash));
            ValidateInput(options, nameof(options));

            try
            {
                IntPtr publicationIdPtr = IntPtr.Zero;
                var status = NativeMethods.satox_content_distribution_publish_content(
                    hash,
                    ref options,
                    ref publicationIdPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to publish content");

                var publicationId = Marshal.PtrToStringAnsi(publicationIdPtr);
                NativeMethods.satox_content_distribution_free_string(publicationIdPtr);

                return publicationId;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to publish content: {ex.Message}");
            }
        }

        /// <summary>
        /// Unpublishes content.
        /// </summary>
        /// <param name="publicationId">The publication ID.</param>
        public void UnpublishContent(string publicationId)
        {
            EnsureInitialized();
            ValidateInput(publicationId, nameof(publicationId));

            try
            {
                var status = NativeMethods.satox_content_distribution_unpublish_content(publicationId);
                if (status != 0)
                    throw new SatoxError("Failed to unpublish content");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to unpublish content: {ex.Message}");
            }
        }

        /// <summary>
        /// Subscribes to content.
        /// </summary>
        /// <param name="publicationId">The publication ID.</param>
        /// <param name="callback">The subscription callback.</param>
        /// <returns>The subscription ID.</returns>
        public string SubscribeToContent(string publicationId, ContentCallback callback)
        {
            EnsureInitialized();
            ValidateInput(publicationId, nameof(publicationId));
            ValidateInput(callback, nameof(callback));

            try
            {
                IntPtr subscriptionIdPtr = IntPtr.Zero;
                var status = NativeMethods.satox_content_distribution_subscribe_to_content(
                    publicationId,
                    callback,
                    ref subscriptionIdPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to subscribe to content");

                var subscriptionId = Marshal.PtrToStringAnsi(subscriptionIdPtr);
                NativeMethods.satox_content_distribution_free_string(subscriptionIdPtr);

                return subscriptionId;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to subscribe to content: {ex.Message}");
            }
        }

        /// <summary>
        /// Unsubscribes from content.
        /// </summary>
        /// <param name="subscriptionId">The subscription ID.</param>
        public void UnsubscribeFromContent(string subscriptionId)
        {
            EnsureInitialized();
            ValidateInput(subscriptionId, nameof(subscriptionId));

            try
            {
                var status = NativeMethods.satox_content_distribution_unsubscribe_from_content(subscriptionId);
                if (status != 0)
                    throw new SatoxError("Failed to unsubscribe from content");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to unsubscribe from content: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets publication information.
        /// </summary>
        /// <param name="publicationId">The publication ID.</param>
        /// <returns>The publication information.</returns>
        public PublicationInfo GetPublicationInfo(string publicationId)
        {
            EnsureInitialized();
            ValidateInput(publicationId, nameof(publicationId));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_content_distribution_get_publication_info(
                    publicationId,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get publication info");

                var info = Marshal.PtrToStructure<PublicationInfo>(infoPtr);
                NativeMethods.satox_content_distribution_free_publication_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get publication info: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets subscription information.
        /// </summary>
        /// <param name="subscriptionId">The subscription ID.</param>
        /// <returns>The subscription information.</returns>
        public SubscriptionInfo GetSubscriptionInfo(string subscriptionId)
        {
            EnsureInitialized();
            ValidateInput(subscriptionId, nameof(subscriptionId));

            try
            {
                IntPtr infoPtr = IntPtr.Zero;
                var status = NativeMethods.satox_content_distribution_get_subscription_info(
                    subscriptionId,
                    ref infoPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get subscription info");

                var info = Marshal.PtrToStructure<SubscriptionInfo>(infoPtr);
                NativeMethods.satox_content_distribution_free_subscription_info(infoPtr);

                return info;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get subscription info: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Content distribution is not initialized");
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

        ~ContentDistribution()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_initialize();

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_shutdown();

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_publish_content(
                string hash,
                ref DistributionOptions options,
                ref IntPtr publicationId
            );

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_unpublish_content(string publicationId);

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_subscribe_to_content(
                string publicationId,
                ContentCallback callback,
                ref IntPtr subscriptionId
            );

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_unsubscribe_from_content(string subscriptionId);

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_get_publication_info(
                string publicationId,
                ref IntPtr info
            );

            [DllImport("satox_ipfs")]
            public static extern int satox_content_distribution_get_subscription_info(
                string subscriptionId,
                ref IntPtr info
            );

            [DllImport("satox_ipfs")]
            public static extern void satox_content_distribution_free_string(IntPtr str);

            [DllImport("satox_ipfs")]
            public static extern void satox_content_distribution_free_publication_info(IntPtr info);

            [DllImport("satox_ipfs")]
            public static extern void satox_content_distribution_free_subscription_info(IntPtr info);
        }
    }

    /// <summary>
    /// Represents content distribution options.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct DistributionOptions
    {
        /// <summary>
        /// Whether to enable encryption.
        /// </summary>
        public bool EnableEncryption;

        /// <summary>
        /// Whether to enable compression.
        /// </summary>
        public bool EnableCompression;

        /// <summary>
        /// The maximum number of subscribers.
        /// </summary>
        public int MaxSubscribers;

        /// <summary>
        /// The time-to-live in seconds.
        /// </summary>
        public long TTL;

        /// <summary>
        /// The access control level.
        /// </summary>
        public int AccessControlLevel;
    }

    /// <summary>
    /// Represents publication information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct PublicationInfo
    {
        /// <summary>
        /// The publication ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Id;

        /// <summary>
        /// The content hash.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ContentHash;

        /// <summary>
        /// The publisher address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Publisher;

        /// <summary>
        /// The number of subscribers.
        /// </summary>
        public int SubscriberCount;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The expiration timestamp.
        /// </summary>
        public long ExpiresAt;

        /// <summary>
        /// Whether encryption is enabled.
        /// </summary>
        public bool IsEncrypted;

        /// <summary>
        /// Whether compression is enabled.
        /// </summary>
        public bool IsCompressed;
    }

    /// <summary>
    /// Represents subscription information.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SubscriptionInfo
    {
        /// <summary>
        /// The subscription ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Id;

        /// <summary>
        /// The publication ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string PublicationId;

        /// <summary>
        /// The subscriber address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Subscriber;

        /// <summary>
        /// The subscription timestamp.
        /// </summary>
        public long SubscribedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long LastUpdatedAt;

        /// <summary>
        /// The number of updates received.
        /// </summary>
        public int UpdateCount;

        /// <summary>
        /// Whether the subscription is active.
        /// </summary>
        public bool IsActive;
    }

    /// <summary>
    /// Represents a content callback.
    /// </summary>
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ContentCallback(IntPtr content, int contentLength, IntPtr metadata);
} 