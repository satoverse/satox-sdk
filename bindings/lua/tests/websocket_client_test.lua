-- Test file for WebSocket Client
local WebSocketClient = require("satox.api.websocket_client")

-- Test initialization
local function testInitialization()
    print("Testing WebSocket client initialization...")
    
    local client = WebSocketClient.new()
    assert(client ~= nil, "Client should not be nil")
    
    -- Test successful initialization
    local success, err = client:initialize("wss://api.example.com", "test-api-key")
    assert(success, "Initialization should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test double initialization
    success, err = client:initialize("wss://api.example.com", "test-api-key")
    assert(not success, "Double initialization should fail")
    assert(err == "WebSocket client already initialized", "Should return initialization error")
    
    -- Test invalid URL
    local invalidClient = WebSocketClient.new()
    success, err = invalidClient:initialize(nil, "test-api-key")
    assert(not success, "Initialization should fail with invalid URL")
    assert(err == "Invalid URL", "Should return URL error")
    
    -- Test invalid API key
    invalidClient = WebSocketClient.new()
    success, err = invalidClient:initialize("wss://api.example.com", nil)
    assert(not success, "Initialization should fail with invalid API key")
    assert(err == "Invalid API key", "Should return API key error")
    
    print("Initialization tests passed!")
end

-- Test reconnection configuration
local function testReconnectConfig()
    print("Testing reconnection configuration...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test valid reconnection config
    local success, err = client:setReconnectConfig(5, 2)
    assert(success, "Setting valid reconnection config should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid reconnect attempts
    success, err = client:setReconnectConfig(-1, 2)
    assert(not success, "Setting invalid reconnect attempts should fail")
    assert(err == "Invalid reconnect attempts", "Should return reconnect attempts error")
    
    -- Test invalid reconnect delay
    success, err = client:setReconnectConfig(5, -1)
    assert(not success, "Setting invalid reconnect delay should fail")
    assert(err == "Invalid reconnect delay", "Should return reconnect delay error")
    
    print("Reconnection configuration tests passed!")
end

-- Test ping configuration
local function testPingConfig()
    print("Testing ping configuration...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test valid ping config
    local success, err = client:setPingConfig(30, 10)
    assert(success, "Setting valid ping config should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid ping interval
    success, err = client:setPingConfig(-1, 10)
    assert(not success, "Setting invalid ping interval should fail")
    assert(err == "Invalid ping interval", "Should return ping interval error")
    
    -- Test invalid pong timeout
    success, err = client:setPingConfig(30, -1)
    assert(not success, "Setting invalid pong timeout should fail")
    assert(err == "Invalid pong timeout", "Should return pong timeout error")
    
    print("Ping configuration tests passed!")
end

-- Test connection
local function testConnection()
    print("Testing connection...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test successful connection
    local success, err = client:connect()
    assert(success, "Connection should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test double connection
    success, err = client:connect()
    assert(not success, "Double connection should fail")
    assert(err == "Already connected", "Should return connection error")
    
    -- Test uninitialized connection
    local uninitializedClient = WebSocketClient.new()
    success, err = uninitializedClient:connect()
    assert(not success, "Uninitialized connection should fail")
    assert(err == "WebSocket client not initialized", "Should return initialization error")
    
    print("Connection tests passed!")
end

-- Test disconnection
local function testDisconnection()
    print("Testing disconnection...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test successful disconnection
    client:connect()
    local success, err = client:disconnect()
    assert(success, "Disconnection should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test double disconnection
    success, err = client:disconnect()
    assert(not success, "Double disconnection should fail")
    assert(err == "Not connected", "Should return disconnection error")
    
    print("Disconnection tests passed!")
end

-- Test message sending
local function testMessageSending()
    print("Testing message sending...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test successful message sending
    client:connect()
    local success, err = client:send("test message")
    assert(success, "Message sending should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid message
    success, err = client:send(nil)
    assert(not success, "Invalid message sending should fail")
    assert(err == "Invalid message", "Should return message error")
    
    -- Test sending while disconnected
    client:disconnect()
    success, err = client:send("test message")
    assert(not success, "Sending while disconnected should fail")
    assert(err == "Not connected", "Should return connection error")
    
    print("Message sending tests passed!")
end

-- Test event subscription
local function testEventSubscription()
    print("Testing event subscription...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test successful subscription
    local handler = function(data) end
    local success, err = client:subscribe("test_event", handler)
    assert(success, "Subscription should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid event
    success, err = client:subscribe(nil, handler)
    assert(not success, "Invalid event subscription should fail")
    assert(err == "Invalid event", "Should return event error")
    
    -- Test invalid handler
    success, err = client:subscribe("test_event", nil)
    assert(not success, "Invalid handler subscription should fail")
    assert(err == "Invalid handler", "Should return handler error")
    
    print("Event subscription tests passed!")
end

-- Test event unsubscription
local function testEventUnsubscription()
    print("Testing event unsubscription...")
    
    local client = WebSocketClient.new()
    client:initialize("wss://api.example.com", "test-api-key")
    
    -- Test successful unsubscription
    local handler = function(data) end
    client:subscribe("test_event", handler)
    local success, err = client:unsubscribe("test_event", handler)
    assert(success, "Unsubscription should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid event
    success, err = client:unsubscribe(nil, handler)
    assert(not success, "Invalid event unsubscription should fail")
    assert(err == "Invalid event", "Should return event error")
    
    -- Test unsubscription of non-existent event
    success, err = client:unsubscribe("non_existent", handler)
    assert(not success, "Non-existent event unsubscription should fail")
    assert(err == "No handlers for event", "Should return handler error")
    
    print("Event unsubscription tests passed!")
end

-- Run all tests
print("Running WebSocket client tests...")
testInitialization()
testReconnectConfig()
testPingConfig()
testConnection()
testDisconnection()
testMessageSending()
testEventSubscription()
testEventUnsubscription()
print("All WebSocket client tests passed!") 