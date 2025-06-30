using System;

namespace Satox.Core
{
    /// <summary>
    /// SatoxError represents an error that occurred in the Satox SDK.
    /// </summary>
    public class SatoxError : Exception
    {
        /// <summary>
        /// Creates a new SatoxError with the specified message.
        /// </summary>
        /// <param name="message">The error message.</param>
        public SatoxError(string message) : base(message)
        {
        }

        /// <summary>
        /// Creates a new SatoxError with the specified message and cause.
        /// </summary>
        /// <param name="message">The error message.</param>
        /// <param name="innerException">The cause of the error.</param>
        public SatoxError(string message, Exception innerException) : base(message, innerException)
        {
        }
    }
} 