package com.satox.blockchain;

import com.satox.transaction.Transaction;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

/**
 * Represents a block in the Satox blockchain.
 * Each block contains a list of transactions and maintains the blockchain's integrity.
 */
public class Block {
    private final String hash;
    private final String previousHash;
    private final long timestamp;
    private final List<Transaction> transactions;
    private final long nonce;
    private final int difficulty;
    private final ReentrantLock lock;

    /**
     * Creates a new block with the specified parameters.
     * 
     * @param previousHash The hash of the previous block
     * @param transactions The list of transactions in this block
     * @param difficulty The mining difficulty for this block
     */
    public Block(String previousHash, List<Transaction> transactions, int difficulty) {
        this.previousHash = previousHash;
        this.transactions = new ArrayList<>(transactions);
        this.timestamp = System.currentTimeMillis();
        this.difficulty = difficulty;
        this.nonce = 0;
        this.lock = new ReentrantLock();
        
        // Calculate the block hash
        this.hash = calculateHash();
    }

    /**
     * Creates a new block with the specified parameters including nonce.
     * 
     * @param previousHash The hash of the previous block
     * @param transactions The list of transactions in this block
     * @param difficulty The mining difficulty for this block
     * @param nonce The nonce value for this block
     */
    public Block(String previousHash, List<Transaction> transactions, int difficulty, long nonce) {
        this.previousHash = previousHash;
        this.transactions = new ArrayList<>(transactions);
        this.timestamp = System.currentTimeMillis();
        this.difficulty = difficulty;
        this.nonce = nonce;
        this.lock = new ReentrantLock();
        
        // Calculate the block hash
        this.hash = calculateHash();
    }

    /**
     * Gets the block hash.
     * 
     * @return The block hash
     */
    public String getHash() {
        lock.lock();
        try {
            return hash;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Gets the previous block hash.
     * 
     * @return The previous block hash
     */
    public String getPreviousHash() {
        lock.lock();
        try {
            return previousHash;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Gets the block timestamp.
     * 
     * @return The block timestamp
     */
    public long getTimestamp() {
        lock.lock();
        try {
            return timestamp;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Gets the list of transactions in this block.
     * 
     * @return The list of transactions
     */
    public List<Transaction> getTransactions() {
        lock.lock();
        try {
            return new ArrayList<>(transactions);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Gets the nonce value for this block.
     * 
     * @return The nonce value
     */
    public long getNonce() {
        lock.lock();
        try {
            return nonce;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Gets the mining difficulty for this block.
     * 
     * @return The mining difficulty
     */
    public int getDifficulty() {
        lock.lock();
        try {
            return difficulty;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Adds a transaction to this block.
     * 
     * @param transaction The transaction to add
     * @throws IllegalStateException if the block is already finalized
     */
    public void addTransaction(Transaction transaction) {
        lock.lock();
        try {
            if (transaction == null) {
                throw new IllegalArgumentException("Transaction cannot be null");
            }
            transactions.add(transaction);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Removes a transaction from this block.
     * 
     * @param transaction The transaction to remove
     * @return true if the transaction was removed, false otherwise
     */
    public boolean removeTransaction(Transaction transaction) {
        lock.lock();
        try {
            return transactions.remove(transaction);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Gets the number of transactions in this block.
     * 
     * @return The number of transactions
     */
    public int getTransactionCount() {
        lock.lock();
        try {
            return transactions.size();
        } finally {
            lock.unlock();
        }
    }

    /**
     * Checks if this block is valid.
     * 
     * @return true if the block is valid, false otherwise
     */
    public boolean isValid() {
        lock.lock();
        try {
            // Check if the hash is valid
            String calculatedHash = calculateHash();
            if (!hash.equals(calculatedHash)) {
                return false;
            }

            // Check if the block meets the difficulty requirement
            if (!hash.startsWith("0".repeat(difficulty))) {
                return false;
            }

            // Basic transaction validation - check if transactions are not null
            for (Transaction transaction : transactions) {
                if (transaction == null) {
                    return false;
                }
            }

            return true;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Calculates the hash of this block.
     * 
     * @return The calculated hash
     */
    private String calculateHash() {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            
            // Create a string representation of the block data
            StringBuilder data = new StringBuilder();
            data.append(previousHash);
            data.append(timestamp);
            data.append(nonce);
            data.append(difficulty);
            
            // Add transaction data
            for (Transaction transaction : transactions) {
                if (transaction != null) {
                    data.append(transaction.getFromAddress());
                    data.append(transaction.getToAddress());
                    data.append(transaction.getAmount());
                    data.append(transaction.getTimestamp());
                }
            }
            
            byte[] hashBytes = digest.digest(data.toString().getBytes(StandardCharsets.UTF_8));
            
            // Convert to hexadecimal string
            StringBuilder hexString = new StringBuilder();
            for (byte b : hashBytes) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }

    /**
     * Creates a string representation of this block.
     * 
     * @return A string representation of the block
     */
    @Override
    public String toString() {
        lock.lock();
        try {
            return String.format("Block{hash='%s', previousHash='%s', timestamp=%d, transactions=%d, nonce=%d, difficulty=%d}",
                    hash, previousHash, timestamp, transactions.size(), nonce, difficulty);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Checks if this block equals another object.
     * 
     * @param obj The object to compare with
     * @return true if the objects are equal, false otherwise
     */
    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        
        Block block = (Block) obj;
        return hash.equals(block.hash);
    }

    /**
     * Calculates the hash code for this block.
     * 
     * @return The hash code
     */
    @Override
    public int hashCode() {
        return hash.hashCode();
    }
} 