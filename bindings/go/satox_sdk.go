/**
 * @file satox_sdk.go
 * @brief Main Go binding for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

package satox

import (
	"context"
	"fmt"
	"sync"
	"time"
)

// SatoxSDK represents the main SDK instance
type SatoxSDK struct {
	coreManager     *CoreManager
	walletManager   *WalletManager
	securityManager *SecurityManager
	assetManager    *AssetManager
	nftManager      *NFTManager
	blockchainManager *BlockchainManager
	ipfsManager     *IPFSManager
	networkManager  *NetworkManager
	databaseManager *DatabaseManager
	
	config     *Config
	isRunning  bool
	mutex      sync.RWMutex
	ctx        context.Context
	cancel     context.CancelFunc
}

// Config represents the SDK configuration
type Config struct {
	DataDir         string `json:"data_dir"`
	Network         string `json:"network"`
	EnableMining    bool   `json:"enable_mining"`
	EnableSync      bool   `json:"enable_sync"`
	SyncIntervalMs  uint32 `json:"sync_interval_ms"`
	MiningThreads   uint32 `json:"mining_threads"`
	MaxConnections  uint32 `json:"max_connections"`
	RPCPort         uint16 `json:"rpc_port"`
	P2PPort         uint16 `json:"p2p_port"`
	EnableRPC       bool   `json:"enable_rpc"`
	EnableP2P       bool   `json:"enable_p2p"`
	LogLevel        string `json:"log_level"`
	LogFile         string `json:"log_file"`
}

// DefaultConfig returns the default configuration
func DefaultConfig() *Config {
	return &Config{
		DataDir:         "./data",
		Network:         "mainnet",
		EnableMining:    false,
		EnableSync:      true,
		SyncIntervalMs:  1000,
		MiningThreads:   1,
		MaxConnections:  10,
		RPCPort:         7777,
		P2PPort:         67777,
		EnableRPC:       true,
		EnableP2P:       true,
		LogLevel:        "info",
		LogFile:         "./logs/satox-sdk.log",
	}
}

// NewSatoxSDK creates a new SDK instance
func NewSatoxSDK() *SatoxSDK {
	ctx, cancel := context.WithCancel(context.Background())
	return &SatoxSDK{
		config: DefaultConfig(),
		ctx:    ctx,
		cancel: cancel,
	}
}

// Initialize initializes the SDK with the given configuration
func (sdk *SatoxSDK) Initialize(config *Config) error {
	sdk.mutex.Lock()
	defer sdk.mutex.Unlock()

	if sdk.isRunning {
		return fmt.Errorf("SDK is already running")
	}

	if config != nil {
		sdk.config = config
	}

	// Initialize core manager
	sdk.coreManager = NewCoreManager()
	if err := sdk.coreManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize core manager: %w", err)
	}

	// Initialize wallet manager
	sdk.walletManager = NewWalletManager()
	if err := sdk.walletManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize wallet manager: %w", err)
	}

	// Initialize security manager
	sdk.securityManager = NewSecurityManager()
	if err := sdk.securityManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize security manager: %w", err)
	}

	// Initialize asset manager
	sdk.assetManager = NewAssetManager()
	if err := sdk.assetManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize asset manager: %w", err)
	}

	// Initialize NFT manager
	sdk.nftManager = NewNFTManager()
	if err := sdk.nftManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize NFT manager: %w", err)
	}

	// Initialize blockchain manager
	sdk.blockchainManager = NewBlockchainManager()
	if err := sdk.blockchainManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize blockchain manager: %w", err)
	}

	// Initialize IPFS manager
	sdk.ipfsManager = NewIPFSManager()
	if err := sdk.ipfsManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize IPFS manager: %w", err)
	}

	// Initialize network manager
	sdk.networkManager = NewNetworkManager()
	if err := sdk.networkManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize network manager: %w", err)
	}

	// Initialize database manager
	sdk.databaseManager = NewDatabaseManager()
	if err := sdk.databaseManager.Initialize(sdk.config); err != nil {
		return fmt.Errorf("failed to initialize database manager: %w", err)
	}

	sdk.isRunning = true
	return nil
}

// Start starts the SDK services
func (sdk *SatoxSDK) Start() error {
	sdk.mutex.Lock()
	defer sdk.mutex.Unlock()

	if !sdk.isRunning {
		return fmt.Errorf("SDK is not initialized")
	}

	// Start core manager
	if err := sdk.coreManager.Start(); err != nil {
		return fmt.Errorf("failed to start core manager: %w", err)
	}

	// Start other managers
	if err := sdk.walletManager.Start(); err != nil {
		return fmt.Errorf("failed to start wallet manager: %w", err)
	}

	if err := sdk.securityManager.Start(); err != nil {
		return fmt.Errorf("failed to start security manager: %w", err)
	}

	if err := sdk.assetManager.Start(); err != nil {
		return fmt.Errorf("failed to start asset manager: %w", err)
	}

	if err := sdk.nftManager.Start(); err != nil {
		return fmt.Errorf("failed to start NFT manager: %w", err)
	}

	if err := sdk.blockchainManager.Start(); err != nil {
		return fmt.Errorf("failed to start blockchain manager: %w", err)
	}

	if err := sdk.ipfsManager.Start(); err != nil {
		return fmt.Errorf("failed to start IPFS manager: %w", err)
	}

	if err := sdk.networkManager.Start(); err != nil {
		return fmt.Errorf("failed to start network manager: %w", err)
	}

	if err := sdk.databaseManager.Start(); err != nil {
		return fmt.Errorf("failed to start database manager: %w", err)
	}

	return nil
}

// Stop stops the SDK services
func (sdk *SatoxSDK) Stop() error {
	sdk.mutex.Lock()
	defer sdk.mutex.Unlock()

	if !sdk.isRunning {
		return nil
	}

	// Stop all managers
	sdk.coreManager.Stop()
	sdk.walletManager.Stop()
	sdk.securityManager.Stop()
	sdk.assetManager.Stop()
	sdk.nftManager.Stop()
	sdk.blockchainManager.Stop()
	sdk.ipfsManager.Stop()
	sdk.networkManager.Stop()
	sdk.databaseManager.Stop()

	sdk.isRunning = false
	return nil
}

// Shutdown gracefully shuts down the SDK
func (sdk *SatoxSDK) Shutdown() error {
	sdk.cancel()
	return sdk.Stop()
}

// IsRunning returns whether the SDK is running
func (sdk *SatoxSDK) IsRunning() bool {
	sdk.mutex.RLock()
	defer sdk.mutex.RUnlock()
	return sdk.isRunning
}

// GetCoreManager returns the core manager
func (sdk *SatoxSDK) GetCoreManager() *CoreManager {
	return sdk.coreManager
}

// GetWalletManager returns the wallet manager
func (sdk *SatoxSDK) GetWalletManager() *WalletManager {
	return sdk.walletManager
}

// GetSecurityManager returns the security manager
func (sdk *SatoxSDK) GetSecurityManager() *SecurityManager {
	return sdk.securityManager
}

// GetAssetManager returns the asset manager
func (sdk *SatoxSDK) GetAssetManager() *AssetManager {
	return sdk.assetManager
}

// GetNFTManager returns the NFT manager
func (sdk *SatoxSDK) GetNFTManager() *NFTManager {
	return sdk.nftManager
}

// GetBlockchainManager returns the blockchain manager
func (sdk *SatoxSDK) GetBlockchainManager() *BlockchainManager {
	return sdk.blockchainManager
}

// GetIPFSManager returns the IPFS manager
func (sdk *SatoxSDK) GetIPFSManager() *IPFSManager {
	return sdk.ipfsManager
}

// GetNetworkManager returns the network manager
func (sdk *SatoxSDK) GetNetworkManager() *NetworkManager {
	return sdk.networkManager
}

// GetDatabaseManager returns the database manager
func (sdk *SatoxSDK) GetDatabaseManager() *DatabaseManager {
	return sdk.databaseManager
}

// GetConfig returns the current configuration
func (sdk *SatoxSDK) GetConfig() *Config {
	return sdk.config
}

// Version returns the SDK version
func (sdk *SatoxSDK) Version() string {
	return "1.0.0"
}

// BuildDate returns the SDK build date
func (sdk *SatoxSDK) BuildDate() string {
	return "2025-06-30"
}

// Connect establishes a connection to the blockchain network
func (sdk *SatoxSDK) Connect() error {
	if !sdk.isRunning {
		return fmt.Errorf("SDK is not running")
	}
	return sdk.networkManager.Connect()
}

// Disconnect disconnects from the blockchain network
func (sdk *SatoxSDK) Disconnect() error {
	if !sdk.isRunning {
		return nil
	}
	return sdk.networkManager.Disconnect()
}

// WaitForShutdown waits for the SDK to be shut down
func (sdk *SatoxSDK) WaitForShutdown() {
	<-sdk.ctx.Done()
}

// WaitForShutdownWithTimeout waits for the SDK to be shut down with a timeout
func (sdk *SatoxSDK) WaitForShutdownWithTimeout(timeout time.Duration) error {
	select {
	case <-sdk.ctx.Done():
		return nil
	case <-time.After(timeout):
		return fmt.Errorf("shutdown timeout exceeded")
	}
} 