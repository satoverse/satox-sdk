package tests

import (
	"satox-sdk/satox"
	"testing"
)

func TestBlockchainManager(t *testing.T) {
	// Test creating a new blockchain manager
	config := `{"network": "mainnet", "rpc_endpoint": "http://localhost:7777"}`
	bm, err := satox.NewBlockchainManager(config)
	if err != nil {
		t.Fatalf("Failed to create blockchain manager: %v", err)
	}
	defer bm.Close()

	// Test getting block height
	height, err := bm.GetBlockHeight()
	if err != nil {
		t.Logf("Warning: Could not get block height: %v", err)
	} else {
		t.Logf("Current block height: %d", height)
	}

	// Test getting best block hash
	hash, err := bm.GetBestBlockHash()
	if err != nil {
		t.Logf("Warning: Could not get best block hash: %v", err)
	} else {
		t.Logf("Best block hash: %s", hash)
	}
}

func TestBlockchainManagerInvalidConfig(t *testing.T) {
	// Test with invalid config
	_, err := satox.NewBlockchainManager("invalid json")
	if err == nil {
		t.Error("Expected error with invalid config, got nil")
	}
}
