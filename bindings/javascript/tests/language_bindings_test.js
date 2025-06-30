const { expect } = require('chai');
const QuantumManager = require('../src/quantum/quantum_manager');
const PostQuantumAlgorithms = require('../src/quantum/post_quantum_algorithms');
const AssetManager = require('../src/assets/AssetManager');
const KeyStorage = require('../src/quantum/key_storage');
const HybridEncryption = require('../src/quantum/hybrid_encryption');

describe('Language Bindings', () => {
    let quantumManager;
    let postQuantumAlgorithms;
    let assetManager;
    let keyStorage;
    let hybridEncryption;

    beforeEach(() => {
        quantumManager = new QuantumManager();
        postQuantumAlgorithms = new PostQuantumAlgorithms();
        assetManager = new AssetManager();
        keyStorage = new KeyStorage();
        hybridEncryption = new HybridEncryption();
    });

    afterEach(() => {
        if (quantumManager) quantumManager.destroy();
        if (postQuantumAlgorithms) postQuantumAlgorithms.destroy();
        if (keyStorage) keyStorage.destroy();
        if (hybridEncryption) hybridEncryption.destroy();
    });

    // Basic Functionality Tests
    describe('Basic Functionality', () => {
        it('should initialize all managers', () => {
            expect(() => quantumManager.initialize()).to.not.throw();
            expect(() => postQuantumAlgorithms.isAlgorithmAvailable('NTRU')).to.not.throw();
            expect(() => assetManager.initialize()).to.not.throw();
            expect(() => keyStorage.initialize()).to.not.throw();
            expect(() => hybridEncryption.initialize()).to.not.throw();
        });

        it('should handle double initialization', () => {
            quantumManager.initialize();
            expect(() => quantumManager.initialize()).to.throw('Already initialized');
        });
    });

    // Quantum Manager Tests
    describe('Quantum Manager', () => {
        beforeEach(() => {
            quantumManager.initialize();
        });

        it('should generate key pairs', () => {
            const keyPair = quantumManager.generateKeyPair();
            expect(keyPair).to.have.property('publicKey');
            expect(keyPair).to.have.property('privateKey');
            expect(keyPair.publicKey).to.be.instanceof(Buffer);
            expect(keyPair.privateKey).to.be.instanceof(Buffer);
        });

        it('should encrypt and decrypt data', () => {
            const keyPair = quantumManager.generateKeyPair();
            const data = Buffer.from('test data');
            
            const encrypted = quantumManager.encrypt(keyPair.publicKey, data);
            expect(encrypted).to.be.instanceof(Buffer);
            
            const decrypted = quantumManager.decrypt(keyPair.privateKey, encrypted);
            expect(decrypted).to.be.instanceof(Buffer);
            expect(decrypted.toString()).to.equal('test data');
        });

        it('should sign and verify data', () => {
            const keyPair = quantumManager.generateKeyPair();
            const data = Buffer.from('test data');
            
            const signature = quantumManager.sign(keyPair.privateKey, data);
            expect(signature).to.be.instanceof(Buffer);
            
            const verified = quantumManager.verify(keyPair.publicKey, data, signature);
            expect(verified).to.be.true;
        });
    });

    // Post-Quantum Algorithms Tests
    describe('Post-Quantum Algorithms', () => {
        it('should check algorithm availability', () => {
            expect(postQuantumAlgorithms.isAlgorithmAvailable('NTRU')).to.be.true;
            expect(postQuantumAlgorithms.isAlgorithmAvailable('BIKE')).to.be.true;
            expect(postQuantumAlgorithms.isAlgorithmAvailable('HQC')).to.be.true;
        });

        it('should generate key pairs for different algorithms', () => {
            const algorithms = ['NTRU', 'BIKE', 'HQC'];
            algorithms.forEach(algorithm => {
                const keyPair = postQuantumAlgorithms.generateKeyPair(algorithm);
                expect(keyPair).to.have.property('publicKey');
                expect(keyPair).to.have.property('privateKey');
            });
        });

        it('should sign and verify with different algorithms', () => {
            const algorithms = ['NTRU', 'BIKE', 'HQC'];
            algorithms.forEach(algorithm => {
                const keyPair = postQuantumAlgorithms.generateKeyPair(algorithm);
                const data = Buffer.from('test data');
                
                const signature = postQuantumAlgorithms.sign(algorithm, keyPair.privateKey, data);
                expect(signature).to.be.instanceof(Buffer);
                
                const verified = postQuantumAlgorithms.verify(algorithm, keyPair.publicKey, data, signature);
                expect(verified).to.be.true;
            });
        });
    });

    // Asset Manager Tests
    describe('Asset Manager', () => {
        beforeEach(() => {
            assetManager.initialize();
        });

        it('should create and validate assets', () => {
            const asset = {
                id: 'test-asset',
                name: 'Test Asset',
                totalSupply: 1000
            };
            
            expect(assetManager.validateAsset(asset)).to.be.true;
        });

        it('should transfer assets', () => {
            const assetId = 'test-asset';
            const from = 'address1';
            const to = 'address2';
            const amount = 100;

            assetManager.assets.set(assetId, {
                id: assetId,
                name: 'Test Asset',
                totalSupply: 1000
            });
            assetManager.balances.set(`${assetId}:${from}`, 200);

            const transfer = assetManager.transferAsset(assetId, from, to, amount);
            expect(transfer).to.have.property('assetId', assetId);
            expect(transfer).to.have.property('from', from);
            expect(transfer).to.have.property('to', to);
            expect(transfer).to.have.property('amount', amount);
        });

        it('should get asset history', () => {
            const assetId = 'test-asset';
            assetManager.assets.set(assetId, {
                id: assetId,
                name: 'Test Asset',
                totalSupply: 1000
            });
            assetManager.balances.set(`${assetId}:address1`, 100);

            const history = assetManager.getAssetHistory(assetId);
            expect(history).to.have.property('asset');
            expect(history).to.have.property('transfers');
            expect(history.transfers).to.be.an('array');
        });
    });

    // Error Handling Tests
    describe('Error Handling', () => {
        it('should handle invalid initialization', () => {
            expect(() => {
                const invalidManager = new QuantumManager();
                invalidManager.manager = null;
                invalidManager.initialize();
            }).to.throw('Failed to initialize quantum manager');
        });

        it('should handle invalid key pairs', () => {
            expect(() => {
                quantumManager.encrypt(Buffer.from('invalid'), Buffer.from('data'));
            }).to.throw('Failed to encrypt data');
        });

        it('should handle invalid assets', () => {
            expect(() => {
                assetManager.validateAsset(null);
            }).to.throw('Invalid asset');
        });
    });

    // Concurrency Tests
    describe('Concurrency', () => {
        it('should handle concurrent key generation', async () => {
            const promises = Array(10).fill().map(() => 
                quantumManager.generateKeyPair()
            );
            
            const keyPairs = await Promise.all(promises);
            expect(keyPairs).to.have.length(10);
            keyPairs.forEach(keyPair => {
                expect(keyPair).to.have.property('publicKey');
                expect(keyPair).to.have.property('privateKey');
            });
        });

        it('should handle concurrent asset transfers', async () => {
            const assetId = 'test-asset';
            assetManager.assets.set(assetId, {
                id: assetId,
                name: 'Test Asset',
                totalSupply: 1000
            });
            assetManager.balances.set(`${assetId}:address1`, 500);

            const promises = Array(5).fill().map((_, i) => 
                assetManager.transferAsset(assetId, 'address1', `address${i + 2}`, 50)
            );
            
            const transfers = await Promise.all(promises);
            expect(transfers).to.have.length(5);
            transfers.forEach(transfer => {
                expect(transfer).to.have.property('amount', 50);
            });
        });
    });

    // Performance Tests
    describe('Performance', () => {
        it('should generate key pairs efficiently', () => {
            const startTime = Date.now();
            for (let i = 0; i < 100; i++) {
                quantumManager.generateKeyPair();
            }
            const endTime = Date.now();
            const duration = endTime - startTime;
            
            // Expect to generate at least 10 key pairs per second
            expect(duration).to.be.lessThan(10000);
        });

        it('should handle asset transfers efficiently', () => {
            const assetId = 'test-asset';
            assetManager.assets.set(assetId, {
                id: assetId,
                name: 'Test Asset',
                totalSupply: 1000
            });
            assetManager.balances.set(`${assetId}:address1`, 1000);

            const startTime = Date.now();
            for (let i = 0; i < 100; i++) {
                assetManager.transferAsset(assetId, 'address1', `address${i + 2}`, 1);
            }
            const endTime = Date.now();
            const duration = endTime - startTime;
            
            // Expect to process at least 10 transfers per second
            expect(duration).to.be.lessThan(10000);
        });
    });

    // Recovery Tests
    describe('Recovery', () => {
        it('should recover from shutdown', () => {
            quantumManager.initialize();
            const keyPair = quantumManager.generateKeyPair();
            quantumManager.shutdown();
            
            quantumManager.initialize();
            const newKeyPair = quantumManager.generateKeyPair();
            expect(newKeyPair).to.have.property('publicKey');
            expect(newKeyPair).to.have.property('privateKey');
        });

        it('should recover from errors', () => {
            assetManager.initialize();
            const assetId = 'test-asset';
            
            try {
                assetManager.transferAsset(assetId, 'address1', 'address2', 100);
            } catch (error) {
                expect(error.message).to.equal('Asset not found');
            }
            
            assetManager.assets.set(assetId, {
                id: assetId,
                name: 'Test Asset',
                totalSupply: 1000
            });
            assetManager.balances.set(`${assetId}:address1`, 200);
            
            const transfer = assetManager.transferAsset(assetId, 'address1', 'address2', 100);
            expect(transfer).to.have.property('amount', 100);
        });
    });
}); 