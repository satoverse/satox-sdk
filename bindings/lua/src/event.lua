local EventManager = {}
EventManager.__index = EventManager

function EventManager.new()
    local self = setmetatable({}, EventManager)
    self.initialized = false
    self.blockchainManager = nil
    self.listeners = {}
    return self
end

function EventManager:initialize(blockchainManager)
    if self.initialized then
        error("EventManager already initialized")
    end
    if not blockchainManager then
        error("BlockchainManager is required")
    end
    self.blockchainManager = blockchainManager
    self.initialized = true
end

function EventManager:on(event, listener)
    if not self.initialized then
        error("EventManager not initialized")
    end
    if not event or type(event) ~= "string" then
        error("Event name is required")
    end
    if not listener or type(listener) ~= "function" then
        error("Listener must be a function")
    end
    if not self.listeners[event] then
        self.listeners[event] = {}
    end
    table.insert(self.listeners[event], listener)
    return true
end

function EventManager:off(event, listener)
    if not self.initialized then
        error("EventManager not initialized")
    end
    if not event or type(event) ~= "string" then
        error("Event name is required")
    end
    if not listener or type(listener) ~= "function" then
        error("Listener must be a function")
    end
    local eventListeners = self.listeners[event]
    if not eventListeners then
        return false
    end
    for i, l in ipairs(eventListeners) do
        if l == listener then
            table.remove(eventListeners, i)
            return true
        end
    end
    return false
end

function EventManager:emit(event, ...)
    if not self.initialized then
        error("EventManager not initialized")
    end
    if not event or type(event) ~= "string" then
        error("Event name is required")
    end
    local eventListeners = self.listeners[event]
    if not eventListeners then
        return 0
    end
    local count = 0
    for _, listener in ipairs(eventListeners) do
        local ok, err = pcall(listener, ...)
        if not ok then
            -- Optionally log or handle listener error
        end
        count = count + 1
    end
    return count
end

function EventManager:clear(event)
    if not self.initialized then
        error("EventManager not initialized")
    end
    if event then
        self.listeners[event] = nil
    else
        self.listeners = {}
    end
    return true
end

return EventManager 