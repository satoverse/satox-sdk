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

/**
 * Satox SDK WebAssembly Example Application
 * Demonstrates all major SDK features including blockchain operations, 
 * asset management, NFT operations, quantum security, and more.
 */

// Import WASM modules
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
    IPFSManager 
} from '@satox/sdk-wasm';

console.log('🚀 Satox SDK WebAssembly Example Application');
console.log('============================================\n');

async function main() {
    try {
        // Initialize core manager
        console.log('1. Initializing Core Manager...');
        const coreManager = await CoreManager.getInstance();
        await coreManager.initialize();
        console.log('   ✅ Core Manager initialized successfully\n');

        // Initialize blockchain manager
        console.log('2. Initializing Blockchain Manager...');
        const blockchainManager = await BlockchainManager.getInstance();
        await blockchainManager.initialize();
        console.log('   ✅ Blockchain Manager initialized successfully\n');

        // Initialize security manager
        console.log('3. Initializing Security Manager...');
        const securityManager = await SecurityManager.getInstance();
        await securityManager.initialize();
        console.log('   ✅ Security Manager initialized successfully\n');

        // Initialize quantum manager
        console.log('4. Initializing Quantum Manager...');
        const quantumManager = await QuantumManager.getInstance();
        await quantumManager.initialize();
        console.log('   ✅ Quantum Manager initialized successfully\n');

        // Initialize wallet manager
        console.log('5. Initializing Wallet Manager...');
        const walletManager = await WalletManager.getInstance();
        await walletManager.initialize();
        console.log('   ✅ Wallet Manager initialized successfully\n');

        // Initialize asset manager
        console.log('6. Initializing Asset Manager...');
        const assetManager = await AssetManager.getInstance();
        await assetManager.initialize();
        console.log('   ✅ Asset Manager initialized successfully\n');

        // Initialize NFT manager
        console.log('7. Initializing NFT Manager...');
        const nftManager = await NFTManager.getInstance();
        await nftManager.initialize();
        console.log('   ✅ NFT Manager initialized successfully\n');

        // Initialize transaction manager
        console.log('8. Initializing Transaction Manager...');
        const transactionManager = await TransactionManager.getInstance();
        await transactionManager.initialize();
        console.log('   ✅ Transaction Manager initialized successfully\n');

        // Initialize network manager
        console.log('9. Initializing Network Manager...');
        const networkManager = await NetworkManager.getInstance();
        await networkManager.initialize();
        console.log('   ✅ Network Manager initialized successfully\n');

        // Initialize database manager
        console.log('10. Initializing Database Manager...');
        const databaseManager = await DatabaseManager.getInstance();
        await databaseManager.initialize();
        console.log('   ✅ Database Manager initialized successfully\n');

        // Initialize API manager
        console.log('11. Initializing API Manager...');
        const apiManager = await APIManager.getInstance();
        await apiManager.initialize();
        console.log('   ✅ API Manager initialized successfully\n');

        // Initialize IPFS manager
        console.log('12. Initializing IPFS Manager...');
        const ipfsManager = await IPFSManager.getInstance();
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
        const quantumKeyPair = await quantumManager.generateKeyPair();
        console.log('   ✅ Generated quantum-resistant key pair');

        const message = new TextEncoder().encode('Hello Quantum World!');
        const quantumSignature = await quantumManager.sign(message, quantumKeyPair.privateKey);
        console.log('   ✅ Created quantum-resistant signature');

        const isValid = await quantumManager.verify(message, quantumSignature, quantumKeyPair.publicKey);
        console.log(`   ✅ Quantum signature verification: ${isValid}`);
        console.log();

        // Demonstrate asset operations
        console.log('15. Asset Operations Demo...');
        const assetConfig = {
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
        const nftConfig = {
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
        const txConfig = {
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
        const data = new TextEncoder().encode('Hello IPFS World!');
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
        const batchConfig = {
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
            console.log(`   ✅ Properly handled error: ${error.message}`);
        }
        console.log();

        // Demonstrate concurrent operations
        console.log('26. Concurrent Operations Demo...');
        const createWallet = async () => {
            const wm = await WalletManager.getInstance();
            await wm.initialize();
            const address = await wm.createWallet();
            await wm.shutdown();
            return address;
        };

        const walletPromises = Array.from({ length: 3 }, createWallet);
        const addresses = await Promise.all(walletPromises);
        console.log(`   ✅ Concurrent wallets created: ${addresses.join(', ')}`);
        console.log();

        // Demonstrate Web Workers
        console.log('27. Web Workers Demo...');
        if (typeof Worker !== 'undefined') {
            const worker = new Worker(`
                importScripts('./satox-sdk-wasm.js');
                
                self.onmessage = async function(e) {
                    try {
                        const walletManager = await WalletManager.getInstance();
                        await walletManager.initialize();
                        const address = await walletManager.createWallet();
                        await walletManager.shutdown();
                        self.postMessage({ success: true, address: address });
                    } catch (error) {
                        self.postMessage({ success: false, error: error.message });
                    }
                };
            `);

            const workerResult = await new Promise((resolve) => {
                worker.onmessage = (e) => resolve(e.data);
                worker.postMessage('create_wallet');
            });

            if (workerResult.success) {
                console.log(`   ✅ Web Worker created wallet: ${workerResult.address}`);
            } else {
                console.log(`   ❌ Web Worker failed: ${workerResult.error}`);
            }
            worker.terminate();
        } else {
            console.log('   ⚠️ Web Workers not supported in this environment');
        }
        console.log();

        // Demonstrate SharedArrayBuffer operations
        console.log('28. SharedArrayBuffer Operations Demo...');
        if (typeof SharedArrayBuffer !== 'undefined') {
            const sharedBuffer = new SharedArrayBuffer(1024);
            const sharedArray = new Uint8Array(sharedBuffer);
            
            // Fill with test data
            for (let i = 0; i < sharedArray.length; i++) {
                sharedArray[i] = i % 256;
            }

            const hash = await securityManager.hash(sharedArray);
            console.log(`   ✅ Hashed SharedArrayBuffer: ${hash}`);
        } else {
            console.log('   ⚠️ SharedArrayBuffer not supported in this environment');
        }
        console.log();

        // Demonstrate performance testing
        console.log('29. Performance Testing Demo...');
        const startTime = performance.now();

        // Create 100 wallets concurrently
        const batchWalletPromises = Array.from({ length: 100 }, createWallet);
        const batchAddresses = await Promise.all(batchWalletPromises);

        const endTime = performance.now();
        console.log(`   ⚡ Created 100 wallets in ${(endTime - startTime).toFixed(2)}ms`);
        console.log();

        // Demonstrate memory management
        console.log('30. Memory Management Demo...');
        const initialMemory = performance.memory ? performance.memory.usedJSHeapSize : 0;
        
        // Perform memory-intensive operations
        for (let i = 0; i < 1000; i++) {
            const testData = new Uint8Array(1024);
            await securityManager.hash(testData);
        }

        const finalMemory = performance.memory ? performance.memory.usedJSHeapSize : 0;
        const memoryUsed = finalMemory - initialMemory;
        console.log(`   💾 Memory used: ${(memoryUsed / 1024 / 1024).toFixed(2)} MB`);
        console.log();

        // Demonstrate cleanup
        console.log('31. Cleanup Operations...');
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

        console.log('🎉 WebAssembly Example Application Completed Successfully!');
        console.log('All SDK features demonstrated and working correctly.');

    } catch (error) {
        console.error('❌ Error in WebAssembly example:', error);
    }
}

// Test functions for demonstration
async function testCoreInitialization() {
    const coreManager = await CoreManager.getInstance();
    await coreManager.initialize();
    await coreManager.shutdown();
}

async function testQuantumOperations() {
    const quantumManager = await QuantumManager.getInstance();
    await quantumManager.initialize();

    const keyPair = await quantumManager.generateKeyPair();
    const message = new TextEncoder().encode('Test message');
    const signature = await quantumManager.sign(message, keyPair.privateKey);
    const isValid = await quantumManager.verify(message, signature, keyPair.publicKey);

    if (!isValid) {
        throw new Error('Quantum signature verification failed');
    }

    await quantumManager.shutdown();
}

async function testAssetOperations() {
    const assetManager = await AssetManager.getInstance();
    await assetManager.initialize();

    const config = {
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
        throw new Error(`Unexpected asset name: ${assetInfo.name}`);
    }

    await assetManager.shutdown();
}

async function testConcurrentOperations() {
    const createWallet = async () => {
        const wm = await WalletManager.getInstance();
        await wm.initialize();
        const address = await wm.createWallet();
        await wm.shutdown();
        return address;
    };

    const promises = Array.from({ length: 5 }, createWallet);
    const addresses = await Promise.all(promises);
    console.log(`   ✅ Concurrent wallets created: ${addresses.join(', ')}`);
}

async function testPerformance() {
    const walletManager = await WalletManager.getInstance();
    await walletManager.initialize();

    const startTime = performance.now();
    const addresses = [];

    for (let i = 0; i < 100; i++) {
        const address = await walletManager.createWallet();
        addresses.push(address);
    }

    const endTime = performance.now();
    console.log(`   ⚡ Created 100 wallets in ${(endTime - startTime).toFixed(2)}ms`);

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
if (typeof window !== 'undefined') {
    // Browser environment
    window.addEventListener('load', main);
} else if (typeof global !== 'undefined') {
    // Node.js environment
    main().catch(console.error);
} 