import { satox_native } from './native/satox_native';
import { ThreadSafeManager } from './thread-safe';

export interface NetworkConfig {
  port?: number;
  maxConnections?: number;
  enableDiscovery?: boolean;
  seedNodes?: string[];
}

export interface ConnectionInfo {
  address: string;
  port: number;
  version: string;
  userAgent: string;
  connectedAt: number;
  lastSeen: number;
  bytesReceived: number;
  bytesSent: number;
  latency: number;
}

export interface PeerInfo {
  address: string;
  port: number;
  version: string;
  userAgent: string;
  subVer: string;
  startingHeight: number;
  bestHeight: number;
  lastSeen: number;
  isInbound: boolean;
  isWhitelisted: boolean;
}

export interface BannedPeer {
  address: string;
  banTime: number;
  banUntil: number;
  reason: string;
}

export interface NetworkStats {
  totalPeers: number;
  connectedPeers: number;
  inboundPeers: number;
  outboundPeers: number;
  bannedPeers: number;
  totalBytesReceived: number;
  totalBytesSent: number;
  averageLatency: number;
  messagesSent: number;
  messagesReceived: number;
}

export class NetworkManager extends ThreadSafeManager {
  private manager: any;
  private stats: NetworkStats = {
    totalPeers: 0,
    connectedPeers: 0,
    inboundPeers: 0,
    outboundPeers: 0,
    bannedPeers: 0,
    totalBytesReceived: 0,
    totalBytesSent: 0,
    averageLatency: 0,
    messagesSent: 0,
    messagesReceived: 0
  };

  constructor() {
    super();
    this.manager = new satox_native.NetworkManager();
  }

  /**
   * Initialize the network manager
   */
  async initialize(config: NetworkConfig): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        const result = this.manager.initialize(config);
        this.setInitialized(result);
        this.setError(null);
        return result;
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to initialize Network Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the network manager
   */
  async shutdown(): Promise<void> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        this.manager.shutdown();
        this.setInitialized(false);
        this.setError(null);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to shutdown Network Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Connect to a peer
   */
  async connect(peerAddress: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.connect(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Disconnect from a peer
   */
  async disconnect(peerAddress: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.disconnect(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Check if connected to a peer
   */
  async isConnected(peerAddress: string): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.isConnected(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get connection information for a peer
   */
  async getConnectionInfo(peerAddress: string): Promise<ConnectionInfo | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getConnectionInfo(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Get peer information
   */
  async getPeerInfo(peerAddress: string): Promise<PeerInfo | null> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getPeerInfo(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return null;
      }
    });
  }

  /**
   * Get all connected peers
   */
  async getPeers(): Promise<PeerInfo[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getPeers();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Add a peer to the connection list
   */
  async addPeer(peerAddress: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.addPeer(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Remove a peer from the connection list
   */
  async removePeer(peerAddress: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.removePeer(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Ban a peer
   */
  async banPeer(peerAddress: string, banTime: number = 86400): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.banPeer(peerAddress, banTime);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Unban a peer
   */
  async unbanPeer(peerAddress: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.unbanPeer(peerAddress);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Get all banned peers
   */
  async getBannedPeers(): Promise<BannedPeer[]> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getBannedPeers();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return [];
      }
    });
  }

  /**
   * Send a message to a specific peer
   */
  async sendMessage(peerAddress: string, message: string): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.sendMessage(peerAddress, message);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Broadcast a message to all connected peers
   */
  async broadcastMessage(message: string): Promise<number> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.broadcastMessage(message);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return 0;
      }
    });
  }

  /**
   * Get network statistics
   */
  async getNetworkStats(): Promise<NetworkStats> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return { ...this.stats };
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return { ...this.stats };
      }
    });
  }

  /**
   * Reset network statistics
   */
  async resetStats(): Promise<void> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.stats = {
        totalPeers: 0,
        connectedPeers: 0,
        inboundPeers: 0,
        outboundPeers: 0,
        bannedPeers: 0,
        totalBytesReceived: 0,
        totalBytesSent: 0,
        averageLatency: 0,
        messagesSent: 0,
        messagesReceived: 0
      };
    });
  }

  /**
   * Enable or disable statistics collection
   */
  async enableStats(enable: boolean): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      // Placeholder implementation
      return true;
    });
  }

  /**
   * Get the last error message
   */
  override getLastError(): string {
    return super.getLastError() || '';
  }

  /**
   * Clear the last error message
   */
  override async clearLastError(): Promise<void> {
    return this.withLock(async () => {
      super.clearLastError();
    });
  }
} 