# Satox SDK Go API Documentation

## Table of Contents
1. [Getting Started](#getting-started)
2. [Core Components](#core-components)
3. [Blockchain Integration](#blockchain-integration)
4. [Asset Management](#asset-management)
5. [NFT Management](#nft-management)
6. [Security](#security)
7. [Examples](#examples)

## Getting Started

### Installation
Install the Satox SDK using go get:

```bash
go get github.com/satox/satox-sdk
```

### Basic Usage
```go
package main

import (
    "context"
    "log"

    "github.com/satox/satox-sdk"
    "github.com/satox/satox-sdk/blockchain"
    "github.com/satox/satox-sdk/asset"
    "github.com/satox/satox-sdk/nft"
)

func main() {
    // Initialize SDK
    sdk := satox.NewSDK()
    if err := sdk.Initialize(); err != nil {
        log.Fatal(err)
    }
    defer sdk.Shutdown()

    // Get managers
    blockchainManager := sdk.BlockchainManager()
    assetManager := sdk.AssetManager()
    nftManager := sdk.NFTManager()

    // Use managers...
}
```

## Core Components

### SDK
The main entry point for the SDK.

```go
type SDK struct {
    // ... internal fields
}

func NewSDK() *SDK
func NewSDKWithConfig(config *Config) *SDK

func (s *SDK) Initialize() error
func (s *SDK) Shutdown() error

func (s *SDK) BlockchainManager() *blockchain.Manager
func (s *SDK) AssetManager() *asset.Manager
func (s *SDK) NFTManager() *nft.Manager
func (s *SDK) SecurityManager() *security.Manager
func (s *SDK) IPFSManager() *ipfs.Manager
```

### Config
Configuration for the SDK.

```go
type Config struct {
    Network       string // "mainnet" or "testnet"
    DataDir       string // Data directory path
    EnableMining  bool   // Enable mining
    MiningThreads int    // Number of mining threads
    EnableSync    bool   // Enable blockchain sync
    SyncInterval  int    // Sync interval in milliseconds
    RPCEndpoint   string // RPC endpoint URL
    RPCUsername   string // RPC username
    RPCPassword   string // RPC password
    Timeout       int    // Timeout in milliseconds
}
```

## Blockchain Integration

### BlockchainManager
Manages blockchain operations.

```go
type Manager struct {
    // ... internal fields
}

func (m *Manager) GetBlockchainInfo(ctx context.Context) (*BlockchainInfo, error)
func (m *Manager) GetBlock(ctx context.Context, hashOrHeight interface{}) (*Block, error)
func (m *Manager) GetTransaction(ctx context.Context, txid string) (*Transaction, error)
func (m *Manager) SendRawTransaction(ctx context.Context, rawTx string) (string, error)
func (m *Manager) CreateRawTransaction(ctx context.Context, inputs []Input, outputs []Output) (string, error)
func (m *Manager) SignRawTransaction(ctx context.Context, rawTx string, privateKeys []string) (string, error)
```

## Asset Management

### AssetManager
Manages asset operations.

```go
type Manager struct {
    // ... internal fields
}

func (m *Manager) CreateAsset(ctx context.Context, metadata map[string]interface{}) (string, error)
func (m *Manager) GetAsset(ctx context.Context, assetID string) (*Asset, error)
func (m *Manager) UpdateAsset(ctx context.Context, assetID string, metadata map[string]interface{}) error
func (m *Manager) TransferAsset(ctx context.Context, assetID, from, to string, amount int64) error
func (m *Manager) GetBalance(ctx context.Context, address, assetID string) (int64, error)
func (m *Manager) GetAssetHistory(ctx context.Context, assetID string) ([]*AssetEvent, error)
```

## NFT Management

### NFTManager
Manages NFT operations.

```go
type Manager struct {
    // ... internal fields
}

func (m *Manager) CreateNFT(ctx context.Context, metadata map[string]interface{}) (string, error)
func (m *Manager) GetNFT(ctx context.Context, nftID string) (*NFT, error)
func (m *Manager) TransferNFT(ctx context.Context, nftID, from, to string) error
func (m *Manager) GetNFTOwner(ctx context.Context, nftID string) (string, error)
func (m *Manager) GetNFTCollection(ctx context.Context, collectionID string) ([]*NFT, error)
```

## Security

### SecurityManager
Manages security features.

```go
type Manager struct {
    // ... internal fields
}

func (m *Manager) Initialize() error
func (m *Manager) Shutdown() error
func (m *Manager) SetPolicy(policy map[string]interface{}) error
func (m *Manager) GetStatus() (*SecurityStatus, error)
func (m *Manager) EnableFeature(feature string) error
func (m *Manager) DisableFeature(feature string) error
```

## Examples

### Creating and Transferring an Asset
```go
// Initialize SDK
sdk := satox.NewSDK()
if err := sdk.Initialize(); err != nil {
    log.Fatal(err)
}
defer sdk.Shutdown()

// Get asset manager
assetManager := sdk.AssetManager()

// Create asset metadata
metadata := map[string]interface{}{
    "name":        "My Asset",
    "symbol":      "MYA",
    "totalSupply": int64(1000000),
    "decimals":    8,
    "reissuable":  true,
}

// Create asset
assetID, err := assetManager.CreateAsset(context.Background(), metadata)
if err != nil {
    log.Fatal(err)
}

// Transfer asset
fromAddress := "my_address"
toAddress := "recipient_address"
amount := int64(1000)
if err := assetManager.TransferAsset(context.Background(), assetID, fromAddress, toAddress, amount); err != nil {
    log.Fatal(err)
}
```

### Creating and Managing NFTs
```go
// Get NFT manager
nftManager := sdk.NFTManager()

// Create NFT metadata
metadata := map[string]interface{}{
    "name":        "My NFT",
    "description": "A unique NFT",
    "image":       "ipfs://image-hash",
    "attributes":  map[string]interface{}{},
}

// Create NFT
nftID, err := nftManager.CreateNFT(context.Background(), metadata)
if err != nil {
    log.Fatal(err)
}

// Transfer NFT
fromAddress := "my_address"
toAddress := "recipient_address"
if err := nftManager.TransferNFT(context.Background(), nftID, fromAddress, toAddress); err != nil {
    log.Fatal(err)
}
```

### Blockchain Operations
```go
// Get blockchain manager
blockchainManager := sdk.BlockchainManager()

// Get blockchain info
info, err := blockchainManager.GetBlockchainInfo(context.Background())
if err != nil {
    log.Fatal(err)
}
log.Printf("Current height: %d", info.Height)

// Get block by hash
block, err := blockchainManager.GetBlock(context.Background(), "block_hash")
if err != nil {
    log.Fatal(err)
}
log.Printf("Block timestamp: %d", block.Timestamp)

// Create and send transaction
inputs := []blockchain.Input{}
outputs := []blockchain.Output{}
// Add inputs and outputs...

rawTx, err := blockchainManager.CreateRawTransaction(context.Background(), inputs, outputs)
if err != nil {
    log.Fatal(err)
}

signedTx, err := blockchainManager.SignRawTransaction(context.Background(), rawTx, privateKeys)
if err != nil {
    log.Fatal(err)
}

txID, err := blockchainManager.SendRawTransaction(context.Background(), signedTx)
if err != nil {
    log.Fatal(err)
}
``` 