local IPFSManager = {}
IPFSManager.__index = IPFSManager

-- Constants
local MAX_FILE_SIZE = 100 * 1024 * 1024  -- 100MB
local SUPPORTED_HASH_ALGORITHMS = {
    "sha2-256",
    "sha2-512",
    "blake2b-256",
    "blake2b-512"
}
local DEFAULT_PIN_DURATION = 30 * 24 * 60 * 60  -- 30 days in seconds

function IPFSManager.new()
    local self = setmetatable({}, IPFSManager)
    self.initialized = false
    self.blockchainManager = nil
    self.pins = {}
    self.cache = {}
    return self
end

function IPFSManager:initialize(blockchainManager)
    if self.initialized then
        return nil, "IPFSManager already initialized"
    end
    if not blockchainManager then
        return nil, "BlockchainManager is required"
    end
    self.blockchainManager = blockchainManager
    self.initialized = true
    return true
end

function IPFSManager:addFile(filePath, options)
    if not self.initialized then
        return nil, "IPFSManager not initialized"
    end
    if not filePath then
        return nil, "File path is required"
    end

    -- Validate file size
    local fileSize = self:_getFileSize(filePath)
    if fileSize > MAX_FILE_SIZE then
        return nil, "File too large"
    end

    -- Read file content
    local content = self:_readFile(filePath)
    if not content then
        return nil, "Failed to read file"
    end

    -- Generate CID
    local cid = self:_generateCID(content, options and options.hashAlgorithm or "sha2-256")
    if not cid then
        return nil, "Failed to generate CID"
    end

    -- Store in cache
    self.cache[cid] = {
        content = content,
        size = fileSize,
        added_at = os.time(),
        pin_duration = options and options.pinDuration or DEFAULT_PIN_DURATION
    }

    -- Pin if requested
    if options and options.pin then
        self:pin(cid, options.pinDuration)
    end

    return {
        cid = cid,
        size = fileSize,
        hash_algorithm = options and options.hashAlgorithm or "sha2-256"
    }
end

function IPFSManager:getFile(cid)
    if not self.initialized then
        return nil, "IPFSManager not initialized"
    end
    if not cid then
        return nil, "CID is required"
    end

    -- Check cache first
    local cached = self.cache[cid]
    if cached then
        return cached.content
    end

    -- Fetch from IPFS network
    local content = self:_fetchFromIPFS(cid)
    if not content then
        return nil, "Failed to fetch file from IPFS"
    end

    -- Cache the content
    self.cache[cid] = {
        content = content,
        size = #content,
        added_at = os.time(),
        pin_duration = DEFAULT_PIN_DURATION
    }

    return content
end

function IPFSManager:pin(cid, duration)
    if not self.initialized then
        return nil, "IPFSManager not initialized"
    end

    if not cid then
        return nil, "CID is required"
    end

    duration = duration or DEFAULT_PIN_DURATION

    -- Check if already pinned
    if self.pins[cid] then
        -- Extend pin duration
        self.pins[cid].expires_at = os.time() + duration
        return self.pins[cid]
    end

    -- Create pin record
    local pin = {
        cid = cid,
        created_at = os.time(),
        expires_at = os.time() + duration
    }

    -- Store pin
    self.pins[cid] = pin

    return pin
end

function IPFSManager:unpin(cid)
    if not self.initialized then
        return nil, "IPFSManager not initialized"
    end

    if not cid then
        return nil, "CID is required"
    end

    if not self.pins[cid] then
        return nil, "CID not pinned"
    end

    -- Remove pin
    self.pins[cid] = nil

    return true
end

function IPFSManager:getPinStatus(cid)
    if not self.initialized then
        return nil, "IPFSManager not initialized"
    end

    if not cid then
        return nil, "CID is required"
    end

    local pin = self.pins[cid]
    if not pin then
        return nil
    end

    return {
        cid = pin.cid,
        created_at = pin.created_at,
        expires_at = pin.expires_at,
        is_expired = os.time() > pin.expires_at
    }
end

function IPFSManager:cleanupExpiredPins()
    if not self.initialized then
        return nil, "IPFSManager not initialized"
    end

    local now = os.time()
    local removed = 0

    for cid, pin in pairs(self.pins) do
        if now > pin.expires_at then
            self.pins[cid] = nil
            removed = removed + 1
        end
    end

    return removed
end

function IPFSManager:_getFileSize(filePath)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use the filesystem API
    return 1024  -- Placeholder size
end

function IPFSManager:_readFile(filePath)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use the filesystem API
    return "file content"  -- Placeholder content
end

function IPFSManager:_generateCID(content, algorithm)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use IPFS's CID generation
    return "Qm" .. algorithm .. "_" .. content:sub(1, 32)  -- Placeholder CID
end

function IPFSManager:_fetchFromIPFS(cid)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use IPFS's API
    return "fetched content"  -- Placeholder content
end

function IPFSManager:_isValidHashAlgorithm(algorithm)
    for _, validAlgorithm in ipairs(SUPPORTED_HASH_ALGORITHMS) do
        if algorithm == validAlgorithm then
            return true
        end
    end
    return false
end

return IPFSManager 