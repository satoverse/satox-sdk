package asset

import (
	"errors"
)

// Asset represents a digital asset in the Satox ecosystem
type Asset struct {
	ID          string
	Name        string
	Description string
	Owner       string
	Value       float64
}

// AssetManager handles asset-related operations
type AssetManager struct {
	assets map[string]*Asset
}

// NewAssetManager creates a new AssetManager instance
func NewAssetManager() *AssetManager {
	return &AssetManager{
		assets: make(map[string]*Asset),
	}
}

// CreateAsset creates a new asset
func (am *AssetManager) CreateAsset(name, description, owner string, value float64) (*Asset, error) {
	if name == "" {
		return nil, errors.New("asset name cannot be empty")
	}

	asset := &Asset{
		Name:        name,
		Description: description,
		Owner:       owner,
		Value:       value,
	}

	am.assets[name] = asset
	return asset, nil
}

// GetAsset retrieves an asset by name
func (am *AssetManager) GetAsset(name string) (*Asset, error) {
	asset, exists := am.assets[name]
	if !exists {
		return nil, errors.New("asset not found")
	}
	return asset, nil
}

// TransferAsset transfers an asset to a new owner
func (am *AssetManager) TransferAsset(name, newOwner string) error {
	asset, err := am.GetAsset(name)
	if err != nil {
		return err
	}

	asset.Owner = newOwner
	return nil
}

// DeleteAsset deletes an asset
func (am *AssetManager) DeleteAsset(name string) error {
	if _, err := am.GetAsset(name); err != nil {
		return err
	}

	delete(am.assets, name)
	return nil
}
