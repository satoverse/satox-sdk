-- tests/nft_test.lua
local luaunit = require('luaunit')
local NFTManager = require('../satox_bindings/nft')

-- Mock IPFS and Blockchain for testing
local MockIPFS = {
    store = function(self, data) return "mock_ipfs_cid", nil end
}
local MockBlockchain = {
    create_nft = function(self, data) return true, nil end,
    transfer_nft = function(self, id, from, to) return true, nil end
}

TestNFTManager = {}

function TestNFTManager:setUp()
    self.nftManager = NFTManager.new(MockIPFS, MockBlockchain)
    self.nftManager:initialize()
end

function TestNFTManager:test_create_nft_correctly()
    local nft_data = {
        id = "test_nft_1",
        name = "My Test NFT",
        description = "A description for the test NFT.",
        owner = "owner_address_1",
        collection = "Test Collection",
        metadata = {},
        is_transferable = true,
        is_burnable = true
    }
    
    local nft, err = self.nftManager:create_nft(nft_data)
    
    luaunit.assertIsNil(err)
    luaunit.assertNotNil(nft)
    if nft then
        luaunit.assertEquals(nft.id, "test_nft_1")
        luaunit.assertEquals(nft.owner, "owner_address_1")
    end
end

function TestNFTManager:test_create_nft_missing_fields()
    -- This call is missing the 'owner' field
    local nft_data = {
        id = "test_nft_2",
        name = "Incomplete NFT",
        description = "This NFT is missing data."
    }
    
    local nft, err = self.nftManager:create_nft(nft_data)
    
    luaunit.assertIsNil(nft)
    luaunit.assertNotNil(err)
    luaunit.assertEquals(err, "Missing required fields")
end

function TestNFTManager:test_transfer_nft()
    -- First, create an NFT to transfer
    local nft_data = {
        id = "transfer_nft_1",
        name = "Transferable NFT",
        description = "This NFT can be transferred.",
        owner = "owner_address_A",
        metadata = {},
        is_transferable = true
    }
    self.nftManager:create_nft(nft_data)
    
    -- Now, transfer it
    local from_owner = "owner_address_A"
    local to_owner = "owner_address_B"
    
    local nft, err = self.nftManager:transfer_nft("transfer_nft_1", from_owner, to_owner)
    
    luaunit.assertIsNil(err)
    luaunit.assertNotNil(nft)
    if nft then
        luaunit.assertEquals(nft.owner, to_owner)
    end
end

os.exit(luaunit.LuaUnit.run())
