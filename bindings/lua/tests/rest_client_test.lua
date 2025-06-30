-- Test file for REST API Client
local RestClient = require("satox.api.rest_client")

-- Test initialization
local function testInitialization()
    print("Testing REST client initialization...")
    
    local client = RestClient.new()
    assert(client ~= nil, "Client should not be nil")
    
    -- Test successful initialization
    local success, err = client:initialize("https://api.example.com", "test-api-key")
    assert(success, "Initialization should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test double initialization
    success, err = client:initialize("https://api.example.com", "test-api-key")
    assert(not success, "Double initialization should fail")
    assert(err == "REST client already initialized", "Should return initialization error")
    
    -- Test invalid base URL
    local invalidClient = RestClient.new()
    success, err = invalidClient:initialize(nil, "test-api-key")
    assert(not success, "Initialization should fail with invalid base URL")
    assert(err == "Invalid base URL", "Should return base URL error")
    
    -- Test invalid API key
    invalidClient = RestClient.new()
    success, err = invalidClient:initialize("https://api.example.com", nil)
    assert(not success, "Initialization should fail with invalid API key")
    assert(err == "Invalid API key", "Should return API key error")
    
    print("Initialization tests passed!")
end

-- Test timeout configuration
local function testTimeoutConfig()
    print("Testing timeout configuration...")
    
    local client = RestClient.new()
    client:initialize("https://api.example.com", "test-api-key")
    
    -- Test valid timeout
    local success, err = client:setTimeout(60)
    assert(success, "Setting valid timeout should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid timeout
    success, err = client:setTimeout(-1)
    assert(not success, "Setting invalid timeout should fail")
    assert(err == "Invalid timeout value", "Should return timeout error")
    
    print("Timeout configuration tests passed!")
end

-- Test retry configuration
local function testRetryConfig()
    print("Testing retry configuration...")
    
    local client = RestClient.new()
    client:initialize("https://api.example.com", "test-api-key")
    
    -- Test valid retry config
    local success, err = client:setRetryConfig(5, 2)
    assert(success, "Setting valid retry config should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid retry attempts
    success, err = client:setRetryConfig(-1, 2)
    assert(not success, "Setting invalid retry attempts should fail")
    assert(err == "Invalid retry attempts", "Should return retry attempts error")
    
    -- Test invalid retry delay
    success, err = client:setRetryConfig(5, -1)
    assert(not success, "Setting invalid retry delay should fail")
    assert(err == "Invalid retry delay", "Should return retry delay error")
    
    print("Retry configuration tests passed!")
end

-- Test GET request
local function testGetRequest()
    print("Testing GET request...")
    
    local client = RestClient.new()
    client:initialize("https://api.example.com", "test-api-key")
    
    -- Test successful GET request
    local response, err = client:get("/test", { param1 = "value1", param2 = "value2" })
    assert(response ~= nil, "Response should not be nil")
    assert(err == nil, "No error should be returned")
    assert(response.status == 200, "Status should be 200")
    assert(response.body.success, "Response should indicate success")
    
    -- Test uninitialized client
    local uninitializedClient = RestClient.new()
    response, err = uninitializedClient:get("/test")
    assert(response == nil, "Response should be nil")
    assert(err == "REST client not initialized", "Should return initialization error")
    
    print("GET request tests passed!")
end

-- Test POST request
local function testPostRequest()
    print("Testing POST request...")
    
    local client = RestClient.new()
    client:initialize("https://api.example.com", "test-api-key")
    
    -- Test successful POST request
    local data = { name = "Test", value = 123 }
    local response, err = client:post("/test", data)
    assert(response ~= nil, "Response should not be nil")
    assert(err == nil, "No error should be returned")
    assert(response.status == 201, "Status should be 201")
    assert(response.body.success, "Response should indicate success")
    assert(response.body.data.received.name == data.name, "Received data should match")
    
    print("POST request tests passed!")
end

-- Test PUT request
local function testPutRequest()
    print("Testing PUT request...")
    
    local client = RestClient.new()
    client:initialize("https://api.example.com", "test-api-key")
    
    -- Test successful PUT request
    local data = { name = "Test", value = 123 }
    local response, err = client:put("/test", data)
    assert(response ~= nil, "Response should not be nil")
    assert(err == nil, "No error should be returned")
    assert(response.status == 200, "Status should be 200")
    assert(response.body.success, "Response should indicate success")
    assert(response.body.data.received.name == data.name, "Received data should match")
    
    print("PUT request tests passed!")
end

-- Test DELETE request
local function testDeleteRequest()
    print("Testing DELETE request...")
    
    local client = RestClient.new()
    client:initialize("https://api.example.com", "test-api-key")
    
    -- Test successful DELETE request
    local response, err = client:delete("/test")
    assert(response ~= nil, "Response should not be nil")
    assert(err == nil, "No error should be returned")
    assert(response.status == 204, "Status should be 204")
    
    print("DELETE request tests passed!")
end

-- Test URL encoding
local function testUrlEncoding()
    print("Testing URL encoding...")
    
    local client = RestClient.new()
    
    -- Test basic encoding
    local encoded = client:urlEncode("test value")
    assert(encoded == "test+value", "Basic encoding should work")
    
    -- Test special characters
    encoded = client:urlEncode("test@value.com")
    assert(encoded == "test%40value.com", "Special characters should be encoded")
    
    -- Test nil input
    encoded = client:urlEncode(nil)
    assert(encoded == "", "Nil input should return empty string")
    
    print("URL encoding tests passed!")
end

-- Run all tests
print("Running REST client tests...")
testInitialization()
testTimeoutConfig()
testRetryConfig()
testGetRequest()
testPostRequest()
testPutRequest()
testDeleteRequest()
testUrlEncoding()
print("All REST client tests passed!") 