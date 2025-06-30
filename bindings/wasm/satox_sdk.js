/**
 * @file satox_sdk.js
 * @brief JavaScript wrapper for Satox SDK WASM binding
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

// Default configuration
const DEFAULT_CONFIG = {
    data_dir: "./data",
    network: "mainnet",
    enable_mining: false,
    enable_sync: true,
    sync_interval_ms: 1000,
    mining_threads: 1,
    max_connections: 10,
    rpc_port: 7777,
    p2p_port: 67777,
    enable_rpc: true,
    enable_p2p: true,
    log_level: "info",
    log_file: "./logs/satox-sdk.log"
};

// SatoxSDK class
class SatoxSDK {
    constructor(config = {}) {
        this.config = { ...DEFAULT_CONFIG, ...config };
        this.isRunning = false;
        this.managers = {};
        this.wasmInstance = null;
    }

    /**
     * Initialize the SDK with WASM
     */
    async initialize() {
        if (this.isRunning) {
            throw new Error("SDK is already running");
        }

        try {
            // Load WASM module
            const response = await fetch('./satox_sdk.wasm');
            const wasmBuffer = await response.arrayBuffer();
            
            // Initialize WASM
            const wasmModule = await WebAssembly.instantiate(wasmBuffer, {
                env: {
                    memory: new WebAssembly.Memory({ initial: 256 }),
                    abort: () => console.error('WASM abort called'),
                    // Add other required environment functions
                }
            });

            this.wasmInstance = wasmModule.instance;
            
            // Initialize core SDK
            const result = this.wasmInstance.exports.satox_sdk_initialize();
            if (result !== 0) {
                throw new Error("Failed to initialize SDK");
            }

            // Create and initialize managers
            this.managers.core = this.wasmInstance.exports.satox_core_manager_create();
            this.managers.wallet = this.wasmInstance.exports.satox_wallet_manager_create();
            this.managers.security = this.wasmInstance.exports.satox_security_manager_create();
            this.managers.asset = this.wasmInstance.exports.satox_asset_manager_create();
            this.managers.nft = this.wasmInstance.exports.satox_nft_manager_create();
            this.managers.blockchain = this.wasmInstance.exports.satox_blockchain_manager_create();
            this.managers.ipfs = this.wasmInstance.exports.satox_ipfs_manager_create();
            this.managers.network = this.wasmInstance.exports.satox_network_manager_create();
            this.managers.database = this.wasmInstance.exports.satox_database_manager_create();

            // Initialize all managers
            const configJson = JSON.stringify(this.config);
            const configPtr = this.stringToPtr(configJson);

            for (const [name, manager] of Object.entries(this.managers)) {
                const initFunc = this.wasmInstance.exports[`satox_${name}_manager_initialize`];
                const result = initFunc(manager, configPtr);
                if (result !== 0) {
                    throw new Error(`Failed to initialize ${name} manager`);
                }
            }

            this.isRunning = true;
        } catch (error) {
            throw new Error(`SDK initialization failed: ${error.message}`);
        }
    }

    /**
     * Start all SDK services
     */
    start() {
        if (!this.isRunning) {
            throw new Error("SDK is not initialized");
        }

        for (const [name, manager] of Object.entries(this.managers)) {
            const startFunc = this.wasmInstance.exports[`satox_${name}_manager_start`];
            const result = startFunc(manager);
            if (result !== 0) {
                throw new Error(`Failed to start ${name} manager`);
            }
        }
    }

    /**
     * Stop all SDK services
     */
    stop() {
        if (!this.isRunning) {
            return;
        }

        for (const [name, manager] of Object.entries(this.managers)) {
            const stopFunc = this.wasmInstance.exports[`satox_${name}_manager_stop`];
            stopFunc(manager);
        }

        this.isRunning = false;
    }

    /**
     * Connect to the blockchain network
     */
    connect() {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const result = this.wasmInstance.exports.satox_network_manager_connect(this.managers.network);
        if (result !== 0) {
            throw new Error("Failed to connect to network");
        }
    }

    /**
     * Disconnect from the blockchain network
     */
    disconnect() {
        if (!this.isRunning) {
            return;
        }
        this.wasmInstance.exports.satox_network_manager_disconnect(this.managers.network);
    }

    /**
     * Shutdown the SDK
     */
    shutdown() {
        this.stop();
        this.wasmInstance.exports.satox_sdk_shutdown();
    }

    /**
     * Get manager by name
     */
    getManager(name) {
        return this.managers[name];
    }

    /**
     * Get SDK version
     */
    getVersion() {
        const versionPtr = this.wasmInstance.exports.satox_sdk_get_version();
        return this.ptrToString(versionPtr);
    }

    /**
     * Get configuration
     */
    getConfig() {
        return this.config;
    }

    /**
     * Check if running
     */
    isRunning() {
        return this.isRunning;
    }

    /**
     * Convert JavaScript string to WASM pointer
     */
    stringToPtr(str) {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(str + '\0');
        const ptr = this.wasmInstance.exports.malloc(bytes.length);
        const memory = new Uint8Array(this.wasmInstance.exports.memory.buffer);
        memory.set(bytes, ptr);
        return ptr;
    }

    /**
     * Convert WASM pointer to JavaScript string
     */
    ptrToString(ptr) {
        if (ptr === 0) return null;
        
        const memory = new Uint8Array(this.wasmInstance.exports.memory.buffer);
        let end = ptr;
        while (memory[end] !== 0) end++;
        
        const bytes = memory.slice(ptr, end);
        const decoder = new TextDecoder();
        return decoder.decode(bytes);
    }

    /**
     * Create wallet
     */
    createWallet(name) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const namePtr = this.stringToPtr(name);
        const resultPtr = this.wasmInstance.exports.satox_wallet_manager_create_wallet(this.managers.wallet, namePtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Get wallet info
     */
    getWalletInfo(address) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const addressPtr = this.stringToPtr(address);
        const resultPtr = this.wasmInstance.exports.satox_wallet_manager_get_wallet_info(this.managers.wallet, addressPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Generate keypair
     */
    generateKeypair() {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const resultPtr = this.wasmInstance.exports.satox_security_manager_generate_keypair(this.managers.security);
        return this.ptrToString(resultPtr);
    }

    /**
     * Sign data
     */
    signData(data, privateKey) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const dataPtr = this.stringToPtr(data);
        const keyPtr = this.stringToPtr(privateKey);
        const resultPtr = this.wasmInstance.exports.satox_security_manager_sign_data(this.managers.security, dataPtr, keyPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Verify signature
     */
    verifySignature(data, signature, publicKey) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const dataPtr = this.stringToPtr(data);
        const sigPtr = this.stringToPtr(signature);
        const keyPtr = this.stringToPtr(publicKey);
        const result = this.wasmInstance.exports.satox_security_manager_verify_signature(this.managers.security, dataPtr, sigPtr, keyPtr);
        return result === 1;
    }

    /**
     * Create asset
     */
    createAsset(config, ownerAddress) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const configPtr = this.stringToPtr(JSON.stringify(config));
        const addressPtr = this.stringToPtr(ownerAddress);
        const resultPtr = this.wasmInstance.exports.satox_asset_manager_create_asset(this.managers.asset, configPtr, addressPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Get asset info
     */
    getAssetInfo(assetId) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const assetPtr = this.stringToPtr(assetId);
        const resultPtr = this.wasmInstance.exports.satox_asset_manager_get_asset_info(this.managers.asset, assetPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Create NFT
     */
    createNFT(assetId, metadata, ownerAddress) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const assetPtr = this.stringToPtr(assetId);
        const metadataPtr = this.stringToPtr(JSON.stringify(metadata));
        const addressPtr = this.stringToPtr(ownerAddress);
        const resultPtr = this.wasmInstance.exports.satox_nft_manager_create_nft(this.managers.nft, assetPtr, metadataPtr, addressPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Get NFT info
     */
    getNFTInfo(nftId) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const nftPtr = this.stringToPtr(nftId);
        const resultPtr = this.wasmInstance.exports.satox_nft_manager_get_nft_info(this.managers.nft, nftPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Get block info
     */
    getBlockInfo(hash) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const hashPtr = this.stringToPtr(hash);
        const resultPtr = this.wasmInstance.exports.satox_blockchain_manager_get_block_info(this.managers.blockchain, hashPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Get transaction info
     */
    getTransactionInfo(txid) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const txidPtr = this.stringToPtr(txid);
        const resultPtr = this.wasmInstance.exports.satox_blockchain_manager_get_transaction_info(this.managers.blockchain, txidPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Upload file to IPFS
     */
    uploadFile(filePath) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const pathPtr = this.stringToPtr(filePath);
        const resultPtr = this.wasmInstance.exports.satox_ipfs_manager_upload_file(this.managers.ipfs, pathPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Get IPFS file info
     */
    getIPFSFileInfo(hash) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const hashPtr = this.stringToPtr(hash);
        const resultPtr = this.wasmInstance.exports.satox_ipfs_manager_get_file_info(this.managers.ipfs, hashPtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * Create database
     */
    createDatabase(name) {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const namePtr = this.stringToPtr(name);
        const resultPtr = this.wasmInstance.exports.satox_database_manager_create_database(this.managers.database, namePtr);
        return this.ptrToString(resultPtr);
    }

    /**
     * List databases
     */
    listDatabases() {
        if (!this.isRunning) {
            throw new Error("SDK is not running");
        }
        const resultPtr = this.wasmInstance.exports.satox_database_manager_list_databases(this.managers.database);
        return this.ptrToString(resultPtr);
    }
}

// Module exports
if (typeof module !== 'undefined' && module.exports) {
    module.exports = SatoxSDK;
} else if (typeof window !== 'undefined') {
    window.SatoxSDK = SatoxSDK;
}

// Version information
SatoxSDK.VERSION = "1.0.0";
SatoxSDK.BUILD_DATE = "2025-06-30";
SatoxSDK.DEFAULT_CONFIG = DEFAULT_CONFIG; 