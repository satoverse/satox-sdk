import { SatoxSDK, BlockchainConfig, NFTConfig, IPFSConfig, AssetConfig, TransactionConfig } from '../src/satox';
import { EventEmitter } from 'events';
import * as fs from 'fs';
import * as path from 'path';

describe('SatoxSDK', () => {
    let sdk: SatoxSDK;
    let blockchainConfig: BlockchainConfig;
    let nftConfig: NFTConfig;
    let ipfsConfig: IPFSConfig;
    let assetConfig: AssetConfig;
    let transactionConfig: TransactionConfig;

    beforeEach(() => {
        sdk = SatoxSDK.getInstance();
        
        // Initialize test configurations
        blockchainConfig = {
            network: 'mainnet',
            rpcUrl: 'http://localhost:7777',
            p2pPort: 60777,
            enableMining: false
        };

        nftConfig = {
            enableMetadata: true,
            enableVersioning: true,
            maxMetadataSize: 1024 * 1024 // 1MB
        };

        ipfsConfig = {
            apiHost: '127.0.0.1',
            apiPort: 5001,
            gatewayHost: '127.0.0.1',
            gatewayPort: 8080,
            enablePinning: true
        };

        assetConfig = {
            enableMetadata: true,
            enablePermissions: true,
            maxMetadataSize: 1024 * 1024 // 1MB
        };

        transactionConfig = {
            enableFeeEstimation: true,
            enableUTXOManagement: true,
            enableMempool: true
        };
    });

    afterEach(async () => {
        await sdk.shutdown();
    });

    // Basic Functionality Tests
    test('Initialization and Shutdown', async () => {
        const result = await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );
        expect(result).toBe(true);
        expect(sdk.isInitialized()).toBe(true);

        await sdk.shutdown();
        expect(sdk.isInitialized()).toBe(false);
    });

    // Blockchain Tests
    test('Blockchain Operations', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const block = await sdk.getBlock('latest');
        expect(block).not.toBeNull();

        const tx = await sdk.getTransaction('test_tx_id');
        expect(tx).not.toBeNull();
    });

    // NFT Tests
    test('NFT Operations', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const metadata = {
            name: 'Test NFT',
            description: 'Test Description',
            properties: {
                type: 'test',
                rarity: 'common'
            }
        };

        const nftId = await sdk.createNFT(metadata);
        expect(nftId).not.toBeNull();

        const nft = await sdk.getNFT(nftId);
        expect(nft).not.toBeNull();
        expect(nft.metadata.name).toBe('Test NFT');
    });

    // IPFS Tests
    test('IPFS Operations', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const testData = Buffer.from('test data');
        const cid = await sdk.addFile(testData);
        expect(cid).not.toBeNull();

        const pinned = await sdk.pinFile(cid);
        expect(pinned).toBe(true);

        const isPinned = await sdk.isPinned(cid);
        expect(isPinned).toBe(true);
    });

    // Asset Tests
    test('Asset Operations', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const metadata = {
            name: 'Test Asset',
            description: 'Test Description',
            properties: {
                type: 'test',
                supply: '1000'
            }
        };

        const assetId = await sdk.createAsset(metadata);
        expect(assetId).not.toBeNull();

        const asset = await sdk.getAsset(assetId);
        expect(asset).not.toBeNull();
        expect(asset.metadata.name).toBe('Test Asset');
    });

    // Transaction Tests
    test('Transaction Operations', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const metadata = {
            type: 'transfer',
            description: 'Test Transfer'
        };

        const txId = await sdk.createTransaction(
            'transfer',
            metadata,
            'sender_address',
            'recipient_address',
            100
        );
        expect(txId).not.toBeNull();

        const tx = await sdk.getTransaction(txId);
        expect(tx).not.toBeNull();
        expect(tx.metadata.type).toBe('transfer');
    });

    // Concurrency Tests
    test('Concurrent Operations', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const operations = Array(10).fill(null).map(async () => {
            const metadata = {
                name: 'Test NFT',
                description: 'Test Description'
            };
            return await sdk.createNFT(metadata);
        });

        const results = await Promise.all(operations);
        expect(results.length).toBe(10);
        results.forEach(result => expect(result).not.toBeNull());
    });

    // Performance Tests
    test('Performance', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        const start = Date.now();
        const operations = Array(1000).fill(null).map(async () => {
            const metadata = {
                name: 'Test NFT',
                description: 'Test Description'
            };
            return await sdk.createNFT(metadata);
        });

        const results = await Promise.all(operations);
        const end = Date.now();
        const duration = end - start;

        expect(results.length).toBe(1000);
        expect(duration).toBeLessThan(5000); // Should complete in less than 5 seconds
    });

    // Error Handling Tests
    test('Error Handling', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        // Test invalid NFT creation
        const invalidMetadata = {
            name: 'Test NFT',
            description: 'A'.repeat(1024 * 1024 + 1) // Exceeds max metadata size
        };

        await expect(sdk.createNFT(invalidMetadata)).rejects.toThrow();

        // Test invalid transaction
        await expect(sdk.createTransaction(
            'invalid_type',
            {},
            'invalid_sender',
            'invalid_recipient',
            -1
        )).rejects.toThrow();
    });

    // Recovery Tests
    test('Recovery', async () => {
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        // Create test data
        const metadata = {
            name: 'Test NFT',
            description: 'Test Description'
        };
        const nftId = await sdk.createNFT(metadata);

        // Simulate failure
        await sdk.shutdown();

        // Reinitialize
        await sdk.initialize(
            blockchainConfig,
            nftConfig,
            ipfsConfig,
            assetConfig,
            transactionConfig
        );

        // Verify data is still accessible
        const nft = await sdk.getNFT(nftId);
        expect(nft).not.toBeNull();
        expect(nft.metadata.name).toBe('Test NFT');
    });
}); 