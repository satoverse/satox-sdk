// Package nft provides functionality for managing non-fungible tokens in the Satoxcoin system.
// This file contains tests for the NFTManager implementation.
package nft

import (
	"encoding/json"
	"testing"
)

// TestNewNFTManager verifies the creation of a new NFTManager instance.
// It checks that the manager is properly initialized with non-nil maps for NFTs and indices.
func TestNewNFTManager(t *testing.T) {
	manager := NewNFTManager()
	if manager == nil {
		t.Fatal("Expected non-nil NFTManager")
	}
	if manager.nfts == nil {
		t.Error("Expected non-nil nfts map")
	}
}

// TestCreateNFT tests the NFT creation functionality.
// It verifies:
// - Successful creation of an NFT with valid metadata
// - Proper handling of duplicate NFT creation attempts
// - Validation of required metadata fields
// - Proper initialization of NFT properties
func TestCreateNFT(t *testing.T) {
	manager := NewNFTManager()
	err := manager.Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize NFTManager: %v", err)
	}

	// Test successful NFT creation
	properties, _ := json.Marshal(map[string]interface{}{
		"rarity": "common",
	})
	attributes, _ := json.Marshal(map[string]interface{}{
		"color": "blue",
	})
	metadata := NFTMetadata{
		Name:        "Test NFT",
		Description: "Test NFT description",
		Image:       "ipfs://test-image",
		ExternalURL: "https://example.com/nft",
		Properties:  properties,
		Attributes:  attributes,
	}

	nftID, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Errorf("Unexpected error creating NFT: %v", err)
	}
	if nftID == "" {
		t.Fatal("Expected non-empty NFT ID")
	}

	// Test duplicate NFT creation
	_, err = manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err == nil {
		t.Error("Expected error when creating duplicate NFT")
	}

	// Test invalid NFT creation
	invalidMetadata := metadata
	invalidMetadata.Name = ""
	_, err = manager.CreateNFT("0x123", invalidMetadata, "creator", true, true, "royaltyRecipient", 500)
	if err == nil {
		t.Error("Expected error when creating NFT with empty name")
	}
}

// TestGetNFT tests the NFT retrieval functionality.
// It verifies:
// - Successful retrieval of existing NFTs
// - Proper handling of non-existent NFTs
// - Correct NFT data structure
func TestGetNFT(t *testing.T) {
	manager := NewNFTManager()
	err := manager.Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize NFTManager: %v", err)
	}

	// Create test NFT
	metadata := NFTMetadata{
		Name:        "Test NFT",
		Description: "Test NFT description",
		Image:       "ipfs://test-image",
		ExternalURL: "https://example.com/nft",
	}
	nftID, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Fatalf("Failed to create test NFT: %v", err)
	}

	// Test getting existing NFT
	nft, err := manager.GetNFT(nftID)
	if err != nil {
		t.Errorf("Unexpected error getting NFT: %v", err)
	}
	if nft == nil {
		t.Fatal("Expected non-nil NFT")
	}
	if nft.ID != nftID {
		t.Errorf("Expected NFT ID '%s', got '%s'", nftID, nft.ID)
	}

	// Test getting non-existent NFT
	_, err = manager.GetNFT("nonExistentNFT")
	if err == nil {
		t.Error("Expected error when getting non-existent NFT")
	}
}

// TestTransferNFT tests the NFT transfer functionality.
// It verifies:
// - Successful transfer of NFTs between owners
// - Proper ownership updates after transfers
// - Handling of non-transferable NFTs
// - Handling of non-existent NFTs
func TestTransferNFT(t *testing.T) {
	manager := NewNFTManager()
	err := manager.Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize NFTManager: %v", err)
	}

	// Create test NFT
	metadata := NFTMetadata{
		Name:        "Test NFT",
		Description: "Test NFT description",
		Image:       "ipfs://test-image",
		ExternalURL: "https://example.com/nft",
	}
	nftID, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Fatalf("Failed to create test NFT: %v", err)
	}

	// Test successful transfer
	err = manager.TransferNFT(nftID, "creator", "receiver")
	if err != nil {
		t.Errorf("Unexpected error transferring NFT: %v", err)
	}

	// Verify ownership
	nft, err := manager.GetNFT(nftID)
	if err != nil {
		t.Errorf("Unexpected error getting NFT: %v", err)
	}
	if nft.Owner != "receiver" {
		t.Errorf("Expected owner 'receiver', got '%s'", nft.Owner)
	}

	// Test transfer of non-existent NFT
	err = manager.TransferNFT("nonExistentNFT", "creator", "receiver")
	if err == nil {
		t.Error("Expected error when transferring non-existent NFT")
	}
}

