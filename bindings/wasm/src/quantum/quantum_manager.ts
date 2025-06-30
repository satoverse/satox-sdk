import { Memory, Pointer, Result } from './types';
import { satox } from './satox';

export class QuantumManager {
    private manager: Pointer;
    private memory: Memory;

    constructor(memory: Memory) {
        this.memory = memory;
        this.manager = satox.quantum_manager_create();
        if (!this.manager) {
            throw new Error('Failed to create quantum manager');
        }
    }

    initialize(): void {
        const result = satox.quantum_manager_initialize(this.manager);
        if (result !== 0) {
            throw new Error('Failed to initialize quantum manager');
        }
    }

    shutdown(): void {
        const result = satox.quantum_manager_shutdown(this.manager);
        if (result !== 0) {
            throw new Error('Failed to shutdown quantum manager');
        }
    }

    generateKeyPair(): Result<{ publicKey: Uint8Array; privateKey: Uint8Array }> {
        const keyPair = satox.quantum_manager_generate_key_pair(this.manager);
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
        const encrypted = satox.quantum_manager_encrypt(
            this.manager,
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
        const decrypted = satox.quantum_manager_decrypt(
            this.manager,
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

    sign(privateKey: Uint8Array, data: Uint8Array): Uint8Array {
        const signature = satox.quantum_manager_sign(
            this.manager,
            this.memory.writeBytes(privateKey),
            privateKey.length,
            this.memory.writeBytes(data),
            data.length
        );

        if (!signature) {
            throw new Error('Failed to sign data');
        }

        const result = this.memory.readBytes(signature.data, signature.length);
        this.memory.free(signature.data);

        return result;
    }

    verify(publicKey: Uint8Array, data: Uint8Array, signature: Uint8Array): boolean {
        const result = satox.quantum_manager_verify(
            this.manager,
            this.memory.writeBytes(publicKey),
            publicKey.length,
            this.memory.writeBytes(data),
            data.length,
            this.memory.writeBytes(signature),
            signature.length
        );

        return result === 0;
    }

    generateRandomBytes(length: number): Uint8Array {
        const randomBytes = satox.quantum_manager_generate_random_bytes(
            this.manager,
            length
        );

        if (!randomBytes) {
            throw new Error('Failed to generate random bytes');
        }

        const result = this.memory.readBytes(randomBytes.data, length);
        this.memory.free(randomBytes.data);

        return result;
    }

    getVersion(): string {
        return satox.quantum_manager_get_version(this.manager);
    }

    destroy(): void {
        if (this.manager) {
            satox.quantum_manager_destroy(this.manager);
            this.manager = null;
        }
    }
} 