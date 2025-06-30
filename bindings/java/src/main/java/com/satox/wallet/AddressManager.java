package com.satox.wallet;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class AddressManager {
    private final ReentrantLock lock;
    private final Map<String, Address> addresses;
    private final KeyManager keyManager;

    public AddressManager() {
        this.lock = new ReentrantLock();
        this.addresses = new ConcurrentHashMap<>();
        this.keyManager = new KeyManager();
    }

    public void initialize() {
        lock.lock();
        try {
            keyManager.initialize();
        } finally {
            lock.unlock();
        }
    }

    public void shutdown() {
        lock.lock();
        try {
            keyManager.shutdown();
            addresses.clear();
        } finally {
            lock.unlock();
        }
    }

    public Address generateAddress(String label) {
        lock.lock();
        try {
            if (addresses.containsKey(label)) {
                throw new IllegalStateException("Address with label " + label + " already exists");
            }
            Address address = new Address(label, keyManager);
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
                throw new IllegalStateException("Address with label " + label + " not found");
            }
            return address;
        } finally {
            lock.unlock();
        }
    }

    public void deleteAddress(String label) {
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

    public String getAddressString(String label) {
        lock.lock();
        try {
            Address address = getAddress(label);
            return address.toString();
        } finally {
            lock.unlock();
        }
    }

    public boolean validateAddress(String address) {
        lock.lock();
        try {
            // Implement address validation logic
            return false; // Placeholder
        } finally {
            lock.unlock();
        }
    }

    public void importAddress(String label, String addressString) {
        lock.lock();
        try {
            if (!validateAddress(addressString)) {
                throw new IllegalArgumentException("Invalid address format");
            }
            if (addresses.containsKey(label)) {
                throw new IllegalStateException("Address with label " + label + " already exists");
            }
            Address address = new Address(label, addressString);
            addresses.put(label, address);
        } finally {
            lock.unlock();
        }
    }
} 