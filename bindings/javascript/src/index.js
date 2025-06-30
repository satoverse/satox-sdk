const { loadBinding } = require('@node-rs/helper')

/**
 * @type {import('./types').SatoxSDK}
 */
module.exports = loadBinding(__dirname, 'satox-sdk', 'satox-sdk')

// Re-export types for TypeScript support
module.exports.types = require('./types')

// Initialize the SDK when the module is loaded
const sdk = module.exports
if (sdk.initialize) {
  try {
    sdk.initialize()
  } catch (error) {
    console.warn('Failed to initialize Satox SDK:', error.message)
  }
}

// Graceful shutdown on process exit
process.on('exit', () => {
  if (sdk.shutdown) {
    try {
      sdk.shutdown()
    } catch (error) {
      // Ignore shutdown errors
    }
  }
})

process.on('SIGINT', () => {
  if (sdk.shutdown) {
    try {
      sdk.shutdown()
    } catch (error) {
      // Ignore shutdown errors
    }
  }
  process.exit(0)
})
