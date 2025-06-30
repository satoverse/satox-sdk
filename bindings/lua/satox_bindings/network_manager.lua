local NetworkManager = {}
NetworkManager.__index = NetworkManager

function NetworkManager.new()
    local self = setmetatable({}, NetworkManager)
    self.initialized = false
    self.connected = false
    self.peers = {}
    self.networkConfig = {
        p2pPort = 60777,
        rpcPort = 7777,
        networkType = "mainnet",
        maxPeers = 50,
        minPeers = 10
    }
    return self
end

function NetworkManager:initialize()
    if self.initialized then
        error("NetworkManager already initialized")
    end
    self.initialized = true
end

function NetworkManager:isInitialized()
    return self.initialized
end

function NetworkManager:connect()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if self.connected then
        error("Already connected to network")
    end
    -- TODO: Implement actual network connection
    self.connected = true
end

function NetworkManager:disconnect()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    self.connected = false
    self.peers = {}
end

function NetworkManager:isConnected()
    return self.connected
end

function NetworkManager:addPeer(address, port)
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    local peerId = address .. ":" .. port
    if self.peers[peerId] then
        error("Peer already exists")
    end
    self.peers[peerId] = {
        address = address,
        port = port,
        connected = false,
        lastSeen = os.time()
    }
end

function NetworkManager:removePeer(address, port)
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    local peerId = address .. ":" .. port
    if not self.peers[peerId] then
        error("Peer not found")
    end
    self.peers[peerId] = nil
end

function NetworkManager:getPeers()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    return self.peers
end

function NetworkManager:getNetworkConfig()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    return self.networkConfig
end

function NetworkManager:sendMessage()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    -- TODO: Implement actual message sending
    return true
end

function NetworkManager:receiveMessage()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    -- TODO: Implement actual message receiving
    return nil
end

function NetworkManager:validatePeer(peerId)
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    if not self.peers[peerId] then
        error("Peer not found")
    end
    -- TODO: Implement actual peer validation
    return true
end

function NetworkManager:getPeerCount()
    if not self.initialized then
        error("NetworkManager not initialized")
    end
    if not self.connected then
        error("Not connected to network")
    end
    local count = 0
    for _ in pairs(self.peers) do
        count = count + 1
    end
    return count
end

return NetworkManager 