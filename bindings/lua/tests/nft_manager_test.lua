-- Test file for NFT Manager
local NFTManager = require("satox.nft.nft_manager")

-- Helper function to create test NFT metadata
local function createTestMetadata()
    return {
        name = "Test NFT",
        description = "Test NFT description",
        image = "ipfs://test-image",
        externalUrl = "https://example.com/nft",
        properties = {
            rarity = "common",
            type = "collectible"
        },
        attributes = {
            color = "blue",
            size = "medium"
        }
    }
end

-- Test initialization
local function testInitialization()
    print("Testing NFTManager initialization...")
    
    local manager = NFTManager.new()
    assert(manager ~= nil)
    
    local success, err = manager:initialize()
    assert(success)
    assert(err == nil)
    
    -- Test double initialization
    success, err = manager:initialize()
    assert(not success)
    assert(err == "NFTManager already initialized")
    
    print("Initialization tests passed!")
end

-- Test NFT creation
local function testNFTCreation()
    print("Testing NFT creation...")
    
    local manager = NFTManager.new()
    manager:initialize()
    
    -- Test successful creation
    local metadata = createTestMetadata()
    local nftData = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local tokenId, err = manager:createNFT(nftData)
    assert(tokenId ~= nil)
    assert(err == nil)
    
    -- Test duplicate creation
    local duplicateId, duplicateErr = manager:createNFT(nftData)
    assert(duplicateId == nil)
    assert(duplicateErr == "NFT already exists")
    
    -- Test invalid creation
    local invalidData = {
        contractAddress = "0x123",
        name = "",
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local invalidId, invalidErr = manager:createNFT(invalidData)
    assert(invalidId == nil)
    assert(invalidErr == "NFT name is required")
    
    print("NFT creation tests passed!")
end

-- Test NFT retrieval
local function testNFTRetrieval()
    print("Testing NFT retrieval...")
    
    local manager = NFTManager.new()
    manager:initialize()
    
    -- Create test NFT
    local metadata = createTestMetadata()
    local nftData = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local tokenId = manager:createNFT(nftData)
    
    -- Test successful retrieval
    local nft, err = manager:getNFT(tokenId)
    assert(nft ~= nil)
    assert(err == nil)
    assert(nft.name == metadata.name)
    assert(nft.description == metadata.description)
    
    -- Test non-existent NFT
    local nonExistentNFT, nonExistentErr = manager:getNFT("non_existent")
    assert(nonExistentNFT == nil)
    assert(nonExistentErr == "NFT not found")
    
    print("NFT retrieval tests passed!")
end

-- Test NFT transfer
local function testNFTTransfer()
    print("Testing NFT transfer...")
    
    local manager = NFTManager.new()
    manager:initialize()
    
    -- Create test NFT
    local metadata = createTestMetadata()
    local nftData = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local tokenId = manager:createNFT(nftData)
    
    -- Test successful transfer
    local success, err = manager:transferNFT(tokenId, "creator", "receiver")
    assert(success)
    assert(err == nil)
    
    -- Verify ownership
    local nft = manager:getNFT(tokenId)
    assert(nft.owner == "receiver")
    
    -- Test non-owner transfer
    local nonOwnerSuccess, nonOwnerErr = manager:transferNFT(tokenId, "creator", "newOwner")
    assert(not nonOwnerSuccess)
    assert(nonOwnerErr == "Not the owner of this NFT")
    
    -- Test non-transferable NFT
    local nonTransferableData = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = false,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local nonTransferableId = manager:createNFT(nonTransferableData)
    local nonTransferableSuccess, nonTransferableErr = manager:transferNFT(nonTransferableId, "creator", "receiver")
    assert(not nonTransferableSuccess)
    assert(nonTransferableErr == "NFT is not transferable")
    
    print("NFT transfer tests passed!")
end

-- Test NFT burning
local function testNFTBurning()
    print("Testing NFT burning...")
    
    local manager = NFTManager.new()
    manager:initialize()
    
    -- Create test NFT
    local metadata = createTestMetadata()
    local nftData = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local tokenId = manager:createNFT(nftData)
    
    -- Test successful burn
    local success, err = manager:burnNFT(tokenId, "creator")
    assert(success)
    assert(err == nil)
    
    -- Verify NFT is burned
    local burnedNFT, burnedErr = manager:getNFT(tokenId)
    assert(burnedNFT == nil)
    assert(burnedErr == "NFT not found")
    
    -- Test non-burnable NFT
    local nonBurnableData = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = false,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local nonBurnableId = manager:createNFT(nonBurnableData)
    local nonBurnableSuccess, nonBurnableErr = manager:burnNFT(nonBurnableId, "creator")
    assert(not nonBurnableSuccess)
    assert(nonBurnableErr == "NFT is not burnable")
    
    print("NFT burning tests passed!")
end

-- Test NFT ownership query
local function testNFTOwnership()
    print("Testing NFT ownership query...")
    
    local manager = NFTManager.new()
    manager:initialize()
    
    -- Create test NFTs
    local metadata = createTestMetadata()
    local nftData1 = {
        contractAddress = "0x123",
        name = metadata.name,
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local tokenId1 = manager:createNFT(nftData1)
    local nftData2 = {
        contractAddress = "0x123",
        name = "Test NFT 2",
        description = metadata.description,
        image = metadata.image,
        externalUrl = metadata.externalUrl,
        properties = metadata.properties,
        attributes = metadata.attributes,
        creator = "creator",
        isTransferable = true,
        isBurnable = true,
        royaltyRecipient = "royaltyRecipient",
        royaltyBasisPoints = 500
    }
    local tokenId2 = manager:createNFT(nftData2)
    
    -- Test ownership query
    local nfts = manager:getNFTsByOwner("creator")
    assert(#nfts == 2)
    
    -- Transfer one NFT
    manager:transferNFT(tokenId1, "creator", "receiver")
    
    -- Test updated ownership
    local creatorNFTs = manager:getNFTsByOwner("creator")
    local receiverNFTs = manager:getNFTsByOwner("receiver")
    assert(#creatorNFTs == 1)
    assert(#receiverNFTs == 1)
    assert(creatorNFTs[1].tokenId == tokenId2)
    assert(receiverNFTs[1].tokenId == tokenId1)
    
    print("NFT ownership query tests passed!")
end

-- Run all tests
print("Running NFT Manager tests...")
testInitialization()
testNFTCreation()
testNFTRetrieval()
testNFTTransfer()
testNFTBurning()
testNFTOwnership()
print("All NFT Manager tests passed!") 