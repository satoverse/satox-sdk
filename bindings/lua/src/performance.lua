local PerformanceManager = {}
PerformanceManager.__index = PerformanceManager

-- Constants
local DEFAULT_CACHE_SIZE = 1000
local DEFAULT_CACHE_TTL = 300  -- 5 minutes in seconds
local MAX_CACHE_SIZE = 10000
local MIN_CACHE_SIZE = 100
local SUPPORTED_OPTIMIZATION_STRATEGIES = {
    "memory",
    "cpu",
    "io",
    "network"
}

function PerformanceManager.new()
    local self = setmetatable({}, PerformanceManager)
    self.initialized = false
    self.blockchainManager = nil
    self.cache = {}
    self.metrics = {
        memory = {
            allocated = 0,
            peak = 0,
            current = 0
        },
        cpu = {
            usage = 0,
            peak = 0
        },
        io = {
            reads = 0,
            writes = 0,
            bytes_read = 0,
            bytes_written = 0
        },
        network = {
            requests = 0,
            bytes_sent = 0,
            bytes_received = 0
        }
    }
    return self
end

function PerformanceManager:initialize(blockchainManager, options)
    if self.initialized then
        error("PerformanceManager already initialized")
    end

    if not blockchainManager then
        error("BlockchainManager is required")
    end

    options = options or {}
    self.blockchainManager = blockchainManager
    self.cacheSize = options.cacheSize or DEFAULT_CACHE_SIZE
    self.cacheTTL = options.cacheTTL or DEFAULT_CACHE_TTL

    if self.cacheSize < MIN_CACHE_SIZE or self.cacheSize > MAX_CACHE_SIZE then
        error("Invalid cache size")
    end

    self.initialized = true
end

function PerformanceManager:setCacheItem(key, value, ttl)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not key then
        error("Cache key is required")
    end

    if not value then
        error("Cache value is required")
    end

    ttl = ttl or self.cacheTTL

    -- Check cache size
    if #self.cache >= self.cacheSize then
        self:_evictOldestItem()
    end

    -- Store item
    self.cache[key] = {
        value = value,
        created_at = os.time(),
        expires_at = os.time() + ttl,
        last_access = os.time()
    }

    return true
end

function PerformanceManager:getCacheItem(key)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not key then
        error("Cache key is required")
    end

    local item = self.cache[key]
    if not item then
        return nil
    end

    -- Check expiration
    if os.time() > item.expires_at then
        self.cache[key] = nil
        return nil
    end

    -- Update last access
    item.last_access = os.time()

    return item.value
end

function PerformanceManager:removeCacheItem(key)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not key then
        error("Cache key is required")
    end

    self.cache[key] = nil
    return true
end

function PerformanceManager:clearCache()
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    self.cache = {}
    return true
end

function PerformanceManager:getMetrics(strategy)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if strategy and not self:_isValidStrategy(strategy) then
        error("Unsupported optimization strategy")
    end

    if strategy then
        return self.metrics[strategy]
    end

    return self.metrics
end

function PerformanceManager:optimize(strategy)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not strategy then
        error("Optimization strategy is required")
    end

    if not self:_isValidStrategy(strategy) then
        error("Unsupported optimization strategy")
    end

    if strategy == "memory" then
        return self:_optimizeMemory()
    elseif strategy == "cpu" then
        return self:_optimizeCPU()
    elseif strategy == "io" then
        return self:_optimizeIO()
    elseif strategy == "network" then
        return self:_optimizeNetwork()
    end
end

function PerformanceManager:trackMemoryAllocation(size)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not size then
        error("Allocation size is required")
    end

    self.metrics.memory.allocated = self.metrics.memory.allocated + size
    self.metrics.memory.current = self.metrics.memory.current + size
    self.metrics.memory.peak = math.max(self.metrics.memory.peak, self.metrics.memory.current)

    return true
end

function PerformanceManager:trackMemoryDeallocation(size)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not size then
        error("Deallocation size is required")
    end

    self.metrics.memory.current = self.metrics.memory.current - size

    return true
end

function PerformanceManager:trackIOOperation(operation, bytes)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not operation then
        error("IO operation is required")
    end

    if not bytes then
        error("Bytes count is required")
    end

    if operation == "read" then
        self.metrics.io.reads = self.metrics.io.reads + 1
        self.metrics.io.bytes_read = self.metrics.io.bytes_read + bytes
    elseif operation == "write" then
        self.metrics.io.writes = self.metrics.io.writes + 1
        self.metrics.io.bytes_written = self.metrics.io.bytes_written + bytes
    else
        error("Invalid IO operation")
    end

    return true
end

function PerformanceManager:trackNetworkOperation(operation, bytes)
    if not self.initialized then
        error("PerformanceManager not initialized")
    end

    if not operation then
        error("Network operation is required")
    end

    if not bytes then
        error("Bytes count is required")
    end

    if operation == "send" then
        self.metrics.network.requests = self.metrics.network.requests + 1
        self.metrics.network.bytes_sent = self.metrics.network.bytes_sent + bytes
    elseif operation == "receive" then
        self.metrics.network.requests = self.metrics.network.requests + 1
        self.metrics.network.bytes_received = self.metrics.network.bytes_received + bytes
    else
        error("Invalid network operation")
    end

    return true
end

-- Private methods
function PerformanceManager:_isValidStrategy(strategy)
    for _, validStrategy in ipairs(SUPPORTED_OPTIMIZATION_STRATEGIES) do
        if strategy == validStrategy then
            return true
        end
    end
    return false
end

function PerformanceManager:_evictOldestItem()
    local oldestKey = nil
    local oldestTime = math.huge

    for key, item in pairs(self.cache) do
        if item.last_access < oldestTime then
            oldestKey = key
            oldestTime = item.last_access
        end
    end

    if oldestKey then
        self.cache[oldestKey] = nil
    end
end

function PerformanceManager:_optimizeMemory()
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would optimize memory usage
    collectgarbage("collect")
    return true
end

function PerformanceManager:_optimizeCPU()
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would optimize CPU usage
    return true
end

function PerformanceManager:_optimizeIO()
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would optimize I/O operations
    return true
end

function PerformanceManager:_optimizeNetwork()
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would optimize network operations
    return true
end

return PerformanceManager 