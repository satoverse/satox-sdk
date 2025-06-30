const { createRequire } = require('module');
const require = createRequire(import.meta.url);
const satox = require('satox');

class KeyStorage {
    constructor() {
        this.storage = satox.key_storage_create();
        if (!this.storage) {
            throw new Error('Failed to create key storage');
        }
    }

    initialize() {
        const result = satox.key_storage_initialize(this.storage);
        if (result !== 0) {
            throw new Error('Failed to initialize key storage');
        }
    }

    shutdown() {
        const result = satox.key_storage_shutdown(this.storage);
        if (result !== 0) {
            throw new Error('Failed to shutdown key storage');
        }
    }

    storeKey(keyId, key, metadata) {
        const result = satox.key_storage_store_key(
            this.storage,
            keyId,
            key,
            key.length,
            {
                algorithm: metadata.algorithm,
                creation_time: metadata.creationTime,
                expiration: metadata.expiration,
                access_levels: metadata.accessLevels,
                tags: metadata.tags,
                tags_count: metadata.tags ? metadata.tags.length : 0
            }
        );

        if (result !== 0) {
            throw new Error('Failed to store key');
        }
    }

    retrieveKey(keyId) {
        const key = satox.key_storage_retrieve_key(this.storage, keyId);
        if (!key) {
            throw new Error('Failed to retrieve key');
        }

        return Buffer.from(key.data, key.length);
    }

    deleteKey(keyId) {
        const result = satox.key_storage_delete_key(this.storage, keyId);
        if (result !== 0) {
            throw new Error('Failed to delete key');
        }
    }

    updateKey(keyId, key) {
        const result = satox.key_storage_update_key(
            this.storage,
            keyId,
            key,
            key.length
        );

        if (result !== 0) {
            throw new Error('Failed to update key');
        }
    }

    getKeyMetadata(keyId) {
        const metadata = satox.key_storage_get_key_metadata(this.storage, keyId);
        if (!metadata) {
            throw new Error('Failed to get key metadata');
        }

        return {
            algorithm: metadata.algorithm,
            creationTime: metadata.creation_time,
            expiration: metadata.expiration,
            accessLevels: metadata.access_levels,
            tags: metadata.tags ? Array.from(metadata.tags) : []
        };
    }

    rotateKey(keyId) {
        const result = satox.key_storage_rotate_key(this.storage, keyId);
        if (result !== 0) {
            throw new Error('Failed to rotate key');
        }
    }

    reencryptKey(keyId, newKey) {
        const result = satox.key_storage_reencrypt_key(
            this.storage,
            keyId,
            newKey,
            newKey.length
        );

        if (result !== 0) {
            throw new Error('Failed to reencrypt key');
        }
    }

    validateKey(keyId) {
        const result = satox.key_storage_validate_key(this.storage, keyId);
        if (result !== 0) {
            throw new Error('Failed to validate key');
        }
    }

    isKeyExpired(keyId) {
        const result = satox.key_storage_is_key_expired(this.storage, keyId);
        return result === 1;
    }

    destroy() {
        if (this.storage) {
            satox.key_storage_destroy(this.storage);
            this.storage = null;
        }
    }
}

export default KeyStorage; 