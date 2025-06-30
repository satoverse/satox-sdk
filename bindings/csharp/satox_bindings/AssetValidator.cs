using System;
using System.Runtime.InteropServices;

namespace Satox.Assets
{
    /// <summary>
    /// AssetValidator provides asset validation functionality.
    /// </summary>
    public class AssetValidator : ThreadSafeManager
    {
        /// <summary>
        /// Creates a new instance of AssetValidator.
        /// </summary>
        public AssetValidator() : base()
        {
        }

        /// <summary>
        /// Initializes the asset validator.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_asset_validator_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize asset validator");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the asset validator.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_asset_validator_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown asset validator");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Validates an asset.
        /// </summary>
        /// <param name="asset">The asset to validate.</param>
        /// <returns>The validation result.</returns>
        public ValidationResult ValidateAsset(Asset asset)
        {
            EnsureInitialized();
            ValidateInput(asset, nameof(asset));

            try
            {
                IntPtr resultPtr = IntPtr.Zero;
                var status = NativeMethods.satox_asset_validator_validate_asset(
                    ref asset,
                    ref resultPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate asset");

                var result = Marshal.PtrToStructure<ValidationResult>(resultPtr);
                NativeMethods.satox_asset_validator_free_validation_result(resultPtr);

                return result;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate asset: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an asset name.
        /// </summary>
        /// <param name="name">The asset name to validate.</param>
        /// <returns>True if the name is valid, false otherwise.</returns>
        public bool ValidateName(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_asset_validator_validate_name(
                    name,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate name");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate name: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an asset symbol.
        /// </summary>
        /// <param name="symbol">The asset symbol to validate.</param>
        /// <returns>True if the symbol is valid, false otherwise.</returns>
        public bool ValidateSymbol(string symbol)
        {
            EnsureInitialized();
            ValidateInput(symbol, nameof(symbol));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_asset_validator_validate_symbol(
                    symbol,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate symbol");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate symbol: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates the number of decimal places.
        /// </summary>
        /// <param name="decimals">The number of decimal places to validate.</param>
        /// <returns>True if the number of decimal places is valid, false otherwise.</returns>
        public bool ValidateDecimals(int decimals)
        {
            EnsureInitialized();

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_asset_validator_validate_decimals(
                    decimals,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate decimals");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate decimals: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates the total supply.
        /// </summary>
        /// <param name="totalSupply">The total supply to validate.</param>
        /// <returns>True if the total supply is valid, false otherwise.</returns>
        public bool ValidateTotalSupply(decimal totalSupply)
        {
            EnsureInitialized();

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_asset_validator_validate_total_supply(
                    (double)totalSupply,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate total supply");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate total supply: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an owner address.
        /// </summary>
        /// <param name="owner">The owner address to validate.</param>
        /// <returns>True if the owner address is valid, false otherwise.</returns>
        public bool ValidateOwner(string owner)
        {
            EnsureInitialized();
            ValidateInput(owner, nameof(owner));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_asset_validator_validate_owner(
                    owner,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate owner");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate owner: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!IsInitialized)
                throw new SatoxError("Asset validator is not initialized");
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

        private static class NativeMethods
        {
            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_initialize();

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_shutdown();

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_validate_asset(
                ref Asset asset,
                ref IntPtr result
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_validate_name(
                string name,
                ref int isValid
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_validate_symbol(
                string symbol,
                ref int isValid
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_validate_decimals(
                int decimals,
                ref int isValid
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_validate_total_supply(
                double totalSupply,
                ref int isValid
            );

            [DllImport("satox_assets")]
            public static extern int satox_asset_validator_validate_owner(
                string owner,
                ref int isValid
            );

            [DllImport("satox_assets")]
            public static extern void satox_asset_validator_free_validation_result(IntPtr result);
        }
    }

    /// <summary>
    /// Represents a validation result.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct ValidationResult
    {
        /// <summary>
        /// Whether the validation was successful.
        /// </summary>
        public int IsValid;

        /// <summary>
        /// The validation error message.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string ErrorMessage;

        /// <summary>
        /// The validation error code.
        /// </summary>
        public int ErrorCode;
    }
}