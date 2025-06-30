const { createRequire } = require('module');
const require = createRequire(import.meta.url);
const satox = require('satox');

class HybridEncryption {
    constructor() {
        this.encryption = satox.hybrid_encryption_create();
        if (!this.encryption) {
            throw new Error('Failed to create hybrid encryption');
        }
    }

    initialize() {
        const result = satox.hybrid_encryption_initialize(this.encryption);
        if (result !== 0) {
            throw new Error('Failed to initialize hybrid encryption');
        }
    }

    shutdown() {
        const result = satox.hybrid_encryption_shutdown(this.encryption);
        if (result !== 0) {
            throw new Error('Failed to shutdown hybrid encryption');
        }
    }

    generateKeyPair() {
        const keyPair = satox.hybrid_encryption_generate_key_pair(this.encryption);
        if (!keyPair) {
            throw new Error('Failed to generate key pair');
        }

        return {
            publicKey: Buffer.from(keyPair.public_key, keyPair.public_key_len),
            privateKey: Buffer.from(keyPair.private_key, keyPair.private_key_len)
        };
    }

    encrypt(publicKey, data) {
        const encrypted = satox.hybrid_encryption_encrypt(
            this.encryption,
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
        const decrypted = satox.hybrid_encryption_decrypt(
            this.encryption,
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

    getSessionKey() {
        const sessionKey = satox.hybrid_encryption_get_session_key(this.encryption);
        if (!sessionKey) {
            throw new Error('Failed to get session key');
        }

        return Buffer.from(sessionKey.data, sessionKey.length);
    }

    rotateSessionKey() {
        const result = satox.hybrid_encryption_rotate_session_key(this.encryption);
        if (result !== 0) {
            throw new Error('Failed to rotate session key');
        }
    }

    getVersion() {
        return satox.hybrid_encryption_get_version(this.encryption);
    }

    destroy() {
        if (this.encryption) {
            satox.hybrid_encryption_destroy(this.encryption);
            this.encryption = null;
        }
    }
}

export default HybridEncryption; 