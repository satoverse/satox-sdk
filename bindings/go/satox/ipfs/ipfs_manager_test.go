package ipfs

import (
	"testing"
	"time"
)

func TestNewIPFSManager(t *testing.T) {
	apiURL := "http://localhost:5001"
	manager := NewIPFSManager(apiURL)

	if manager == nil {
		t.Error("Expected non-nil IPFSManager")
	}

	if manager.shell == nil {
		t.Error("Expected non-nil IPFS shell")
	}

	if manager.timeout != 30*time.Second {
		t.Errorf("Expected timeout of 30s, got %v", manager.timeout)
	}
}

func TestInitialize(t *testing.T) {
	apiURL := "http://localhost:5001"
	manager := NewIPFSManager(apiURL)

	// Test initialization
	err := manager.Initialize()
	if err != nil {
		t.Logf("Initialization failed (expected if IPFS daemon is not running): %v", err)
		return
	}

	// Test double initialization
	err = manager.Initialize()
	if err == nil {
		t.Error("Expected error on double initialization")
	}
}

func TestAddAndGetData(t *testing.T) {
	apiURL := "http://localhost:5001"
	manager := NewIPFSManager(apiURL)

	// Initialize manager
	err := manager.Initialize()
	if err != nil {
		t.Logf("Initialization failed (expected if IPFS daemon is not running): %v", err)
		return
	}

	// Test data
	testData := []byte("Hello, IPFS!")

	// Add data
	hash, err := manager.AddData(testData)
	if err != nil {
		t.Errorf("Failed to add data: %v", err)
		return
	}

	// Get data
	retrievedData, err := manager.GetData(hash)
	if err != nil {
		t.Errorf("Failed to get data: %v", err)
		return
	}

	// Compare data
	if string(retrievedData) != string(testData) {
		t.Errorf("Retrieved data does not match original data")
	}
}

func TestPinOperations(t *testing.T) {
	apiURL := "http://localhost:5001"
	manager := NewIPFSManager(apiURL)

	// Initialize manager
	err := manager.Initialize()
	if err != nil {
		t.Logf("Initialization failed (expected if IPFS daemon is not running): %v", err)
		return
	}

	// Add test data
	testData := []byte("Test data for pinning")
	hash, err := manager.AddData(testData)
	if err != nil {
		t.Errorf("Failed to add test data: %v", err)
		return
	}

	// Test pinning
	err = manager.PinHash(hash)
	if err != nil {
		t.Errorf("Failed to pin hash: %v", err)
		return
	}

	// Test is pinned
	isPinned, err := manager.IsPinned(hash)
	if err != nil {
		t.Errorf("Failed to check if hash is pinned: %v", err)
		return
	}
	if !isPinned {
		t.Error("Hash should be pinned")
	}

	// Test unpinning
	err = manager.UnpinHash(hash)
	if err != nil {
		t.Errorf("Failed to unpin hash: %v", err)
		return
	}

	// Verify unpinned
	isPinned, err = manager.IsPinned(hash)
	if err != nil {
		t.Errorf("Failed to check if hash is pinned: %v", err)
		return
	}
	if isPinned {
		t.Error("Hash should not be pinned")
	}
}

func TestTimeoutOperations(t *testing.T) {
	apiURL := "http://localhost:5001"
	manager := NewIPFSManager(apiURL)

	// Test default timeout
	if manager.GetTimeout() != 30*time.Second {
		t.Error("Default timeout should be 30 seconds")
	}

	// Test setting timeout
	newTimeout := 60 * time.Second
	manager.SetTimeout(newTimeout)
	if manager.GetTimeout() != newTimeout {
		t.Error("Timeout should be updated to 60 seconds")
	}
}
