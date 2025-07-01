package com.satox.core;

import java.util.Date;
import java.util.Map;

public class NFT {
    private String id;
    private String name;
    private String description;
    private String imageUrl;
    private Map<String, Object> metadata;
    private String owner;
    private Date createdAt;

    public NFT() {}

    public String getId() { return id; }
    public void setId(String id) { this.id = id; }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getDescription() { return description; }
    public void setDescription(String description) { this.description = description; }

    public String getImageUrl() { return imageUrl; }
    public void setImageUrl(String imageUrl) { this.imageUrl = imageUrl; }

    public Map<String, Object> getMetadata() { return metadata; }
    public void setMetadata(Map<String, Object> metadata) { this.metadata = metadata; }

    public String getOwner() { return owner; }
    public void setOwner(String owner) { this.owner = owner; }

    public Date getCreatedAt() { return createdAt; }
    public void setCreatedAt(Date createdAt) { this.createdAt = createdAt; }
} 