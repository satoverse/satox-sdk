package satox

import (
	"testing"
)

func TestSDKInitialization(t *testing.T) {
	// Test SDK initialization
	err := Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize SDK: %v", err)
	}

	// Test SDK version
	version := GetVersion()
	if version == "" {
		t.Error("Expected non-empty version string")
	}

	// Test SDK shutdown
	Shutdown()
}

func TestAssetManagerBasic(t *testing.T) {
	// Initialize SDK
	err := Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize SDK: %v", err)
	}
	defer Shutdown()

	// Create asset manager
	assetManager, err := NewAssetManager()
	if err != nil {
		t.Fatalf("Failed to create asset manager: %v", err)
	}
	defer assetManager.Close()

	// Test that manager was created
	if assetManager == nil {
		t.Error("Expected non-nil asset manager")
	}
}

func TestSecurityManagerBasic(t *testing.T) {
	// Initialize SDK
	err := Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize SDK: %v", err)
	}
	defer Shutdown()

	// Create security manager
	securityManager, err := NewSecurityManager()
	if err != nil {
		t.Fatalf("Failed to create security manager: %v", err)
	}
	defer securityManager.Close()

	// Test that manager was created
	if securityManager == nil {
		t.Error("Expected non-nil security manager")
	}
}

func TestNFTManagerBasic(t *testing.T) {
	// Initialize SDK
	err := Initialize()
	if err != nil {
		t.Fatalf("Failed to initialize SDK: %v", err)
	}
	defer Shutdown()

	// Create NFT manager
	nftManager, err := NewNftManager()
	if err != nil {
		t.Fatalf("Failed to create NFT manager: %v", err)
	}
	defer nftManager.Close()

	// Test that manager was created
	if nftManager == nil {
		t.Error("Expected non-nil NFT manager")
	}
}
