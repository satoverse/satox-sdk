const crypto = require("crypto");

class SecurityManager {
  constructor() {
    this.initialized = false;
    this.keys = new Map();
    this.encryptionEnabled = false;
  }

  initialize() {
    if (this.initialized) {
      throw new Error("SecurityManager already initialized");
    }
    this.initialized = true;
  }

  isInitialized() {
    return this.initialized;
  }

  generateKey(keyId) {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    if (this.keys.has(keyId)) {
      throw new Error("Key ID already exists");
    }

    const key = crypto.randomBytes(32);
    this.keys.set(keyId, key);
    return keyId;
  }

  removeKey(keyId) {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    if (!this.keys.has(keyId)) {
      throw new Error("Key ID not found");
    }
    this.keys.delete(keyId);
  }

  enableEncryption() {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    this.encryptionEnabled = true;
  }

  disableEncryption() {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    this.encryptionEnabled = false;
  }

  isEncryptionEnabled() {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    return this.encryptionEnabled;
  }

  encrypt(data, keyId) {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    if (!this.encryptionEnabled) {
      throw new Error("Encryption is not enabled");
    }
    if (!this.keys.has(keyId)) {
      throw new Error("Key ID not found");
    }

    const key = this.keys.get(keyId);
    const iv = crypto.randomBytes(16);
    const cipher = crypto.createCipheriv("aes-256-gcm", key, iv);

    let encrypted = cipher.update(data, "utf8", "hex");
    encrypted += cipher.final("hex");

    const authTag = cipher.getAuthTag();

    return {
      encrypted,
      iv: iv.toString("hex"),
      authTag: authTag.toString("hex"),
    };
  }

  decrypt(encryptedData, keyId) {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    if (!this.encryptionEnabled) {
      throw new Error("Encryption is not enabled");
    }
    if (!this.keys.has(keyId)) {
      throw new Error("Key ID not found");
    }

    const key = this.keys.get(keyId);
    const decipher = crypto.createDecipheriv(
      "aes-256-gcm",
      key,
      Buffer.from(encryptedData.iv, "hex"),
    );

    decipher.setAuthTag(Buffer.from(encryptedData.authTag, "hex"));

    let decrypted = decipher.update(encryptedData.encrypted, "hex", "utf8");
    decrypted += decipher.final("utf8");

    return decrypted;
  }

  hash(data) {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    return crypto.createHash("sha256").update(data).digest("hex");
  }

  verifyHash(data, hash) {
    if (!this.initialized) {
      throw new Error("SecurityManager not initialized");
    }
    return this.hash(data) === hash;
  }
}

module.exports = SecurityManager;
