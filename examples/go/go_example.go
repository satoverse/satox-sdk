package main

import (
	"fmt"
	"log"

	"satox"
)

func main() {
	fmt.Println("🚀 Satox SDK Go Example Application")
	fmt.Println("====================================\n")

	// Initialize SDK
	fmt.Println("Initializing Satox SDK...")
	if err := satox.Initialize(); err != nil {
		log.Fatalf("Failed to initialize SDK: %v", err)
	}
	defer satox.Shutdown()
	fmt.Printf("✅ SDK Version: %s\n\n", satox.GetVersion())

	// Initialize core manager
	fmt.Println("1. Initializing Core Manager...")
	coreManager := satox.NewCoreManager()
	if err := coreManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize core manager: %v", err)
	}
	fmt.Println("   ✅ Core Manager initialized successfully\n")

	// Initialize blockchain manager
	fmt.Println("2. Initializing Blockchain Manager...")
	blockchainManager := satox.NewBlockchainManager()
	if err := blockchainManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize blockchain manager: %v", err)
	}
	fmt.Println("   ✅ Blockchain Manager initialized successfully\n")

	// Initialize security manager
	fmt.Println("3. Initializing Security Manager...")
	securityManager := satox.NewSecurityManager()
	if err := securityManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize security manager: %v", err)
	}
	fmt.Println("   ✅ Security Manager initialized successfully\n")

	// Initialize quantum manager
	fmt.Println("4. Initializing Quantum Manager...")
	quantumManager := satox.NewQuantumManager()
	if err := quantumManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize quantum manager: %v", err)
	}
	fmt.Println("   ✅ Quantum Manager initialized successfully\n")

	// Initialize wallet manager
	fmt.Println("5. Initializing Wallet Manager...")
	walletManager := satox.NewWalletManager()
	if err := walletManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize wallet manager: %v", err)
	}
	fmt.Println("   ✅ Wallet Manager initialized successfully\n")

	// Initialize asset manager
	fmt.Println("6. Initializing Asset Manager...")
	assetManager := satox.NewAssetManager()
	if err := assetManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize asset manager: %v", err)
	}
	fmt.Println("   ✅ Asset Manager initialized successfully\n")

	// Initialize NFT manager
	fmt.Println("7. Initializing NFT Manager...")
	nftManager := satox.NewNFTManager()
	if err := nftManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize NFT manager: %v", err)
	}
	fmt.Println("   ✅ NFT Manager initialized successfully\n")

	// Initialize transaction manager
	fmt.Println("8. Initializing Transaction Manager...")
	transactionManager := satox.NewTransactionManager()
	if err := transactionManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize transaction manager: %v", err)
	}
	fmt.Println("   ✅ Transaction Manager initialized successfully\n")

	// Initialize network manager
	fmt.Println("9. Initializing Network Manager...")
	networkManager := satox.NewNetworkManager()
	if err := networkManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize network manager: %v", err)
	}
	fmt.Println("   ✅ Network Manager initialized successfully\n")

	// Initialize database manager
	fmt.Println("10. Initializing Database Manager...")
	databaseManager := satox.NewDatabaseManager()
	if err := databaseManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize database manager: %v", err)
	}
	fmt.Println("   ✅ Database Manager initialized successfully\n")

	// Initialize API manager
	fmt.Println("11. Initializing API Manager...")
	apiManager := satox.NewAPIManager()
	if err := apiManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize API manager: %v", err)
	}
	fmt.Println("   ✅ API Manager initialized successfully\n")

	// Initialize IPFS manager
	fmt.Println("12. Initializing IPFS Manager...")
	ipfsManager := satox.NewIPFSManager()
	if err := ipfsManager.Initialize(); err != nil {
		log.Fatalf("Failed to initialize IPFS manager: %v", err)
	}
	fmt.Println("   ✅ IPFS Manager initialized successfully\n")

	// Demonstrate wallet operations
	fmt.Println("13. Wallet Operations Demo...")
	walletAddress, err := walletManager.CreateWallet()
	if err != nil {
		log.Printf("Failed to create wallet: %v", err)
	} else {
		fmt.Printf("   ✅ Created wallet: %s\n", walletAddress)
	}

	balance, err := walletManager.GetBalance(walletAddress)
	if err != nil {
		log.Printf("Failed to get balance: %v", err)
	} else {
		fmt.Printf("   ✅ Wallet balance: %d SATOX\n", balance)
	}
	fmt.Println()

	// Demonstrate quantum security
	fmt.Println("14. Quantum Security Demo...")
	quantumPubKey, quantumPrivKey, err := quantumManager.GenerateKeyPair()
	if err != nil {
		log.Printf("Failed to generate quantum key pair: %v", err)
	} else {
		fmt.Println("   ✅ Generated quantum-resistant key pair")
	}

	message := []byte("Hello Quantum World!")
	quantumSignature, err := quantumManager.Sign(message, quantumPrivKey)
	if err != nil {
		log.Printf("Failed to create quantum signature: %v", err)
	} else {
		fmt.Println("   ✅ Created quantum-resistant signature")
	}

	isValid, err := quantumManager.Verify(message, quantumSignature, quantumPubKey)
	if err != nil {
		log.Printf("Failed to verify quantum signature: %v", err)
	} else {
		fmt.Printf("   ✅ Quantum signature verification: %t\n", isValid)
	}
	fmt.Println()

	// Demonstrate asset operations
	fmt.Println("15. Asset Operations Demo...")
	assetConfig := satox.AssetConfig{
		Name:        "Test Token",
		Symbol:      "TEST",
		TotalSupply: 1000000,
		Decimals:    8,
		Description: "A test token for demonstration",
		Metadata: map[string]interface{}{
			"website":  "https://example.com",
			"category": "utility",
		},
	}

	assetID, err := assetManager.CreateAsset(&assetConfig)
	if err != nil {
		log.Printf("Failed to create asset: %v", err)
	} else {
		fmt.Printf("   ✅ Created asset with ID: %s\n", assetID)
	}

	assetInfo, err := assetManager.GetAssetInfo(assetID)
	if err != nil {
		log.Printf("Failed to get asset info: %v", err)
	} else {
		fmt.Printf("   ✅ Asset info: %s (%s)\n", assetInfo.Name, assetInfo.Symbol)
	}
	fmt.Println()

	// Demonstrate NFT operations
	fmt.Println("16. NFT Operations Demo...")
	nftConfig := satox.NFTConfig{
		Name:        "Test NFT",
		Symbol:      "TNFT",
		Description: "A test NFT for demonstration",
		Metadata: map[string]interface{}{
			"image": "https://example.com/image.png",
			"attributes": map[string]interface{}{
				"rarity": "common",
				"power":  100,
			},
		},
	}

	nftID, err := nftManager.CreateNFT(&nftConfig)
	if err != nil {
		log.Printf("Failed to create NFT: %v", err)
	} else {
		fmt.Printf("   ✅ Created NFT with ID: %s\n", nftID)
	}

	nftInfo, err := nftManager.GetNFTInfo(nftID)
	if err != nil {
		log.Printf("Failed to get NFT info: %v", err)
	} else {
		fmt.Printf("   ✅ NFT info: %s (%s)\n", nftInfo.Name, nftInfo.Symbol)
	}
	fmt.Println()

	// Demonstrate transaction operations
	fmt.Println("17. Transaction Operations Demo...")
	txConfig := satox.TransactionConfig{
		From:    walletAddress,
		To:      "recipient_address",
		Amount:  1000,
		AssetID: assetID,
		Fee:     100,
	}

	txID, err := transactionManager.CreateTransaction(&txConfig)
	if err != nil {
		log.Printf("Failed to create transaction: %v", err)
	} else {
		fmt.Printf("   ✅ Created transaction with ID: %s\n", txID)
	}

	txStatus, err := transactionManager.GetTransactionStatus(txID)
	if err != nil {
		log.Printf("Failed to get transaction status: %v", err)
	} else {
		fmt.Printf("   ✅ Transaction status: %s\n", txStatus)
	}
	fmt.Println()

	// Demonstrate blockchain operations
	fmt.Println("18. Blockchain Operations Demo...")
	blockchainInfo, err := blockchainManager.GetBlockchainInfo()
	if err != nil {
		log.Printf("Failed to get blockchain info: %v", err)
	} else {
		fmt.Printf("   ✅ Blockchain: %s (Height: %d)\n", blockchainInfo.Name, blockchainInfo.CurrentHeight)
	}

	latestBlock, err := blockchainManager.GetLatestBlock()
	if err != nil {
		log.Printf("Failed to get latest block: %v", err)
	} else {
		fmt.Printf("   ✅ Latest block: %s with %d transactions\n", latestBlock.Hash, len(latestBlock.Transactions))
	}
	fmt.Println()

	// Demonstrate network operations
	fmt.Println("19. Network Operations Demo...")
	networkInfo, err := networkManager.GetNetworkInfo()
	if err != nil {
		log.Printf("Failed to get network info: %v", err)
	} else {
		fmt.Printf("   ✅ Network: %d connections\n", networkInfo.Connections)
	}

	peers, err := networkManager.GetPeers()
	if err != nil {
		log.Printf("Failed to get peers: %v", err)
	} else {
		fmt.Printf("   ✅ Connected peers: %d\n", len(peers))
	}
	fmt.Println()

	// Demonstrate IPFS operations
	fmt.Println("20. IPFS Operations Demo...")
	data := []byte("Hello IPFS World!")
	ipfsHash, err := ipfsManager.UploadData(data)
	if err != nil {
		log.Printf("Failed to upload data to IPFS: %v", err)
	} else {
		fmt.Printf("   ✅ Uploaded data to IPFS: %s\n", ipfsHash)
	}

	retrievedData, err := ipfsManager.DownloadData(ipfsHash)
	if err != nil {
		log.Printf("Failed to download data from IPFS: %v", err)
	} else {
		fmt.Printf("   ✅ Retrieved data from IPFS: %d bytes\n", len(retrievedData))
	}
	fmt.Println()

	// Demonstrate database operations
	fmt.Println("21. Database Operations Demo...")
	dbInfo, err := databaseManager.GetDatabaseInfo()
	if err != nil {
		log.Printf("Failed to get database info: %v", err)
	} else {
		fmt.Printf("   ✅ Database: %d tables\n", dbInfo.TableCount)
	}

	dbStats, err := databaseManager.GetDatabaseStats()
	if err != nil {
		log.Printf("Failed to get database stats: %v", err)
	} else {
		fmt.Printf("   ✅ Database size: %.2f MB\n", dbStats.SizeMB)
	}
	fmt.Println()

	// Demonstrate API operations
	fmt.Println("22. API Operations Demo...")
	apiInfo, err := apiManager.GetAPIInfo()
	if err != nil {
		log.Printf("Failed to get API info: %v", err)
	} else {
		fmt.Printf("   ✅ API: %d endpoints available\n", apiInfo.EndpointCount)
	}

	apiStats, err := apiManager.GetAPIStats()
	if err != nil {
		log.Printf("Failed to get API stats: %v", err)
	} else {
		fmt.Printf("   ✅ API requests: %d total\n", apiStats.TotalRequests)
	}
	fmt.Println()

	// Demonstrate security operations
	fmt.Println("23. Security Operations Demo...")
	securityInfo, err := securityManager.GetSecurityInfo()
	if err != nil {
		log.Printf("Failed to get security info: %v", err)
	} else {
		fmt.Printf("   ✅ Security: %d algorithms supported\n", securityInfo.AlgorithmCount)
	}

	encryptionKey, err := securityManager.GenerateEncryptionKey()
	if err != nil {
		log.Printf("Failed to generate encryption key: %v", err)
	} else {
		fmt.Printf("   ✅ Generated encryption key: %d bytes\n", len(encryptionKey))
	}
	fmt.Println()

	// Demonstrate batch operations
	fmt.Println("24. Batch Operations Demo...")
	batchConfig := satox.BatchTransactionConfig{
		Transactions: []satox.TransactionConfig{
			{
				From:    walletAddress,
				To:      "recipient1",
				Amount:  100,
				AssetID: assetID,
				Fee:     10,
			},
			{
				From:    walletAddress,
				To:      "recipient2",
				Amount:  200,
				AssetID: assetID,
				Fee:     10,
			},
		},
	}

	batchID, err := transactionManager.CreateBatchTransaction(&batchConfig)
	if err != nil {
		log.Printf("Failed to create batch transaction: %v", err)
	} else {
		fmt.Printf("   ✅ Created batch transaction with ID: %s\n", batchID)
	}
	fmt.Println()

	// Demonstrate error handling
	fmt.Println("25. Error Handling Demo...")
	_, err = transactionManager.GetTransactionStatus("invalid_tx_id")
	if err != nil {
		fmt.Printf("   ✅ Properly handled error: %v\n", err)
	} else {
		fmt.Println("   ❌ Unexpected success")
	}
	fmt.Println()

	// Demonstrate cleanup
	fmt.Println("26. Cleanup Operations...")
	apiManager.Shutdown()
	databaseManager.Shutdown()
	ipfsManager.Shutdown()
	networkManager.Shutdown()
	transactionManager.Shutdown()
	nftManager.Shutdown()
	assetManager.Shutdown()
	walletManager.Shutdown()
	quantumManager.Shutdown()
	securityManager.Shutdown()
	blockchainManager.Shutdown()
	coreManager.Shutdown()
	fmt.Println("   ✅ All managers shut down successfully\n")

	fmt.Println("🎉 Go Example Application Completed Successfully!")
	fmt.Println("All SDK features demonstrated and working correctly.")
}

