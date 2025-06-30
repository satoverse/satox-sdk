using System;
using System.Threading;

namespace Satox.Bindings.Core
{
    /// <summary>
    /// Base class providing thread safety for all Satox managers
    /// </summary>
    public abstract class ThreadSafeManager : IDisposable
    {
        private readonly ReaderWriterLockSlim _lock;
        private volatile bool _initialized;
        private volatile bool _disposed;

        protected ThreadSafeManager()
        {
            _lock = new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Gets whether the manager is initialized
        /// </summary>
        public bool IsInitialized => _initialized;

        /// <summary>
        /// Gets whether the manager is disposed
        /// </summary>
        public bool IsDisposed => _disposed;

        /// <summary>
        /// Acquires a read lock for thread-safe read operations
        /// </summary>
        protected IDisposable AcquireReadLock()
        {
            return new ReadLock(_lock);
        }

        /// <summary>
        /// Acquires a write lock for thread-safe write operations
        /// </summary>
        protected IDisposable AcquireWriteLock()
        {
            return new WriteLock(_lock);
        }

        /// <summary>
        /// Ensures the manager is initialized, throws if not
        /// </summary>
        protected void EnsureInitialized()
        {
            using (AcquireReadLock())
            {
                if (_disposed)
                    throw new ObjectDisposedException(GetType().Name);

                if (!_initialized)
                    throw new InvalidOperationException($"{GetType().Name} is not initialized");
            }
        }

        /// <summary>
        /// Sets the initialized state in a thread-safe manner
        /// </summary>
        protected void SetInitialized(bool initialized)
        {
            using (AcquireWriteLock())
            {
                _initialized = initialized;
            }
        }

        /// <summary>
        /// Validates that the manager is not disposed
        /// </summary>
        protected void ValidateNotDisposed()
        {
            using (AcquireReadLock())
            {
                if (_disposed)
                    throw new ObjectDisposedException(GetType().Name);
            }
        }

        /// <summary>
        /// Disposes the manager in a thread-safe manner
        /// </summary>
        public virtual void Dispose()
        {
            using (AcquireWriteLock())
            {
                if (!_disposed)
                {
                    DisposeCore();
                    _disposed = true;
                }
            }
        }

        /// <summary>
        /// Override to implement custom disposal logic
        /// </summary>
        protected virtual void DisposeCore()
        {
            // Override in derived classes
        }

        /// <summary>
        /// Finalizer
        /// </summary>
        ~ThreadSafeManager()
        {
            Dispose();
        }

        /// <summary>
        /// Read lock wrapper for RAII pattern
        /// </summary>
        private class ReadLock : IDisposable
        {
            private readonly ReaderWriterLockSlim _lock;

            public ReadLock(ReaderWriterLockSlim lockObj)
            {
                _lock = lockObj;
                _lock.EnterReadLock();
            }

            public void Dispose()
            {
                _lock.ExitReadLock();
            }
        }

        /// <summary>
        /// Write lock wrapper for RAII pattern
        /// </summary>
        private class WriteLock : IDisposable
        {
            private readonly ReaderWriterLockSlim _lock;

            public WriteLock(ReaderWriterLockSlim lockObj)
            {
                _lock = lockObj;
                _lock.EnterWriteLock();
            }

            public void Dispose()
            {
                _lock.ExitWriteLock();
            }
        }
    }
} 