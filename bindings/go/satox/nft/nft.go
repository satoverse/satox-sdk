package nft

import (
	"context"
	"errors"
	"sync"
)

// NFT represents a non-fungible token in the Satox ecosystem
type NFT struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Description string                 `json:"description"`
	Metadata    map[string]interface{} `json:"metadata"`
	Owner       string                 `json:"owner"`
	Collection  string                 `json:"collection"`
	Properties  map[string]interface{} `json:"properties"`
}

// NFTManager handles NFT operations
type NFTManager struct {
	nfts       map[string]*NFT
	mu         sync.RWMutex
	ipfs       IPFSInterface
	blockchain BlockchainInterface
}

// IPFSInterface defines the interface for IPFS operations
type IPFSInterface interface {
	Store(data []byte) (string, error)
	Retrieve(cid string) ([]byte, error)
}

// BlockchainInterface defines the interface for blockchain operations
type BlockchainInterface interface {
	CreateNFT(nft *NFT) error
	UpdateNFT(nft *NFT) error
	GetNFT(id string) (*NFT, error)
	TransferNFT(from, to string) error
}

// NewNFTManager creates a new NFTManager instance
func NewNFTManager(ipfs IPFSInterface, blockchain BlockchainInterface) *NFTManager {
	return &NFTManager{
		nfts:       make(map[string]*NFT),
		ipfs:       ipfs,
		blockchain: blockchain,
	}
}

// CreateNFT creates a new NFT
func (nm *NFTManager) CreateNFT(ctx context.Context, nft *NFT) error {
	nm.mu.Lock()
	defer nm.mu.Unlock()

	if _, exists := nm.nfts[nft.ID]; exists {
		return errors.New("NFT already exists")
	}

	// Store metadata in IPFS
	metadataCID, err := nm.ipfs.Store([]byte(nft.Description))
	if err != nil {
		return err
	}
	nft.Metadata["ipfs_cid"] = metadataCID

	// Create NFT on blockchain
	if err := nm.blockchain.CreateNFT(nft); err != nil {
		return err
	}

	nm.nfts[nft.ID] = nft
	return nil
}

// GetNFT retrieves an NFT by ID
func (nm *NFTManager) GetNFT(ctx context.Context, id string) (*NFT, error) {
	nm.mu.RLock()
	defer nm.mu.RUnlock()

	nft, exists := nm.nfts[id]
	if !exists {
		return nil, errors.New("NFT not found")
	}
	return nft, nil
}

// UpdateNFT updates an existing NFT
func (nm *NFTManager) UpdateNFT(ctx context.Context, nft *NFT) error {
	nm.mu.Lock()
	defer nm.mu.Unlock()

	if _, exists := nm.nfts[nft.ID]; !exists {
		return errors.New("NFT not found")
	}

	if err := nm.blockchain.UpdateNFT(nft); err != nil {
		return err
	}

	nm.nfts[nft.ID] = nft
	return nil
}

// TransferNFT transfers an NFT between addresses
func (nm *NFTManager) TransferNFT(ctx context.Context, from, to string) error {
	return nm.blockchain.TransferNFT(from, to)
}

// UpdateMetadata updates NFT metadata
func (nm *NFTManager) UpdateMetadata(ctx context.Context, id string, metadata map[string]interface{}) error {
	nm.mu.Lock()
	defer nm.mu.Unlock()

	nft, exists := nm.nfts[id]
	if !exists {
		return errors.New("NFT not found")
	}

	nft.Metadata = metadata
	return nm.UpdateNFT(ctx, nft)
}

// GetNFTsByCollection retrieves all NFTs in a collection
func (nm *NFTManager) GetNFTsByCollection(ctx context.Context, collection string) ([]*NFT, error) {
	nm.mu.RLock()
	defer nm.mu.RUnlock()

	var nfts []*NFT
	for _, nft := range nm.nfts {
		if nft.Collection == collection {
			nfts = append(nfts, nft)
		}
	}
	return nfts, nil
}

// GetNFTsByOwner retrieves all NFTs owned by an address
func (nm *NFTManager) GetNFTsByOwner(ctx context.Context, owner string) ([]*NFT, error) {
	nm.mu.RLock()
	defer nm.mu.RUnlock()

	var nfts []*NFT
	for _, nft := range nm.nfts {
		if nft.Owner == owner {
			nfts = append(nfts, nft)
		}
	}
	return nfts, nil
}
