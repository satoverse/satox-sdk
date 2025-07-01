package com.satox.core;

import java.util.Map;

public class Asset {
    private String id;
    private String name;
    private String symbol;
    private AssetType type;
    private int decimals;
    private long totalSupply;
    private Map<String, Object> metadata;

    public Asset() {}

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getSymbol() { return symbol; }
    public void setSymbol(String symbol) { this.symbol = symbol; }

    public AssetType getType() { return type; }
    public void setType(AssetType type) { this.type = type; }

    public int getDecimals() { return decimals; }
    public void setDecimals(int decimals) { this.decimals = decimals; }

    public long getTotalSupply() { return totalSupply; }
    public void setTotalSupply(long totalSupply) { this.totalSupply = totalSupply; }

    public Map<String, Object> getMetadata() { return metadata; }
    public void setMetadata(Map<String, Object> metadata) { this.metadata = metadata; }
} 