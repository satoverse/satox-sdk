-- tests/ipfs_test.lua
local luaunit = require('luaunit')
local IPFSModule = require('../satox_bindings/ipfs')

-- Mock HTTP for testing
local MockHTTP = {
    post = function(self, url, options)
        if url:find("/add") then
            return { json = function() return { Hash = "mock_cid_123" } end }, nil
        elseif url:find("/pin/add") then
            return { json = function() return { Keys = { ["mock_cid_123"] = {} } } end }, nil
        elseif url:find("/pin/ls") then
            return { json = function() return { Keys = { ["mock_cid_123"] = {} } } end }, nil
        end
        return { json = function() return {} end }, nil
    end,
    get = function(self, url)
        return { body = "mock_retrieved_data" }, nil
    end
}

TestIPFSManager = {}

function TestIPFSManager:setUp()
    -- Mock the http module
    package.loaded.http = MockHTTP
    
    self.config = {
        api_url = "http://localhost:5001/api/v0",
        gateway_url = "http://localhost:8080"
    }
    self.ipfsManager = IPFSModule.new(self.config)
end

function TestIPFSManager:test_store_data()
    local test_data = "Hello, IPFS!"
    local cid, err = self.ipfsManager:store(test_data)
    
    luaunit.assertIsNil(err)
    luaunit.assertNotNil(cid)
    luaunit.assertEquals(cid, "mock_cid_123")
end

function TestIPFSManager:test_retrieve_data()
    local test_cid = "mock_cid_123"
    local data, err = self.ipfsManager:retrieve(test_cid)
    
    luaunit.assertIsNil(err)
    luaunit.assertNotNil(data)
    luaunit.assertEquals(data, "mock_retrieved_data")
end

function TestIPFSManager:test_pin_cid()
    local test_cid = "mock_cid_123"
    local success, err = self.ipfsManager:pin(test_cid)
    
    luaunit.assertIsNil(err)
    luaunit.assertIsTrue(success)
end

function TestIPFSManager:test_unpin_cid()
    local test_cid = "mock_cid_123"
    local success, err = self.ipfsManager:unpin(test_cid)
    
    luaunit.assertIsNil(err)
    luaunit.assertIsTrue(success)
end

function TestIPFSManager:test_get_gateway_url()
    local test_cid = "mock_cid_123"
    local url = self.ipfsManager:get_gateway_url(test_cid)
    
    luaunit.assertEquals(url, "http://localhost:8080/ipfs/mock_cid_123")
end

function TestIPFSManager:test_is_pinned()
    local test_cid = "mock_cid_123"
    local is_pinned, err = self.ipfsManager:is_pinned(test_cid)
    
    luaunit.assertIsNil(err)
    luaunit.assertIsTrue(is_pinned)
end

os.exit(luaunit.LuaUnit.run())
