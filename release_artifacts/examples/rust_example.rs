use satox_sdk::{
    blockchain::BlockchainManager,
    asset::AssetManager,
    nft::NFTManager,
    security::SecurityManager,
    quantum::QuantumManager,
    wallet::WalletManager,
    transaction::TransactionManager,
    network::NetworkManager,
    database::DatabaseManager,
    api::APIManager,
    core::CoreManager,
    ipfs::IPFSManager,
    error::SatoxError,
};
use std::sync::Arc;
use tokio;

#[tokio::main]
async fn main() -> Result<(), SatoxError> {
    println!("🚀 Satox SDK Rust Example Application");
    println!("=====================================\n");

    // Initialize core manager
    println!("1. Initializing Core Manager...");
    let core_manager = Arc::new(CoreManager::new());
    core_manager.initialize().await?;
    println!("   ✅ Core Manager initialized successfully\n");

    // Initialize blockchain manager
    println!("2. Initializing Blockchain Manager...");
    let blockchain_manager = Arc::new(BlockchainManager::new());
    blockchain_manager.initialize().await?;
    println!("   ✅ Blockchain Manager initialized successfully\n");

    // Initialize security manager with quantum features
    println!("3. Initializing Security Manager with Quantum Features...");
    let security_manager = Arc::new(SecurityManager::new());
    security_manager.initialize().await?;
    
    // Initialize quantum manager
    let quantum_manager = Arc::new(QuantumManager::new());
    quantum_manager.initialize().await?;
    println!("   ✅ Security and Quantum Managers initialized successfully\n");

    // Initialize wallet manager
    println!("4. Initializing Wallet Manager...");
    let wallet_manager = Arc::new(WalletManager::new());
    wallet_manager.initialize().await?;
    println!("   ✅ Wallet Manager initialized successfully\n");

    // Initialize asset manager
    println!("5. Initializing Asset Manager...");
    let asset_manager = Arc::new(AssetManager::new());
    asset_manager.initialize().await?;
    println!("   ✅ Asset Manager initialized successfully\n");

    // Initialize NFT manager
    println!("6. Initializing NFT Manager...");
    let nft_manager = Arc::new(NFTManager::new());
    nft_manager.initialize().await?;
    println!("   ✅ NFT Manager initialized successfully\n");

    // Initialize transaction manager
    println!("7. Initializing Transaction Manager...");
    let transaction_manager = Arc::new(TransactionManager::new());
    transaction_manager.initialize().await?;
    println!("   ✅ Transaction Manager initialized successfully\n");

    // Initialize network manager
    println!("8. Initializing Network Manager...");
    let network_manager = Arc::new(NetworkManager::new());
    network_manager.initialize().await?;
    println!("   ✅ Network Manager initialized successfully\n");

    // Initialize database manager
    println!("9. Initializing Database Manager...");
    let database_manager = Arc::new(DatabaseManager::new());
    database_manager.initialize().await?;
    println!("   ✅ Database Manager initialized successfully\n");

    // Initialize API manager
    println!("10. Initializing API Manager...");
    let api_manager = Arc::new(APIManager::new());
    api_manager.initialize().await?;
    println!("   ✅ API Manager initialized successfully\n");

    // Initialize IPFS manager
    println!("11. Initializing IPFS Manager...");
    let ipfs_manager = Arc::new(IPFSManager::new());
    ipfs_manager.initialize().await?;
    println!("   ✅ IPFS Manager initialized successfully\n");

    // Demonstrate wallet operations
    println!("12. Wallet Operations Demo...");
    let wallet_address = wallet_manager.create_wallet().await?;
    println!("   ✅ Created wallet: {}", wallet_address);
    
    let balance = wallet_manager.get_balance(&wallet_address).await?;
    println!("   ✅ Wallet balance: {} SATOX", balance);
    println!();

    // Demonstrate quantum security
    println!("13. Quantum Security Demo...");
    let (quantum_pub_key, quantum_priv_key) = quantum_manager.generate_keypair().await?;
    println!("   ✅ Generated quantum-resistant key pair");
    
    let message = b"Hello Quantum World!";
    let quantum_signature = quantum_manager.sign(message, &quantum_priv_key).await?;
    println!("   ✅ Created quantum-resistant signature");
    
    let is_valid = quantum_manager.verify(message, &quantum_signature, &quantum_pub_key).await?;
    println!("   ✅ Quantum signature verification: {}", is_valid);
    println!();

    // Demonstrate asset operations
    println!("14. Asset Operations Demo...");
    let asset_config = satox_sdk::asset::AssetConfig {
        name: "Test Token".to_string(),
        symbol: "TEST".to_string(),
        total_supply: 1000000,
        decimals: 8,
        description: "A test token for demonstration".to_string(),
        metadata: serde_json::json!({
            "website": "https://example.com",
            "category": "utility"
        }),
    };
    
    let asset_id = asset_manager.create_asset(&asset_config).await?;
    println!("   ✅ Created asset with ID: {}", asset_id);
    
    let asset_info = asset_manager.get_asset_info(&asset_id).await?;
    println!("   ✅ Asset info: {} ({})", asset_info.name, asset_info.symbol);
    println!();

    // Demonstrate NFT operations
    println!("15. NFT Operations Demo...");
    let nft_config = satox_sdk::nft::NFTConfig {
        name: "Test NFT".to_string(),
        symbol: "TNFT".to_string(),
        description: "A test NFT for demonstration".to_string(),
        metadata: serde_json::json!({
            "image": "https://example.com/image.png",
            "attributes": {
                "rarity": "common",
                "power": 100
            }
        }),
    };
    
    let nft_id = nft_manager.create_nft(&nft_config).await?;
    println!("   ✅ Created NFT with ID: {}", nft_id);
    
    let nft_info = nft_manager.get_nft_info(&nft_id).await?;
    println!("   ✅ NFT info: {} ({})", nft_info.name, nft_info.symbol);
    println!();

    // Demonstrate transaction operations
    println!("16. Transaction Operations Demo...");
    let tx_config = satox_sdk::transaction::TransactionConfig {
        from: wallet_address.clone(),
        to: "recipient_address".to_string(),
        amount: 1000,
        asset_id: asset_id.clone(),
        fee: 100,
    };
    
    let tx_id = transaction_manager.create_transaction(&tx_config).await?;
    println!("   ✅ Created transaction with ID: {}", tx_id);
    
    let tx_status = transaction_manager.get_transaction_status(&tx_id).await?;
    println!("   ✅ Transaction status: {:?}", tx_status);
    println!();

    // Demonstrate blockchain operations
    println!("17. Blockchain Operations Demo...");
    let blockchain_info = blockchain_manager.get_blockchain_info().await?;
    println!("   ✅ Blockchain: {} (Height: {})", blockchain_info.name, blockchain_info.current_height);
    
    let latest_block = blockchain_manager.get_latest_block().await?;
    println!("   ✅ Latest block: {} with {} transactions", latest_block.hash, latest_block.transactions.len());
    println!();

    // Demonstrate network operations
    println!("18. Network Operations Demo...");
    let network_info = network_manager.get_network_info().await?;
    println!("   ✅ Network: {} connections", network_info.connections);
    
    let peers = network_manager.get_peers().await?;
    println!("   ✅ Connected peers: {}", peers.len());
    println!();

    // Demonstrate IPFS operations
    println!("19. IPFS Operations Demo...");
    let data = b"Hello IPFS World!";
    let ipfs_hash = ipfs_manager.upload_data(data).await?;
    println!("   ✅ Uploaded data to IPFS: {}", ipfs_hash);
    
    let retrieved_data = ipfs_manager.download_data(&ipfs_hash).await?;
    println!("   ✅ Retrieved data from IPFS: {} bytes", retrieved_data.len());
    println!();

    // Demonstrate database operations
    println!("20. Database Operations Demo...");
    let db_info = database_manager.get_database_info().await?;
    println!("   ✅ Database: {} tables", db_info.table_count);
    
    let db_stats = database_manager.get_database_stats().await?;
    println!("   ✅ Database size: {} MB", db_stats.size_mb);
    println!();

    // Demonstrate API operations
    println!("21. API Operations Demo...");
    let api_info = api_manager.get_api_info().await?;
    println!("   ✅ API: {} endpoints available", api_info.endpoint_count);
    
    let api_stats = api_manager.get_api_stats().await?;
    println!("   ✅ API requests: {} total", api_stats.total_requests);
    println!();

    // Demonstrate security operations
    println!("22. Security Operations Demo...");
    let security_info = security_manager.get_security_info().await?;
    println!("   ✅ Security: {} algorithms supported", security_info.algorithm_count);
    
    let encryption_key = security_manager.generate_encryption_key().await?;
    println!("   ✅ Generated encryption key: {} bytes", encryption_key.len());
    println!();

    // Demonstrate batch operations
    println!("23. Batch Operations Demo...");
    let batch_config = satox_sdk::transaction::BatchTransactionConfig {
        transactions: vec![
            satox_sdk::transaction::TransactionConfig {
                from: wallet_address.clone(),
                to: "recipient1".to_string(),
                amount: 100,
                asset_id: asset_id.clone(),
                fee: 10,
            },
            satox_sdk::transaction::TransactionConfig {
                from: wallet_address.clone(),
                to: "recipient2".to_string(),
                amount: 200,
                asset_id: asset_id.clone(),
                fee: 10,
            },
        ],
    };
    
    let batch_id = transaction_manager.create_batch_transaction(&batch_config).await?;
    println!("   ✅ Created batch transaction with ID: {}", batch_id);
    println!();

    // Demonstrate error handling
    println!("24. Error Handling Demo...");
    match transaction_manager.get_transaction_status("invalid_tx_id").await {
        Ok(_) => println!("   ❌ Unexpected success"),
        Err(e) => println!("   ✅ Properly handled error: {}", e),
    }
    println!();

    // Demonstrate cleanup
    println!("25. Cleanup Operations...");
    api_manager.shutdown().await?;
    database_manager.shutdown().await?;
    ipfs_manager.shutdown().await?;
    network_manager.shutdown().await?;
    transaction_manager.shutdown().await?;
    nft_manager.shutdown().await?;
    asset_manager.shutdown().await?;
    wallet_manager.shutdown().await?;
    quantum_manager.shutdown().await?;
    security_manager.shutdown().await?;
    blockchain_manager.shutdown().await?;
    core_manager.shutdown().await?;
    println!("   ✅ All managers shut down successfully\n");

    println!("🎉 Rust Example Application Completed Successfully!");
    println!("All SDK features demonstrated and working correctly.");
    
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_core_initialization() {
        let core_manager = CoreManager::new();
        assert!(core_manager.initialize().await.is_ok());
        core_manager.shutdown().await;
    }

    #[tokio::test]
    async fn test_quantum_operations() {
        let quantum_manager = QuantumManager::new();
        quantum_manager.initialize().await.unwrap();
        
        let (pub_key, priv_key) = quantum_manager.generate_keypair().await.unwrap();
        let message = b"Test message";
        let signature = quantum_manager.sign(message, &priv_key).await.unwrap();
        let is_valid = quantum_manager.verify(message, &signature, &pub_key).await.unwrap();
        
        assert!(is_valid);
        quantum_manager.shutdown().await;
    }

    #[tokio::test]
    async fn test_asset_operations() {
        let asset_manager = AssetManager::new();
        asset_manager.initialize().await.unwrap();
        
        let config = satox_sdk::asset::AssetConfig {
            name: "Test Asset".to_string(),
            symbol: "TEST".to_string(),
            total_supply: 1000000,
            decimals: 8,
            description: "Test asset".to_string(),
            metadata: serde_json::json!({}),
        };
        
        let asset_id = asset_manager.create_asset(&config).await.unwrap();
        let asset_info = asset_manager.get_asset_info(&asset_id).await.unwrap();
        
        assert_eq!(asset_info.name, "Test Asset");
        asset_manager.shutdown().await;
    }
} 