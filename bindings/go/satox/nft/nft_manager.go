package nft

import (
	"encoding/json"
	"errors"
	"sync"
	"time"
)

// NFTMetadata represents the metadata for an NFT
type NFTMetadata struct {
	Name        string          `json:"name"`
	Description string          `json:"description"`
	Image       string          `json:"image"`
	ExternalURL string          `json:"externalUrl"`
	Attributes  json.RawMessage `json:"attributes"`
	Properties  json.RawMessage `json:"properties"`
}

// NFT represents a non-fungible token
type NFT struct {
	ID                 string      `json:"id"`
	ContractAddress    string      `json:"contractAddress"`
	Owner              string      `json:"owner"`
	Creator            string      `json:"creator"`
	Metadata           NFTMetadata `json:"metadata"`
	TokenID            uint64      `json:"tokenId"`
	TokenURI           string      `json:"tokenUri"`
	IsTransferable     bool        `json:"isTransferable"`
	IsBurnable         bool        `json:"isBurnable"`
	RoyaltyRecipient   string      `json:"royaltyRecipient"`
	RoyaltyBasisPoints uint32      `json:"royaltyBasisPoints"`
	CreatedAt          int64       `json:"createdAt"`
	UpdatedAt          int64       `json:"updatedAt"`
}

// NFTManager manages NFTs in the system
type NFTManager struct {
	nfts         map[string]*NFT
	ownerNFTs    map[string][]string // owner -> []nftID
	contractNFTs map[string][]string // contract -> []nftID
	creatorNFTs  map[string][]string // creator -> []nftID
	mutex        sync.RWMutex
	initialized  bool
}

// NewNFTManager creates a new NFTManager instance
func NewNFTManager() *NFTManager {
	return &NFTManager{
		nfts:         make(map[string]*NFT),
		ownerNFTs:    make(map[string][]string),
		contractNFTs: make(map[string][]string),
		creatorNFTs:  make(map[string][]string),
	}
}

// Initialize initializes the NFTManager
func (nm *NFTManager) Initialize() error {
	nm.mutex.Lock()
	defer nm.mutex.Unlock()

	if nm.initialized {
		return errors.New("NFTManager already initialized")
	}

	nm.initialized = true
	return nil
}

// CreateNFT creates a new NFT
func (nm *NFTManager) CreateNFT(contractAddress string, metadata NFTMetadata, creator string, isTransferable, isBurnable bool, royaltyRecipient string, royaltyBasisPoints uint32) (string, error) {
	nm.mutex.Lock()
	defer nm.mutex.Unlock()

	if !nm.initialized {
		return "", errors.New("NFTManager not initialized")
	}

	// Validate metadata
	if err := nm.validateMetadata(metadata); err != nil {
		return "", err
	}

	// Generate NFT ID
	nftID := generateNFTID(contractAddress, metadata)

	// Create NFT
	nft := &NFT{
		ID:                 nftID,
		ContractAddress:    contractAddress,
		Owner:              creator, // Creator is initial owner
		Creator:            creator,
		Metadata:           metadata,
		TokenID:            uint64(len(nm.nfts) + 1),
		TokenURI:           metadata.ExternalURL,
		IsTransferable:     isTransferable,
		IsBurnable:         isBurnable,
		RoyaltyRecipient:   royaltyRecipient,
		RoyaltyBasisPoints: royaltyBasisPoints,
		CreatedAt:          time.Now().Unix(),
		UpdatedAt:          time.Now().Unix(),
	}

	// Store NFT
	nm.nfts[nftID] = nft

	// Update indices
	nm.ownerNFTs[creator] = append(nm.ownerNFTs[creator], nftID)
	nm.contractNFTs[contractAddress] = append(nm.contractNFTs[contractAddress], nftID)
	nm.creatorNFTs[creator] = append(nm.creatorNFTs[creator], nftID)

	return nftID, nil
}

// GetNFT retrieves an NFT by ID
func (nm *NFTManager) GetNFT(nftID string) (*NFT, error) {
	nm.mutex.RLock()
	defer nm.mutex.RUnlock()

	if !nm.initialized {
		return nil, errors.New("NFTManager not initialized")
	}

	nft, exists := nm.nfts[nftID]
	if !exists {
		return nil, errors.New("NFT not found")
	}

	return nft, nil
}

