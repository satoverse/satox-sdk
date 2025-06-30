const { BlockchainManager, AssetManager, SecurityManager, NFTManager, IPFSManager } = require('./build/Release/satox_native');

console.log('Testing Satox SDK TypeScript Bindings...\n');

// Test BlockchainManager
console.log('1. Testing BlockchainManager...');
try {
    const blockchain = new BlockchainManager();
    console.log('   ✓ BlockchainManager constructor works');
    
    const initResult = blockchain.initialize();
    console.log('   ✓ BlockchainManager.initialize() returned:', initResult);
    
    const height = blockchain.getCurrentHeight();
    console.log('   ✓ BlockchainManager.getCurrentHeight() returned:', height);
    
    const stats = blockchain.getStatistics();
    console.log('   ✓ BlockchainManager.getStatistics() returned:', stats);
    
} catch (error) {
    console.log('   ✗ BlockchainManager error:', error.message);
}

// Test AssetManager
console.log('\n2. Testing AssetManager...');
try {
    const asset = new AssetManager();
    console.log('   ✓ AssetManager constructor works');
    
    // Test createAsset with dummy data
    const createResult = asset.createAsset('owner_addr', 'TestAsset', 'TST', 1000, true);
    console.log('   ✓ AssetManager.createAsset() returned:', createResult);
    
    // Test getAssetMetadata
    const meta = asset.getAssetMetadata('TestAsset');
    console.log('   ✓ AssetManager.getAssetMetadata() returned:', meta);
    
    // Test getAssetBalance
    const balance = asset.getAssetBalance('owner_addr', 'TestAsset');
    console.log('   ✓ AssetManager.getAssetBalance() returned:', balance);
    
    // Test transferAsset
    const transferResult = asset.transferAsset('owner_addr', 'recipient_addr', 'TestAsset', 100);
    console.log('   ✓ AssetManager.transferAsset() returned:', transferResult);
    
    // Test getAssetStats
    const stats = asset.getAssetStats();
    console.log('   ✓ AssetManager.getAssetStats() returned:', stats);
    
} catch (error) {
    console.log('   ✗ AssetManager error:', error.message);
}

// Test SecurityManager
console.log('\n3. Testing SecurityManager...');
try {
    const security = new SecurityManager();
    console.log('   ✓ SecurityManager constructor works');
    
    const initResult = security.initialize();
    console.log('   ✓ SecurityManager.initialize() returned:', initResult);
    
    // Note: getCacheSize() method not implemented in wrapper
    
} catch (error) {
    console.log('   ✗ SecurityManager error:', error.message);
}

// Test NFTManager
console.log('\n4. Testing NFTManager...');
try {
    const nft = new NFTManager();
    console.log('   ✓ NFTManager constructor works');
    
    const initResult = nft.initialize();
    console.log('   ✓ NFTManager.initialize() returned:', initResult);
    
    const error = nft.getLastError();
    console.log('   ✓ NFTManager.getLastError() returned:', error);
    
} catch (error) {
    console.log('   ✗ NFTManager error:', error.message);
}

// Test IPFSManager
console.log('\n5. Testing IPFSManager...');
try {
    const ipfs = new IPFSManager();
    console.log('   ✓ IPFSManager constructor works');
    
    const initResult = ipfs.initialize();
    console.log('   ✓ IPFSManager.initialize() returned:', initResult);
    
    const error = ipfs.getLastError();
    console.log('   ✓ IPFSManager.getLastError() returned:', error);
    
} catch (error) {
    console.log('   ✗ IPFSManager error:', error.message);
}

console.log('\n✅ All tests completed!'); 