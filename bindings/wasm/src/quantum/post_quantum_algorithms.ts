import { Memory, Pointer, Result } from './types';
import { satox } from './satox';

export interface AlgorithmInfo {
    name: string;
    securityLevel: number;
    keySize: number;
    signatureSize: number;
    isRecommended: boolean;
    description: string;
}

export class PostQuantumAlgorithms {
    private algorithms: Pointer;
    private memory: Memory;

    constructor(memory: Memory) {
        this.memory = memory;
        this.algorithms = satox.post_quantum_algorithms_create();
        if (!this.algorithms) {
            throw new Error('Failed to create post-quantum algorithms');
        }
    }

    initialize(): void {
        const result = satox.post_quantum_algorithms_initialize(this.algorithms);
        if (result !== 0) {
            throw new Error('Failed to initialize post-quantum algorithms');
        }
    }

    shutdown(): void {
        const result = satox.post_quantum_algorithms_shutdown(this.algorithms);
        if (result !== 0) {
            throw new Error('Failed to shutdown post-quantum algorithms');
        }
    }

    generateKeyPair(algorithm: string): Result<{ publicKey: Uint8Array; privateKey: Uint8Array }> {
        const keyPair = satox.post_quantum_algorithms_generate_key_pair(
            this.algorithms,
            this.memory.writeString(algorithm)
        );

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

    sign(algorithm: string, privateKey: Uint8Array, data: Uint8Array): Uint8Array {
        const signature = satox.post_quantum_algorithms_sign(
            this.algorithms,
            this.memory.writeString(algorithm),
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

    verify(algorithm: string, publicKey: Uint8Array, data: Uint8Array, signature: Uint8Array): boolean {
        const result = satox.post_quantum_algorithms_verify(
            this.algorithms,
            this.memory.writeString(algorithm),
            this.memory.writeBytes(publicKey),
            publicKey.length,
            this.memory.writeBytes(data),
            data.length,
            this.memory.writeBytes(signature),
            signature.length
        );

        return result === 0;
    }

    encrypt(algorithm: string, publicKey: Uint8Array, data: Uint8Array): Uint8Array {
        const encrypted = satox.post_quantum_algorithms_encrypt(
            this.algorithms,
            this.memory.writeString(algorithm),
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

    decrypt(algorithm: string, privateKey: Uint8Array, encrypted: Uint8Array): Uint8Array {
        const decrypted = satox.post_quantum_algorithms_decrypt(
            this.algorithms,
            this.memory.writeString(algorithm),
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

    getAlgorithmInfo(algorithm: string): AlgorithmInfo {
        const info = satox.post_quantum_algorithms_get_algorithm_info(
            this.algorithms,
            this.memory.writeString(algorithm)
        );

        if (!info) {
            throw new Error('Failed to get algorithm info');
        }

        return {
            name: this.memory.readString(info.name),
            securityLevel: info.security_level,
            keySize: info.key_size,
            signatureSize: info.signature_size,
            isRecommended: info.is_recommended === 1,
            description: this.memory.readString(info.description)
        };
    }

    getAvailableAlgorithms(): string[] {
        const algorithms = satox.post_quantum_algorithms_get_available_algorithms(
            this.algorithms
        );

        if (!algorithms) {
            throw new Error('Failed to get available algorithms');
        }

        const result: string[] = [];
        for (let i = 0; i < algorithms.count; i++) {
            result.push(this.memory.readString(algorithms.list[i]));
        }

        return result;
    }

    isAlgorithmAvailable(algorithm: string): boolean {
        const result = satox.post_quantum_algorithms_is_algorithm_available(
            this.algorithms,
            this.memory.writeString(algorithm)
        );

        return result === 1;
    }

    isAlgorithmRecommended(algorithm: string): boolean {
        const result = satox.post_quantum_algorithms_is_algorithm_recommended(
            this.algorithms,
            this.memory.writeString(algorithm)
        );

        return result === 1;
    }

    getDefaultAlgorithm(): string {
        return satox.post_quantum_algorithms_get_default_algorithm(this.algorithms);
    }

    destroy(): void {
        if (this.algorithms) {
            satox.post_quantum_algorithms_destroy(this.algorithms);
            this.algorithms = null;
        }
    }
} 