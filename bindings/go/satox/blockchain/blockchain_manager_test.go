package blockchain

import (
	"context"
	"testing"
	"time"
)

func TestNewBlockchainManager(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}
	if manager == nil {
		t.Fatal("NewBlockchainManager returned nil")
	}
}

func TestGetBlock(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	block, err := manager.GetBlock(context.Background(), "test-hash")
	if err != nil {
		t.Fatalf("GetBlock failed: %v", err)
	}
	if block == nil {
		t.Fatal("GetBlock returned nil block")
	}
	if block.Hash != "test-hash" {
		t.Errorf("Expected hash test-hash, got %s", block.Hash)
	}
}

func TestGetBlockByHeight(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	block, err := manager.GetBlockByHeight(context.Background(), 1)
	if err != nil {
		t.Fatalf("GetBlockByHeight failed: %v", err)
	}
	if block == nil {
		t.Fatal("GetBlockByHeight returned nil block")
	}
	if block.Height != 1 {
		t.Errorf("Expected height 1, got %d", block.Height)
	}
}

func TestGetTransaction(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	tx, err := manager.GetTransaction(context.Background(), "test-hash")
	if err != nil {
		t.Fatalf("GetTransaction failed: %v", err)
	}
	if tx == nil {
		t.Fatal("GetTransaction returned nil transaction")
	}
	if tx.Hash != "test-hash" {
		t.Errorf("Expected hash test-hash, got %s", tx.Hash)
	}
}

func TestGetBalance(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	balance, err := manager.GetBalance(context.Background(), "test-address")
	if err != nil {
		t.Fatalf("GetBalance failed: %v", err)
	}
	if balance < 0 {
		t.Errorf("Expected non-negative balance, got %f", balance)
	}
}

func TestGetTransactionHistory(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	txs, err := manager.GetTransactionHistory(context.Background(), "test-address", 10, 0)
	if err != nil {
		t.Fatalf("GetTransactionHistory failed: %v", err)
	}
	if txs == nil {
		t.Fatal("GetTransactionHistory returned nil transactions")
	}
	if len(txs) > 10 {
		t.Errorf("Expected at most 10 transactions, got %d", len(txs))
	}
}

func TestBroadcastTransaction(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	tx := &Transaction{
		Hash:      "test-hash",
		From:      "test-from",
		To:        "test-to",
		Amount:    1.0,
		Fee:       0.1,
		Timestamp: time.Now(),
		Status:    "pending",
		Data:      []byte("test-data"),
	}

	err = manager.BroadcastTransaction(context.Background(), tx)
	if err != nil {
		t.Fatalf("BroadcastTransaction failed: %v", err)
	}
}

func TestGetNetworkStatus(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	status, err := manager.GetNetworkStatus(context.Background())
	if err != nil {
		t.Fatalf("GetNetworkStatus failed: %v", err)
	}
	if status == nil {
		t.Fatal("GetNetworkStatus returned nil status")
	}
}

func TestSubscribeToBlocks(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	err = manager.SubscribeToBlocks(func(block *Block) error {
		if block == nil {
			t.Error("Received nil block")
		}
		return nil
	})
	if err != nil {
		t.Fatalf("SubscribeToBlocks failed: %v", err)
	}
}

func TestSubscribeToTransactions(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	err = manager.SubscribeToTransactions(func(tx *Transaction) error {
		if tx == nil {
			t.Error("Received nil transaction")
		}
		return nil
	})
	if err != nil {
		t.Fatalf("SubscribeToTransactions failed: %v", err)
	}
}

func TestClose(t *testing.T) {
	manager, err := NewBlockchainManager(
		"https://api.example.com",
		"wss://ws.example.com",
		"https://graphql.example.com",
		"test-api-key",
	)
	if err != nil {
		t.Fatalf("NewBlockchainManager failed: %v", err)
	}

	err = manager.Close()
	if err != nil {
		t.Fatalf("Close failed: %v", err)
	}
}
