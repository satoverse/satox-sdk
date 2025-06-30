-- Thread-Safe Manager for Lua Bindings
-- Provides comprehensive thread safety using Lua's concurrency primitives

local ffi = require("ffi")
local bit = require("bit")

-- Load threading library (LuaJIT or compatible)
local thread = require("thread") or require("lanes") or require("copas")

-- Thread state enumeration
local ThreadState = {
    UNINITIALIZED = "uninitialized",
    INITIALIZING = "initializing", 
    INITIALIZED = "initialized",
    SHUTTING_DOWN = "shutting_down",
    SHUTDOWN = "shutdown",
    ERROR = "error"
}

-- Thread safety statistics
local ThreadSafeStats = {}
ThreadSafeStats.__index = ThreadSafeStats

function ThreadSafeStats.new()
    local self = setmetatable({}, ThreadSafeStats)
    self.total_operations = 0
    self.concurrent_operations = 0
    self.max_concurrent_operations = 0
    self.average_operation_time = 0.0
    self.total_operation_time = 0.0
    self.lock_contention_count = 0
    self.last_operation_time = 0.0
    return self
end

-- Thread-safe manager base class
local ThreadSafeManager = {}
ThreadSafeManager.__index = ThreadSafeManager

function ThreadSafeManager.new(name)
    name = name or "ThreadSafeManager"
    local self = setmetatable({}, ThreadSafeManager)
    
    self.name = name
    self.state = ThreadState.UNINITIALIZED
    self.initialized = false
    self.disposed = false
    self.last_error = ""
    self.error_count = 0
    
    -- Thread safety primitives
    self.state_lock = thread.newlock()
    self.error_lock = thread.newlock()
    self.stats_lock = thread.newlock()
    self.operation_lock = thread.newlock()
    self.event_lock = thread.newlock()
    self.resource_lock = thread.newlock()
    self.config_lock = thread.newlock()
    self.cache_lock = thread.newlock()
    
    -- Statistics
    self.stats = ThreadSafeStats.new()
    self.operation_start_times = {}
    self.active_operations = 0
    self.operation_counter = 0
    
    -- Event handling
    self.event_handlers = {}
    self.event_stats = {}
    
    -- Resource management
    self.resources = {}
    
    -- Configuration
    self.config = {}
    
    -- Cache
    self.cache = {}
    self.cache_timestamps = {}
    
    -- Performance monitoring
    self.performance_enabled = true
    self.start_time = os.time()
    
    return self
end

function ThreadSafeManager:ensure_initialized()
    self.state_lock:lock()
    if self.state == ThreadState.UNINITIALIZED then
        self:set_error("Manager not initialized")
        self.state_lock:unlock()
        return false
    elseif self.state == ThreadState.SHUTDOWN then
        self:set_error("Manager has been disposed")
        self.state_lock:unlock()
        return false
    elseif self.state == ThreadState.ERROR then
        self:set_error("Manager is in error state")
        self.state_lock:unlock()
        return false
    end
    self.state_lock:unlock()
    return true
end

function ThreadSafeManager:ensure_not_disposed()
    self.state_lock:lock()
    if self.state == ThreadState.SHUTDOWN then
        self:set_error("Manager has been disposed")
        self.state_lock:unlock()
        return false
    end
    self.state_lock:unlock()
    return true
end

function ThreadSafeManager:set_state(state)
    self.state_lock:lock()
    self.state = state
    self.state_lock:unlock()
end

function ThreadSafeManager:get_state()
    self.state_lock:lock()
    local state = self.state
    self.state_lock:unlock()
    return state
end

function ThreadSafeManager:wait_for_state(target_state, timeout)
    timeout = timeout or 5.0
    local start_time = os.time()
    
    while os.time() - start_time < timeout do
        if self:get_state() == target_state then
            return true
        end
        thread.sleep(0.01)
    end
    return false
end

function ThreadSafeManager:begin_operation()
    if not self.performance_enabled then
        return 0
    end
    
    self.stats_lock:lock()
    local operation_id = self.operation_counter
    self.operation_counter = self.operation_counter + 1
    self.active_operations = self.active_operations + 1
    self.operation_start_times[operation_id] = os.time()
    self.stats.concurrent_operations = self.active_operations
    self.stats.max_concurrent_operations = math.max(self.stats.max_concurrent_operations, self.active_operations)
    self.stats_lock:unlock()
    
    return operation_id
end

