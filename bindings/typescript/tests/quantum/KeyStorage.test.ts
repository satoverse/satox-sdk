import { KeyStorage, KeyMetadata } from '../../src/quantum/KeyStorage';
import { SatoxError } from '../../src/types';

describe('KeyStorage', () => {
    let storage: KeyStorage;
    const testKey = new Uint8Array([1, 2, 3, 4, 5]);
    const testMetadata: KeyMetadata = {
        algorithm: 'test-algo',
        creationTime: Date.now(),
        expiration: Date.now() + 3600000,
        accessLevels: ['read', 'write'],
        tags: ['test', 'key']
    };

    beforeEach(() => {
        storage = new KeyStorage();
        storage.initialize();
    });

    afterEach(() => {
        storage.shutdown();
    });

    describe('initialization', () => {
        it('should initialize and shutdown correctly', () => {
            const newStorage = new KeyStorage();
            expect(newStorage.isInitialized()).toBe(false);
            expect(newStorage.initialize()).toBe(true);
            expect(newStorage.isInitialized()).toBe(true);
            expect(newStorage.shutdown()).toBe(true);
            expect(newStorage.isInitialized()).toBe(false);
        });
    });

    describe('key operations', () => {
        it('should store and retrieve keys', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            expect(keyId).toBeDefined();
            
            const retrievedKey = await storage.getKey(keyId);
            expect(retrievedKey).toEqual(testKey);
        });

        it('should delete keys', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            expect(await storage.deleteKey(keyId)).toBe(true);
            await expect(storage.getKey(keyId)).rejects.toThrow(SatoxError);
        });

        it('should update keys', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            const newKey = new Uint8Array([6, 7, 8, 9, 10]);
            
            expect(await storage.updateKey(keyId, newKey)).toBe(true);
            const updatedKey = await storage.getKey(keyId);
            expect(updatedKey).toEqual(newKey);
        });
    });

    describe('metadata operations', () => {
        it('should get and update metadata', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            
            const retrievedMetadata = await storage.getKeyMetadata(keyId);
            expect(retrievedMetadata).toEqual(testMetadata);
            
            const newMetadata: KeyMetadata = {
                ...testMetadata,
                tags: ['updated', 'key']
            };
            expect(await storage.updateKeyMetadata(keyId, newMetadata)).toBe(true);
            
            const updatedMetadata = await storage.getKeyMetadata(keyId);
            expect(updatedMetadata).toEqual(newMetadata);
        });
    });

    describe('key rotation', () => {
        it('should rotate keys', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            const newKey = new Uint8Array([6, 7, 8, 9, 10]);
            
            expect(await storage.rotateKey(keyId, newKey)).toBe(true);
            const rotatedKey = await storage.getKey(keyId);
            expect(rotatedKey).toEqual(newKey);
        });
    });

    describe('key reencryption', () => {
        it('should reencrypt keys', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            expect(await storage.reencryptKey(keyId)).toBe(true);
        });
    });

    describe('key validation', () => {
        it('should validate keys', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            expect(await storage.validateKey(keyId)).toBe(true);
        });
    });

    describe('key expiration', () => {
        it('should handle key expiration', async () => {
            const expiredMetadata: KeyMetadata = {
                ...testMetadata,
                expiration: Date.now() - 1000 // Expired
            };
            const keyId = await storage.storeKey(testKey, expiredMetadata);
            expect(await storage.isKeyExpired(keyId)).toBe(true);
        });
    });

    describe('access control', () => {
        it('should manage access levels', async () => {
            const keyId = await storage.storeKey(testKey, testMetadata);
            
            expect(await storage.hasAccess(keyId, 'read')).toBe(true);
            expect(await storage.hasAccess(keyId, 'admin')).toBe(false);
            
            expect(await storage.addAccessLevel(keyId, 'admin')).toBe(true);
            expect(await storage.hasAccess(keyId, 'admin')).toBe(true);
            
            expect(await storage.removeAccessLevel(keyId, 'admin')).toBe(true);
            expect(await storage.hasAccess(keyId, 'admin')).toBe(false);
        });
    });

    describe('key counting', () => {
        it('should count keys', async () => {
            expect(await storage.getKeyCount()).toBe(0);
            
            await storage.storeKey(testKey, testMetadata);
            expect(await storage.getKeyCount()).toBe(1);
            
            await storage.storeKey(new Uint8Array([6, 7, 8]), testMetadata);
            expect(await storage.getKeyCount()).toBe(2);
        });
    });

    describe('key listing', () => {
        it('should list all key IDs', async () => {
            const keyId1 = await storage.storeKey(testKey, testMetadata);
            const keyId2 = await storage.storeKey(new Uint8Array([6, 7, 8]), testMetadata);
            
            const keyIds = await storage.getAllKeyIds();
            expect(keyIds).toContain(keyId1);
            expect(keyIds).toContain(keyId2);
        });
    });

    describe('error handling', () => {
        it('should throw when not initialized', async () => {
            const uninitializedStorage = new KeyStorage();
            
            await expect(uninitializedStorage.storeKey(testKey, testMetadata))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedStorage.getKey('test-id'))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedStorage.deleteKey('test-id'))
                .rejects.toThrow(SatoxError);
        });

        it('should throw on invalid inputs', async () => {
            await expect(storage.storeKey(null as any, testMetadata))
                .rejects.toThrow(SatoxError);
            await expect(storage.storeKey(testKey, null as any))
                .rejects.toThrow(SatoxError);
            await expect(storage.getKey(''))
                .rejects.toThrow(SatoxError);
            await expect(storage.deleteKey(''))
                .rejects.toThrow(SatoxError);
        });
    });
}); 