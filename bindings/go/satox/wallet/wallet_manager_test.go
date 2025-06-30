package wallet

import (
	"testing"
)

func TestNewWalletManager(t *testing.T) {
	manager := NewWalletManager()
	if manager == nil {
		t.Fatal("NewWalletManager returned nil")
	}
}

func TestCreateWallet(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}
	if wallet == nil {
		t.Fatal("CreateWallet returned nil wallet")
	}
	if wallet.Address == "" {
		t.Error("Wallet address is empty")
	}
	if len(wallet.PrivateKey) != 32 {
		t.Errorf("Expected private key length 32, got %d", len(wallet.PrivateKey))
	}
	if len(wallet.PublicKey) != 32 {
		t.Errorf("Expected public key length 32, got %d", len(wallet.PublicKey))
	}
	if wallet.CreatedAt.IsZero() {
		t.Error("CreatedAt is zero")
	}
	if wallet.UpdatedAt.IsZero() {
		t.Error("UpdatedAt is zero")
	}
	if wallet.Balance != 0 {
		t.Errorf("Expected balance 0, got %f", wallet.Balance)
	}
	if wallet.Nonce != 0 {
		t.Errorf("Expected nonce 0, got %d", wallet.Nonce)
	}
}

func TestGetWallet(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	retrieved, err := manager.GetWallet(wallet.Address)
	if err != nil {
		t.Fatalf("GetWallet failed: %v", err)
	}
	if retrieved == nil {
		t.Fatal("GetWallet returned nil wallet")
	}
	if retrieved.Address != wallet.Address {
		t.Errorf("Expected address %s, got %s", wallet.Address, retrieved.Address)
	}
}

func TestGetWalletNotFound(t *testing.T) {
	manager := NewWalletManager()

	_, err := manager.GetWallet("nonexistent")
	if err == nil {
		t.Error("Expected error for nonexistent wallet")
	}
}

func TestListWallets(t *testing.T) {
	manager := NewWalletManager()

	wallet1, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	wallet2, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	wallets := manager.ListWallets()
	if len(wallets) != 2 {
		t.Errorf("Expected 2 wallets, got %d", len(wallets))
	}

	found1 := false
	found2 := false
	for _, w := range wallets {
		if w.Address == wallet1.Address {
			found1 = true
		}
		if w.Address == wallet2.Address {
			found2 = true
		}
	}

	if !found1 {
		t.Error("Wallet 1 not found in list")
	}
	if !found2 {
		t.Error("Wallet 2 not found in list")
	}
}

func TestUpdateBalance(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	err = manager.UpdateBalance(wallet.Address, 100.0)
	if err != nil {
		t.Fatalf("UpdateBalance failed: %v", err)
	}

	retrieved, err := manager.GetWallet(wallet.Address)
	if err != nil {
		t.Fatalf("GetWallet failed: %v", err)
	}
	if retrieved.Balance != 100.0 {
		t.Errorf("Expected balance 100.0, got %f", retrieved.Balance)
	}
}

func TestUpdateNonce(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	err = manager.UpdateNonce(wallet.Address, 42)
	if err != nil {
		t.Fatalf("UpdateNonce failed: %v", err)
	}

	retrieved, err := manager.GetWallet(wallet.Address)
	if err != nil {
		t.Fatalf("GetWallet failed: %v", err)
	}
	if retrieved.Nonce != 42 {
		t.Errorf("Expected nonce 42, got %d", retrieved.Nonce)
	}
}

func TestDeleteWallet(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	err = manager.DeleteWallet(wallet.Address)
	if err != nil {
		t.Fatalf("DeleteWallet failed: %v", err)
	}

	_, err = manager.GetWallet(wallet.Address)
	if err == nil {
		t.Error("Expected error for deleted wallet")
	}
}

func TestSignMessage(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	message := []byte("Hello, World!")
	signature, err := manager.SignMessage(wallet.Address, message)
	if err != nil {
		t.Fatalf("SignMessage failed: %v", err)
	}
	if len(signature) != 64 {
		t.Errorf("Expected signature length 64, got %d", len(signature))
	}
}

func TestVerifySignature(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	message := []byte("Hello, World!")
	signature := make([]byte, 64)

	valid, err := manager.VerifySignature(wallet.Address, message, signature)
	if err != nil {
		t.Fatalf("VerifySignature failed: %v", err)
	}
	if !valid {
		t.Error("Expected valid signature")
	}
}

func TestExportWallet(t *testing.T) {
	manager := NewWalletManager()

	wallet, err := manager.CreateWallet()
	if err != nil {
		t.Fatalf("CreateWallet failed: %v", err)
	}

	privateKey, err := manager.ExportWallet(wallet.Address)
	if err != nil {
		t.Fatalf("ExportWallet failed: %v", err)
	}
	if len(privateKey) != 32 {
		t.Errorf("Expected private key length 32, got %d", len(privateKey))
	}
}

func TestImportWallet(t *testing.T) {
	manager := NewWalletManager()

	privateKey := make([]byte, 32)
	wallet, err := manager.ImportWallet(privateKey)
	if err != nil {
		t.Fatalf("ImportWallet failed: %v", err)
	}
	if wallet == nil {
		t.Fatal("ImportWallet returned nil wallet")
	}
	if wallet.Address == "" {
		t.Error("Wallet address is empty")
	}
	if len(wallet.PrivateKey) != 32 {
		t.Errorf("Expected private key length 32, got %d", len(wallet.PrivateKey))
	}
	if len(wallet.PublicKey) != 32 {
		t.Errorf("Expected public key length 32, got %d", len(wallet.PublicKey))
	}
	if wallet.CreatedAt.IsZero() {
		t.Error("CreatedAt is zero")
	}
	if wallet.UpdatedAt.IsZero() {
		t.Error("UpdatedAt is zero")
	}
	if wallet.Balance != 0 {
		t.Errorf("Expected balance 0, got %f", wallet.Balance)
	}
	if wallet.Nonce != 0 {
		t.Errorf("Expected nonce 0, got %d", wallet.Nonce)
	}
}

func TestImportWalletInvalidKey(t *testing.T) {
	manager := NewWalletManager()

	invalidKey := make([]byte, 16)
	_, err := manager.ImportWallet(invalidKey)
	if err == nil {
		t.Error("Expected error for invalid private key length")
	}
}
