import { HybridEncryption } from '../../src/quantum/HybridEncryption';
import { SatoxError } from '../../src/types';

describe('HybridEncryption', () => {
    let hybrid: HybridEncryption;

    beforeEach(() => {
        hybrid = new HybridEncryption();
        hybrid.initialize();
    });

    afterEach(() => {
        hybrid.shutdown();
    });

    describe('initialization', () => {
        it('should initialize and shutdown correctly', () => {
            const newHybrid = new HybridEncryption();
            expect(newHybrid.isInitialized()).toBe(false);
            expect(newHybrid.initialize()).toBe(true);
            expect(newHybrid.isInitialized()).toBe(true);
            expect(newHybrid.shutdown()).toBe(true);
            expect(newHybrid.isInitialized()).toBe(false);
        });
    });

    describe('key pair generation', () => {
        it('should generate valid key pairs', async () => {
            const [publicKey, privateKey] = await hybrid.generateKeyPair();
            expect(publicKey).toBeDefined();
            expect(privateKey).toBeDefined();
            expect(publicKey.length).toBeGreaterThan(0);
            expect(privateKey.length).toBeGreaterThan(0);
        });

        it('should generate unique key pairs', async () => {
            const [pub1, priv1] = await hybrid.generateKeyPair();
            const [pub2, priv2] = await hybrid.generateKeyPair();
            expect(pub1).not.toEqual(pub2);
            expect(priv1).not.toEqual(priv2);
        });
    });

    describe('encryption and decryption', () => {
        it('should encrypt and decrypt data correctly', async () => {
            const [publicKey, privateKey] = await hybrid.generateKeyPair();
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            const encrypted = await hybrid.encrypt(publicKey, data);
            expect(encrypted).toBeDefined();
            expect(encrypted).not.toEqual(data);
            
            const decrypted = await hybrid.decrypt(privateKey, encrypted);
            expect(decrypted).toEqual(data);
        });

        it('should produce different ciphertexts for same plaintext', async () => {
            const [publicKey] = await hybrid.generateKeyPair();
            const data = new Uint8Array([1, 2, 3, 4, 5]);
            
            const encrypted1 = await hybrid.encrypt(publicKey, data);
            const encrypted2 = await hybrid.encrypt(publicKey, data);
            expect(encrypted1).not.toEqual(encrypted2);
        });
    });

    describe('session key management', () => {
        it('should get and rotate session keys', async () => {
            const sessionKey1 = await hybrid.getSessionKey();
            expect(sessionKey1).toBeDefined();
            expect(sessionKey1.length).toBeGreaterThan(0);

            expect(await hybrid.rotateSessionKey()).toBe(true);
            const sessionKey2 = await hybrid.getSessionKey();
            expect(sessionKey2).toBeDefined();
            expect(sessionKey2).not.toEqual(sessionKey1);
        });
    });

    describe('version info', () => {
        it('should return version string', () => {
            const version = hybrid.getVersion();
            expect(version).toBeDefined();
            expect(typeof version).toBe('string');
        });
    });

    describe('error handling', () => {
        it('should throw when not initialized', async () => {
            const uninitializedHybrid = new HybridEncryption();
            
            await expect(uninitializedHybrid.generateKeyPair())
                .rejects.toThrow(SatoxError);
            await expect(uninitializedHybrid.encrypt(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedHybrid.decrypt(new Uint8Array(), new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(uninitializedHybrid.getSessionKey())
                .rejects.toThrow(SatoxError);
            await expect(uninitializedHybrid.rotateSessionKey())
                .rejects.toThrow(SatoxError);
        });

        it('should throw on invalid inputs', async () => {
            await expect(hybrid.encrypt(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
            await expect(hybrid.decrypt(null as any, new Uint8Array()))
                .rejects.toThrow(SatoxError);
        });
    });
}); 