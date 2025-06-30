package com.satox.wallet;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class WalletManager {
    private final ReentrantLock lock;
    private final Map<String, Wallet> wallets;
    private final KeyManager keyManager;
    private final AddressManager addressManager;

    public WalletManager() {
        this.lock = new ReentrantLock();
        this.wallets = new ConcurrentHashMap<>();
        this.keyManager = new KeyManager();
        this.addressManager = new AddressManager();
    }

    public void initialize() {
        lock.lock();
        try {
            keyManager.initialize();
            addressManager.initialize();
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() {
        lock.lock();
        try {
            keyManager.shutdown();
            addressManager.shutdown();
            wallets.clear();
        } finally {
            lock.unlock();
        }
    }

    public Wallet createWallet(String name) {
        lock.lock();
        try {
            if (wallets.containsKey(name)) {
                throw new IllegalStateException("Wallet with name " + name + " already exists");
            }
            Wallet wallet = new Wallet(name, addressManager);
            wallets.put(name, wallet);
            return wallet;
        } finally {
            lock.unlock();
        }
    }

    public Wallet getWallet(String name) {
        lock.lock();
        try {
            Wallet wallet = wallets.get(name);
            if (wallet == null) {
                throw new IllegalStateException("Wallet with name " + name + " not found");
            }
            return wallet;
        } finally {
            lock.unlock();
        }
    }

    public void deleteWallet(String name) {
        lock.lock();
        try {
            Wallet wallet = wallets.remove(name);
            if (wallet != null) {
                wallet.cleanup();
            }
        } finally {
            lock.unlock();
        }
    }

    public List<Wallet> listWallets() {
        lock.lock();
        try {
            return new ArrayList<>(wallets.values());
        } finally {
            lock.unlock();
        }
    }

    public boolean hasWallet(String name) {
        lock.lock();
        try {
            return wallets.containsKey(name);
        } finally {
            lock.unlock();
        }
    }

    public void backupWallet(String name, String backupPath) {
        lock.lock();
        try {
            Wallet wallet = getWallet(name);
            wallet.backup(backupPath);
        } finally {
            lock.unlock();
        }
    }

    public void restoreWallet(String name, String backupPath) {
        lock.lock();
        try {
            if (hasWallet(name)) {
                throw new IllegalStateException("Wallet with name " + name + " already exists");
            }
            Wallet wallet = Wallet.restore(name, backupPath, addressManager);
            wallets.put(name, wallet);
        } finally {
            lock.unlock();
        }
    }
} 