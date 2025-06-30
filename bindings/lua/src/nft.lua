local NFTManager = {}
NFTManager.__index = NFTManager

-- Constants
local MAX_NFT_NAME_LENGTH = 128
local MAX_NFT_DESCRIPTION_LENGTH = 1024
local MAX_NFT_METADATA_SIZE = 4096
local SUPPORTED_NFT_STANDARDS = {
    "ERC721",
    "ERC1155",
    "SATOX721",
    "SATOX1155"
}

local updates = {}

function NFTManager.new()
    local self = setmetatable({}, NFTManager)
    self.initialized = false
    self.blockchainManager = nil
    self.assetManager = nil
    self.nfts = {}
    return self
end

function NFTManager:initialize(blockchainManager, assetManager)
    if self.initialized then
        return nil, "NFTManager already initialized"
    end
    if not blockchainManager then
        return nil, "BlockchainManager is required"
    end
    if not assetManager then
        return nil, "AssetManager is required"
    end
    self.blockchainManager = blockchainManager
    self.assetManager = assetManager
    self.initialized = true
    return true
end

function NFTManager:createNFT(name, description, metadata, standard, owner)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end
    if not name then
        return nil, "Invalid NFT name"
    end
    if description and #description > 256 then
        return nil, "Description too long"
    end
    if metadata and #metadata > 4096 then
        return nil, "Metadata too large"
    end
    if not standard then
        return nil, "Invalid NFT standard"
    end
    if not owner then
        return nil, "Owner is required"
    end

    -- Validate parameters
    if not name or #name > MAX_NFT_NAME_LENGTH then
        return nil, "Invalid NFT name"
    end

    if description and #description > MAX_NFT_DESCRIPTION_LENGTH then
        return nil, "Description too long"
    end

    if metadata and #metadata > MAX_NFT_METADATA_SIZE then
        return nil, "Metadata too large"
    end

    if not self:_isValidStandard(standard) then
        return nil, "Invalid NFT standard"
    end

    if not owner then
        return nil, "Owner is required"
    end

    -- Generate NFT ID
    local nftId = self:_generateNFTId(name, owner)

    -- Create NFT
    local nft = {
        id = nftId,
        name = name,
        description = description or "",
        metadata = metadata or {},
        standard = standard,
        owner = owner,
        created_at = os.time(),
        version = 1,
        transfers = {},
        royalties = {}
    }

    -- Store NFT
    self.nfts[nftId] = nft

    return nft
end

function NFTManager:getNFT(id)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end
    if not id then
        return nil, "NFT ID is required"
    end

    return self.nfts[id]
end

function NFTManager:updateNFT(id, name, description, metadata)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end
    if not id then
        return nil, "NFT ID is required"
    end

    local nft = self.nfts[id]
    if not nft then
        return nil, "NFT not found"
    end

    -- Validate updates
    if name and #name > MAX_NFT_NAME_LENGTH then
        return nil, "Invalid NFT name"
    end

    if description and #description > MAX_NFT_DESCRIPTION_LENGTH then
        return nil, "Description too long"
    end

    if metadata and #metadata > MAX_NFT_METADATA_SIZE then
        return nil, "Metadata too large"
    end

    -- Apply updates
    for key, value in pairs(updates) do
        if key ~= "id" and key ~= "owner" and key ~= "created_at" and 
           key ~= "version" and key ~= "transfers" and key ~= "royalties" then
            nft[key] = value
        end
    end

    nft.version = nft.version + 1

    return nft
end

function NFTManager:transferNFT(id, from, to, amount)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end
    if not id or not from or not to then
        return nil, "NFT ID, from, and to addresses are required"
    end

    local nft = self.nfts[id]
    if not nft then
        return nil, "NFT not found"
    end

    if nft.owner ~= from then
        return nil, "Invalid owner"
    end

    amount = amount or 1
    if amount < 1 then
        return nil, "Invalid amount"
    end

    -- Create transfer record
    local transfer = {
        from = from,
        to = to,
        amount = amount,
        timestamp = os.time()
    }

    -- Update NFT
    nft.owner = to
    table.insert(nft.transfers, transfer)
    nft.version = nft.version + 1

    return transfer
end

function NFTManager:setRoyalties(nftId, royalties)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end

    if not nftId then
        return nil, "NFT ID is required"
    end

    local nft = self.nfts[nftId]
    if not nft then
        return nil, "NFT not found"
    end

    -- Validate royalties
    if not royalties or type(royalties) ~= "table" then
        return nil, "Invalid royalties"
    end

    local totalPercentage = 0
    for _, royalty in ipairs(royalties) do
        if not royalty.address or not royalty.percentage then
            return nil, "Invalid royalty format"
        end
        if royalty.percentage < 0 or royalty.percentage > 100 then
            return nil, "Invalid royalty percentage"
        end
        totalPercentage = totalPercentage + royalty.percentage
    end

    if totalPercentage > 100 then
        return nil, "Total royalty percentage cannot exceed 100%"
    end

    -- Update royalties
    nft.royalties = royalties
    nft.version = nft.version + 1

    return nft
end

function NFTManager:getNFTTransfers(nftId)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end

    if not nftId then
        return nil, "NFT ID is required"
    end

    local nft = self.nfts[nftId]
    if not nft then
        return nil, "NFT not found"
    end

    return nft.transfers
end

function NFTManager:getNFTRoyalties(nftId)
    if not self.initialized then
        return nil, "NFTManager not initialized"
    end

    if not nftId then
        return nil, "NFT ID is required"
    end

    local nft = self.nfts[nftId]
    if not nft then
        return nil, "NFT not found"
    end

    return nft.royalties
end

function NFTManager:_generateNFTId(name, owner)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use a proper hashing algorithm
    return "nft_" .. name .. "_" .. owner .. "_" .. os.time()
end

function NFTManager:_isValidStandard(standard)
    for _, validStandard in ipairs(SUPPORTED_NFT_STANDARDS) do
        if standard == validStandard then
            return true
        end
    end
    return false
end

return NFTManager 