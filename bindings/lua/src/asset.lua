local AssetManager = {}
AssetManager.__index = AssetManager

-- Constants
local MAX_ASSET_NAME_LENGTH = 64
local MAX_ASSET_DESCRIPTION_LENGTH = 1024
local MAX_ASSET_METADATA_SIZE = 1024 * 1024  -- 1MB
local SUPPORTED_ASSET_TYPES = {
    "fungible",
    "non_fungible",
    "semi_fungible"
}

function AssetManager.new()
    local self = setmetatable({}, AssetManager)
    self.initialized = false
    self.blockchainManager = nil
    self.quantumManager = nil
    self.assets = {}
    return self
end

function AssetManager:initialize(blockchainManager, quantumManager)
    if self.initialized then
        error("AssetManager already initialized")
    end

    if not blockchainManager then
        error("BlockchainManager is required")
    end

    if not quantumManager then
        error("QuantumManager is required")
    end

    self.blockchainManager = blockchainManager
    self.quantumManager = quantumManager
    self.initialized = true
end

function AssetManager:createAsset(name, type, description, metadata, owner)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    -- Validate parameters
    if not name or #name > MAX_ASSET_NAME_LENGTH then
        error("Invalid asset name")
    end

    if not type or not self:_isValidAssetType(type) then
        error("Invalid asset type")
    end

    if description and #description > MAX_ASSET_DESCRIPTION_LENGTH then
        error("Description too long")
    end

    if metadata and #metadata > MAX_ASSET_METADATA_SIZE then
        error("Metadata too large")
    end

    if not owner then
        error("Owner address is required")
    end

    -- Generate asset ID
    local assetId = self:_generateAssetId(name, owner)

    -- Create asset
    local asset = {
        id = assetId,
        name = name,
        type = type,
        description = description or "",
        metadata = metadata or {},
        owner = owner,
        created_at = os.time(),
        supply = 0,
        decimals = 18,
        version = 1
    }

    -- Store asset
    self.assets[assetId] = asset

    return asset
end

function AssetManager:getAsset(assetId)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    if not assetId then
        error("Asset ID is required")
    end

    return self.assets[assetId]
end

function AssetManager:updateAsset(assetId, updates)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    if not assetId then
        error("Asset ID is required")
    end

    local asset = self.assets[assetId]
    if not asset then
        error("Asset not found")
    end

    -- Validate updates
    if updates.name and #updates.name > MAX_ASSET_NAME_LENGTH then
        error("Invalid asset name")
    end

    if updates.description and #updates.description > MAX_ASSET_DESCRIPTION_LENGTH then
        error("Description too long")
    end

    if updates.metadata and #updates.metadata > MAX_ASSET_METADATA_SIZE then
        error("Metadata too large")
    end

    -- Apply updates
    for key, value in pairs(updates) do
        if key ~= "id" and key ~= "created_at" and key ~= "version" then
            asset[key] = value
        end
    end

    asset.version = asset.version + 1

    return asset
end

function AssetManager:mintAsset(assetId, amount, recipient)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    if not assetId then
        error("Asset ID is required")
    end

    if not amount or amount <= 0 then
        error("Invalid amount")
    end

    if not recipient then
        error("Recipient address is required")
    end

    local asset = self.assets[assetId]
    if not asset then
        error("Asset not found")
    end

    -- Update supply
    asset.supply = asset.supply + amount

    -- Create mint transaction
    local transaction = {
        type = "mint",
        asset_id = assetId,
        amount = amount,
        recipient = recipient,
        timestamp = os.time()
    }

    -- Submit transaction to blockchain
    return self.blockchainManager:addTransaction(transaction)
end

function AssetManager:burnAsset(assetId, amount, owner)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    if not assetId then
        error("Asset ID is required")
    end

    if not amount or amount <= 0 then
        error("Invalid amount")
    end

    if not owner then
        error("Owner address is required")
    end

    local asset = self.assets[assetId]
    if not asset then
        error("Asset not found")
    end

    if asset.owner ~= owner then
        error("Unauthorized")
    end

    if asset.supply < amount then
        error("Insufficient supply")
    end

    -- Update supply
    asset.supply = asset.supply - amount

    -- Create burn transaction
    local transaction = {
        type = "burn",
        asset_id = assetId,
        amount = amount,
        owner = owner,
        timestamp = os.time()
    }

    -- Submit transaction to blockchain
    return self.blockchainManager:addTransaction(transaction)
end

function AssetManager:transferAsset(assetId, amount, from, to)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    if not assetId then
        error("Asset ID is required")
    end

    if not amount or amount <= 0 then
        error("Invalid amount")
    end

    if not from or not to then
        error("From and to addresses are required")
    end

    local asset = self.assets[assetId]
    if not asset then
        error("Asset not found")
    end

    -- Create transfer transaction
    local transaction = {
        type = "transfer",
        asset_id = assetId,
        amount = amount,
        from = from,
        to = to,
        timestamp = os.time()
    }

    -- Submit transaction to blockchain
    return self.blockchainManager:addTransaction(transaction)
end

function AssetManager:getAssetBalance(assetId, address)
    if not self.initialized then
        error("AssetManager not initialized")
    end

    if not assetId then
        error("Asset ID is required")
    end

    if not address then
        error("Address is required")
    end

    local asset = self.assets[assetId]
    if not asset then
        error("Asset not found")
    end

    -- Get balance from blockchain
    return self.blockchainManager:getBalance(assetId, address)
end

function AssetManager:_generateAssetId(name, owner)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use a proper hashing algorithm
    return "asset_" .. name .. "_" .. owner .. "_" .. os.time()
end

function AssetManager:_isValidAssetType(type)
    for _, validType in ipairs(SUPPORTED_ASSET_TYPES) do
        if type == validType then
            return true
        end
    end
    return false
end

return AssetManager 