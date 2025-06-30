# Satox SDK Lua API Documentation

## Table of Contents
1. [Getting Started](#getting-started)
2. [Core Components](#core-components)
3. [Blockchain Integration](#blockchain-integration)
4. [Asset Management](#asset-management)
5. [NFT Management](#nft-management)
6. [Security](#security)
7. [Examples](#examples)

## Getting Started

### Installation
Install the Satox SDK using LuaRocks:

```bash
luarocks install satox-sdk
```

### Basic Usage
```lua
local satox = require("satox")
local blockchain = require("satox.blockchain")
local asset = require("satox.asset")
local nft = require("satox.nft")

-- Initialize SDK
local sdk = satox.new()
sdk:initialize()

-- Get managers
local blockchain_manager = sdk:get_blockchain_manager()
local asset_manager = sdk:get_asset_manager()
local nft_manager = sdk:get_nft_manager()

-- Use managers...
```

## Core Components

### SDK
The main entry point for the SDK.

```lua
local SDK = {
    -- Internal fields
}

function SDK.new(config)
    -- Create new SDK instance
end

function SDK:initialize()
    -- Initialize SDK with configuration
end

function SDK:shutdown()
    -- Shutdown SDK and cleanup resources
end

function SDK:get_blockchain_manager()
    -- Get blockchain manager instance
end

function SDK:get_asset_manager()
    -- Get asset manager instance
end

function SDK:get_nft_manager()
    -- Get NFT manager instance
end

function SDK:get_security_manager()
    -- Get security manager instance
end

function SDK:get_ipfs_manager()
    -- Get IPFS manager instance
end
```

### Config
Configuration for the SDK.

```lua
local Config = {
    network = "mainnet",      -- "mainnet" or "testnet"
    data_dir = "",           -- Data directory path
    enable_mining = false,   -- Enable mining
    mining_threads = 1,      -- Number of mining threads
    enable_sync = true,      -- Enable blockchain sync
    sync_interval = 1000,    -- Sync interval in milliseconds
    rpc_endpoint = "",       -- RPC endpoint URL
    rpc_username = "",       -- RPC username
    rpc_password = "",       -- RPC password
    timeout = 30000         -- Timeout in milliseconds
}
```

## Blockchain Integration

### BlockchainManager
Manages blockchain operations.

```lua
local BlockchainManager = {
    -- Internal fields
}

function BlockchainManager:get_blockchain_info()
    -- Get blockchain information
end

function BlockchainManager:get_block(hash_or_height)
    -- Get block by hash or height
end

function BlockchainManager:get_transaction(txid)
    -- Get transaction information
end

function BlockchainManager:send_raw_transaction(raw_tx)
    -- Send raw transaction
end

function BlockchainManager:create_raw_transaction(inputs, outputs)
    -- Create raw transaction
end

function BlockchainManager:sign_raw_transaction(raw_tx, private_keys)
    -- Sign raw transaction
end
```

## Asset Management

### AssetManager
Manages asset operations.

```lua
local AssetManager = {
    -- Internal fields
}

function AssetManager:create_asset(metadata)
    -- Create a new asset
end

function AssetManager:get_asset(asset_id)
    -- Get asset information
end

function AssetManager:update_asset(asset_id, metadata)
    -- Update asset metadata
end

function AssetManager:transfer_asset(asset_id, from, to, amount)
    -- Transfer asset
end

function AssetManager:get_balance(address, asset_id)
    -- Get asset balance
end

function AssetManager:get_asset_history(asset_id)
    -- Get asset history
end
```

## NFT Management

### NFTManager
Manages NFT operations.

```lua
local NFTManager = {
    -- Internal fields
}

function NFTManager:create_nft(metadata)
    -- Create a new NFT
end

function NFTManager:get_nft(nft_id)
    -- Get NFT information
end

function NFTManager:transfer_nft(nft_id, from, to)
    -- Transfer NFT
end

function NFTManager:get_nft_owner(nft_id)
    -- Get NFT owner
end

function NFTManager:get_nft_collection(collection_id)
    -- Get NFT collection
end
```

## Security

### SecurityManager
Manages security features.

```lua
local SecurityManager = {
    -- Internal fields
}

function SecurityManager:initialize()
    -- Initialize security manager
end

function SecurityManager:shutdown()
    -- Shutdown security manager
end

function SecurityManager:set_policy(policy)
    -- Set security policy
end

function SecurityManager:get_status()
    -- Get security status
end

function SecurityManager:enable_feature(feature)
    -- Enable security feature
end

function SecurityManager:disable_feature(feature)
    -- Disable security feature
end
```

## Examples

### Creating and Transferring an Asset
```lua
-- Initialize SDK
local sdk = satox.new()
sdk:initialize()

-- Get asset manager
local asset_manager = sdk:get_asset_manager()

-- Create asset metadata
local metadata = {
    name = "My Asset",
    symbol = "MYA",
    total_supply = 1000000,
    decimals = 8,
    reissuable = true
}

-- Create asset
local asset_id = asset_manager:create_asset(metadata)

-- Transfer asset
local from_address = "my_address"
local to_address = "recipient_address"
local amount = 1000
local success = asset_manager:transfer_asset(asset_id, from_address, to_address, amount)
```

### Creating and Managing NFTs
```lua
-- Get NFT manager
local nft_manager = sdk:get_nft_manager()

-- Create NFT metadata
local metadata = {
    name = "My NFT",
    description = "A unique NFT",
    image = "ipfs://image-hash",
    attributes = {}
}

-- Create NFT
local nft_id = nft_manager:create_nft(metadata)

-- Transfer NFT
local from_address = "my_address"
local to_address = "recipient_address"
local success = nft_manager:transfer_nft(nft_id, from_address, to_address)
```

### Blockchain Operations
```lua
-- Get blockchain manager
local blockchain_manager = sdk:get_blockchain_manager()

-- Get blockchain info
local info = blockchain_manager:get_blockchain_info()
print("Current height: " .. info.height)

-- Get block by hash
local block = blockchain_manager:get_block("block_hash")
print("Block timestamp: " .. block.timestamp)

-- Create and send transaction
local inputs = {}
local outputs = {}
-- Add inputs and outputs...

local raw_tx = blockchain_manager:create_raw_transaction(inputs, outputs)
local signed_tx = blockchain_manager:sign_raw_transaction(raw_tx, private_keys)
local tx_id = blockchain_manager:send_raw_transaction(signed_tx)
``` 