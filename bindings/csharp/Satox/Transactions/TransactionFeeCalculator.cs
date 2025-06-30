using System;
using System.Runtime.InteropServices;

namespace Satox.Transactions
{
    /// <summary>
    /// TransactionFeeCalculator provides transaction fee calculation functionality.
    /// </summary>
    public class TransactionFeeCalculator : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of TransactionFeeCalculator.
        /// </summary>
        public TransactionFeeCalculator()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the transaction fee calculator.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_transaction_fee_calculator_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize transaction fee calculator");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the transaction fee calculator.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_transaction_fee_calculator_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown transaction fee calculator");

            _initialized = false;
        }

        /// <summary>
        /// Calculates the transaction fee.
        /// </summary>
        /// <param name="amount">The transaction amount.</param>
        /// <param name="priority">The transaction priority.</param>
        /// <returns>The calculated fee.</returns>
        public decimal CalculateFee(decimal amount, TransactionPriority priority)
        {
            EnsureInitialized();

            try
            {
                double fee = 0;
                var status = NativeMethods.satox_transaction_fee_calculator_calculate_fee(
                    (double)amount,
                    (int)priority,
                    ref fee
                );

                if (status != 0)
                    throw new SatoxError("Failed to calculate fee");

                return (decimal)fee;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to calculate fee: {ex.Message}");
            }
        }

        /// <summary>
        /// Calculates the minimum transaction fee.
        /// </summary>
        /// <returns>The minimum fee.</returns>
        public decimal CalculateMinimumFee()
        {
            EnsureInitialized();

            try
            {
                double fee = 0;
                var status = NativeMethods.satox_transaction_fee_calculator_calculate_minimum_fee(
                    ref fee
                );

                if (status != 0)
                    throw new SatoxError("Failed to calculate minimum fee");

                return (decimal)fee;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to calculate minimum fee: {ex.Message}");
            }
        }

        /// <summary>
        /// Calculates the maximum transaction fee.
        /// </summary>
        /// <returns>The maximum fee.</returns>
        public decimal CalculateMaximumFee()
        {
            EnsureInitialized();

            try
            {
                double fee = 0;
                var status = NativeMethods.satox_transaction_fee_calculator_calculate_maximum_fee(
                    ref fee
                );

                if (status != 0)
                    throw new SatoxError("Failed to calculate maximum fee");

                return (decimal)fee;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to calculate maximum fee: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the fee multiplier for a given priority.
        /// </summary>
        /// <param name="priority">The transaction priority.</param>
        /// <returns>The fee multiplier.</returns>
        public decimal GetFeeMultiplier(TransactionPriority priority)
        {
            EnsureInitialized();

            try
            {
                double multiplier = 0;
                var status = NativeMethods.satox_transaction_fee_calculator_get_fee_multiplier(
                    (int)priority,
                    ref multiplier
                );

                if (status != 0)
                    throw new SatoxError("Failed to get fee multiplier");

                return (decimal)multiplier;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get fee multiplier: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets the base fee.
        /// </summary>
        /// <returns>The base fee.</returns>
        public decimal GetBaseFee()
        {
            EnsureInitialized();

            try
            {
                double fee = 0;
                var status = NativeMethods.satox_transaction_fee_calculator_get_base_fee(
                    ref fee
                );

                if (status != 0)
                    throw new SatoxError("Failed to get base fee");

                return (decimal)fee;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get base fee: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Transaction fee calculator is not initialized");
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

        ~TransactionFeeCalculator()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_initialize();

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_shutdown();

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_calculate_fee(
                double amount,
                int priority,
                ref double fee
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_calculate_minimum_fee(
                ref double fee
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_calculate_maximum_fee(
                ref double fee
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_get_fee_multiplier(
                int priority,
                ref double multiplier
            );

            [DllImport("satox_transactions")]
            public static extern int satox_transaction_fee_calculator_get_base_fee(
                ref double fee
            );
        }
    }

    /// <summary>
    /// Represents the transaction priority.
    /// </summary>
    public enum TransactionPriority
    {
        /// <summary>
        /// Low priority transaction.
        /// </summary>
        Low = 0,

        /// <summary>
        /// Medium priority transaction.
        /// </summary>
        Medium = 1,

        /// <summary>
        /// High priority transaction.
        /// </summary>
        High = 2,

        /// <summary>
        /// Urgent priority transaction.
        /// </summary>
        Urgent = 3
    }
} 