using satox_bindings;

namespace Satox.Assets
{
    /// <summary>
    /// AssetManager provides asset management functionality.
    /// </summary>
    public class AssetManager : satox_bindings.AssetManager
    {
        // Re-export only
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