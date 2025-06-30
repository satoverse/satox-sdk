package com.satox.quantum;

/**
 * KeyPair represents a pair of public and private keys.
 */
public class KeyPair {
    private final byte[] publicKey;
    private final byte[] privateKey;

    /**
     * Creates a new KeyPair with the specified public and private keys.
     *
     * @param publicKey The public key
     * @param privateKey The private key
     */
    public KeyPair(byte[] publicKey, byte[] privateKey) {
        this.publicKey = publicKey.clone();
        this.privateKey = privateKey.clone();
    }

    /**
     * Gets the public key.
     *
     * @return A copy of the public key
     */
    public byte[] getPublicKey() {
        return publicKey.clone();
    }

    /**
     * Gets the private key.
     *
     * @return A copy of the private key
     */
    public byte[] getPrivateKey() {
        return privateKey.clone();
    }
} 