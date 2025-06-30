import { QuantumManager } from '../../src/quantum/QuantumManager';
import { SatoxError } from '../../src/types';

describe('QuantumManager', () => {
    let manager: QuantumManager;

    beforeEach(() => {
        manager = new QuantumManager();
        manager.initialize();
    });

    afterEach(() => {
        manager.shutdown();
    });

    describe('initialization', () => {
        it('should initialize and shutdown correctly', () => {
            const newManager = new QuantumManager();
            expect(newManager.isInitialized()).toBe(false);
            expect(newManager.initialize()).toBe(true);
            expect(newManager.isInitialized()).toBe(true);
            expect(newManager.shutdown()).toBe(true);
            expect(newManager.isInitialized()).toBe(false);
        });
    });

    describe('key pair generation', () => {
        it('should generate valid key pairs', async () => {
            const [publicKey, privateKey] = await manager.generateKeyPair();
            expect(publicKey).toBeDefined();
            expect(privateKey).toBeDefined();
            expect(publicKey.length).toBeGreaterThan(0);
            expect(privateKey.length).toBeGreaterThan(0);
        });
    });

    describe('encryption and decryption', () => {
        it('should encrypt and decrypt data correctly', async () => {
            const [publicKey, privateKey] = await manager.generateKeyPair();
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            const encrypted = await manager.encrypt(publicKey, data);
            expect(encrypted).toBeDefined();
            expect(encrypted).not.toEqual(data);
            
            const decrypted = await manager.decrypt(privateKey, encrypted);
            expect(decrypted).toEqual(data);
        });
    });

    describe('signing and verification', () => {
        it('should sign and verify data correctly', async () => {
            const [publicKey, privateKey] = await manager.generateKeyPair();
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            const signature = await manager.sign(privateKey, data);
            expect(signature).toBeDefined();
            expect(signature.length).toBeGreaterThan(0);
            
            expect(await manager.verify(publicKey, data, signature)).toBe(true);
            expect(await manager.verify(publicKey, data, new Uint8Array([0]))).toBe(false);
        });
    });

    describe('random number generation', () => {
        it('should generate random numbers within range', async () => {
            const min = 0;
            const max = 100;
            for (let i = 0; i < 100; i++) {
                const num = await manager.generateRandomNumber(min, max);
                expect(num).toBeGreaterThanOrEqual(min);
                expect(num).toBeLessThanOrEqual(max);
            }
        });
    });

    describe('random bytes generation', () => {
        it('should generate random bytes of specified length', async () => {
            const length = 32;
            const bytes1 = await manager.generateRandomBytes(length);
            const bytes2 = await manager.generateRandomBytes(length);
            
            expect(bytes1.length).toBe(length);
            expect(bytes2.length).toBe(length);
            expect(bytes1).not.toEqual(bytes2); // Very unlikely to be equal
        });
    });

    describe('version and algorithm info', () => {
        it('should return version string', () => {
            const version = manager.getVersion();
            expect(version).toBeDefined();
            expect(typeof version).toBe('string');
        });

        it('should return algorithm name', () => {
            const algorithm = manager.getAlgorithm();
            expect(algorithm).toBeDefined();
            expect(typeof algorithm).toBe('string');
        });

        it('should return available algorithms', () => {
            const algorithms = manager.getAvailableAlgorithms();
            expect(algorithms).toBeDefined();
            expect(Array.isArray(algorithms)).toBe(true);
            expect(algorithms.length).toBeGreaterThan(0);
        });
    });

    describe('error handling', () => {
        it('should throw when not initialized', async () => {
            const uninitializedManager = new QuantumManager();
            
            await expect(uninitializedManager.generateKeyPair())
                .rejects.toThrow(SatoxError);
            await expect(uninitializedManager.encrypt(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedManager.decrypt(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedManager.sign(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedManager.verify(new Uint8Array(), new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
        });

        it('should throw on invalid inputs', async () => {
            await expect(manager.encrypt(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(manager.decrypt(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(manager.sign(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(manager.verify(null as any, new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(manager.generateRandomNumber(10, 5)) // min > max
                .rejects.toThrow(SatoxError);
            await expect(manager.generateRandomBytes(0)) // invalid length
                .rejects.toThrow(SatoxError);
        });
    });
}); 