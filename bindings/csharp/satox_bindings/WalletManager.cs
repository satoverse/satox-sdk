using System;
using System.Runtime.InteropServices;

namespace Satox.Wallet
{
    /// <summary>
    /// WalletManager provides wallet management functionality.
    /// </summary>
    public class WalletManager : ThreadSafeManager
    {
        /// <summary>
        /// Creates a new instance of WalletManager.
        /// </summary>
        public WalletManager() : base()
        {
        }

        /// <summary>
        /// Initializes the wallet manager.
        /// </summary>
        public void Initialize()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (IsInitialized)
                    return;

                var status = NativeMethods.satox_wallet_manager_initialize();
                if (status != 0)
                    throw new SatoxError("Failed to initialize wallet manager");

                SetInitialized(true);
            }
        }

        /// <summary>
        /// Shuts down the wallet manager.
        /// </summary>
        public void Shutdown()
        {
            ValidateNotDisposed();

            using (AcquireWriteLock())
            {
                if (!IsInitialized)
                    return;

                var status = NativeMethods.satox_wallet_manager_shutdown();
                if (status != 0)
                    throw new SatoxError("Failed to shutdown wallet manager");

                SetInitialized(false);
            }
        }

        /// <summary>
        /// Creates a new wallet.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        /// <param name="password">The wallet password.</param>
        /// <returns>The created wallet.</returns>
        public Wallet CreateWallet(string name, string password)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(password, nameof(password));

            try
            {
                IntPtr walletPtr = IntPtr.Zero;
                var status = NativeMethods.satox_wallet_manager_create_wallet(
                    name,
                    password,
                    ref walletPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to create wallet");

                var wallet = Marshal.PtrToStructure<Wallet>(walletPtr);
                NativeMethods.satox_wallet_manager_free_wallet(walletPtr);

                return wallet;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to create wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Opens a wallet.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        /// <param name="password">The wallet password.</param>
        /// <returns>The opened wallet.</returns>
        public Wallet OpenWallet(string name, string password)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(password, nameof(password));

            try
            {
                IntPtr walletPtr = IntPtr.Zero;
                var status = NativeMethods.satox_wallet_manager_open_wallet(
                    name,
                    password,
                    ref walletPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to open wallet");

                var wallet = Marshal.PtrToStructure<Wallet>(walletPtr);
                NativeMethods.satox_wallet_manager_free_wallet(walletPtr);

                return wallet;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to open wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Closes a wallet.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        public void CloseWallet(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                var status = NativeMethods.satox_wallet_manager_close_wallet(name);
                if (status != 0)
                    throw new SatoxError("Failed to close wallet");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to close wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Deletes a wallet.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        /// <param name="password">The wallet password.</param>
        public void DeleteWallet(string name, string password)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(password, nameof(password));

            try
            {
                var status = NativeMethods.satox_wallet_manager_delete_wallet(name, password);
                if (status != 0)
                    throw new SatoxError("Failed to delete wallet");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to delete wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a wallet by name.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        /// <returns>The wallet information.</returns>
        public Wallet GetWallet(string name)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));

            try
            {
                IntPtr walletPtr = IntPtr.Zero;
                var status = NativeMethods.satox_wallet_manager_get_wallet(
                    name,
                    ref walletPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to get wallet");

                var wallet = Marshal.PtrToStructure<Wallet>(walletPtr);
                NativeMethods.satox_wallet_manager_free_wallet(walletPtr);

                return wallet;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets all wallets.
        /// </summary>
        /// <returns>An array of wallets.</returns>
        public Wallet[] GetAllWallets()
        {
            EnsureInitialized();

            try
            {
                IntPtr walletsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_wallet_manager_get_all_wallets(
                    ref walletsPtr,
                    ref count
                );

                if (status != 0)
                    throw new SatoxError("Failed to get all wallets");

                var wallets = new Wallet[count];
                var currentPtr = walletsPtr;

                for (int i = 0; i < count; i++)
                {
                    wallets[i] = Marshal.PtrToStructure<Wallet>(currentPtr);
                    currentPtr = IntPtr.Add(currentPtr, Marshal.SizeOf<Wallet>());
                }

                NativeMethods.satox_wallet_manager_free_wallets(walletsPtr);

                return wallets;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get all wallets: {ex.Message}");
            }
        }

        /// <summary>
        /// Changes the password of a wallet.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        /// <param name="oldPassword">The old password.</param>
        /// <param name="newPassword">The new password.</param>
        public void ChangePassword(string name, string oldPassword, string newPassword)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(oldPassword, nameof(oldPassword));
            ValidateInput(newPassword, nameof(newPassword));

            try
            {
                var status = NativeMethods.satox_wallet_manager_change_password(name, oldPassword, newPassword);
                if (status != 0)
                    throw new SatoxError("Failed to change password");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to change password: {ex.Message}");
            }
        }

        /// <summary>
        /// Backs up a wallet.
        /// </summary>
        /// <param name="name">The wallet name.</param>
        /// <param name="password">The wallet password.</param>
        /// <param name="backupPath">The backup file path.</param>
        public void BackupWallet(string name, string password, string backupPath)
        {
            EnsureInitialized();
            ValidateInput(name, nameof(name));
            ValidateInput(password, nameof(password));
            ValidateInput(backupPath, nameof(backupPath));

            try
            {
                var status = NativeMethods.satox_wallet_manager_backup_wallet(name, password, backupPath);
                if (status != 0)
                    throw new SatoxError("Failed to backup wallet");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to backup wallet: {ex.Message}");
            }
        }

        /// <summary>
        /// Restores a wallet from backup.
        /// </summary>
        /// <param name="backupPath">The backup file path.</param>
        /// <param name="name">The wallet name.</param>
        /// <param name="password">The wallet password.</param>
        public void RestoreWallet(string backupPath, string name, string password)
        {
            EnsureInitialized();
            ValidateInput(backupPath, nameof(backupPath));
            ValidateInput(name, nameof(name));
            ValidateInput(password, nameof(password));

            try
            {
                var status = NativeMethods.satox_wallet_manager_restore_wallet(backupPath, name, password);
                if (status != 0)
                    throw new SatoxError("Failed to restore wallet");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to restore wallet: {ex.Message}");
            }
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
            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_initialize();

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_shutdown();

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_create_wallet(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string password,
                ref IntPtr wallet
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_open_wallet(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string password,
                ref IntPtr wallet
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_close_wallet([MarshalAs(UnmanagedType.LPStr)] string name);

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_delete_wallet(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string password
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_get_wallet(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                ref IntPtr wallet
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_get_all_wallets(
                ref IntPtr wallets,
                ref int count
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_change_password(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string oldPassword,
                [MarshalAs(UnmanagedType.LPStr)] string newPassword
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_backup_wallet(
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string password,
                [MarshalAs(UnmanagedType.LPStr)] string backupPath
            );

            [DllImport("satox_wallet")]
            public static extern int satox_wallet_manager_restore_wallet(
                [MarshalAs(UnmanagedType.LPStr)] string backupPath,
                [MarshalAs(UnmanagedType.LPStr)] string name,
                [MarshalAs(UnmanagedType.LPStr)] string password
            );

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_manager_free_wallet(IntPtr wallet);

            [DllImport("satox_wallet")]
            public static extern void satox_wallet_manager_free_wallets(IntPtr wallets);
        }
    }

    /// <summary>
    /// Represents a wallet.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Wallet
    {
        /// <summary>
        /// The wallet name.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Name;

        /// <summary>
        /// The wallet address.
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string Address;

        /// <summary>
        /// The wallet balance.
        /// </summary>
        public double Balance;

        /// <summary>
        /// The creation timestamp.
        /// </summary>
        public long CreatedAt;

        /// <summary>
        /// The last update timestamp.
        /// </summary>
        public long UpdatedAt;

        /// <summary>
        /// Whether the wallet is encrypted.
        /// </summary>
        public int IsEncrypted;

        /// <summary>
        /// Whether the wallet is locked.
        /// </summary>
        public int IsLocked;
    }
}