// TransferNFT transfers an NFT from one owner to another
func (nm *NFTManager) TransferNFT(nftID, from, to string) error {
	nm.mutex.Lock()
	defer nm.mutex.Unlock()

	if !nm.initialized {
		return errors.New("NFTManager not initialized")
	}

	// Check if NFT exists
	nft, exists := nm.nfts[nftID]
	if !exists {
		return errors.New("NFT not found")
	}

	// Check if NFT is transferable
	if !nft.IsTransferable {
		return errors.New("NFT is not transferable")
	}

	// Check if sender is the owner
	if nft.Owner != from {
		return errors.New("Sender is not the owner")
	}

	// Update NFT
	nft.Owner = to
	nft.UpdatedAt = time.Now().Unix()

	// Update indices
	nm.removeFromOwnerNFTs(from, nftID)
	nm.ownerNFTs[to] = append(nm.ownerNFTs[to], nftID)

	return nil
}

// BurnNFT burns an NFT
func (nm *NFTManager) BurnNFT(nftID, owner string) error {
	nm.mutex.Lock()
	defer nm.mutex.Unlock()

	if !nm.initialized {
		return errors.New("NFTManager not initialized")
	}

	// Check if NFT exists
	nft, exists := nm.nfts[nftID]
	if !exists {
		return errors.New("NFT not found")
	}

	// Check if NFT is burnable
	if !nft.IsBurnable {
		return errors.New("NFT is not burnable")
	}

	// Check if sender is the owner
	if nft.Owner != owner {
		return errors.New("Sender is not the owner")
	}

	// Remove NFT from indices
	nm.removeFromOwnerNFTs(owner, nftID)
	nm.removeFromContractNFTs(nft.ContractAddress, nftID)
	nm.removeFromCreatorNFTs(nft.Creator, nftID)

	// Remove NFT
	delete(nm.nfts, nftID)

	return nil
}

// GetNFTsByOwner gets all NFTs owned by an address
func (nm *NFTManager) GetNFTsByOwner(owner string) ([]*NFT, error) {
	nm.mutex.RLock()
	defer nm.mutex.RUnlock()

	if !nm.initialized {
		return nil, errors.New("NFTManager not initialized")
	}

	nftIDs, exists := nm.ownerNFTs[owner]
	if !exists {
		return []*NFT{}, nil
	}

	nfts := make([]*NFT, 0, len(nftIDs))
	for _, nftID := range nftIDs {
		if nft, exists := nm.nfts[nftID]; exists {
			nfts = append(nfts, nft)
		}
	}

	return nfts, nil
}

// validateMetadata validates NFT metadata
func (nm *NFTManager) validateMetadata(metadata NFTMetadata) error {
	if metadata.Name == "" {
		return errors.New("NFT name cannot be empty")
	}

	if metadata.Description == "" {
		return errors.New("NFT description cannot be empty")
	}

	if metadata.Image == "" {
		return errors.New("NFT image cannot be empty")
	}

	return nil
}

// generateNFTID generates a unique NFT ID
func generateNFTID(contractAddress string, metadata NFTMetadata) string {
	// In a real implementation, this would use a more sophisticated ID generation
	// For now, we'll use a simple concatenation of metadata fields
	return contractAddress + "_" + metadata.Name
}

// Helper functions for managing indices
func (nm *NFTManager) removeFromOwnerNFTs(owner, nftID string) {
	nfts := nm.ownerNFTs[owner]
	for i, id := range nfts {
		if id == nftID {
			nm.ownerNFTs[owner] = append(nfts[:i], nfts[i+1:]...)
			break
		}
	}
}

func (nm *NFTManager) removeFromContractNFTs(contract, nftID string) {
	nfts := nm.contractNFTs[contract]
	for i, id := range nfts {
		if id == nftID {
			nm.contractNFTs[contract] = append(nfts[:i], nfts[i+1:]...)
			break
		}
	}
}

func (nm *NFTManager) removeFromCreatorNFTs(creator, nftID string) {
	nfts := nm.creatorNFTs[creator]
	for i, id := range nfts {
		if id == nftID {
			nm.creatorNFTs[creator] = append(nfts[:i], nfts[i+1:]...)
			break
		}
	}
}
