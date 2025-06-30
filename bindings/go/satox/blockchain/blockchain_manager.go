package blockchain

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"sync"
	"time"

	"github.com/satox/satox-sdk/bindings/go/api"
)

// BlockchainManager handles blockchain operations
type BlockchainManager struct {
	mu      sync.RWMutex
	api     *api.RestAPIClient
	ws      *api.WebSocketClient
	graphql *api.GraphQLClient
}

// Block represents a blockchain block
type Block struct {
	Hash         string    `json:"hash"`
	Height       uint64    `json:"height"`
	Timestamp    time.Time `json:"timestamp"`
	Transactions []string  `json:"transactions"`
	Reward       float64   `json:"reward"`
	Difficulty   float64   `json:"difficulty"`
	Size         int64     `json:"size"`
}

// Transaction represents a blockchain transaction
type Transaction struct {
	Hash      string    `json:"hash"`
	From      string    `json:"from"`
	To        string    `json:"to"`
	Amount    float64   `json:"amount"`
	Fee       float64   `json:"fee"`
	Timestamp time.Time `json:"timestamp"`
	Status    string    `json:"status"`
	Data      []byte    `json:"data"`
}

// NewBlockchainManager creates a new blockchain manager
func NewBlockchainManager(apiURL, wsURL, graphqlURL, apiKey string) (*BlockchainManager, error) {
	restAPI := api.NewRestAPIClient(apiURL, apiKey)
	wsClient, err := api.NewWebSocketClient(wsURL, apiKey)
	if err != nil {
		return nil, fmt.Errorf("failed to create WebSocket client: %w", err)
	}
	graphqlClient := api.NewGraphQLClient(graphqlURL, apiKey)

	return &BlockchainManager{
		api:     restAPI,
		ws:      wsClient,
		graphql: graphqlClient,
	}, nil
}

// GetBlock retrieves a block by hash
func (m *BlockchainManager) GetBlock(ctx context.Context, hash string) (*Block, error) {
	resp, err := m.api.Get(ctx, fmt.Sprintf("/blocks/%s", hash), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to get block: %w", err)
	}

	var block Block
	if err := json.Unmarshal(resp.Body, &block); err != nil {
		return nil, fmt.Errorf("failed to unmarshal block: %w", err)
	}

	return &block, nil
}

// GetBlockByHeight retrieves a block by height
func (m *BlockchainManager) GetBlockByHeight(ctx context.Context, height uint64) (*Block, error) {
	resp, err := m.api.Get(ctx, fmt.Sprintf("/blocks/height/%d", height), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to get block by height: %w", err)
	}

	var block Block
	if err := json.Unmarshal(resp.Body, &block); err != nil {
		return nil, fmt.Errorf("failed to unmarshal block: %w", err)
	}

	return &block, nil
}

// GetTransaction retrieves a transaction by hash
func (m *BlockchainManager) GetTransaction(ctx context.Context, hash string) (*Transaction, error) {
	resp, err := m.api.Get(ctx, fmt.Sprintf("/transactions/%s", hash), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to get transaction: %w", err)
	}

	var tx Transaction
	if err := json.Unmarshal(resp.Body, &tx); err != nil {
		return nil, fmt.Errorf("failed to unmarshal transaction: %w", err)
	}

	return &tx, nil
}

// GetBalance retrieves the balance for an address
func (m *BlockchainManager) GetBalance(ctx context.Context, address string) (float64, error) {
	resp, err := m.api.Get(ctx, fmt.Sprintf("/addresses/%s/balance", address), nil)
	if err != nil {
		return 0, fmt.Errorf("failed to get balance: %w", err)
	}

	var result struct {
		Balance float64 `json:"balance"`
	}
	if err := json.Unmarshal(resp.Body, &result); err != nil {
		return 0, fmt.Errorf("failed to unmarshal balance: %w", err)
	}

	return result.Balance, nil
}

// GetTransactionHistory retrieves transaction history for an address
func (m *BlockchainManager) GetTransactionHistory(ctx context.Context, address string, limit, offset int) ([]*Transaction, error) {
	resp, err := m.api.Get(ctx, fmt.Sprintf("/addresses/%s/transactions?limit=%d&offset=%d", address, limit, offset), nil)
	if err != nil {
		return nil, fmt.Errorf("failed to get transaction history: %w", err)
	}

	var transactions []*Transaction
	if err := json.Unmarshal(resp.Body, &transactions); err != nil {
		return nil, fmt.Errorf("failed to unmarshal transactions: %w", err)
	}

	return transactions, nil
}

// BroadcastTransaction broadcasts a transaction to the network
func (m *BlockchainManager) BroadcastTransaction(ctx context.Context, tx *Transaction) error {
	txBytes, err := json.Marshal(tx)
	if err != nil {
		return fmt.Errorf("failed to marshal transaction: %w", err)
	}

	resp, err := m.api.Post(ctx, "/transactions", txBytes, nil)
	if err != nil {
		return fmt.Errorf("failed to broadcast transaction: %w", err)
	}

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("failed to broadcast transaction: status %d", resp.StatusCode)
	}

	return nil
}

// GetNetworkStatus retrieves the current network status
func (m *BlockchainManager) GetNetworkStatus(ctx context.Context) (map[string]interface{}, error) {
	resp, err := m.api.Get(ctx, "/network/status", nil)
	if err != nil {
		return nil, fmt.Errorf("failed to get network status: %w", err)
	}

	var status map[string]interface{}
	if err := json.Unmarshal(resp.Body, &status); err != nil {
		return nil, fmt.Errorf("failed to unmarshal network status: %w", err)
	}

	return status, nil
}

// SubscribeToBlocks subscribes to new blocks
func (m *BlockchainManager) SubscribeToBlocks(handler func(*Block) error) error {
	m.ws.Subscribe("block", func(data []byte) error {
		var block Block
		if err := json.Unmarshal(data, &block); err != nil {
			return fmt.Errorf("failed to unmarshal block: %w", err)
		}
		return handler(&block)
	})
	return nil
}

// SubscribeToTransactions subscribes to new transactions
func (m *BlockchainManager) SubscribeToTransactions(handler func(*Transaction) error) error {
	m.ws.Subscribe("transaction", func(data []byte) error {
		var tx Transaction
		if err := json.Unmarshal(data, &tx); err != nil {
			return fmt.Errorf("failed to unmarshal transaction: %w", err)
		}
		return handler(&tx)
	})
	return nil
}

// Close closes the blockchain manager
func (m *BlockchainManager) Close() error {
	return m.ws.Close()
}

// Example usage:
/*
manager, err := NewBlockchainManager(
	"https://api.example.com",
	"wss://ws.example.com",
	"https://graphql.example.com",
	"your-api-key",
)
if err != nil {
	log.Fatal(err)
}
defer manager.Close()

// Get block
block, err := manager.GetBlock(context.Background(), "block-hash")
if err != nil {
	log.Fatal(err)
}

// Get transaction
tx, err := manager.GetTransaction(context.Background(), "tx-hash")
if err != nil {
	log.Fatal(err)
}

// Get balance
balance, err := manager.GetBalance(context.Background(), "address")
if err != nil {
	log.Fatal(err)
}

// Subscribe to blocks
err = manager.SubscribeToBlocks(func(block *Block) error {
	fmt.Printf("New block: %+v\n", block)
	return nil
})
if err != nil {
	log.Fatal(err)
}
*/