function ThreadSafeManager:end_operation(operation_id)
    if not self.performance_enabled or operation_id == 0 then
        return
    end
    
    self.stats_lock:lock()
    if self.operation_start_times[operation_id] then
        local operation_time = os.time() - self.operation_start_times[operation_id]
        self.operation_start_times[operation_id] = nil
        
        self.active_operations = self.active_operations - 1
        self.stats.total_operations = self.stats.total_operations + 1
        self.stats.total_operation_time = self.stats.total_operation_time + operation_time
        self.stats.average_operation_time = self.stats.total_operation_time / self.stats.total_operations
        self.stats.last_operation_time = os.time() - self.start_time
        self.stats.concurrent_operations = self.active_operations
    end
    self.stats_lock:unlock()
end

function ThreadSafeManager:set_error(error)
    self.error_lock:lock()
    self.last_error = error
    self.error_count = self.error_count + 1
    self:set_state(ThreadState.ERROR)
    self.error_lock:unlock()
end

function ThreadSafeManager:clear_error()
    self.error_lock:lock()
    self.last_error = ""
    if self.state == ThreadState.ERROR then
        self:set_state(ThreadState.INITIALIZED)
    end
    self.error_lock:unlock()
end

function ThreadSafeManager:get_last_error()
    self.error_lock:lock()
    local error = self.last_error
    self.error_lock:unlock()
    return error
end

function ThreadSafeManager:get_error_count()
    self.error_lock:lock()
    local count = self.error_count
    self.error_lock:unlock()
    return count
end

function ThreadSafeManager:is_initialized()
    return self:get_state() == ThreadState.INITIALIZED
end

function ThreadSafeManager:is_disposed()
    return self:get_state() == ThreadState.SHUTDOWN
end

function ThreadSafeManager:get_stats()
    self.stats_lock:lock()
    local stats = {
        total_operations = self.stats.total_operations,
        concurrent_operations = self.stats.concurrent_operations,
        max_concurrent_operations = self.stats.max_concurrent_operations,
        average_operation_time = self.stats.average_operation_time,
        total_operation_time = self.stats.total_operation_time,
        lock_contention_count = self.stats.lock_contention_count,
        last_operation_time = self.stats.last_operation_time
    }
    self.stats_lock:unlock()
    return stats
end

function ThreadSafeManager:reset_stats()
    self.stats_lock:lock()
    self.stats = ThreadSafeStats.new()
    self.operation_start_times = {}
    self.active_operations = 0
    self.operation_counter = 0
    self.stats_lock:unlock()
end

function ThreadSafeManager:enable_performance_monitoring()
    self.performance_enabled = true
end

function ThreadSafeManager:disable_performance_monitoring()
    self.performance_enabled = false
end

function ThreadSafeManager:get_uptime()
    return os.time() - self.start_time
end

function ThreadSafeManager:set_config(key, value)
    self.config_lock:lock()
    self.config[key] = value
    self.config_lock:unlock()
end

function ThreadSafeManager:get_config(key, default)
    self.config_lock:lock()
    local value = self.config[key] or default
    self.config_lock:unlock()
    return value
end

function ThreadSafeManager:set_cache(key, value, ttl)
    ttl = ttl or 300.0
    self.cache_lock:lock()
    self.cache[key] = value
    self.cache_timestamps[key] = os.time() + ttl
    self.cache_lock:unlock()
end

function ThreadSafeManager:get_cache(key, default)
    self.cache_lock:lock()
    if self.cache[key] then
        if os.time() < self.cache_timestamps[key] then
            local value = self.cache[key]
            self.cache_lock:unlock()
            return value
        else
            -- Expired, remove from cache
            self.cache[key] = nil
            self.cache_timestamps[key] = nil
        end
    end
    self.cache_lock:unlock()
    return default
end

function ThreadSafeManager:clear_cache()
    self.cache_lock:lock()
    self.cache = {}
    self.cache_timestamps = {}
    self.cache_lock:unlock()
end

function ThreadSafeManager:add_event_handler(event, handler)
    self.event_lock:lock()
    if not self.event_handlers[event] then
        self.event_handlers[event] = {}
    end
    table.insert(self.event_handlers[event], handler)
    self.event_lock:unlock()
end

function ThreadSafeManager:remove_event_handler(event, handler_index)
    self.event_lock:lock()
    if self.event_handlers[event] and self.event_handlers[event][handler_index] then
        table.remove(self.event_handlers[event], handler_index)
    end
    self.event_lock:unlock()
end

function ThreadSafeManager:emit_event(event, ...)
    local handlers = {}
    self.event_lock:lock()
    if self.event_handlers[event] then
        handlers = self.event_handlers[event]
        self.event_stats[event] = (self.event_stats[event] or 0) + 1
    end
    self.event_lock:unlock()
    
    -- Call handlers outside of lock to prevent deadlocks
    for _, handler in ipairs(handlers) do
        local ok, err = pcall(handler, ...)
        if not ok then
            self:set_error("Error in event handler: " .. tostring(err))
        end
    end
end

