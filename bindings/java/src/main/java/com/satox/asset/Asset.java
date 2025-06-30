package com.satox.asset;

import java.util.UUID;
import java.time.Instant;
import java.util.Map;
import java.util.HashMap;

public class Asset {
    private final String id;
    private String name;
    private String symbol;
    private double totalSupply;
    private String owner;
    private Map<String, Object> metadata;
    private Instant createdAt;
    private Instant updatedAt;
    private boolean reissuable;
    private AssetStatus status;
    public Asset(String name, String symbol, double totalSupply, String owner) {
        this.id = UUID.randomUUID().toString();
        this.name = name;
        this.symbol = symbol;
        this.totalSupply = totalSupply;
        this.owner = owner;
        this.metadata = new HashMap<>();
        this.createdAt = Instant.now();
        this.updatedAt = this.createdAt;
        this.reissuable = false;
        this.status = AssetStatus.ACTIVE;
    }
    public String getId() { return id; }
    public String getName() { return name; }
    public void setName(String name) { this.name = name; this.updatedAt = Instant.now(); }
    public String getSymbol() { return symbol; }
    public void setSymbol(String symbol) { this.symbol = symbol; this.updatedAt = Instant.now(); }
    public double getTotalSupply() { return totalSupply; }
    public void setTotalSupply(double totalSupply) { this.totalSupply = totalSupply; this.updatedAt = Instant.now(); }
    public String getOwner() { return owner; }
    public void setOwner(String owner) { this.owner = owner; this.updatedAt = Instant.now(); }
    public Map<String, Object> getMetadata() { return metadata; }
    public void setMetadata(Map<String, Object> metadata) { this.metadata = metadata; this.updatedAt = Instant.now(); }
    public void addMetadata(String key, Object value) { this.metadata.put(key, value); this.updatedAt = Instant.now(); }
    public Object getMetadata(String key) { return this.metadata.get(key); }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }
    public boolean isReissuable() { return reissuable; }
    public void setReissuable(boolean reissuable) { this.reissuable = reissuable; this.updatedAt = Instant.now(); }
    public AssetStatus getStatus() { return status; }
    public void setStatus(AssetStatus status) { this.status = status; this.updatedAt = Instant.now(); }
    public String getDescription() { return ""; }
    public String getValue() { return ""; }
    public enum AssetStatus { ACTIVE, FROZEN, DELETED }
}
