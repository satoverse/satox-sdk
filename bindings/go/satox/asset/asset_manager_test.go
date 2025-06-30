package asset

import (
	"testing"
)

func TestNewManager(t *testing.T) {
	manager := NewManager()
	if manager == nil {
		t.Error("NewManager returned nil")
	}
	if manager.initialized {
		t.Error("New manager should not be initialized")
	}
	if manager.assets == nil {
		t.Error("assets map should be initialized")
	}
	if manager.balances == nil {
		t.Error("balances map should be initialized")
	}
}

func TestInitialize(t *testing.T) {
	manager := NewManager()
	err := manager.Initialize()
	if err != nil {
		t.Errorf("Initialize failed: %v", err)
	}
	if !manager.initialized {
		t.Error("manager should be initialized")
	}
}

func TestShutdown(t *testing.T) {
	manager := NewManager()
	manager.Initialize()
	err := manager.Shutdown()
	if err != nil {
		t.Errorf("Shutdown failed: %v", err)
	}
	if manager.initialized {
		t.Error("manager should not be initialized after shutdown")
	}
}

func TestCreateAsset(t *testing.T) {
	manager := NewManager()
	manager.Initialize()

	config := &AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		Description: "Test Description",
		Decimals:    8,
		TotalSupply: 1000000,
		Owner:       "owner1",
	}

	asset, err := manager.CreateAsset(config)
	if err != nil {
		t.Errorf("CreateAsset failed: %v", err)
	}

	if asset == nil {
		t.Error("CreateAsset returned nil asset")
	}

	if asset.ID != config.Symbol {
		t.Errorf("Expected asset ID %s, got %s", config.Symbol, asset.ID)
	}

	// Test duplicate asset creation
	_, err = manager.CreateAsset(config)
	if err == nil {
		t.Error("Expected error when creating duplicate asset")
	}
}

func TestGetAsset(t *testing.T) {
	manager := NewManager()
	manager.Initialize()

	config := &AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		Description: "Test Description",
		Decimals:    8,
		TotalSupply: 1000000,
		Owner:       "owner1",
	}

	manager.CreateAsset(config)

	asset, err := manager.GetAsset("TEST")
	if err != nil {
		t.Errorf("GetAsset failed: %v", err)
	}

	if asset == nil {
		t.Error("GetAsset returned nil asset")
	}

	if asset.Symbol != "TEST" {
		t.Errorf("Expected symbol TEST, got %s", asset.Symbol)
	}

	// Test non-existent asset
	_, err = manager.GetAsset("NONEXISTENT")
	if err == nil {
		t.Error("Expected error when getting non-existent asset")
	}
}

func TestUpdateAsset(t *testing.T) {
	manager := NewManager()
	manager.Initialize()

	config := &AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		Description: "Test Description",
		Decimals:    8,
		TotalSupply: 1000000,
		Owner:       "owner1",
	}

	manager.CreateAsset(config)

	updateConfig := &AssetUpdateConfig{
		Name:        "Updated Asset",
		Description: "Updated Description",
		TotalSupply: 2000000,
	}

	err := manager.UpdateAsset("TEST", updateConfig)
	if err != nil {
		t.Errorf("UpdateAsset failed: %v", err)
	}

	asset, _ := manager.GetAsset("TEST")
	if asset.Name != updateConfig.Name {
		t.Errorf("Expected name %s, got %s", updateConfig.Name, asset.Name)
	}

	if asset.Description != updateConfig.Description {
		t.Errorf("Expected description %s, got %s", updateConfig.Description, asset.Description)
	}

	if asset.TotalSupply != updateConfig.TotalSupply {
		t.Errorf("Expected total supply %d, got %d", updateConfig.TotalSupply, asset.TotalSupply)
	}
}

func TestDeleteAsset(t *testing.T) {
	manager := NewManager()
	manager.Initialize()

	config := &AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		Description: "Test Description",
		Decimals:    8,
		TotalSupply: 1000000,
		Owner:       "owner1",
	}

	manager.CreateAsset(config)

	err := manager.DeleteAsset("TEST")
	if err != nil {
		t.Errorf("DeleteAsset failed: %v", err)
	}

	_, err = manager.GetAsset("TEST")
	if err == nil {
		t.Error("Expected error when getting deleted asset")
	}
}

func TestGetAssetBalance(t *testing.T) {
	manager := NewManager()
	manager.Initialize()

	config := &AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		Description: "Test Description",
		Decimals:    8,
		TotalSupply: 1000000,
		Owner:       "owner1",
	}

	manager.CreateAsset(config)

	balance, err := manager.GetAssetBalance("TEST", "owner1")
	if err != nil {
		t.Errorf("GetAssetBalance failed: %v", err)
	}

	if balance != config.TotalSupply {
		t.Errorf("Expected balance %d, got %d", config.TotalSupply, balance)
	}

	// Test non-existent address
	balance, err = manager.GetAssetBalance("TEST", "nonexistent")
	if err != nil {
		t.Errorf("GetAssetBalance failed for non-existent address: %v", err)
	}
	if balance != 0 {
		t.Errorf("Expected balance 0 for non-existent address, got %d", balance)
	}
}

func TestTransferAsset(t *testing.T) {
	manager := NewManager()
	manager.Initialize()

	config := &AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		Description: "Test Description",
		Decimals:    8,
		TotalSupply: 1000000,
		Owner:       "owner1",
	}

	manager.CreateAsset(config)

	transferConfig := &TransferConfig{
		AssetID:     "TEST",
		FromAddress: "owner1",
		ToAddress:   "owner2",
		Amount:      500000,
	}

	err := manager.TransferAsset(transferConfig)
	if err != nil {
		t.Errorf("TransferAsset failed: %v", err)
	}

	fromBalance, _ := manager.GetAssetBalance("TEST", "owner1")
	if fromBalance != 500000 {
		t.Errorf("Expected from balance %d, got %d", 500000, fromBalance)
	}

	toBalance, _ := manager.GetAssetBalance("TEST", "owner2")
	if toBalance != 500000 {
		t.Errorf("Expected to balance %d, got %d", 500000, toBalance)
	}

	// Test insufficient balance
	transferConfig.Amount = 1000000
	err = manager.TransferAsset(transferConfig)
	if err == nil {
		t.Error("Expected error when transferring more than balance")
	}
}
