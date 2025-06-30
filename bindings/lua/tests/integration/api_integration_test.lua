-- Test error handling across clients
local function testErrorHandling()
    print("Testing error handling across clients...")
    
    local restClient = RestClient.new()
    local wsClient = WebSocketClient.new()
    local gqlClient = GraphQLClient.new()
    
    -- Initialize all clients
    restClient:initialize("https://api.example.com", "test-api-key")
    wsClient:initialize("wss://api.example.com/ws", "test-api-key")
    gqlClient:initialize("https://api.example.com/graphql", "test-api-key")
    
    -- Test REST error handling
    local restResult, restErr = restClient:get("/invalid-endpoint")
    assert(restResult == nil, "REST result should be nil for invalid endpoint")
    assert(restErr ~= nil, "REST error should be returned")
    
    -- Test WebSocket error handling
    local connected = wsClient:connect()
    assert(connected, "WebSocket connection should succeed")
    
    local wsResult, wsErr = wsClient:send("invalid-message")
    assert(wsResult == false, "WebSocket send should fail for invalid message")
    assert(wsErr ~= nil, "WebSocket error should be returned")
    
    -- Test GraphQL error handling
    local gqlResult, gqlErr = gqlClient:query([[
        query {
            invalidField {
                id
            }
        }
    ]])
    assert(gqlResult == nil, "GraphQL result should be nil for invalid query")
    assert(gqlErr ~= nil, "GraphQL error should be returned")
    
    -- Cleanup
    wsClient:disconnect()
    
    print("Error handling tests passed!")
end

RestClient = RestClient or {}
WebSocketClient = WebSocketClient or {}
GraphQLClient = GraphQLClient or {}