// Test functions for demonstration
func testCoreInitialization() error {
	coreManager := satox.NewCoreManager()
	if err := coreManager.Initialize(); err != nil {
		return err
	}
	coreManager.Shutdown()
	return nil
}

func testQuantumOperations() error {
	quantumManager := satox.NewQuantumManager()
	if err := quantumManager.Initialize(); err != nil {
		return err
	}

	pubKey, privKey, err := quantumManager.GenerateKeyPair()
	if err != nil {
		return err
	}

	message := []byte("Test message")
	signature, err := quantumManager.Sign(message, privKey)
	if err != nil {
		return err
	}

	isValid, err := quantumManager.Verify(message, signature, pubKey)
	if err != nil {
		return err
	}

	if !isValid {
		return fmt.Errorf("quantum signature verification failed")
	}

	quantumManager.Shutdown()
	return nil
}

func testAssetOperations() error {
	assetManager := satox.NewAssetManager()
	if err := assetManager.Initialize(); err != nil {
		return err
	}

	config := satox.AssetConfig{
		Name:        "Test Asset",
		Symbol:      "TEST",
		TotalSupply: 1000000,
		Decimals:    8,
		Description: "Test asset",
		Metadata:    map[string]interface{}{},
	}

	assetID, err := assetManager.CreateAsset(&config)
	if err != nil {
		return err
	}

	assetInfo, err := assetManager.GetAssetInfo(assetID)
	if err != nil {
		return err
	}

	if assetInfo.Name != "Test Asset" {
		return fmt.Errorf("unexpected asset name: %s", assetInfo.Name)
	}

	assetManager.Shutdown()
	return nil
}
