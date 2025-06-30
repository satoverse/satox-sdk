-- Satox SDK Lua Bindings
-- Main entry point for the Lua SDK

local satox = {}

-- Core functionality - load individual manager files
satox.core = require("satox.core_manager")
satox.database = require("satox.database_manager")
satox.blockchain = require("satox.blockchain_manager")
satox.asset = require("satox.asset_manager")
satox.nft = require("satox.nft_manager")
satox.ipfs = require("satox.ipfs_manager")
satox.network = require("satox.network_manager")
satox.security = require("satox.security_manager")
satox.wallet = require("satox.wallet_manager")

-- Utility functions
function satox.initialize(config)
    config = config or {}
    local success, err = satox.core.initialize(config)
    if not success then
        error("Failed to initialize Satox SDK: " .. tostring(err))
    end
    return true
end

function satox.shutdown()
    local success, err = satox.core.shutdown()
    if not success then
        error("Failed to shutdown Satox SDK: " .. tostring(err))
    end
    return true
end

function satox.get_version()
    return satox.core.get_version()
end

-- Convenience functions for common operations
function satox.create_asset(config)
    return satox.asset.create_asset(config)
end

function satox.get_blockchain_info()
    return satox.blockchain.get_blockchain_info()
end

function satox.create_wallet()
    return satox.wallet.create_wallet()
end

return satox 