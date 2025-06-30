package com.satox.wallet;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class Wallet {
    private final String name;
    private final ReentrantLock lock;
    private final Map<String, Address> addresses;
    private final AddressManager addressManager;

    public Wallet(String name, AddressManager addressManager) {
        this.name = name;
        this.lock = new ReentrantLock();
        this.addresses = new ConcurrentHashMap<>();
        this.addressManager = addressManager;
    }

    public String getName() {
        return name;
    }

    public Address addAddress(String label) {
        lock.lock();
        try {
            if (addresses.containsKey(label)) {
                throw new IllegalStateException("Address with label " + label + " already exists in wallet");
            }
            Address address = addressManager.generateAddress(label);
            addresses.put(label, address);
            return address;
        } finally {
            lock.unlock();
        }
    }

    public Address getAddress(String label) {
        lock.lock();
        try {
            Address address = addresses.get(label);
            if (address == null) {
                throw new IllegalStateException("Address with label " + label + " not found in wallet");
            }
            return address;
        } finally {
            lock.unlock();
        }
    }

    public void removeAddress(String label) {
        lock.lock();
        try {
            addresses.remove(label);
        } finally {
            lock.unlock();
        }
    }

    public List<Address> listAddresses() {
        lock.lock();
        try {
            return new ArrayList<>(addresses.values());
        } finally {
            lock.unlock();
        }
    }

    public boolean hasAddress(String label) {
        lock.lock();
        try {
            return addresses.containsKey(label);
        } finally {
            lock.unlock();
        }
    }

    public void backup(String backupPath) {
        lock.lock();
        try {
            // Implement wallet backup logic
        } finally {
            lock.unlock();
        }
    }

    public static Wallet restore(String name, String backupPath, AddressManager addressManager) {
        // Implement wallet restore logic
        return new Wallet(name, addressManager);
    }

    public void cleanup() {
        lock.lock();
        try {
            addresses.clear();
        } finally {
            lock.unlock();
        }
    }

    public byte[] sign(String addressLabel, byte[] data) {
        lock.lock();
        try {
            Address address = getAddress(addressLabel);
            return address.sign(data);
        } finally {
            lock.unlock();
        }
    }

    public boolean verify(String addressLabel, byte[] data, byte[] signature) {
        lock.lock();
        try {
            Address address = getAddress(addressLabel);
            return address.verify(data, signature);
        } finally {
            lock.unlock();
        }
    }
} 