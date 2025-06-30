/**
 * $(basename "$1")
 * $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * 
 * Copyright (c) 2025 Satoxcoin Core Developers
 * MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import {
    CoreManager,
    BlockchainManager,
    SecurityManager,
    QuantumManager,
    WalletManager,
    AssetManager,
    NFTManager,
    TransactionManager,
    NetworkManager,
    DatabaseManager,
    APIManager,
    IPFSManager,
    SatoxError,
    AssetConfig,
    NFTConfig,
    TransactionConfig,
    BatchTransactionConfig,
    QuantumKeyPair
} from '@satox/sdk';

async function main() {
    console.log('🚀 Satox SDK TypeScript Example Application');
    console.log('==========================================\n');

    try {
        // Initialize core manager
        console.log('1. Initializing Core Manager...');
        const coreManager = CoreManager.getInstance();
        await coreManager.initialize();
        console.log('   ✅ Core Manager initialized successfully\n');

        // Initialize blockchain manager
        console.log('2. Initializing Blockchain Manager...');
        const blockchainManager = BlockchainManager.getInstance();
        await blockchainManager.initialize();
        console.log('   ✅ Blockchain Manager initialized successfully\n');

        // Initialize security manager
        console.log('3. Initializing Security Manager...');
        const securityManager = SecurityManager.getInstance();
        await securityManager.initialize();
        console.log('   ✅ Security Manager initialized successfully\n');

        // Initialize quantum manager
        console.log('4. Initializing Quantum Manager...');
        const quantumManager = QuantumManager.getInstance();
        await quantumManager.initialize();
        console.log('   ✅ Quantum Manager initialized successfully\n');

        // Initialize wallet manager
        console.log('5. Initializing Wallet Manager...');
        const walletManager = WalletManager.getInstance();
        await walletManager.initialize();
        console.log('   ✅ Wallet Manager initialized successfully\n');

        // Initialize asset manager
        console.log('6. Initializing Asset Manager...');
        const assetManager = AssetManager.getInstance();
        await assetManager.initialize();
        console.log('   ✅ Asset Manager initialized successfully\n');

        // Initialize NFT manager
        console.log('7. Initializing NFT Manager...');
        const nftManager = NFTManager.getInstance();
        await nftManager.initialize();
        console.log('   ✅ NFT Manager initialized successfully\n');

        // Initialize transaction manager
        console.log('8. Initializing Transaction Manager...');
        const transactionManager = TransactionManager.getInstance();
        await transactionManager.initialize();
        console.log('   ✅ Transaction Manager initialized successfully\n');

        // Initialize network manager
        console.log('9. Initializing Network Manager...');
        const networkManager = NetworkManager.getInstance();
        await networkManager.initialize();
        console.log('   ✅ Network Manager initialized successfully\n');

        // Initialize database manager
        console.log('10. Initializing Database Manager...');
        const databaseManager = DatabaseManager.getInstance();
        await databaseManager.initialize();
        console.log('   ✅ Database Manager initialized successfully\n');

        // Initialize API manager
        console.log('11. Initializing API Manager...');
        const apiManager = APIManager.getInstance();
        await apiManager.initialize();
        console.log('   ✅ API Manager initialized successfully\n');

        // Initialize IPFS manager
        console.log('12. Initializing IPFS Manager...');
        const ipfsManager = IPFSManager.getInstance();
        await ipfsManager.initialize();
        console.log('   ✅ IPFS Manager initialized successfully\n');

        // Demonstrate wallet operations
        console.log('13. Wallet Operations Demo...');
        const walletAddress = await walletManager.createWallet();
        console.log(`   ✅ Created wallet: ${walletAddress}`);

        const balance = await walletManager.getBalance(walletAddress);
        console.log(`   ✅ Wallet balance: ${balance} SATOX`);
        console.log();

        // Demonstrate quantum security
        console.log('14. Quantum Security Demo...');
        const quantumKeyPair: QuantumKeyPair = await quantumManager.generateKeyPair();
        console.log('   ✅ Generated quantum-resistant key pair');

        const message = Buffer.from('Hello Quantum World!', 'utf8');
        const quantumSignature = await quantumManager.sign(message, quantumKeyPair.privateKey);
        console.log('   ✅ Created quantum-resistant signature');

        const isValid = await quantumManager.verify(message, quantumSignature, quantumKeyPair.publicKey);
        console.log(`   ✅ Quantum signature verification: ${isValid}`);
        console.log();

        // Demonstrate asset operations
        console.log('15. Asset Operations Demo...');
        const assetConfig: AssetConfig = {
            name: 'Test Token',
            symbol: 'TEST',
            totalSupply: 1000000,
            decimals: 8,
            description: 'A test token for demonstration',
            metadata: {
                website: 'https://example.com',
                category: 'utility'
            }
        };

        const assetId = await assetManager.createAsset(assetConfig);
        console.log(`   ✅ Created asset with ID: ${assetId}`);

        const assetInfo = await assetManager.getAssetInfo(assetId);
        console.log(`   ✅ Asset info: ${assetInfo.name} (${assetInfo.symbol})`);
        console.log();

        // Demonstrate NFT operations
        console.log('16. NFT Operations Demo...');
        const nftConfig: NFTConfig = {
            name: 'Test NFT',
            symbol: 'TNFT',
            description: 'A test NFT for demonstration',
            metadata: {
                image: 'https://example.com/image.png',
                attributes: {
                    rarity: 'common',
                    power: 100
                }
            }
        };

        const nftId = await nftManager.createNFT(nftConfig);
        console.log(`   ✅ Created NFT with ID: ${nftId}`);

        const nftInfo = await nftManager.getNFTInfo(nftId);
        console.log(`   ✅ NFT info: ${nftInfo.name} (${nftInfo.symbol})`);
        console.log();

        // Demonstrate transaction operations
        console.log('17. Transaction Operations Demo...');
        const txConfig: TransactionConfig = {
            from: walletAddress,
            to: 'recipient_address',
            amount: 1000,
            assetId: assetId,
            fee: 100
        };

        const txId = await transactionManager.createTransaction(txConfig);
        console.log(`   ✅ Created transaction with ID: ${txId}`);

        const txStatus = await transactionManager.getTransactionStatus(txId);
        console.log(`   ✅ Transaction status: ${txStatus}`);
        console.log();

        // Demonstrate blockchain operations
        console.log('18. Blockchain Operations Demo...');
        const blockchainInfo = await blockchainManager.getBlockchainInfo();
        console.log(`   ✅ Blockchain: ${blockchainInfo.name} (Height: ${blockchainInfo.currentHeight})`);

        const latestBlock = await blockchainManager.getLatestBlock();
        console.log(`   ✅ Latest block: ${latestBlock.hash} with ${latestBlock.transactions.length} transactions`);
        console.log();

        // Demonstrate network operations
        console.log('19. Network Operations Demo...');
        const networkInfo = await networkManager.getNetworkInfo();
        console.log(`   ✅ Network: ${networkInfo.connections} connections`);

        const peers = await networkManager.getPeers();
        console.log(`   ✅ Connected peers: ${peers.length}`);
        console.log();

        // Demonstrate IPFS operations
        console.log('20. IPFS Operations Demo...');
        const data = Buffer.from('Hello IPFS World!', 'utf8');
        const ipfsHash = await ipfsManager.uploadData(data);
        console.log(`   ✅ Uploaded data to IPFS: ${ipfsHash}`);

        const retrievedData = await ipfsManager.downloadData(ipfsHash);
        console.log(`   ✅ Retrieved data from IPFS: ${retrievedData.length} bytes`);
        console.log();

        // Demonstrate database operations
        console.log('21. Database Operations Demo...');
        const dbInfo = await databaseManager.getDatabaseInfo();
        console.log(`   ✅ Database: ${dbInfo.tableCount} tables`);

        const dbStats = await databaseManager.getDatabaseStats();
        console.log(`   ✅ Database size: ${dbStats.sizeMB.toFixed(2)} MB`);
        console.log();

        // Demonstrate API operations
        console.log('22. API Operations Demo...');
        const apiInfo = await apiManager.getAPIInfo();
        console.log(`   ✅ API: ${apiInfo.endpointCount} endpoints available`);

        const apiStats = await apiManager.getAPIStats();
        console.log(`   ✅ API requests: ${apiStats.totalRequests} total`);
        console.log();

        // Demonstrate security operations
        console.log('23. Security Operations Demo...');
        const securityInfo = await securityManager.getSecurityInfo();
        console.log(`   ✅ Security: ${securityInfo.algorithmCount} algorithms supported`);

        const encryptionKey = await securityManager.generateEncryptionKey();
        console.log(`   ✅ Generated encryption key: ${encryptionKey.length} bytes`);
        console.log();

        // Demonstrate batch operations
        console.log('24. Batch Operations Demo...');
        const batchConfig: BatchTransactionConfig = {
            transactions: [
                {
                    from: walletAddress,
                    to: 'recipient1',
                    amount: 100,
                    assetId: assetId,
                    fee: 10
                },
                {
                    from: walletAddress,
                    to: 'recipient2',
                    amount: 200,
                    assetId: assetId,
                    fee: 10
                }
            ]
        };

        const batchId = await transactionManager.createBatchTransaction(batchConfig);
        console.log(`   ✅ Created batch transaction with ID: ${batchId}`);
        console.log();

        // Demonstrate error handling
        console.log('25. Error Handling Demo...');
        try {
            await transactionManager.getTransactionStatus('invalid_tx_id');
            console.log('   ❌ Unexpected success');
        } catch (error) {
            if (error instanceof SatoxError) {
                console.log(`   ✅ Properly handled error: ${error.message}`);
            }
        }
        console.log();

        // Demonstrate concurrent operations
        console.log('26. Concurrent Operations Demo...');
        const walletPromises = Array.from({ length: 3 }, async () => {
            const wm = WalletManager.getInstance();
            await wm.initialize();
            const address = await wm.createWallet();
            await wm.shutdown();
            return address;
        });

        const addresses = await Promise.all(walletPromises);
        console.log(`   ✅ Concurrent wallets created: ${addresses.join(', ')}`);
        console.log();

        // Demonstrate event handling
        console.log('27. Event Handling Demo...');
        transactionManager.on('transactionStatusChanged', (event) => {
            console.log(`   📡 Transaction ${event.transactionId} status changed to ${event.status}`);
        });

        // Demonstrate async/await patterns
        console.log('28. Async/Await Patterns Demo...');
        const asyncOperations = async () => {
            const results = await Promise.allSettled([
                walletManager.createWallet(),
                assetManager.getAssetInfo(assetId),
                nftManager.getNFTInfo(nftId)
            ]);

            results.forEach((result, index) => {
                if (result.status === 'fulfilled') {
                    console.log(`   ✅ Async operation ${index + 1} completed successfully`);
                } else {
                    console.log(`   ❌ Async operation ${index + 1} failed: ${result.reason}`);
                }
            });
        };

        await asyncOperations();
        console.log();

        // Demonstrate cleanup
        console.log('29. Cleanup Operations...');
        await apiManager.shutdown();
        await databaseManager.shutdown();
        await ipfsManager.shutdown();
        await networkManager.shutdown();
        await transactionManager.shutdown();
        await nftManager.shutdown();
        await assetManager.shutdown();
        await walletManager.shutdown();
        await quantumManager.shutdown();
        await securityManager.shutdown();
        await blockchainManager.shutdown();
        await coreManager.shutdown();
        console.log('   ✅ All managers shut down successfully\n');

        console.log('🎉 TypeScript Example Application Completed Successfully!');
        console.log('All SDK features demonstrated and working correctly.');

    } catch (error) {
        console.error('❌ Error in TypeScript example:', error);
        if (error instanceof SatoxError) {
            console.error('Satox Error:', error.message);
        }
    }
}

// Test functions for demonstration
async function testCoreInitialization(): Promise<void> {
    const coreManager = CoreManager.getInstance();
    await coreManager.initialize();
    await coreManager.shutdown();
}

async function testQuantumOperations(): Promise<void> {
    const quantumManager = QuantumManager.getInstance();
    await quantumManager.initialize();

    const keyPair = await quantumManager.generateKeyPair();
    const message = Buffer.from('Test message', 'utf8');
    const signature = await quantumManager.sign(message, keyPair.privateKey);
    const isValid = await quantumManager.verify(message, signature, keyPair.publicKey);

    if (!isValid) {
        throw new SatoxError('Quantum signature verification failed');
    }

    await quantumManager.shutdown();
}

async function testAssetOperations(): Promise<void> {
    const assetManager = AssetManager.getInstance();
    await assetManager.initialize();

    const config: AssetConfig = {
        name: 'Test Asset',
        symbol: 'TEST',
        totalSupply: 1000000,
        decimals: 8,
        description: 'Test asset',
        metadata: {}
    };

    const assetId = await assetManager.createAsset(config);
    const assetInfo = await assetManager.getAssetInfo(assetId);

    if (assetInfo.name !== 'Test Asset') {
        throw new SatoxError(`Unexpected asset name: ${assetInfo.name}`);
    }

    await assetManager.shutdown();
}

async function testConcurrentOperations(): Promise<void> {
    const promises = Array.from({ length: 5 }, async () => {
        const wm = WalletManager.getInstance();
        await wm.initialize();
        const address = await wm.createWallet();
        await wm.shutdown();
        return address;
    });

    const addresses = await Promise.all(promises);
    console.log(`   ✅ Concurrent wallets created: ${addresses.join(', ')}`);
}

async function testPerformance(): Promise<void> {
    const walletManager = WalletManager.getInstance();
    await walletManager.initialize();

    const startTime = Date.now();
    const addresses: string[] = [];

    for (let i = 0; i < 100; i++) {
        const address = await walletManager.createWallet();
        addresses.push(address);
    }

    const endTime = Date.now();
    console.log(`   ⚡ Created 100 wallets in ${endTime - startTime}ms`);

    await walletManager.shutdown();
}

// Export for testing
export {
    main,
    testCoreInitialization,
    testQuantumOperations,
    testAssetOperations,
    testConcurrentOperations,
    testPerformance
};

// Run the main function if this file is executed directly
if (require.main === module) {
    main().catch(console.error);
} 