const EventEmitter = require("events");
const http = require("http");
const url = require("url");

class APIManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.serverRunning = false;
    this.endpoints = new Map();
    this.config = {
      port: 7777, // Satoxcoin RPC port
      maxConnections: 100,
      timeout: 30000,
      cors: {
        enabled: true,
        origin: "*",
      },
    };
    this.server = null;
    this.connections = new Set();
    this.startTime = null;
  }

  initialize() {
    if (this.initialized) {
      throw new Error("API Manager is already initialized");
    }
    this.initialized = true;
    this.emit("initialized");
  }

  isInitialized() {
    return this.initialized;
  }

  startServer() {
    if (!this.initialized) {
      throw new Error("API Manager must be initialized before starting server");
    }
    if (this.serverRunning) {
      throw new Error("Server is already running");
    }

    this.server = http.createServer((req, res) => {
      this.handleRequest(req, res);
    });

    this.server.on("connection", (socket) => {
      this.connections.add(socket);
      socket.on("close", () => {
        this.connections.delete(socket);
      });
    });

    this.server.listen(this.config.port, () => {
      this.serverRunning = true;
      this.startTime = Date.now();
      this.emit("serverStarted", { port: this.config.port });
    });

    this.server.on("error", (error) => {
      this.emit("error", error);
    });
  }

  stopServer() {
    if (!this.initialized) {
      throw new Error("API Manager must be initialized before stopping server");
    }
    if (!this.serverRunning) {
      throw new Error("Server is not running");
    }

    return new Promise((resolve, reject) => {
      this.server.close((error) => {
        if (error) {
          reject(error);
          return;
        }

        // Close all existing connections
        for (const socket of this.connections) {
          socket.destroy();
        }
        this.connections.clear();

        this.serverRunning = false;
        this.startTime = null;
        this.emit("serverStopped");
        resolve();
      });
    });
  }

  registerEndpoint(path, method, handler) {
    if (!this.initialized) {
      throw new Error(
        "API Manager must be initialized before registering endpoints",
      );
    }
    if (!this.serverRunning) {
      throw new Error("Server must be running to register endpoints");
    }

    const endpointKey = `${method.toUpperCase()}:${path}`;
    if (this.endpoints.has(endpointKey)) {
      throw new Error(`Endpoint ${endpointKey} already exists`);
    }

    this.endpoints.set(endpointKey, handler);
    this.emit("endpointRegistered", { path, method });
  }

  unregisterEndpoint(path, method) {
    if (!this.initialized) {
      throw new Error(
        "API Manager must be initialized before unregistering endpoints",
      );
    }
    if (!this.serverRunning) {
      throw new Error("Server must be running to unregister endpoints");
    }

    const endpointKey = `${method.toUpperCase()}:${path}`;
    if (!this.endpoints.has(endpointKey)) {
      throw new Error(`Endpoint ${endpointKey} does not exist`);
    }

    this.endpoints.delete(endpointKey);
    this.emit("endpointUnregistered", { path, method });
  }

  listEndpoints() {
    if (!this.initialized) {
      throw new Error(
        "API Manager must be initialized before listing endpoints",
      );
    }
    if (!this.serverRunning) {
      throw new Error("Server must be running to list endpoints");
    }

    return Array.from(this.endpoints.keys()).map((key) => {
      const [method, path] = key.split(":");
      return { method, path };
    });
  }

  setConfig(key, value) {
    if (!this.initialized) {
      throw new Error("API Manager must be initialized before setting config");
    }

    if (key in this.config) {
      this.config[key] = value;
      this.emit("configChanged", { key, value });
    } else {
      throw new Error(`Invalid config key: ${key}`);
    }
  }

  getConfig(key) {
    if (!this.initialized) {
      throw new Error("API Manager must be initialized before getting config");
    }

    if (key in this.config) {
      return this.config[key];
    }
    throw new Error(`Invalid config key: ${key}`);
  }

  async handleRequest(req, res) {
    if (!this.initialized || !this.serverRunning) {
      res.writeHead(503);
      res.end(JSON.stringify({ error: "Service Unavailable" }));
      return;
    }

    const parsedUrl = url.parse(req.url, true);
    const path = parsedUrl.pathname;
    const method = req.method.toUpperCase();
    const endpointKey = `${method}:${path}`;

    // Handle CORS
    if (this.config.cors.enabled) {
      res.setHeader("Access-Control-Allow-Origin", this.config.cors.origin);
      res.setHeader(
        "Access-Control-Allow-Methods",
        "GET, POST, PUT, DELETE, OPTIONS",
      );
      res.setHeader(
        "Access-Control-Allow-Headers",
        "Content-Type, Authorization",
      );

      if (method === "OPTIONS") {
        res.writeHead(204);
        res.end();
        return;
      }
    }

    // Set timeout
    req.setTimeout(this.config.timeout);

    // Handle request
    if (this.endpoints.has(endpointKey)) {
      try {
        const handler = this.endpoints.get(endpointKey);
        const result = await handler(req, parsedUrl.query);
        res.writeHead(200, { "Content-Type": "application/json" });
        res.end(JSON.stringify(result));
      } catch (error) {
        res.writeHead(500);
        res.end(JSON.stringify({ error: error.message }));
      }
    } else {
      res.writeHead(404);
      res.end(JSON.stringify({ error: "Not Found" }));
    }
  }

  getServerStatus() {
    if (!this.initialized) {
      throw new Error(
        "API Manager must be initialized before getting server status",
      );
    }

    return {
      running: this.serverRunning,
      endpoints: this.listEndpoints(),
      uptime: this.startTime ? Date.now() - this.startTime : 0,
      connections: this.connections.size,
      config: { ...this.config },
    };
  }
}

module.exports = APIManager;
