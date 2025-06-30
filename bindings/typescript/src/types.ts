/**
 * @file types.ts
 * @brief TypeScript type definitions for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

// Common types
export type Bytes = Uint8Array;
export type Address = string;
export type Hash = string;
export type Signature = string;
export type PublicKey = string;
export type PrivateKey = string;

// Block types
export interface BlockInfo {
    hash: Hash;
    height: number;
    timestamp: number;
    parentHash: Hash;
    transactions: TransactionInfo[];
    metadata: Record<string, any>;
}

// Transaction types
export interface TransactionInfo {
    hash: Hash;
    from: Address;
    to: Address;
    value: string;
    data: Bytes;
    nonce: number;
    timestamp: number;
    status: TransactionStatus;
    metadata: Record<string, any>;
}

export enum TransactionStatus {
    Pending = 'pending',
    Confirmed = 'confirmed',
    Failed = 'failed',
    Unknown = 'unknown'
}

export interface TransactionReceipt {
    transactionHash: Hash;
    blockHash: Hash;
    blockNumber: number;
    status: boolean;
    gasUsed: number;
    logs: TransactionLog[];
}

export interface TransactionLog {
    address: Address;
    topics: string[];
    data: Bytes;
    blockNumber: number;
    transactionHash: Hash;
    logIndex: number;
}

// Network types
export interface NodeInfo {
    host: string;
    port: number;
    version: string;
    networkType: string;
    isConnected: boolean;
    lastSeen: number;
    metadata: Record<string, any>;
}

export interface NetworkStatus {
    connectedNodes: number;
    networkType: string;
    version: string;
    uptime: number;
    metadata: Record<string, any>;
}

// Security types
export interface KeyPair {
    privateKey: PrivateKey;
    publicKey: PublicKey;
}

export interface AlgorithmInfo {
    name: string;
    version: string;
    securityLevel: string;
    parameters: Record<string, any>;
}

// Config types
export interface ConfigOptions {
    format: string;
    path: string;
    autoSave: boolean;
    encryption: boolean;
}

// Quantum types
export interface QuantumKeyPair {
    privateKey: PrivateKey;
    publicKey: PublicKey;
    algorithm: string;
    metadata: Record<string, any>;
}

export interface KeyMetadata {
    algorithm: string;
    created: number;
    expires: number;
    access: string[];
    metadata: Record<string, any>;
}

// Error types
export class SatoxError extends Error {
    constructor(message: string, public code: string) {
        super(message);
        this.name = 'SatoxError';
    }
}

// Callback types
export type MessageHandler = (message: Bytes) => void;
export type ErrorHandler = (error: SatoxError) => void;
export type StatusHandler = (status: any) => void;

export enum DatabaseType {
    SQLITE = 'sqlite',
    POSTGRESQL = 'postgresql',
    MYSQL = 'mysql',
    REDIS = 'redis',
    MONGODB = 'mongodb',
    SUPABASE = 'supabase',
    FIREBASE = 'firebase',
    AWS = 'aws',
    AZURE = 'azure',
    GOOGLE_CLOUD = 'google_cloud'
}

export enum AssetType {
    TOKEN = 'token',
    NFT = 'nft',
    CURRENCY = 'currency'
}

export enum NetworkType {
    MAINNET = 'mainnet',
    TESTNET = 'testnet',
    REGTEST = 'regtest'
}

// Base configuration interface
export interface BaseConfig {
    name?: string;
    enableLogging?: boolean;
    logPath?: string;
}

// Core configuration
export interface CoreConfig extends BaseConfig {
    network?: 'mainnet' | 'testnet' | 'regtest';
    data_dir?: string;
    enable_mining?: boolean;
    enable_sync?: boolean;
    sync_interval_ms?: number;
    mining_threads?: number;
    rpc_endpoint?: string;
    rpc_username?: string;
    rpc_password?: string;
    timeout_ms?: number;
}

// Database configuration
export interface DatabaseConfig extends BaseConfig {
    maxConnections?: number;
    connectionTimeout?: number;
}

// Security configuration
export interface SecurityConfig extends BaseConfig {
    enablePQC?: boolean;
    enableInputValidation?: boolean;
    enableRateLimiting?: boolean;
}

// Wallet configuration
export interface WalletConfig extends BaseConfig {
    encryptionEnabled?: boolean;
    backupEnabled?: boolean;
    autoBackupInterval?: number;
}

// Asset configuration
export interface AssetConfig extends BaseConfig {
    maxAssets?: number;
    enableHistory?: boolean;
    historyRetentionDays?: number;
}

// NFT configuration
export interface NFTConfig extends BaseConfig {
    maxNFTs?: number;
    enableMetadata?: boolean;
    metadataValidation?: boolean;
}

// IPFS configuration
export interface IPFSConfig extends BaseConfig {
    api_endpoint?: string;
    enable_pinning?: boolean;
    pinning_service?: string;
    gateway_url?: string;
}

// Blockchain configuration
export interface BlockchainConfig extends BaseConfig {
    network?: string;
    enable_sync?: boolean;
    sync_mode?: 'fast' | 'full';
    prune_height?: number;
}

// Network configuration
export interface NetworkConfig extends BaseConfig {
    host?: string;
    port?: number;
    enable_ssl?: boolean;
    ssl_cert_path?: string;
    ssl_key_path?: string;
    max_connections?: number;
    connection_timeout?: number;
}

// SDK configuration
export interface SDKConfig {
    core?: CoreConfig;
    database?: DatabaseConfig;
    security?: SecurityConfig;
    wallet?: WalletConfig;
    asset?: AssetConfig;
    nft?: NFTConfig;
    ipfs?: IPFSConfig;
    blockchain?: BlockchainConfig;
    network?: NetworkConfig;
}

// Asset metadata
export interface AssetMetadata {
    name: string;
    symbol: string;
    total_supply: number;
    decimals: number;
    description?: string;
    website?: string;
    icon?: string;
    owner?: string;
    attributes?: Record<string, any>;
}

// Asset information
export interface AssetInfo {
    id: string;
    metadata: AssetMetadata;
    created_at: string;
    state: 'created' | 'issued' | 'transferred' | 'burned';
    total_transfers: number;
    current_holders: number;
}

// NFT metadata
export interface NFTMetadata {
    name: string;
    description?: string;
    image?: string;
    external_url?: string;
    attributes?: Array<{
        trait_type: string;
        value: string | number;
        display_type?: string;
    }>;
    properties?: Record<string, any>;
    metadata?: Record<string, any>;
}

// NFT information
export interface NFTInfo {
    id: string;
    metadata: NFTMetadata;
    created_at: string;
    minted_at?: string;
    owner?: string;
    state: 'created' | 'minted' | 'transferred' | 'burned';
    token_uri?: string;
    history: string[];
}

// Wallet information
export interface WalletInfo {
    id: string;
    name: string;
    created_at: string;
    balance: number;
    address_count: number;
    transaction_count: number;
    last_activity?: string;
}

// Address information
export interface AddressInfo {
    address: string;
    wallet_id: string;
    created_at: string;
    balance: number;
    transaction_count: number;
    is_change: boolean;
    derivation_path?: string;
}

// Transaction information
export interface TransactionInfo {
    id: string;
    from_address: string;
    to_address: string;
    amount: number;
    fee: number;
    timestamp: string;
    status: 'pending' | 'confirmed' | 'failed';
    block_height?: number;
    confirmations?: number;
    raw_transaction?: string;
}

// Security key pair
export interface KeyPair {
    public_key: string;
    private_key: string;
    key_id: string;
    created_at: string;
    algorithm: 'PQC_KYBER' | 'PQC_DILITHIUM' | 'PQC_SPHINCS';
}

// Signature information
export interface SignatureInfo {
    signature: string;
    key_id: string;
    algorithm: string;
    created_at: string;
    data_hash: string;
}

// Database query result
export interface QueryResult {
    success: boolean;
    data?: any[];
    error?: string;
    affected_rows?: number;
    execution_time?: number;
}

// Block information
export interface BlockInfo {
    height: number;
    hash: string;
    previous_hash: string;
    timestamp: string;
    transactions: number;
    size: number;
    difficulty: number;
    nonce: number;
    merkle_root: string;
}

// Blockchain information
export interface BlockchainInfo {
    network: string;
    current_height: number;
    best_block_hash: string;
    difficulty: number;
    hash_rate: number;
    total_transactions: number;
    total_supply: number;
    is_syncing: boolean;
    sync_progress?: number;
}

// Network peer information
export interface PeerInfo {
    address: string;
    port: number;
    version: string;
    subversion: string;
    connection_time: number;
    last_seen: string;
    services: string[];
    inbound: boolean;
    sync_headers: number;
    sync_blocks: number;
}

// Network connection information
export interface ConnectionInfo {
    total_connections: number;
    inbound_connections: number;
    outbound_connections: number;
    banned_peers: number;
    network_type: string;
    node_type: string;
}

// IPFS file information
export interface IPFSFileInfo {
    hash: string;
    name: string;
    size: number;
    type: 'file' | 'directory';
    pinned: boolean;
    uploaded_at: string;
    metadata?: Record<string, any>;
}

// Core status information
export interface CoreStatus {
    initialized: boolean;
    network: string;
    uptime: string;
    version: string;
    build_date: string;
    features: string[];
}

// Event types
export interface SDKEvent {
    type: string;
    timestamp: string;
    data?: any;
}

export interface WalletEvent extends SDKEvent {
    type: 'wallet_created' | 'wallet_deleted' | 'address_generated' | 'transaction_sent' | 'transaction_received';
    wallet_id: string;
    data?: WalletInfo | AddressInfo | TransactionInfo;
}

export interface AssetEvent extends SDKEvent {
    type: 'asset_created' | 'asset_transferred' | 'asset_burned';
    asset_id: string;
    data?: AssetInfo | TransactionInfo;
}

export interface NFTEvent extends SDKEvent {
    type: 'nft_created' | 'nft_minted' | 'nft_transferred' | 'nft_burned';
    nft_id: string;
    data?: NFTInfo | TransactionInfo;
}

export interface SecurityEvent extends SDKEvent {
    type: 'key_generated' | 'data_signed' | 'data_verified' | 'data_encrypted' | 'data_decrypted';
    key_id?: string;
    data?: KeyPair | SignatureInfo;
}

// Callback types
export type EventCallback = (event: SDKEvent) => void;
export type ErrorCallback = (error: SatoxError) => void;
export type SuccessCallback<T = any> = (result: T) => void;

// Promise-based result types
export type AsyncResult<T> = Promise<T>;
export type AsyncVoid = Promise<void>;
export type AsyncBoolean = Promise<boolean>;
export type AsyncString = Promise<string>;
export type AsyncNumber = Promise<number>;

// Manager interface
export interface IManager {
    isInitialized(): boolean;
    getLastError(): string;
    getStats(): Record<string, any>;
    resetStats(): void;
}

// SDK interface
export interface ISDK {
    initialize(config: SDKConfig): AsyncBoolean;
    shutdown(): AsyncVoid;
    isInitialized(): boolean;
    getVersion(): string;
    getConfig(): SDKConfig;
    on(event: string, callback: EventCallback): void;
    off(event: string, callback: EventCallback): void;
} 