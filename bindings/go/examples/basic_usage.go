package main

import (
	"fmt"
	"log"
	"satox-sdk/satox"
)

func main() {
	fmt.Println("🚀 Satox SDK Go Example")
	fmt.Println("========================")

	// Initialize the SDK
	fmt.Println("\n1. Initializing SDK...")
	if err := satox.Initialize(); err != nil {
		log.Fatalf("Failed to initialize SDK: %v", err)
	}
	defer satox.Shutdown()

	fmt.Printf("✅ SDK Version: %s\n", satox.GetVersion())

	// Create blockchain manager
	fmt.Println("\n2. Creating Blockchain Manager...")
	config := `{
		"network": "mainnet",
		"rpc_endpoint": "http://localhost:7777",
		"rpc_username": "user",
		"rpc_password": "pass"
	}`
	
	bm, err := satox.NewBlockchainManager(config)
	if err != nil {
		log.Fatalf("Failed to create blockchain manager: %v", err)
	}
	defer bm.Close()
	fmt.Println("✅ Blockchain Manager created")

	// Get blockchain info
	fmt.Println("\n3. Getting Blockchain Info...")
	height, err := bm.GetBlockHeight()
	if err != nil {
		fmt.Printf("⚠️  Could not get block height: %v\n", err)
	} else {
		fmt.Printf("✅ Current block height: %d\n", height)
	}

	hash, err := bm.GetBestBlockHash()
	if err != nil {
		fmt.Printf("⚠️  Could not get best block hash: %v\n", err)
	} else {
		fmt.Printf("✅ Best block hash: %s\n", hash)
	}

	fmt.Println("\n🎉 Example completed successfully!")
} 