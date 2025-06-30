const net = require("net");
const EventEmitter = require("events");

class NetworkManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.connected = false;
    this.peers = new Map();
    this.networkConfig = {
      p2pPort: 60777, // Satoxcoin P2P port
      rpcPort: 7777, // Satoxcoin RPC port
      maxConnections: 100,
      timeout: 30000, // 30 seconds
      network: "mainnet",
    };
    this.server = null;
  }

  initialize(config = {}) {
    if (this.initialized) {
      throw new Error("NetworkManager already initialized");
    }

    // Merge provided config with defaults
    this.networkConfig = { ...this.networkConfig, ...config };
    this.initialized = true;
  }

  isInitialized() {
    return this.initialized;
  }

  async connect(host, port = this.networkConfig.p2pPort) {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    if (this.connected) {
      throw new Error("Already connected");
    }

    return new Promise((resolve, reject) => {
      const socket = new net.Socket();

      socket.setTimeout(this.networkConfig.timeout);

      socket.on("connect", () => {
        this.connected = true;
        const peerId = `${host}:${port}`;
        this.peers.set(peerId, {
          socket,
          host,
          port,
          connected: true,
          lastSeen: Date.now(),
        });
        this.emit("peerConnected", peerId);
        resolve(peerId);
      });

      socket.on("error", (error) => {
        this.emit("error", error);
        reject(error);
      });

      socket.on("timeout", () => {
        socket.destroy();
        this.emit("timeout", `${host}:${port}`);
      });

      socket.on("close", () => {
        const peerId = `${host}:${port}`;
        if (this.peers.has(peerId)) {
          this.peers.delete(peerId);
          this.emit("peerDisconnected", peerId);
        }
        if (this.connected) {
          this.connected = false;
        }
      });

      socket.on("data", (data) => {
        this.handleReceivedData(socket, data);
      });

      socket.connect(port, host);
    });
  }

  disconnect(peerId) {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    if (!this.peers.has(peerId)) {
      throw new Error("Peer not found");
    }

    const peer = this.peers.get(peerId);
    peer.socket.destroy();
    this.peers.delete(peerId);
    this.emit("peerDisconnected", peerId);
  }

  async startServer(port = this.networkConfig.p2pPort) {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    if (this.server) {
      throw new Error("Server already running");
    }

    return new Promise((resolve, reject) => {
      this.server = net.createServer((socket) => {
        const peerId = `${socket.remoteAddress}:${socket.remotePort}`;

        socket.setTimeout(this.networkConfig.timeout);

        this.peers.set(peerId, {
          socket,
          host: socket.remoteAddress,
          port: socket.remotePort,
          connected: true,
          lastSeen: Date.now(),
        });

        socket.on("data", (data) => {
          this.handleReceivedData(socket, data);
        });

        socket.on("close", () => {
          if (this.peers.has(peerId)) {
            this.peers.delete(peerId);
            this.emit("peerDisconnected", peerId);
          }
        });

        socket.on("error", (error) => {
          this.emit("error", error);
        });

        socket.on("timeout", () => {
          socket.destroy();
        });

        this.emit("peerConnected", peerId);
      });

      this.server.on("error", (error) => {
        this.emit("error", error);
        reject(error);
      });

      this.server.listen(port, () => {
        this.emit("serverStarted", port);
        resolve(port);
      });
    });
  }

  stopServer() {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    if (!this.server) {
      throw new Error("Server not running");
    }

    return new Promise((resolve) => {
      this.server.close(() => {
        this.server = null;
        this.emit("serverStopped");
        resolve();
      });
    });
  }

  sendMessage(peerId, message) {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    if (!this.peers.has(peerId)) {
      throw new Error("Peer not found");
    }

    const peer = this.peers.get(peerId);
    if (!peer.connected) {
      throw new Error("Peer not connected");
    }

    try {
      const data = JSON.stringify(message);
      peer.socket.write(data);
      peer.lastSeen = Date.now();
      return true;
    } catch (error) {
      this.emit("error", error);
      return false;
    }
  }

  handleReceivedData(socket, data) {
    try {
      const message = JSON.parse(data.toString());
      const peerId = `${socket.remoteAddress}:${socket.remotePort}`;

      if (this.peers.has(peerId)) {
        const peer = this.peers.get(peerId);
        peer.lastSeen = Date.now();
      }

      this.emit("message", peerId, message);
    } catch (error) {
      this.emit("error", error);
    }
  }

  getPeers() {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }

    const peers = [];
    for (const [peerId, peer] of this.peers.entries()) {
      peers.push({
        id: peerId,
        host: peer.host,
        port: peer.port,
        connected: peer.connected,
        lastSeen: peer.lastSeen,
      });
    }
    return peers;
  }

  getPeerCount() {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    return this.peers.size;
  }

  isConnected() {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    return this.connected;
  }

  getNetworkConfig() {
    if (!this.initialized) {
      throw new Error("NetworkManager not initialized");
    }
    return { ...this.networkConfig };
  }
}

module.exports = NetworkManager;
