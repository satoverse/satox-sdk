package asset

import (
	"errors"
	"sync"
	"time"
)

// Manager handles asset operations
type Manager struct {
	initialized bool
	assets      map[string]*Asset
	balances    map[string]map[string]int64 // assetID -> address -> balance
	mu          sync.RWMutex
}

// NewManager creates a new Manager instance
func NewManager() *Manager {
	return &Manager{
		initialized: false,
		assets:      make(map[string]*Asset),
		balances:    make(map[string]map[string]int64),
	}
}

// Initialize initializes the asset manager
func (am *Manager) Initialize() error {
	am.mu.Lock()
	defer am.mu.Unlock()

	if am.initialized {
		return nil
	}

	am.initialized = true
	return nil
}

// Shutdown shuts down the asset manager
func (am *Manager) Shutdown() error {
	am.mu.Lock()
	defer am.mu.Unlock()

	if !am.initialized {
		return nil
	}

	am.initialized = false
	return nil
}

// CreateAsset creates a new asset
func (am *Manager) CreateAsset(config *AssetConfig) (*Asset, error) {
	am.mu.Lock()
	defer am.mu.Unlock()

	am.ensureInitialized()

	// Check if asset already exists
	if _, exists := am.assets[config.Symbol]; exists {
		return nil, errors.New("asset already exists")
	}

	now := time.Now().Unix()
	asset := &Asset{
		ID:          config.Symbol,
		Name:        config.Name,
		Symbol:      config.Symbol,
		Description: config.Description,
		Decimals:    config.Decimals,
		TotalSupply: config.TotalSupply,
		Owner:       config.Owner,
		CreatedAt:   now,
		UpdatedAt:   now,
	}

	am.assets[config.Symbol] = asset
	am.balances[config.Symbol] = make(map[string]int64)
	am.balances[config.Symbol][config.Owner] = config.TotalSupply

	return asset, nil
}

// GetAsset gets an asset by its ID
func (am *Manager) GetAsset(id string) (*Asset, error) {
	am.mu.RLock()
	defer am.mu.RUnlock()

	am.ensureInitialized()

	asset, exists := am.assets[id]
	if !exists {
		return nil, errors.New("asset not found")
	}

	return asset, nil
}

// UpdateAsset updates an asset
func (am *Manager) UpdateAsset(id string, config *AssetUpdateConfig) error {
	am.mu.Lock()
	defer am.mu.Unlock()

	am.ensureInitialized()

	asset, exists := am.assets[id]
	if !exists {
		return errors.New("asset not found")
	}

	asset.Name = config.Name
	asset.Description = config.Description
	asset.TotalSupply = config.TotalSupply
	asset.UpdatedAt = time.Now().Unix()

	return nil
}

// DeleteAsset deletes an asset
func (am *Manager) DeleteAsset(id string) error {
	am.mu.Lock()
	defer am.mu.Unlock()

	am.ensureInitialized()

	if _, exists := am.assets[id]; !exists {
		return errors.New("asset not found")
	}

	delete(am.assets, id)
	delete(am.balances, id)

	return nil
}

// GetAssetBalance gets the balance of an asset for an address
func (am *Manager) GetAssetBalance(assetID string, address string) (int64, error) {
	am.mu.RLock()
	defer am.mu.RUnlock()

	am.ensureInitialized()

	if _, exists := am.assets[assetID]; !exists {
		return 0, errors.New("asset not found")
	}

	balances, exists := am.balances[assetID]
	if !exists {
		return 0, errors.New("asset has no balances")
	}

	balance, exists := balances[address]
	if !exists {
		return 0, nil
	}

	return balance, nil
}

// TransferAsset transfers an asset
func (am *Manager) TransferAsset(config *TransferConfig) error {
	am.mu.Lock()
	defer am.mu.Unlock()

	am.ensureInitialized()

	if _, exists := am.assets[config.AssetID]; !exists {
		return errors.New("asset not found")
	}

	balances, exists := am.balances[config.AssetID]
	if !exists {
		return errors.New("asset has no balances")
	}

	fromBalance, exists := balances[config.FromAddress]
	if !exists || fromBalance < config.Amount {
		return errors.New("insufficient balance")
	}

	balances[config.FromAddress] -= config.Amount
	balances[config.ToAddress] += config.Amount

	return nil
}

func (am *Manager) ensureInitialized() {
	if !am.initialized {
		panic("asset manager not initialized")
	}
}
