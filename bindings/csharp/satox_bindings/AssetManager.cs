using System;
using System.Runtime.InteropServices;

namespace Satox.Assets
{
    /// <summary>
    /// AssetManager provides asset management functionality.
    /// </summary>
    public class AssetManager : ThreadSafeManager
    {
        /// <summary>
        /// Creates a new instance of AssetManager.
        /// </summary>
        public AssetManager() : base()
        {
        }

        /// <summary>
        /// Initializes the asset manager.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_asset_manager_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize asset manager");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the asset manager.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_asset_manager_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown asset manager");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Creates a new asset.
        /// </summary>
        /// <param name="name">The asset name.</param>
        /// <param name="symbol">The asset symbol.</param>
        /// <param name="decimals">The number of decimal places.</param>
        /// <param name="totalSupply">The total supply.</param>
        /// <returns>The created asset.</returns>
        public Asset CreateAsset(string name, string symbol, int decimals, decimal totalSupply)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(symbol, nameof(symbol));

            try
            {
                IntPtr assetPtr = IntPtr.Zero;
                var status = NativeMethods.satox_asset_manager_create_asset(
                    name,
                    symbol,
                    decimals,
                    (double)totalSupply,
                    ref assetPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to create asset");

                var asset = Marshal.PtrToStructure<Asset>(assetPtr);
                NativeMethods.satox_asset_manager_free_asset(assetPtr);

                return asset;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to create asset: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets an asset by its ID.
        /// </summary>
        /// <param name="assetId">The asset ID.</param>
        /// <returns>The asset information.</returns>
        public Asset GetAsset(string assetId)
        {
            EnsureInitialized();
            ValidateInput(assetId, nameof(assetId));

            try
            {
                IntPtr assetPtr = IntPtr.Zero;
                var status = NativeMethods.satox_asset_manager_get_asset(
                    assetId,
                    ref assetPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get asset");

                var asset = Marshal.PtrToStructure<Asset>(assetPtr);
                NativeMethods.satox_asset_manager_free_asset(assetPtr);

                return asset;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get asset: {ex.Message}");
            }
        }

        /// <summary>
        /// Updates an asset.
        /// </summary>
        /// <param name="asset">The asset to update.</param>
        /// <returns>The updated asset.</returns>
        public Asset UpdateAsset(Asset asset)
        {
            EnsureInitialized();
            ValidateInput(asset, nameof(asset));

            try
            {
                IntPtr updatedAssetPtr = IntPtr.Zero;
                var status = NativeMethods.satox_asset_manager_update_asset(
                    ref asset,
                    ref updatedAssetPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to update asset");

                var updatedAsset = Marshal.PtrToStructure<Asset>(updatedAssetPtr);
                NativeMethods.satox_asset_manager_free_asset(updatedAssetPtr);

                return updatedAsset;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to update asset: {ex.Message}");
            }
        }

        /// <summary>
        /// Deletes an asset.
        /// </summary>
        /// <param name="assetId">The asset ID.</param>
        public void DeleteAsset(string assetId)
        {
            EnsureInitialized();
            ValidateInput(assetId, nameof(assetId));

            try
            {
                var status = NativeMethods.satox_asset_manager_delete_asset(assetId);
                if (status != 0)
                    throw new SatoxError("Failed to delete asset");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to delete asset: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets all assets.
        /// </summary>
        /// <returns>An array of assets.</returns>
        public Asset[] GetAllAssets()
        {
            EnsureInitialized();

            try
            {
                IntPtr assetsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_asset_manager_get_all_assets(
                    ref assetsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get all assets");

                var assets = new Asset[count];
                var currentPtr = assetsPtr;

                for (int i = 0; i < count; i++)
                {
                    assets[i] = Marshal.PtrToStructure<Asset>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<Asset>());
                }

                NativeMethods.satox_asset_manager_free_assets(assetsPtr);

                return assets;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get all assets: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets assets by owner.
        /// </summary>
        /// <param name="owner">The owner address.</param>
        /// <returns>An array of assets owned by the specified address.</returns>
        public Asset[] GetAssetsByOwner(string owner)
        {
            EnsureInitialized();
            ValidateInput(owner, nameof(owner));

            try
            {
                IntPtr assetsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_asset_manager_get_assets_by_owner(
                    owner,
                    ref assetsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get assets by owner");

                var assets = new Asset[count];
                var currentPtr = assetsPtr;

                for (int i = 0; i < count; i++)
                {
                    assets[i] = Marshal.PtrToStructure<Asset>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<Asset>());
                }

                NativeMethods.satox_asset_manager_free_assets(assetsPtr);

                return assets;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get assets by owner: {ex.Message}");
            }
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrWhiteSpace(input))
                throw new ArgumentException($"{paramName} cannot be null or empty", paramName);
        }

        protected override void DisposeCore()
        {
            if (IsInitialized)
                Shutdown();
        }

        private static class NativeMethods
        {
            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_initialize();

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_shutdown();

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_create_asset(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string symbol,
                int decimals,
                double totalSupply,
                ref IntPtr asset
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_get_asset(
                [MarshalAs(UnmanagedType.LPStr)] string assetId,
                ref IntPtr asset
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_update_asset(
                ref Asset asset,
                ref IntPtr updatedAsset
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_delete_asset(
                [MarshalAs(UnmanagedType.LPStr)] string assetId
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_get_all_assets(
                ref IntPtr assets,
                ref int count
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_manager_get_assets_by_owner(
                [MarshalAs(UnmanagedType.LPStr)] string owner,
                ref IntPtr assets,
                ref int count
            );

            [DllImport("satox_assets")]
            public static extern void satox_asset_manager_free_asset(IntPtr asset);

            [DllImport("satox_assets")]
            public static extern void satox_asset_manager_free_assets(IntPtr assets);
        }
    }

    /// <summary>
    /// Represents an asset.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Asset
    {
        /// <summary>
        /// The asset ID.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Id;

        /// <summary>
        /// The asset name.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        /// <summary>
        /// The asset symbol.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Symbol;

        /// <summary>
        /// The asset description.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Description;

        /// <summary>
        /// The number of decimal places.
        /// </summary>
        public int Decimals;

        /// <summary>
        /// The total supply.
        /// </summary>
        public double TotalSupply;

        /// <summary>
        /// The owner address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Owner;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long UpdatedAt;
    }
}