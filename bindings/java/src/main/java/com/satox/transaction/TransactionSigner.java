package com.satox.transaction;

import java.util.concurrent.locks.ReentrantLock;
import java.security.Signature;
import java.security.KeyFactory;
import java.security.PrivateKey;
import java.security.spec.PKCS8EncodedKeySpec;

public class TransactionSigner {
    private final ReentrantLock lock;
    private boolean initialized;

    public TransactionSigner() {
        this.lock = new ReentrantLock();
        this.initialized = false;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("TransactionSigner already initialized");
            }
            initialized = true;
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("TransactionSigner not initialized");
            }
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public void sign(Transaction transaction, byte[] privateKeyBytes) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("TransactionSigner not initialized");
            }
            if (transaction == null) {
                throw new IllegalArgumentException("Transaction cannot be null");
            }
            if (privateKeyBytes == null) {
                throw new IllegalArgumentException("Private key cannot be null");
            }

            try {
                // Convert private key bytes to PrivateKey object
                KeyFactory keyFactory = KeyFactory.getInstance("RSA");
                PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(privateKeyBytes);
                PrivateKey privateKey = keyFactory.generatePrivate(keySpec);

                // Create signature
                Signature signature = Signature.getInstance("SHA256withRSA");
                signature.initSign(privateKey);
                signature.update(transaction.getDataToSign());

                // Sign transaction
                byte[] signatureBytes = signature.sign();
                transaction.setSignature(signatureBytes);
            } catch (Exception e) {
                throw new IllegalStateException("Failed to sign transaction", e);
            }
        } finally {
            lock.unlock();
        }
    }

    public boolean isInitialized() {
        lock.lock();
        try {
            return initialized;
        } finally {
            lock.unlock();
        }
    }
} 