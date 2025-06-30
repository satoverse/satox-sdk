const { create } = require("ipfs-http-client");
const { Buffer } = require("buffer");
const EventEmitter = require("events");
const fs = require("fs").promises;
const path = require("path");

class IPFSManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.ipfs = null;
    this.pins = new Map();
    this.config = {
      host: "ipfs.infura.io",
      port: 5001,
      protocol: "https",
      timeout: 30000,
    };
  }

  async initialize(config = {}) {
    if (this.initialized) {
      throw new Error("IPFSManager already initialized");
    }

    // Merge provided config with defaults
    this.config = { ...this.config, ...config };

    try {
      this.ipfs = create({
        host: this.config.host,
        port: this.config.port,
        protocol: this.config.protocol,
        timeout: this.config.timeout,
      });

      // Test connection
      await this.ipfs.id();
      this.initialized = true;
      this.emit("initialized");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to initialize IPFS: ${error.message}`);
    }
  }

  isInitialized() {
    return this.initialized;
  }

  async uploadFile(filePath) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      const fileContent = await fs.readFile(filePath);
      const fileName = path.basename(filePath);

      const result = await this.ipfs.add({
        path: fileName,
        content: fileContent,
      });

      this.emit("fileUploaded", {
        cid: result.cid.toString(),
        path: fileName,
      });

      return {
        cid: result.cid.toString(),
        path: fileName,
        size: result.size,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to upload file: ${error.message}`);
    }
  }

  async downloadFile(cid, outputPath) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      const stream = this.ipfs.cat(cid);
      const chunks = [];

      for await (const chunk of stream) {
        chunks.push(chunk);
      }

      const fileContent = Buffer.concat(chunks);
      await fs.writeFile(outputPath, fileContent);

      this.emit("fileDownloaded", {
        cid,
        path: outputPath,
      });

      return {
        cid,
        path: outputPath,
        size: fileContent.length,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to download file: ${error.message}`);
    }
  }

  async pinFile(cid) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      await this.ipfs.pin.add(cid);
      this.pins.set(cid, {
        timestamp: Date.now(),
        status: "pinned",
      });

      this.emit("filePinned", cid);
      return true;
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to pin file: ${error.message}`);
    }
  }

  async unpinFile(cid) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      await this.ipfs.pin.rm(cid);
      this.pins.delete(cid);

      this.emit("fileUnpinned", cid);
      return true;
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to unpin file: ${error.message}`);
    }
  }

  async getPinnedFiles() {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      const pins = await this.ipfs.pin.ls();
      const result = [];

      for await (const pin of pins) {
        result.push({
          cid: pin.cid.toString(),
          type: pin.type,
        });
      }

      return result;
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to get pinned files: ${error.message}`);
    }
  }

  async getFileInfo(cid) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      const stats = await this.ipfs.files.stat(`/ipfs/${cid}`);
      return {
        cid,
        size: stats.size,
        type: stats.type,
        blocks: stats.blocks,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to get file info: ${error.message}`);
    }
  }

  async uploadDirectory(dirPath) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      const files = [];
      const dirName = path.basename(dirPath);

      // Read directory contents
      const entries = await fs.readdir(dirPath, { withFileTypes: true });

      for (const entry of entries) {
        const fullPath = path.join(dirPath, entry.name);
        if (entry.isFile()) {
          const content = await fs.readFile(fullPath);
          files.push({
            path: path.join(dirName, entry.name),
            content,
          });
        }
      }

      const result = await this.ipfs.add(files);
      const rootCid = result[result.length - 1].cid.toString();

      this.emit("directoryUploaded", {
        cid: rootCid,
        path: dirName,
        fileCount: files.length,
      });

      return {
        cid: rootCid,
        path: dirName,
        fileCount: files.length,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to upload directory: ${error.message}`);
    }
  }

  async downloadDirectory(cid, outputPath) {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      await fs.mkdir(outputPath, { recursive: true });

      for await (const file of this.ipfs.ls(cid)) {
        if (file.type === "file") {
          const filePath = path.join(outputPath, file.name);
          const content = [];

          for await (const chunk of this.ipfs.cat(file.cid)) {
            content.push(chunk);
          }

          await fs.writeFile(filePath, Buffer.concat(content));
        }
      }

      this.emit("directoryDownloaded", {
        cid,
        path: outputPath,
      });

      return {
        cid,
        path: outputPath,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to download directory: ${error.message}`);
    }
  }

  async shutdown() {
    if (!this.initialized) {
      throw new Error("IPFSManager not initialized");
    }

    try {
      // Cleanup any resources
      this.pins.clear();
      this.initialized = false;
      this.emit("shutdown");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to shutdown: ${error.message}`);
    }
  }
}

module.exports = IPFSManager;
