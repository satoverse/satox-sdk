const { EventEmitter } = require("events");

class CoreManager extends EventEmitter {
  constructor() {
    super();
    this._initialized = false;
    this._version = "1.0.0";
    this._buildInfo = {
      version: this._version,
      buildDate: new Date().toISOString(),
      platform: process.platform,
      arch: process.arch,
    };
    this._config = {
      network: "mainnet",
      p2pPort: 60777,
      rpcPort: 7777,
      slip44: 9007, // Using the registered SATOX SLIP-44 code
    };
  }

  /**
   * Initialize the Core Manager
   * @returns {Promise<void>}
   */
  async initialize() {
    if (this._initialized) {
      throw new Error("CoreManager already initialized");
    }

    try {
      // Initialize core components
      this._initialized = true;
      this.emit("initialized");
    } catch (error) {
      throw new Error(`Failed to initialize CoreManager: ${error.message}`);
    }
  }

  /**
   * Check if the manager is initialized
   * @returns {boolean}
   */
  isInitialized() {
    return this._initialized;
  }

  /**
   * Get the current version
   * @returns {string}
   */
  getVersion() {
    return this._version;
  }

  /**
   * Get build information
   * @returns {Object}
   */
  getBuildInfo() {
    return { ...this._buildInfo };
  }

  /**
   * Get current configuration
   * @returns {Object}
   */
  getConfig() {
    return { ...this._config };
  }

  /**
   * Update configuration
   * @param {Object} newConfig - New configuration values
   * @returns {Promise<void>}
   */
  async updateConfig(newConfig) {
    if (!this._initialized) {
      throw new Error("CoreManager not initialized");
    }

    try {
      this._config = {
        ...this._config,
        ...newConfig,
      };
      this.emit("configUpdated", this._config);
    } catch (error) {
      throw new Error(`Failed to update configuration: ${error.message}`);
    }
  }

  /**
   * Reset the manager to its initial state
   * @returns {Promise<void>}
   */
  async reset() {
    if (!this._initialized) {
      throw new Error("CoreManager not initialized");
    }

    try {
      this._initialized = false;
      this._config = {
        network: "mainnet",
        p2pPort: 60777,
        rpcPort: 7777,
        slip44: 9007,
      };
      this.emit("reset");
    } catch (error) {
      throw new Error(`Failed to reset CoreManager: ${error.message}`);
    }
  }

  /**
   * Perform health check
   * @returns {Promise<Object>}
   */
  async healthCheck() {
    if (!this._initialized) {
      throw new Error("CoreManager not initialized");
    }

    try {
      return {
        status: "healthy",
        timestamp: new Date().toISOString(),
        version: this._version,
        network: this._config.network,
      };
    } catch (error) {
      throw new Error(`Health check failed: ${error.message}`);
    }
  }

  /**
   * Clean up resources
   * @returns {Promise<void>}
   */
  async cleanup() {
    if (!this._initialized) {
      return;
    }

    try {
      this._initialized = false;
      this.emit("cleanup");
    } catch (error) {
      throw new Error(`Cleanup failed: ${error.message}`);
    }
  }
}

module.exports = CoreManager;
