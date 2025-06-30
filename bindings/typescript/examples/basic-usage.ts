/**
 * Basic usage example for Satox SDK TypeScript Bindings
 */

import {
    CoreManager,
    DatabaseManager,
    BlockchainManager,
    AssetManager,
    WalletManager,
    DatabaseType,
    NetworkType,
    CoreConfig,
    DatabaseConfig,
    SatoxError
} from '../src';

async function main() {
    console.log('🚀 Satox SDK TypeScript Example');
    console.log('================================');
    
    // Initialize core manager
    console.log('\n1. Initializing Core Manager...');
    const coreManager = new CoreManager();
    const config: CoreConfig = {
        name: 'satox_sdk_example',
        enableLogging: true,
        logLevel: 'info',
        maxThreads: 4
    };
    
    try {
        await coreManager.initialize(config);
        console.log('✅ Core Manager initialized successfully');
        
        // Get status
        const status = await coreManager.getStatus();
        console.log('📊 SDK Status:', status);
        
    } catch (error) {
        console.error('❌ Failed to initialize Core Manager:', error);
        return;
    }
    
    // Initialize database manager
    console.log('\n2. Initializing Database Manager...');
    const dbManager = new DatabaseManager();
    const dbConfig: DatabaseConfig = {
        type: DatabaseType.SQLITE,
        connectionString: ':memory:'
    };
    
    try {
        await dbManager.initialize(dbConfig);
        await dbManager.connect(DatabaseType.SQLITE, ':memory:');
        console.log('✅ Database Manager initialized and connected');
        
        // Execute a test query
        const result = await dbManager.executeQuery('SELECT 1 as test');
        console.log('📊 Test query result:', result);
        
    } catch (error) {
        console.warn('⚠️  Database Manager issue:', error);
    }
    
    // Initialize blockchain manager
    console.log('\n3. Initializing Blockchain Manager...');
    const blockchainManager = new BlockchainManager();
    
    try {
        await blockchainManager.initialize({});
        await blockchainManager.connect(
            NetworkType.MAINNET,
            'http://localhost:7777',
            { username: 'user', password: 'pass' }
        );
        console.log('✅ Blockchain Manager initialized and connected');
        
        // Get block height
        const height = await blockchainManager.getBlockHeight();
        console.log('📊 Current block height:', height);
        
    } catch (error) {
        console.warn('⚠️  Blockchain Manager issue:', error);
    }
    
    // Initialize asset manager
    console.log('\n4. Initializing Asset Manager...');
    const assetManager = new AssetManager();
    
    try {
        await assetManager.initialize({});
        console.log('✅ Asset Manager initialized');
        
        // Create a test asset
        const assetConfig = {
            name: 'Test Token',
            symbol: 'TEST',
            description: 'A test token created with Satox SDK',
            precision: 8,
            reissuable: true,
            totalSupply: 1000000
        };
        
        const asset = await assetManager.createAsset(assetConfig);
        console.log('✅ Asset created:', asset.name, `(${asset.symbol})`);
        
    } catch (error) {
        console.warn('⚠️  Asset Manager issue:', error);
    }
    
    // Initialize wallet manager
    console.log('\n5. Initializing Wallet Manager...');
    const walletManager = new WalletManager();
    
    try {
        await walletManager.initialize({});
        console.log('✅ Wallet Manager initialized');
        
        // Create a wallet
        const wallet = await walletManager.createWallet();
        console.log('✅ Wallet created:', wallet.address);
        
    } catch (error) {
        console.warn('⚠️  Wallet Manager issue:', error);
    }
    
    // Shutdown
    console.log('\n6. Shutting down...');
    try {
        await coreManager.shutdown();
        console.log('✅ SDK shutdown successfully');
    } catch (error) {
        console.warn('⚠️  Shutdown issue:', error);
    }
    
    console.log('\n🎉 Example completed successfully!');
}

// Run the example
if (require.main === module) {
    main().catch(console.error);
} 