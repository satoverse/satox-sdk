-- NFT management module for Satox SDK

local NFTManager = {}
NFTManager.__index = NFTManager

function NFTManager.new(ipfs, blockchain)
    assert(ipfs and blockchain, "Missing dependencies for NFTManager")
    local self = setmetatable({}, NFTManager)
    self.nfts = {}
    self.ipfs = ipfs
    self.blockchain = blockchain
    self.owners = {}
    self.collections = {}
    self._initialized = false
    return self
end

function NFTManager:initialize()
    if self._initialized then return nil, "NFTManager already initialized" end
    if not self.ipfs then return nil, "IPFS client not set" end
    if not self.blockchain then return nil, "Blockchain client not set" end
    self._initialized = true
    return true
end

-- Create a new NFT
function NFTManager:create_nft(nft_data)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    if self.nfts[nft_data.id] then
        return nil, "NFT already exists"
    end

    -- Validate required fields
    if not nft_data.name or not nft_data.description or not nft_data.owner then
        return nil, "Missing required fields"
    end

    -- Set timestamps
    nft_data.created_at = os.time()
    nft_data.updated_at = os.time()

    -- Store metadata in IPFS
    local metadata_cid, err = self.ipfs:store(nft_data.description)
    if err then
        return nil, "Failed to store metadata in IPFS: " .. err
    end
    nft_data.metadata.ipfs_cid = metadata_cid

    -- Create NFT on blockchain
    local success, err = self.blockchain:create_nft(nft_data)
    if not success then
        return nil, "Failed to create NFT on blockchain: " .. err
    end

    -- Store NFT
    self.nfts[nft_data.id] = nft_data

    -- Update indices
    self.owners[nft_data.owner] = self.owners[nft_data.owner] or {}
    table.insert(self.owners[nft_data.owner], nft_data.id)

    if nft_data.collection then
        self.collections[nft_data.collection] = self.collections[nft_data.collection] or {}
        table.insert(self.collections[nft_data.collection], nft_data.id)
    end

    return nft_data
end

-- Get an NFT by ID
function NFTManager:get_nft(id)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    local nft = self.nfts[id]
    if not nft then
        return nil, "NFT not found"
    end
    return nft
end

-- Update an existing NFT
function NFTManager:update_nft(nft_data)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end
    if not self.nfts[nft_data.id] then
        return nil, "NFT not found"
    end
    nft_data.updated_at = os.time()
    if nft_data.description ~= self.nfts[nft_data.id].description then
        local metadata_cid, err1 = self.ipfs:store(nft_data.description)
        if err1 then
            return nil, "Failed to store metadata in IPFS: " .. err1
        end
        nft_data.metadata.ipfs_cid = metadata_cid
    end
    local success, err2 = self.blockchain:update_nft(nft_data)
    if not success then
        return nil, "Failed to update NFT on blockchain: " .. err2
    end
    -- Update indices if owner or collection changed
    if nft_data.owner ~= self.nfts[nft_data.id].owner then
        -- Remove from old owner
        local old_owner = self.nfts[nft_data.id].owner
        for i, nft_id in ipairs(self.owners[old_owner]) do
            if nft_id == nft_data.id then
                table.remove(self.owners[old_owner], i)
                break
            end
        end
        -- Add to new owner
        self.owners[nft_data.owner] = self.owners[nft_data.owner] or {}
        table.insert(self.owners[nft_data.owner], nft_data.id)
    end

    if nft_data.collection ~= self.nfts[nft_data.id].collection then
        -- Remove from old collection
        local old_collection = self.nfts[nft_data.id].collection
        if old_collection then
            for i, nft_id in ipairs(self.collections[old_collection]) do
                if nft_id == nft_data.id then
                    table.remove(self.collections[old_collection], i)
                    break
                end
            end
        end
        -- Add to new collection
        if nft_data.collection then
            self.collections[nft_data.collection] = self.collections[nft_data.collection] or {}
            table.insert(self.collections[nft_data.collection], nft_data.id)
        end
    end

    self.nfts[nft_data.id] = nft_data
    return nft_data
end

