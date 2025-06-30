-- Test file for IPFS Manager
local IPFSManager = require("satox.ipfs.ipfs_manager")

-- Test initialization
local function testInitialization()
    print("Testing IPFSManager initialization...")
    
    local manager = IPFSManager.new()
    assert(manager ~= nil, "Manager should not be nil")
    
    -- Test successful initialization
    local success, err = manager:initialize("localhost", 5001)
    assert(success, "Initialization should succeed")
    assert(err == nil, "No error should be returned")
    
    -- Test invalid host
    local invalidHostSuccess, invalidHostErr = manager:initialize("", 5001)
    assert(not invalidHostSuccess, "Initialization should fail with invalid host")
    assert(invalidHostErr == "Invalid host", "Should return host validation error")
    
    -- Test invalid port
    local invalidPortSuccess, invalidPortErr = manager:initialize("localhost", -1)
    assert(not invalidPortSuccess, "Initialization should fail with invalid port")
    assert(invalidPortErr == "Invalid port", "Should return port validation error")
    
    print("Initialization tests passed!")
end

-- Test data operations
local function testDataOperations()
    print("Testing IPFS data operations...")
    
    local manager = IPFSManager.new()
    manager:initialize("localhost", 5001)
    
    -- Test adding data
    local testData = {
        name = "Test Data",
        content = "This is test content"
    }
    local hash, err = manager:addData(testData)
    assert(hash ~= nil, "Hash should not be nil")
    assert(err == nil, "No error should be returned")
    assert(type(hash) == "string", "Hash should be a string")
    
    -- Test getting data
    local retrievedData, retrieveErr = manager:getData(hash)
    assert(retrievedData ~= nil, "Retrieved data should not be nil")
    assert(retrieveErr == nil, "No error should be returned")
    assert(retrievedData.name == testData.name, "Retrieved data should match original")
    assert(retrievedData.content == testData.content, "Retrieved content should match original")
    
    -- Test getting non-existent data
    local nonExistentData, nonExistentErr = manager:getData("non_existent_hash")
    assert(nonExistentData == nil, "Non-existent data should return nil")
    assert(nonExistentErr == "Data not found", "Should return not found error")
    
    print("Data operation tests passed!")
end

-- Test pinning operations
local function testPinningOperations()
    print("Testing IPFS pinning operations...")
    
    local manager = IPFSManager.new()
    manager:initialize("localhost", 5001)
    
    -- Add test data
    local testData = {
        name = "Test Data",
        content = "This is test content"
    }
    local hash = manager:addData(testData)
    
    -- Test pinning
    local pinSuccess, pinErr = manager:pinHash(hash)
    assert(pinSuccess, "Pinning should succeed")
    assert(pinErr == nil, "No error should be returned")
    
    -- Test pinning non-existent hash
    local nonExistentPinSuccess, nonExistentPinErr = manager:pinHash("non_existent_hash")
    assert(not nonExistentPinSuccess, "Pinning should fail")
    assert(nonExistentPinErr == "Hash not found", "Should return not found error")
    
    -- Test unpinning
    local unpinSuccess, unpinErr = manager:unpinHash(hash)
    assert(unpinSuccess, "Unpinning should succeed")
    assert(unpinErr == nil, "No error should be returned")
    
    -- Test unpinning non-existent hash
    local nonExistentUnpinSuccess, nonExistentUnpinErr = manager:unpinHash("non_existent_hash")
    assert(not nonExistentUnpinSuccess, "Unpinning should fail")
    assert(nonExistentUnpinErr == "Hash not found", "Should return not found error")
    
    print("Pinning operation tests passed!")
end

-- Test timeout operations
local function testTimeoutOperations()
    print("Testing IPFS timeout operations...")
    
    local manager = IPFSManager.new()
    manager:initialize("localhost", 5001)
    
    -- Test default timeout
    local defaultTimeout = manager:getTimeout()
    assert(defaultTimeout == 30, "Default timeout should be 30 seconds")
    
    -- Test setting timeout
    local newTimeout = 60
    manager:setTimeout(newTimeout)
    local updatedTimeout = manager:getTimeout()
    assert(updatedTimeout == newTimeout, "Timeout should be updated")
    
    -- Test invalid timeout
    local invalidTimeout = -1
    local success, err = manager:setTimeout(invalidTimeout)
    assert(not success, "Setting invalid timeout should fail")
    assert(err == "Invalid timeout value", "Should return timeout validation error")
    
    print("Timeout operation tests passed!")
end

-- Run all tests
print("Running IPFS Manager tests...")
testInitialization()
testDataOperations()
testPinningOperations()
testTimeoutOperations()
print("All IPFS Manager tests passed!") 