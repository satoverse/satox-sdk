-- Asset management module for Satox SDK

local AssetManager = {}
AssetManager.__index = AssetManager

function AssetManager.new(ipfs, blockchain)
    assert(ipfs and blockchain, "Missing dependencies for AssetManager")
    local self = setmetatable({}, AssetManager)
    self.assets = {}
    self.ipfs = ipfs
    self.blockchain = blockchain
    return self
end

function AssetManager:create_asset(asset_data)
    if self.assets[asset_data.id] then
        return nil, "Asset already exists"
    end
    local metadata_cid, err = self.ipfs:store(asset_data.description)
    if err then return nil, "Failed to store metadata in IPFS: " .. err end
    asset_data.metadata = asset_data.metadata or {}
    asset_data.metadata.ipfs_cid = metadata_cid
    local success, err2 = self.blockchain:create_asset(asset_data)
    if not success then return nil, "Failed to create asset on blockchain: " .. (err2 or "") end
    self.assets[asset_data.id] = asset_data
    return asset_data
end

function AssetManager:get_asset(id)
    local asset = self.assets[id]
    if not asset then return nil, "Asset not found" end
    return asset
end

function AssetManager:update_asset(asset_data)
    if not self.assets[asset_data.id] then return nil, "Asset not found" end
    local success, err = self.blockchain:update_asset(asset_data)
    if not success then return nil, "Failed to update asset on blockchain: " .. (err or "") end
    self.assets[asset_data.id] = asset_data
    return asset_data
end

function AssetManager:transfer_asset(from, to, amount)
    return self.blockchain:transfer_asset(from, to, amount)
end

function AssetManager:reissue_asset(id, amount)
    local asset = self.assets[id]
    if not asset then return nil, "Asset not found" end
    if not asset.reissuable then return nil, "Asset is not reissuable" end
    asset.supply = asset.supply + amount
    return self:update_asset(asset)
end

function AssetManager:update_metadata(id, metadata)
    local asset = self.assets[id]
    if not asset then return nil, "Asset not found" end
    asset.metadata = metadata
    return self:update_asset(asset)
end

function AssetManager:bulk_transfer(transfers)
    for _, transfer in ipairs(transfers) do
        local success, err = self:transfer_asset(transfer.from, transfer.to, transfer.amount)
        if not success then return nil, "Failed to transfer asset: " .. (err or "") end
    end
    return true
end

return { new = AssetManager.new } 