-- Transfer an NFT between addresses
function NFTManager:transfer_nft(id, from, to)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    local nft = self.nfts[id]
    if not nft then
        return nil, "NFT not found"
    end

    if nft.owner ~= from then
        return nil, "Current owner mismatch"
    end

    if not nft.is_transferable then
        return nil, "NFT is not transferable"
    end

    local success, err = self.blockchain:transfer_nft(id, from, to)
    if not success then
        return nil, "Failed to transfer NFT on blockchain: " .. err
    end

    -- Update NFT owner
    nft.owner = to
    nft.updated_at = os.time()

    -- Update indices
    for i, nft_id in ipairs(self.owners[from]) do
        if nft_id == id then
            table.remove(self.owners[from], i)
            break
        end
    end
    self.owners[to] = self.owners[to] or {}
    table.insert(self.owners[to], id)

    return nft
end

-- Burn an NFT
function NFTManager:burn_nft(id, owner)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    local nft = self.nfts[id]
    if not nft then
        return nil, "NFT not found"
    end

    if nft.owner ~= owner then
        return nil, "Current owner mismatch"
    end

    if not nft.is_burnable then
        return nil, "NFT is not burnable"
    end

    local success, err = self.blockchain:burn_nft(id, owner)
    if not success then
        return nil, "Failed to burn NFT on blockchain: " .. err
    end

    -- Remove from indices
    for i, nft_id in ipairs(self.owners[owner]) do
        if nft_id == id then
            table.remove(self.owners[owner], i)
            break
        end
    end

    if nft.collection then
        for i, nft_id in ipairs(self.collections[nft.collection]) do
            if nft_id == id then
                table.remove(self.collections[nft.collection], i)
                break
            end
        end
    end

    -- Remove NFT
    self.nfts[id] = nil

    return true
end

-- Update NFT metadata
function NFTManager:update_metadata(id, metadata)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    local nft = self.nfts[id]
    if not nft then
        return nil, "NFT not found"
    end

    nft.metadata = metadata
    nft.updated_at = os.time()
    return self:update_nft(nft)
end

-- Get NFTs by collection
function NFTManager:get_nfts_by_collection(collection)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end
    local collection_nfts = {}
    for _, nft_id in ipairs(self.collections[collection] or {}) do
        table.insert(collection_nfts, self.nfts[nft_id])
    end
    return collection_nfts
end

-- Get NFTs by owner
function NFTManager:get_nfts_by_owner(owner)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end
    local owner_nfts = {}
    for _, nft_id in ipairs(self.owners[owner] or {}) do
        table.insert(owner_nfts, self.nfts[nft_id])
    end
    return owner_nfts
end

-- Search NFTs by properties
function NFTManager:search_nfts(query)
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    local results = {}
    for id, nft in pairs(self.nfts) do
        local match = true
        for key, value in pairs(query) do
            if nft[key] ~= value then
                match = false
                break
            end
        end
        if match then
            table.insert(results, nft)
        end
    end
    return results
end

-- Get NFT statistics
function NFTManager:get_statistics()
    if not self._initialized then
        return nil, "NFTManager not initialized"
    end

    local stats = {
        total_nfts = 0,
        total_owners = 0,
        total_collections = 0,
        recent_activity = {}
    }

    -- Count NFTs
    for _ in pairs(self.nfts) do
        stats.total_nfts = stats.total_nfts + 1
    end

    -- Count owners
    for _ in pairs(self.owners) do
        stats.total_owners = stats.total_owners + 1
    end

    -- Count collections
    for _ in pairs(self.collections) do
        stats.total_collections = stats.total_collections + 1
    end

    -- Get recent activity (last 10 updates)
    local recent = {}
    for _, nft in pairs(self.nfts) do
        table.insert(recent, {
            id = nft.id,
            updated_at = nft.updated_at
        })
    end
    table.sort(recent, function(a, b) return a.updated_at > b.updated_at end)
    for i = 1, math.min(10, #recent) do
        table.insert(stats.recent_activity, recent[i])
    end

    return stats
end

-- Alias for industry standard naming
function NFTManager:createNFT(nftData)
    return self:create_nft(nftData)
end

function NFTManager:getNFT(_)
    -- Implementation
end

function NFTManager:listNFTs()
    -- Implementation
end

return { new = NFTManager.new } 