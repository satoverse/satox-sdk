const crypto = require("crypto");
const EventEmitter = require("events");

class NFT {
  constructor(id, name, description, owner, metadata = {}) {
    this.id = id;
    this.name = name;
    this.description = description;
    this.owner = owner;
    this.metadata = metadata;
    this.createdAt = Date.now();
    this.updatedAt = Date.now();
  }

  toJSON() {
    return {
      id: this.id,
      name: this.name,
      description: this.description,
      owner: this.owner,
      metadata: this.metadata,
      createdAt: this.createdAt,
      updatedAt: this.updatedAt,
    };
  }
}

class NFTManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.nfts = new Map();
    this.ownershipHistory = new Map();
  }

  initialize() {
    if (this.initialized) {
      throw new Error("NFTManager already initialized");
    }
    this.initialized = true;
  }

  isInitialized() {
    return this.initialized;
  }

  createNFT(name, description, owner, metadata = {}) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }

    const id = `nft_${crypto.randomBytes(16).toString("hex")}`;
    const nft = new NFT(id, name, description, owner, metadata);

    this.nfts.set(id, nft);
    this.ownershipHistory.set(id, [
      {
        owner,
        timestamp: Date.now(),
        type: "creation",
      },
    ]);

    this.emit("nftCreated", nft);
    return nft;
  }

  getNFT(id) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }
    if (!this.nfts.has(id)) {
      throw new Error("NFT not found");
    }
    return this.nfts.get(id);
  }

  transferNFT(id, fromOwner, toOwner) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }
    if (!this.nfts.has(id)) {
      throw new Error("NFT not found");
    }

    const nft = this.nfts.get(id);
    if (nft.owner !== fromOwner) {
      throw new Error("Current owner does not match");
    }

    nft.owner = toOwner;
    nft.updatedAt = Date.now();

    const history = this.ownershipHistory.get(id) || [];
    history.push({
      from: fromOwner,
      to: toOwner,
      timestamp: Date.now(),
      type: "transfer",
    });
    this.ownershipHistory.set(id, history);

    this.emit("nftTransferred", nft);
    return nft;
  }

  updateNFTMetadata(id, owner, metadata) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }
    if (!this.nfts.has(id)) {
      throw new Error("NFT not found");
    }

    const nft = this.nfts.get(id);
    if (nft.owner !== owner) {
      throw new Error("Not authorized to update NFT");
    }

    nft.metadata = { ...nft.metadata, ...metadata };
    nft.updatedAt = Date.now();

    this.emit("nftUpdated", nft);
    return nft;
  }

  deleteNFT(id, owner) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }
    if (!this.nfts.has(id)) {
      throw new Error("NFT not found");
    }

    const nft = this.nfts.get(id);
    if (nft.owner !== owner) {
      throw new Error("Not authorized to delete NFT");
    }

    this.nfts.delete(id);
    this.ownershipHistory.delete(id);

    this.emit("nftDeleted", id);
    return true;
  }

  getNFTsByOwner(owner) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }

    const nfts = [];
    for (const nft of this.nfts.values()) {
      if (nft.owner === owner) {
        nfts.push(nft);
      }
    }
    return nfts;
  }

  getOwnershipHistory(id) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }
    if (!this.ownershipHistory.has(id)) {
      throw new Error("NFT not found");
    }
    return [...this.ownershipHistory.get(id)];
  }

  searchNFTs(query) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }

    const results = [];
    for (const nft of this.nfts.values()) {
      if (
        nft.name.toLowerCase().includes(query.toLowerCase()) ||
        nft.description.toLowerCase().includes(query.toLowerCase()) ||
        Object.values(nft.metadata).some((value) =>
          String(value).toLowerCase().includes(query.toLowerCase()),
        )
      ) {
        results.push(nft);
      }
    }
    return results;
  }

  getNFTCount() {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }
    return this.nfts.size;
  }

  validateNFT(nft) {
    if (!this.initialized) {
      throw new Error("NFTManager not initialized");
    }

    if (!nft.id || !nft.name || !nft.owner) {
      return false;
    }

    if (typeof nft.name !== "string" || typeof nft.description !== "string") {
      return false;
    }

    if (typeof nft.metadata !== "object") {
      return false;
    }

    return true;
  }
}

module.exports = { NFTManager, NFT };
