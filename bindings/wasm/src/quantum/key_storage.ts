import { Memory, Pointer, Result } from './types';
import { satox } from './satox';

export interface KeyMetadata {
    algorithm: string;
    creationTime: number;
    expiration: number;
    accessLevels: number;
    tags: string[];
}

export class KeyStorage {
    private storage: Pointer;
    private memory: Memory;

    constructor(memory: Memory) {
        this.memory = memory;
        this.storage = satox.key_storage_create();
        if (!this.storage) {
            throw new Error('Failed to create key storage');
        }
    }

    initialize(): void {
        const result = satox.key_storage_initialize(this.storage);
        if (result !== 0) {
            throw new Error('Failed to initialize key storage');
        }
    }

    shutdown(): void {
        const result = satox.key_storage_shutdown(this.storage);
        if (result !== 0) {
            throw new Error('Failed to shutdown key storage');
        }
    }

    storeKey(keyId: string, key: Uint8Array, metadata: KeyMetadata): void {
        const result = satox.key_storage_store_key(
            this.storage,
            this.memory.writeString(keyId),
            this.memory.writeBytes(key),
            key.length,
            {
                algorithm: this.memory.writeString(metadata.algorithm),
                creation_time: metadata.creationTime,
                expiration: metadata.expiration,
                access_levels: metadata.accessLevels,
                tags: metadata.tags.map(tag => this.memory.writeString(tag)),
                tags_count: metadata.tags.length
            }
        );

        if (result !== 0) {
            throw new Error('Failed to store key');
        }
    }

    retrieveKey(keyId: string): Uint8Array {
        const key = satox.key_storage_retrieve_key(
            this.storage,
            this.memory.writeString(keyId)
        );

        if (!key) {
            throw new Error('Failed to retrieve key');
        }

        const result = this.memory.readBytes(key.data, key.length);
        this.memory.free(key.data);

        return result;
    }

    deleteKey(keyId: string): void {
        const result = satox.key_storage_delete_key(
            this.storage,
            this.memory.writeString(keyId)
        );

        if (result !== 0) {
            throw new Error('Failed to delete key');
        }
    }

    updateKey(keyId: string, key: Uint8Array): void {
        const result = satox.key_storage_update_key(
            this.storage,
            this.memory.writeString(keyId),
            this.memory.writeBytes(key),
            key.length
        );

        if (result !== 0) {
            throw new Error('Failed to update key');
        }
    }

    getKeyMetadata(keyId: string): KeyMetadata {
        const metadata = satox.key_storage_get_key_metadata(
            this.storage,
            this.memory.writeString(keyId)
        );

        if (!metadata) {
            throw new Error('Failed to get key metadata');
        }

        const result = {
            algorithm: this.memory.readString(metadata.algorithm),
            creationTime: metadata.creation_time,
            expiration: metadata.expiration,
            accessLevels: metadata.access_levels,
            tags: []
        };

        if (metadata.tags) {
            for (let i = 0; i < metadata.tags_count; i++) {
                result.tags.push(this.memory.readString(metadata.tags[i]));
            }
        }

        return result;
    }

    rotateKey(keyId: string): void {
        const result = satox.key_storage_rotate_key(
            this.storage,
            this.memory.writeString(keyId)
        );

        if (result !== 0) {
            throw new Error('Failed to rotate key');
        }
    }

    reencryptKey(keyId: string, newKey: Uint8Array): void {
        const result = satox.key_storage_reencrypt_key(
            this.storage,
            this.memory.writeString(keyId),
            this.memory.writeBytes(newKey),
            newKey.length
        );

        if (result !== 0) {
            throw new Error('Failed to reencrypt key');
        }
    }

    validateKey(keyId: string): void {
        const result = satox.key_storage_validate_key(
            this.storage,
            this.memory.writeString(keyId)
        );

        if (result !== 0) {
            throw new Error('Failed to validate key');
        }
    }

    isKeyExpired(keyId: string): boolean {
        const result = satox.key_storage_is_key_expired(
            this.storage,
            this.memory.writeString(keyId)
        );

        return result === 1;
    }

    destroy(): void {
        if (this.storage) {
            satox.key_storage_destroy(this.storage);
            this.storage = null;
        }
    }
} 