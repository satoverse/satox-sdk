package com.satox.quantum;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;

public class PostQuantumAlgorithms {
    private static PostQuantumAlgorithms instance;
    private final ReentrantLock lock;
    private final Map<String, AlgorithmInfo> algorithms;
    private boolean initialized;

    public static class AlgorithmInfo {
        private final String name;
        private final int securityLevel;
        private final int keySize;
        private final int signatureSize;
        private final boolean isRecommended;
        private final String description;

        public AlgorithmInfo(String name, int securityLevel, int keySize, int signatureSize, 
                           boolean isRecommended, String description) {
            this.name = name;
            this.securityLevel = securityLevel;
            this.keySize = keySize;
            this.signatureSize = signatureSize;
            this.isRecommended = isRecommended;
            this.description = description;
        }

        public String getName() { return name; }
        public int getSecurityLevel() { return securityLevel; }
        public int getKeySize() { return keySize; }
        public int getSignatureSize() { return signatureSize; }
        public boolean isRecommended() { return isRecommended; }
        public String getDescription() { return description; }
    }

    private PostQuantumAlgorithms() {
        this.lock = new ReentrantLock();
        this.algorithms = new ConcurrentHashMap<>();
        this.initialized = false;
    }

    public static synchronized PostQuantumAlgorithms getInstance() {
        if (instance == null) {
            instance = new PostQuantumAlgorithms();
        }
        return instance;
    }

    public void initialize() {
        lock.lock();
        try {
            if (initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms already initialized");
            }
            populateAlgorithms();
            initialized = true;
        } finally {
            lock.unlock();
        }
    }

    private void populateAlgorithms() {
        // Key encapsulation mechanisms (KEMs)
        algorithms.put("CRYSTALS-Kyber", new AlgorithmInfo(
            "CRYSTALS-Kyber",
            256,
            2048,
            0,
            true,
            "NIST PQC candidate for key encapsulation"
        ));

        algorithms.put("NTRU", new AlgorithmInfo(
            "NTRU",
            192,
            2048,
            0,
            false,
            "NTRU lattice-based encryption"
        ));

        algorithms.put("SABER", new AlgorithmInfo(
            "SABER",
            192,
            2048,
            0,
            false,
            "Lattice-based key encapsulation"
        ));

        // Signature schemes
        algorithms.put("CRYSTALS-Dilithium", new AlgorithmInfo(
            "CRYSTALS-Dilithium",
            128,
            0,
            2048,
            true,
            "NIST PQC candidate for digital signatures"
        ));

        algorithms.put("FALCON", new AlgorithmInfo(
            "FALCON",
            256,
            0,
            2048,
            true,
            "NIST PQC candidate for digital signatures"
        ));

        algorithms.put("SPHINCS+", new AlgorithmInfo(
            "SPHINCS+",
            256,
            0,
            2048,
            true,
            "Hash-based signature scheme"
        ));
    }

    public void shutdown() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            algorithms.clear();
            initialized = false;
        } finally {
            lock.unlock();
        }
    }

    public List<String> getAvailableAlgorithms() {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            return new ArrayList<>(algorithms.keySet());
        } finally {
            lock.unlock();
        }
    }

    public AlgorithmInfo getAlgorithmInfo(String algorithmName) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            if (!algorithms.containsKey(algorithmName)) {
                throw new IllegalArgumentException("Algorithm not found: " + algorithmName);
            }
            return algorithms.get(algorithmName);
        } finally {
            lock.unlock();
        }
    }

    public String getRecommendedAlgorithm(int minSecurityLevel) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            return algorithms.entrySet().stream()
                .filter(e -> e.getValue().isRecommended() && 
                           e.getValue().getSecurityLevel() >= minSecurityLevel)
                .map(Map.Entry::getKey)
                .findFirst()
                .orElseThrow(() -> new IllegalStateException("No suitable algorithm found"));
        } finally {
            lock.unlock();
        }
    }

    public KeyPair generateKeyPair(String algorithmName) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            if (!algorithms.containsKey(algorithmName)) {
                throw new IllegalArgumentException("Algorithm not found: " + algorithmName);
            }
            // Implement actual key generation based on algorithm
            return null; // Placeholder
        } finally {
            lock.unlock();
        }
    }

    public boolean isAlgorithmAvailable(String algorithmName) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            return algorithms.containsKey(algorithmName);
        } finally {
            lock.unlock();
        }
    }

    public boolean isAlgorithmRecommended(String algorithmName) {
        lock.lock();
        try {
            if (!initialized) {
                throw new IllegalStateException("PostQuantumAlgorithms not initialized");
            }
            AlgorithmInfo info = algorithms.get(algorithmName);
            return info != null && info.isRecommended();
        } finally {
            lock.unlock();
        }
    }
} 