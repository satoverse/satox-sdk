-- Add module path
package.path = package.path .. ";bindings/lua/?.lua"
-- Test file for GraphQL Client
local GraphQLClient = require("satox.api.graphql_client")
-- Test initialization
local function testInitialization()
    print("Testing GraphQL client initialization...")
    
    local client = GraphQLClient.new()
    assert(client ~= nil, "Client should not be nil")
    
    -- Test successful initialization
    local success, err = client:initialize("https://api.example.com/graphql", "test-api-key")
    assert(success, "Initialization should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test double initialization
    success, err = client:initialize("https://api.example.com/graphql", "test-api-key")
    assert(not success, "Double initialization should fail")
    assert(err == "GraphQLClient already initialized", "Should return initialization error")
    
    -- Test invalid URL
    local invalidClient = GraphQLClient.new()
    success, err = invalidClient:initialize(nil, "test-api-key")
    assert(not success, "Initialization should fail with invalid URL")
    assert(err == "Invalid URL", "Should return URL error")
    
    -- Test invalid API key
    invalidClient = GraphQLClient.new()
    success, err = invalidClient:initialize("https://api.example.com/graphql", nil)
    assert(not success, "Initialization should fail with invalid API key")
    assert(err == "Invalid API key", "Should return API key error")
    
    print("Initialization tests passed!")
end
