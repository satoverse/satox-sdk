using System;
using System.Runtime.InteropServices;

namespace Satox.Database
{
    /// <summary>
    /// QueryBuilder provides query building functionality.
    /// </summary>
    public class QueryBuilder : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of QueryBuilder.
        /// </summary>
        public QueryBuilder()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the query builder.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_query_builder_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize query builder");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the query builder.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_query_builder_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown query builder");

            _initialized = false;
        }

        /// <summary>
        /// Builds a select query.
        /// </summary>
        /// <param name="table">The table name.</param>
        /// <param name="columns">The column names.</param>
        /// <param name="conditions">The conditions.</param>
        /// <returns>The built query.</returns>
        public string BuildSelectQuery(string table, string[] columns, string[] conditions)
        {
            EnsureInitialized();
            ValidateInput(table, nameof(table));
            ValidateInput(columns, nameof(columns));
            ValidateInput(conditions, nameof(conditions));

            try
            {
                IntPtr queryPtr = IntPtr.Zero;
                var status = NativeMethods.satox_query_builder_build_select_query(
                    table,
                    columns,
                    columns.Length,
                    conditions,
                    conditions.Length,
                    ref queryPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to build select query");

                var query = Marshal.PtrToStringAnsi(queryPtr);
                NativeMethods.satox_query_builder_free_string(queryPtr);

                return query;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to build select query: {ex.Message}");
            }
        }

        /// <summary>
        /// Builds an insert query.
        /// </summary>
        /// <param name="table">The table name.</param>
        /// <param name="columns">The column names.</param>
        /// <param name="values">The values.</param>
        /// <returns>The built query.</returns>
        public string BuildInsertQuery(string table, string[] columns, string[] values)
        {
            EnsureInitialized();
            ValidateInput(table, nameof(table));
            ValidateInput(columns, nameof(columns));
            ValidateInput(values, nameof(values));

            try
            {
                IntPtr queryPtr = IntPtr.Zero;
                var status = NativeMethods.satox_query_builder_build_insert_query(
                    table,
                    columns,
                    columns.Length,
                    values,
                    values.Length,
                    ref queryPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to build insert query");

                var query = Marshal.PtrToStringAnsi(queryPtr);
                NativeMethods.satox_query_builder_free_string(queryPtr);

                return query;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to build insert query: {ex.Message}");
            }
        }

        /// <summary>
        /// Builds an update query.
        /// </summary>
        /// <param name="table">The table name.</param>
        /// <param name="columns">The column names.</param>
        /// <param name="values">The values.</param>
        /// <param name="conditions">The conditions.</param>
        /// <returns>The built query.</returns>
        public string BuildUpdateQuery(string table, string[] columns, string[] values, string[] conditions)
        {
            EnsureInitialized();
            ValidateInput(table, nameof(table));
            ValidateInput(columns, nameof(columns));
            ValidateInput(values, nameof(values));
            ValidateInput(conditions, nameof(conditions));

            try
            {
                IntPtr queryPtr = IntPtr.Zero;
                var status = NativeMethods.satox_query_builder_build_update_query(
                    table,
                    columns,
                    columns.Length,
                    values,
                    values.Length,
                    conditions,
                    conditions.Length,
                    ref queryPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to build update query");

                var query = Marshal.PtrToStringAnsi(queryPtr);
                NativeMethods.satox_query_builder_free_string(queryPtr);

                return query;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to build update query: {ex.Message}");
            }
        }

        /// <summary>
        /// Builds a delete query.
        /// </summary>
        /// <param name="table">The table name.</param>
        /// <param name="conditions">The conditions.</param>
        /// <returns>The built query.</returns>
        public string BuildDeleteQuery(string table, string[] conditions)
        {
            EnsureInitialized();
            ValidateInput(table, nameof(table));
            ValidateInput(conditions, nameof(conditions));

            try
            {
                IntPtr queryPtr = IntPtr.Zero;
                var status = NativeMethods.satox_query_builder_build_delete_query(
                    table,
                    conditions,
                    conditions.Length,
                    ref queryPtr
                );

                if (status != 0)
                    throw new SatoxError("Failed to build delete query");

                var query = Marshal.PtrToStringAnsi(queryPtr);
                NativeMethods.satox_query_builder_free_string(queryPtr);

                return query;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to build delete query: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Query builder is not initialized");
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

        private void ValidateInput(string[] input, string paramName)
        {
            if (input == null || input.Length == 0)
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

        ~QueryBuilder()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_database")]
            public static extern int satox_query_builder_initialize();

            [DllImport("satox_database")]
            public static extern int satox_query_builder_shutdown();

            [DllImport("satox_database")]
            public static extern int satox_query_builder_build_select_query(
                string table,
                string[] columns,
                int columnCount,
                string[] conditions,
                int conditionCount,
                ref IntPtr query
            );

            [DllImport("satox_database")]
            public static extern int satox_query_builder_build_insert_query(
                string table,
                string[] columns,
                int columnCount,
                string[] values,
                int valueCount,
                ref IntPtr query
            );

            [DllImport("satox_database")]
            public static extern int satox_query_builder_build_update_query(
                string table,
                string[] columns,
                int columnCount,
                string[] values,
                int valueCount,
                string[] conditions,
                int conditionCount,
                ref IntPtr query
            );

            [DllImport("satox_database")]
            public static extern int satox_query_builder_build_delete_query(
                string table,
                string[] conditions,
                int conditionCount,
                ref IntPtr query
            );

            [DllImport("satox_database")]
            public static extern void satox_query_builder_free_string(IntPtr str);
        }
    }
} 