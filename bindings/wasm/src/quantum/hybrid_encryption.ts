import { Memory, Pointer, Result } from './types';
import { satox } from './satox';

export class HybridEncryption {
    private encryption: Pointer;
    private memory: Memory;

    constructor(memory: Memory) {
        this.memory = memory;
        this.encryption = satox.hybrid_encryption_create();
        if (!this.encryption) {
            throw new Error('Failed to create hybrid encryption');
        }
    }

    initialize(): void {
        const result = satox.hybrid_encryption_initialize(this.encryption);
        if (result !== 0) {
            throw new Error('Failed to initialize hybrid encryption');
        }
    }

    shutdown(): void {
        const result = satox.hybrid_encryption_shutdown(this.encryption);
        if (result !== 0) {
            throw new Error('Failed to shutdown hybrid encryption');
        }
    }

    generateKeyPair(): Result<{ publicKey: Uint8Array; privateKey: Uint8Array }> {
        const keyPair = satox.hybrid_encryption_generate_key_pair(this.encryption);
        if (!keyPair) {
            throw new Error('Failed to generate key pair');
        }

        const publicKey = this.memory.readBytes(keyPair.public_key, keyPair.public_key_len);
        const privateKey = this.memory.readBytes(keyPair.private_key, keyPair.private_key_len);

        this.memory.free(keyPair.public_key);
        this.memory.free(keyPair.private_key);

        return {
            publicKey,
            privateKey
        };
    }

    encrypt(publicKey: Uint8Array, data: Uint8Array): Uint8Array {
        const encrypted = satox.hybrid_encryption_encrypt(
            this.encryption,
            this.memory.writeBytes(publicKey),
            publicKey.length,
            this.memory.writeBytes(data),
            data.length
        );

        if (!encrypted) {
            throw new Error('Failed to encrypt data');
        }

        const result = this.memory.readBytes(encrypted.data, encrypted.length);
        this.memory.free(encrypted.data);

        return result;
    }

    decrypt(privateKey: Uint8Array, encrypted: Uint8Array): Uint8Array {
        const decrypted = satox.hybrid_encryption_decrypt(
            this.encryption,
            this.memory.writeBytes(privateKey),
            privateKey.length,
            this.memory.writeBytes(encrypted),
            encrypted.length
        );

        if (!decrypted) {
            throw new Error('Failed to decrypt data');
        }

        const result = this.memory.readBytes(decrypted.data, decrypted.length);
        this.memory.free(decrypted.data);

        return result;
    }

    getSessionKey(): Uint8Array {
        const sessionKey = satox.hybrid_encryption_get_session_key(this.encryption);
        if (!sessionKey) {
            throw new Error('Failed to get session key');
        }

        const result = this.memory.readBytes(sessionKey.data, sessionKey.length);
        this.memory.free(sessionKey.data);

        return result;
    }

    rotateSessionKey(): void {
        const result = satox.hybrid_encryption_rotate_session_key(this.encryption);
        if (result !== 0) {
            throw new Error('Failed to rotate session key');
        }
    }

    getVersion(): string {
        return satox.hybrid_encryption_get_version(this.encryption);
    }

    destroy(): void {
        if (this.encryption) {
            satox.hybrid_encryption_destroy(this.encryption);
            this.encryption = null;
        }
    }
} 