function ThreadSafeManager:add_resource(name, resource)
    self.resource_lock:lock()
    self.resources[name] = resource
    self.resource_lock:unlock()
end

function ThreadSafeManager:get_resource(name)
    self.resource_lock:lock()
    local resource = self.resources[name]
    self.resource_lock:unlock()
    return resource
end

function ThreadSafeManager:remove_resource(name)
    self.resource_lock:lock()
    self.resources[name] = nil
    self.resource_lock:unlock()
end

function ThreadSafeManager:clear_resources()
    self.resource_lock:lock()
    self.resources = {}
    self.resource_lock:unlock()
end

function ThreadSafeManager:dispose()
    local current_state = self:get_state()
    if current_state == ThreadState.SHUTDOWN then
        return
    end
    
    self:set_state(ThreadState.SHUTTING_DOWN)
    
    -- Clear cache and resources
    self:clear_cache()
    self:clear_resources()
    
    -- Call implementation-specific disposal
    self:dispose_impl()
    
    self:set_state(ThreadState.SHUTDOWN)
end

function ThreadSafeManager:dispose_impl()
    -- Override in subclasses
end

-- Thread-safe singleton pattern
local ThreadSafeSingleton = {}
ThreadSafeSingleton.__index = ThreadSafeSingleton

ThreadSafeSingleton._instances = {}
ThreadSafeSingleton._instance_locks = {}
ThreadSafeSingleton._singleton_lock = thread.newlock()

function ThreadSafeSingleton.new(name)
    name = name or "ThreadSafeSingleton"
    
    ThreadSafeSingleton._singleton_lock:lock()
    if not ThreadSafeSingleton._instances[name] then
        ThreadSafeSingleton._instances[name] = ThreadSafeManager.new(name)
        ThreadSafeSingleton._instance_locks[name] = thread.newlock()
    end
    ThreadSafeSingleton._singleton_lock:unlock()
    
    return ThreadSafeSingleton._instances[name]
end

function ThreadSafeSingleton.get_instance(name)
    return ThreadSafeSingleton.new(name)
end

function ThreadSafeSingleton.reset_instance(name)
    ThreadSafeSingleton._singleton_lock:lock()
    ThreadSafeSingleton._instances[name] = nil
    ThreadSafeSingleton._instance_locks[name] = nil
    ThreadSafeSingleton._singleton_lock:unlock()
end

-- Thread-safe event emitter
local ThreadSafeEventEmitter = {}
ThreadSafeEventEmitter.__index = ThreadSafeEventEmitter

function ThreadSafeEventEmitter.new()
    local self = setmetatable({}, ThreadSafeEventEmitter)
    self.lock = thread.newlock()
    self.listeners = {}
    self.event_stats = {}
    return self
end

function ThreadSafeEventEmitter:on(event, callback)
    self.lock:lock()
    if not self.listeners[event] then
        self.listeners[event] = {}
    end
    table.insert(self.listeners[event], callback)
    self.lock:unlock()
end

function ThreadSafeEventEmitter:off(event, callback)
    self.lock:lock()
    if self.listeners[event] then
        for i, listener in ipairs(self.listeners[event]) do
            if listener == callback then
                table.remove(self.listeners[event], i)
                break
            end
        end
    end
    self.lock:unlock()
end

function ThreadSafeEventEmitter:emit(event, ...)
    local listeners = {}
    self.lock:lock()
    if self.listeners[event] then
        listeners = self.listeners[event]
        self.event_stats[event] = (self.event_stats[event] or 0) + 1
    end
    self.lock:unlock()
    
    -- Call listeners outside of lock to prevent deadlocks
    for _, listener in ipairs(listeners) do
        local ok, err = pcall(listener, ...)
        if not ok then
            print("Error in event listener: " .. tostring(err))
        end
    end
end

function ThreadSafeEventEmitter:clear_listeners(event)
    self.lock:lock()
    if event then
        self.listeners[event] = nil
        self.event_stats[event] = nil
    else
        self.listeners = {}
        self.event_stats = {}
    end
    self.lock:unlock()
end

function ThreadSafeEventEmitter:get_event_stats()
    self.lock:lock()
    local stats = {}
    for event, count in pairs(self.event_stats) do
        stats[event] = count
    end
    self.lock:unlock()
    return stats
end

function ThreadSafeEventEmitter:get_listener_count(event)
    self.lock:lock()
    local count = self.listeners[event] and #self.listeners[event] or 0
    self.lock:unlock()
    return count
end

-- Export the module
return {
    ThreadSafeManager = ThreadSafeManager,
    ThreadSafeSingleton = ThreadSafeSingleton,
    ThreadSafeEventEmitter = ThreadSafeEventEmitter,
    ThreadSafeStats = ThreadSafeStats,
    ThreadState = ThreadState
} 