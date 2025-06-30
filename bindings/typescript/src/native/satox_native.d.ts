declare module './satox_native' {
  export class BlockchainManager {
    constructor();
    initialize(config: any): boolean;
    shutdown(): void;
    connect(nodeAddress: string): boolean;
    disconnect(): boolean;
    isConnected(): boolean;
    getState(): number;
    getBlock(blockHash: string): any;
    getBlockByHeight(height: number): any;
    getLatestBlock(): any;
    getBlocks(startHeight: number, endHeight: number): any[];
    getTransaction(txHash: string): any;
    broadcastTransaction(transaction: any): boolean;
    validateTransaction(transaction: any): boolean;
    getTransactionsByBlock(blockHash: string): any[];
    getInfo(): any;
    getCurrentHeight(): number;
    getBestBlockHash(): string;
    getDifficulty(): number;
    getNetworkHashRate(): number;
    getStats(): any;
    resetStats(): void;
    enableStats(enable: boolean): boolean;
    getLastError(): string;
    clearLastError(): void;
  }

  export class AssetManager {
    constructor();
    initialize(): boolean;
    shutdown(): void;
    createAsset(asset: any): string;
    getAsset(assetId: string): any;
    getAssetMetadata(assetId: string): any;
    updateAsset(assetId: string, update: any): boolean;
    deleteAsset(assetId: string): boolean;
    listAssets(limit: number, offset: number): any[];
    searchAssets(query: string, limit: number): any[];
    getAssetBalance(assetId: string, address: string): number;
    transferAsset(transfer: any): string;
    getAssetHistory(assetId: string, limit: number): any[];
    validateAssetName(assetName: string): boolean;
    getAssetStats(): any;
    isInitialized(): boolean;
    getLastError(): string;
    clearLastError(): void;
  }

  export class NetworkManager {
    constructor();
    initialize(config: any): boolean;
    shutdown(): void;
    connect(peerAddress: string): boolean;
    disconnect(peerAddress: string): boolean;
    isConnected(peerAddress: string): boolean;
    getConnectionInfo(peerAddress: string): any;
    getPeerInfo(peerAddress: string): any;
    getPeers(): any[];
    addPeer(peerAddress: string): boolean;
    removePeer(peerAddress: string): boolean;
    banPeer(peerAddress: string, banTime: number): boolean;
    unbanPeer(peerAddress: string): boolean;
    getBannedPeers(): any[];
    sendMessage(peerAddress: string, message: string): boolean;
    broadcastMessage(message: string): number;
    getNetworkStats(): any;
    resetStats(): void;
    enableStats(enable: boolean): boolean;
    getLastError(): string;
    clearLastError(): void;
  }

  export class SecurityManager {
    constructor();
    initialize(): boolean;
    shutdown(): void;
    generateKeyPair(): any;
    getPublicKey(privateKey: string): string;
    getPrivateKey(address: string): string;
    signMessage(message: string, privateKey: string): any;
    verifySignature(message: string, signature: string, publicKey: string): boolean;
    signTransaction(transaction: any, privateKey: string): any;
    verifyTransaction(transaction: any, signature: string, publicKey: string): boolean;
    hashData(data: string, algorithm: string): string;
    validateAddress(address: string): boolean;
    generateAddress(): string;
    encryptData(data: string, publicKey: string): any;
    decryptData(encryptedData: string, iv: string, privateKey: string): string;
    generateToken(payload: any, secret: string, expiration: number): string;
    validateToken(token: string, secret: string): any;
    getSecurityStats(): any;
    resetStats(): void;
    enableStats(enable: boolean): boolean;
    getLastError(): string;
    clearLastError(): void;
  }

  export class NFTManager {
    constructor();
    initialize(): boolean;
    shutdown(): void;
    createNFT(nft: any): string;
    getNFT(nftId: string): any;
    updateNFT(nftId: string, update: any): boolean;
    deleteNFT(nftId: string): boolean;
    listNFTs(limit: number, offset: number): any[];
    searchNFTs(query: string, limit: number): any[];
    transferNFT(transfer: any): string;
    getOwnershipHistory(nftId: string, limit: number): any[];
    validateNFTMetadata(metadata: any): boolean;
    getNFTStats(): any;
    isInitialized(): boolean;
    getLastError(): string;
    clearLastError(): void;
  }

  export class IPFSManager {
    constructor();
    initialize(config: any): boolean;
    shutdown(): void;
    addFile(fileData: string, fileName: string): string;
    getFile(cid: string): any;
    removeFile(cid: string): boolean;
    listFiles(limit: number, offset: number): any[];
    searchFiles(query: string, limit: number): any[];
    pinFile(cid: string): boolean;
    unpinFile(cid: string): boolean;
    getPinnedFiles(): any[];
    addDirectory(directory: any): string;
    getDirectory(cid: string): any;
    validateCID(cid: string): boolean;
    getIPFSStats(): any;
    isInitialized(): boolean;
    getLastError(): string;
    clearLastError(): void;
  }

  export const satox_native: {
    BlockchainManager: typeof BlockchainManager;
    AssetManager: typeof AssetManager;
    NetworkManager: typeof NetworkManager;
    SecurityManager: typeof SecurityManager;
    NFTManager: typeof NFTManager;
    IPFSManager: typeof IPFSManager;
  };
} 