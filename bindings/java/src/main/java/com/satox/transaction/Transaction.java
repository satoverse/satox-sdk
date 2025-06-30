package com.satox.transaction;

public class Transaction {
    private byte[] signature;
    private String fromAddress;
    private String toAddress;
    private double amount;
    private long timestamp;

    public byte[] getDataToSign() {
        // Return dummy data for now
        return new byte[0];
    }

    public void setSignature(byte[] signature) {
        this.signature = signature;
    }

    public String getFromAddress() {
        return fromAddress;
    }

    public String getToAddress() {
        return toAddress;
    }

    public double getAmount() {
        return amount;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public byte[] getSignature() {
        return signature;
    }
} 