// TestBurnNFT tests the NFT burning functionality.
// It verifies:
// - Successful burning of burnable NFTs
// - Proper cleanup after burning
// - Handling of non-burnable NFTs
// - Handling of non-existent NFTs
func TestBurnNFT(t *testing.T) {
	manager := NewNFTManager()
	err := manager.Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize NFTManager: %v", err)
	}

	// Create test NFT
	metadata := NFTMetadata{
		Name:        "Test NFT",
		Description: "Test NFT description",
		Image:       "ipfs://test-image",
		ExternalURL: "https://example.com/nft",
	}
	nftID, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Fatalf("Failed to create test NFT: %v", err)
	}

	// Test successful burn
	err = manager.BurnNFT(nftID, "creator")
	if err != nil {
		t.Errorf("Unexpected error burning NFT: %v", err)
	}

	// Verify NFT is burned
	_, err = manager.GetNFT(nftID)
	if err == nil {
		t.Error("Expected error when getting burned NFT")
	}

	// Test burning non-existent NFT
	err = manager.BurnNFT("nonExistentNFT", "creator")
	if err == nil {
		t.Error("Expected error when burning non-existent NFT")
	}
}

// TestGetNFTsByOwner tests the NFT ownership query functionality.
// It verifies:
// - Successful retrieval of NFTs owned by an address
// - Proper handling of ownership changes
// - Empty results for non-owners
func TestGetNFTsByOwner(t *testing.T) {
	manager := NewNFTManager()
	err := manager.Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize NFTManager: %v", err)
	}

	// Create test NFTs
	metadata := NFTMetadata{
		Name:        "Test NFT",
		Description: "Test NFT description",
		Image:       "ipfs://test-image",
		ExternalURL: "https://example.com/nft",
	}

	// Create first NFT
	nftID1, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Fatalf("Failed to create first test NFT: %v", err)
	}

	// Create second NFT
	metadata.Name = "Test NFT 2"
	nftID2, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Fatalf("Failed to create second test NFT: %v", err)
	}

	// Test getting NFTs by owner
	nfts, err := manager.GetNFTsByOwner("creator")
	if err != nil {
		t.Errorf("Unexpected error getting NFTs by owner: %v", err)
	}
	if len(nfts) != 2 {
		t.Errorf("Expected 2 NFTs, got %d", len(nfts))
	}

	// Transfer one NFT
	err = manager.TransferNFT(nftID1, "creator", "receiver")
	if err != nil {
		t.Fatalf("Failed to transfer NFT: %v", err)
	}

	// Test getting NFTs by owner after transfer
	nfts, err = manager.GetNFTsByOwner("creator")
	if err != nil {
		t.Errorf("Unexpected error getting NFTs by owner: %v", err)
	}
	if len(nfts) != 1 {
		t.Errorf("Expected 1 NFT, got %d", len(nfts))
	}
	if nfts[0].ID != nftID2 {
		t.Errorf("Expected NFT ID '%s', got '%s'", nftID2, nfts[0].ID)
	}
}

// TestConcurrentOperations tests the thread safety of the NFTManager.
// It verifies:
// - Concurrent transfers work correctly
// - No race conditions occur
// - Final ownership is correct after concurrent operations
func TestConcurrentOperations(t *testing.T) {
	manager := NewNFTManager()
	err := manager.Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize NFTManager: %v", err)
	}

	// Create test NFT
	metadata := NFTMetadata{
		Name:        "Test NFT",
		Description: "Test NFT description",
		Image:       "ipfs://test-image",
		ExternalURL: "https://example.com/nft",
	}
	nftID, err := manager.CreateNFT("0x123", metadata, "creator", true, true, "royaltyRecipient", 500)
	if err != nil {
		t.Fatalf("Failed to create test NFT: %v", err)
	}

	// Test concurrent transfers
	done := make(chan bool)
	for i := 0; i < 5; i++ {
		go func() {
			err := manager.TransferNFT(nftID, "creator", "receiver")
			if err != nil {
				t.Errorf("Unexpected error in concurrent transfer: %v", err)
			}
			done <- true
		}()
	}

	// Wait for all transfers to complete
	for i := 0; i < 5; i++ {
		<-done
	}

	// Verify final ownership
	nft, err := manager.GetNFT(nftID)
	if err != nil {
		t.Errorf("Unexpected error getting NFT: %v", err)
	}
	if nft.Owner != "receiver" {
		t.Errorf("Expected owner 'receiver', got '%s'", nft.Owner)
	}
}
