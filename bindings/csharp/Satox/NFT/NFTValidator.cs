using System;
using System.Runtime.InteropServices;

namespace Satox.NFT
{
    /// <summary>
    /// NFTValidator provides NFT validation functionality.
    /// </summary>
    public class NFTValidator : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of NFTValidator.
        /// </summary>
        public NFTValidator()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the NFT validator.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_nft_validator_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize NFT validator");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the NFT validator.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_nft_validator_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown NFT validator");

            _initialized = false;
        }

        /// <summary>
        /// Validates an NFT.
        /// </summary>
        /// <param name="nft">The NFT to validate.</param>
        /// <returns>True if the NFT is valid, false otherwise.</returns>
        public bool ValidateNFT(NFT nft)
        {
            EnsureInitialized();
            ValidateInput(nft, nameof(nft));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_nft_validator_validate_nft(
                    ref nft,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate NFT");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate NFT: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an NFT name.
        /// </summary>
        /// <param name="name">The NFT name to validate.</param>
        /// <returns>True if the name is valid, false otherwise.</returns>
        public bool ValidateName(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_nft_validator_validate_name(
                    name,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate NFT name");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate NFT name: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an NFT description.
        /// </summary>
        /// <param name="description">The NFT description to validate.</param>
        /// <returns>True if the description is valid, false otherwise.</returns>
        public bool ValidateDescription(string description)
        {
            EnsureInitialized();
            ValidateInput(description, nameof(description));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_nft_validator_validate_description(
                    description,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate NFT description");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate NFT description: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an NFT URI.
        /// </summary>
        /// <param name="uri">The NFT URI to validate.</param>
        /// <returns>True if the URI is valid, false otherwise.</returns>
        public bool ValidateUri(string uri)
        {
            EnsureInitialized();
            ValidateInput(uri, nameof(uri));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_nft_validator_validate_uri(
                    uri,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate NFT URI");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate NFT URI: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an NFT owner.
        /// </summary>
        /// <param name="owner">The NFT owner to validate.</param>
        /// <returns>True if the owner is valid, false otherwise.</returns>
        public bool ValidateOwner(string owner)
        {
            EnsureInitialized();
            ValidateInput(owner, nameof(owner));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_nft_validator_validate_owner(
                    owner,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate NFT owner");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate NFT owner: {ex.Message}");
            }
        }

        /// <summary>
        /// Validates an NFT transfer.
        /// </summary>
        /// <param name="nftId">The NFT ID.</param>
        /// <param name="from">The sender address.</param>
        /// <param name="to">The recipient address.</param>
        /// <returns>True if the transfer is valid, false otherwise.</returns>
        public bool ValidateTransfer(string nftId, string from, string to)
        {
            EnsureInitialized();
            ValidateInput(nftId, nameof(nftId));
            ValidateInput(from, nameof(from));
            ValidateInput(to, nameof(to));

            try
            {
                int isValid = 0;
                var status = NativeMethods.satox_nft_validator_validate_transfer(
                    nftId,
                    from,
                    to,
                    ref isValid
                );

                if (status != 0)
                    throw new SatoxError("Failed to validate NFT transfer");

                return isValid != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to validate NFT transfer: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("NFT validator is not initialized");
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

        ~NFTValidator()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_initialize();

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_shutdown();

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_validate_nft(
                ref NFT nft,
                ref int isValid
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_validate_name(
                string name,
                ref int isValid
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_validate_description(
                string description,
                ref int isValid
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_validate_uri(
                string uri,
                ref int isValid
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_validate_owner(
                string owner,
                ref int isValid
            );

            [DllImport("satox_nft")]
            public static extern int satox_nft_validator_validate_transfer(
                string nftId,
                string from,
                string to,
                ref int isValid
            );
        }
    }
} 