import { PostQuantumAlgorithms, AlgorithmInfo } from '../../src/quantum/PostQuantumAlgorithms';
import { SatoxError } from '../../src/types';

describe('PostQuantumAlgorithms', () => {
    let algorithms: PostQuantumAlgorithms;

    beforeEach(() => {
        algorithms = new PostQuantumAlgorithms();
        algorithms.initialize();
    });

    afterEach(() => {
        algorithms.shutdown();
    });

    describe('initialization', () => {
        it('should initialize and shutdown correctly', () => {
            const newAlgorithms = new PostQuantumAlgorithms();
            expect(newAlgorithms.isInitialized()).toBe(false);
            expect(newAlgorithms.initialize()).toBe(true);
            expect(newAlgorithms.isInitialized()).toBe(true);
            expect(newAlgorithms.shutdown()).toBe(true);
            expect(newAlgorithms.isInitialized()).toBe(false);
        });
    });

    describe('key pair generation', () => {
        it('should generate valid key pairs', async () => {
            const [publicKey, privateKey] = await algorithms.generateKeyPair('test-algo');
            expect(publicKey).toBeDefined();
            expect(privateKey).toBeDefined();
            expect(publicKey.length).toBeGreaterThan(0);
            expect(privateKey.length).toBeGreaterThan(0);
        });
    });

    describe('signing and verification', () => {
        it('should sign and verify data correctly', async () => {
            const [publicKey, privateKey] = await algorithms.generateKeyPair('test-algo');
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            const signature = await algorithms.sign(privateKey, data);
            expect(signature).toBeDefined();
            expect(signature.length).toBeGreaterThan(0);
            
            expect(await algorithms.verify(publicKey, data, signature)).toBe(true);
            expect(await algorithms.verify(publicKey, data, new Uint8Array([0]))).toBe(false);
        });
    });

    describe('encryption and decryption', () => {
        it('should encrypt and decrypt data correctly', async () => {
            const [publicKey, privateKey] = await algorithms.generateKeyPair('test-algo');
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            const encrypted = await algorithms.encrypt(publicKey, data);
            expect(encrypted).toBeDefined();
            expect(encrypted).not.toEqual(data);
            
            const decrypted = await algorithms.decrypt(privateKey, encrypted);
            expect(decrypted).toEqual(data);
        });
    });

    describe('algorithm information', () => {
        it('should get algorithm info', () => {
            const info = algorithms.getAlgorithmInfo('test-algo');
            expect(info).toBeDefined();
            expect(info.name).toBe('test-algo');
            expect(info.securityLevel).toBeGreaterThan(0);
            expect(info.keySize).toBeGreaterThan(0);
            expect(info.signatureSize).toBeGreaterThan(0);
            expect(typeof info.isRecommended).toBe('boolean');
            expect(typeof info.description).toBe('string');
        });

        it('should get available algorithms', () => {
            const available = algorithms.getAvailableAlgorithms();
            expect(available).toBeDefined();
            expect(Array.isArray(available)).toBe(true);
            expect(available.length).toBeGreaterThan(0);
        });

        it('should get recommended algorithms', () => {
            const recommended = algorithms.getRecommendedAlgorithms();
            expect(recommended).toBeDefined();
            expect(Array.isArray(recommended)).toBe(true);
            expect(recommended.length).toBeGreaterThan(0);
        });
    });

    describe('algorithm checks', () => {
        it('should check algorithm availability', () => {
            expect(algorithms.isAlgorithmAvailable('test-algo')).toBe(true);
            expect(algorithms.isAlgorithmAvailable('non-existent')).toBe(false);
        });

        it('should check algorithm recommendation', () => {
            const isRecommended = algorithms.isAlgorithmRecommended('test-algo');
            expect(typeof isRecommended).toBe('boolean');
        });
    });

    describe('algorithm properties', () => {
        it('should get security level', () => {
            const level = algorithms.getAlgorithmSecurityLevel('test-algo');
            expect(level).toBeGreaterThan(0);
        });

        it('should get key size', () => {
            const size = algorithms.getAlgorithmKeySize('test-algo');
            expect(size).toBeGreaterThan(0);
        });

        it('should get signature size', () => {
            const size = algorithms.getAlgorithmSignatureSize('test-algo');
            expect(size).toBeGreaterThan(0);
        });
    });

    describe('default algorithm', () => {
        it('should get and set default algorithm', () => {
            const defaultAlgo = algorithms.getDefaultAlgorithm();
            expect(defaultAlgo).toBeDefined();
            expect(typeof defaultAlgo).toBe('string');
            
            expect(algorithms.setDefaultAlgorithm('test-algo')).toBe(true);
            expect(algorithms.getDefaultAlgorithm()).toBe('test-algo');
        });
    });

    describe('error handling', () => {
        it('should throw when not initialized', async () => {
            const uninitializedAlgorithms = new PostQuantumAlgorithms();
            
            await expect(uninitializedAlgorithms.generateKeyPair('test-algo'))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedAlgorithms.sign(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedAlgorithms.verify(new Uint8Array(), new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedAlgorithms.encrypt(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedAlgorithms.decrypt(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
        });

        it('should throw on invalid inputs', async () => {
            await expect(algorithms.generateKeyPair(''))
                .rejects.toThrow(SatoxError);
            await expect(algorithms.sign(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(algorithms.verify(null as any, new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(algorithms.encrypt(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(algorithms.decrypt(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
        });
    });

    describe('algorithm consistency', () => {
        it('should maintain consistency across operations', async () => {
            const [publicKey, privateKey] = await algorithms.generateKeyPair('test-algo');
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            // Test encryption/decryption consistency
            const encrypted = await algorithms.encrypt(publicKey, data);
            const decrypted = await algorithms.decrypt(privateKey, encrypted);
            expect(decrypted).toEqual(data);
            
            // Test signing/verification consistency
            const signature = await algorithms.sign(privateKey, data);
            expect(await algorithms.verify(publicKey, data, signature)).toBe(true);
        });
    });
}); 