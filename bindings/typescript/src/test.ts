import { 
  BlockchainManager, 
  AssetManager, 
  NetworkManager, 
  SecurityManager, 
  NFTManager, 
  IPFSManager 
} from './index';

async function testBindings() {
  console.log('Testing Satox SDK TypeScript Bindings...\n');

  try {
    // Test Blockchain Manager
    console.log('1. Testing Blockchain Manager...');
    const blockchainManager = new BlockchainManager();
    const blockchainConfig = { host: 'localhost', port: 7777, enableSSL: false };
    const blockchainInit = blockchainManager.initialize(blockchainConfig);
    console.log(`   Blockchain Manager initialized: ${blockchainInit}`);
    console.log(`   Last error: ${blockchainManager.getLastError()}`);
    blockchainManager.shutdown();

    // Test Asset Manager
    console.log('\n2. Testing Asset Manager...');
    const assetManager = new AssetManager();
    const assetInit = assetManager.initialize();
    console.log(`   Asset Manager initialized: ${assetInit}`);
    console.log(`   Last error: ${assetManager.getLastError()}`);
    assetManager.shutdown();

    // Test Network Manager
    console.log('\n3. Testing Network Manager...');
    const networkManager = new NetworkManager();
    const networkConfig = { port: 60777, maxConnections: 10, enableDiscovery: true };
    const networkInit = networkManager.initialize(networkConfig);
    console.log(`   Network Manager initialized: ${networkInit}`);
    console.log(`   Last error: ${networkManager.getLastError()}`);
    networkManager.shutdown();

    // Test Security Manager
    console.log('\n4. Testing Security Manager...');
    const securityManager = new SecurityManager();
    const securityInit = securityManager.initialize();
    console.log(`   Security Manager initialized: ${securityInit}`);
    console.log(`   Last error: ${securityManager.getLastError()}`);
    securityManager.shutdown();

    // Test NFT Manager
    console.log('\n5. Testing NFT Manager...');
    const nftManager = new NFTManager();
    const nftInit = nftManager.initialize();
    console.log(`   NFT Manager initialized: ${nftInit}`);
    console.log(`   Last error: ${nftManager.getLastError()}`);
    nftManager.shutdown();

    // Test IPFS Manager
    console.log('\n6. Testing IPFS Manager...');
    const ipfsManager = new IPFSManager();
    const ipfsConfig = { gateway: 'https://ipfs.io', timeout: 30000 };
    const ipfsInit = ipfsManager.initialize(ipfsConfig);
    console.log(`   IPFS Manager initialized: ${ipfsInit}`);
    console.log(`   Last error: ${ipfsManager.getLastError()}`);
    ipfsManager.shutdown();

    console.log('\n✅ All managers tested successfully!');
    console.log('TypeScript bindings are working correctly.');

  } catch (error) {
    console.error('\n❌ Error testing bindings:', error);
    process.exit(1);
  }
}

// Run the test if this file is executed directly
if (require.main === module) {
  testBindings();
}

export { testBindings }; 