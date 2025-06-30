const { createRequire } = require('module');
const require = createRequire(import.meta.url);
const satox = require('satox');

class QuantumManager {
    constructor() {
        this.manager = satox.quantum_manager_create();
        if (!this.manager) {
            throw new Error('Failed to create quantum manager');
        }
    }

    initialize() {
        const result = satox.quantum_manager_initialize(this.manager);
        if (result !== 0) {
            throw new Error('Failed to initialize quantum manager');
        }
    }

    shutdown() {
        const result = satox.quantum_manager_shutdown(this.manager);
        if (result !== 0) {
            throw new Error('Failed to shutdown quantum manager');
        }
    }

    generateKeyPair() {
        const keyPair = satox.quantum_manager_generate_key_pair(this.manager);
        if (!keyPair) {
            throw new Error('Failed to generate key pair');
        }

        return {
            publicKey: Buffer.from(keyPair.public_key, keyPair.public_key_len),
            privateKey: Buffer.from(keyPair.private_key, keyPair.private_key_len)
        };
    }

    encrypt(publicKey, data) {
        const encrypted = satox.quantum_manager_encrypt(
            this.manager,
            publicKey,
            publicKey.length,
            data,
            data.length
        );

        if (!encrypted) {
            throw new Error('Failed to encrypt data');
        }

        return Buffer.from(encrypted.data, encrypted.length);
    }

    decrypt(privateKey, encrypted) {
        const decrypted = satox.quantum_manager_decrypt(
            this.manager,
            privateKey,
            privateKey.length,
            encrypted,
            encrypted.length
        );

        if (!decrypted) {
            throw new Error('Failed to decrypt data');
        }

        return Buffer.from(decrypted.data, decrypted.length);
    }

    sign(privateKey, data) {
        const signature = satox.quantum_manager_sign(
            this.manager,
            privateKey,
            privateKey.length,
            data,
            data.length
        );

        if (!signature) {
            throw new Error('Failed to sign data');
        }

        return Buffer.from(signature.data, signature.length);
    }

    verify(publicKey, data, signature) {
        const result = satox.quantum_manager_verify(
            this.manager,
            publicKey,
            publicKey.length,
            data,
            data.length,
            signature,
            signature.length
        );

        return result === 0;
    }

    generateRandomBytes(length) {
        const randomBytes = satox.quantum_manager_generate_random_bytes(
            this.manager,
            length
        );

        if (!randomBytes) {
            throw new Error('Failed to generate random bytes');
        }

        return Buffer.from(randomBytes.data, length);
    }

    getVersion() {
        return satox.quantum_manager_get_version(this.manager);
    }

    destroy() {
        if (this.manager) {
            satox.quantum_manager_destroy(this.manager);
            this.manager = null;
        }
    }
}

export default QuantumManager; 