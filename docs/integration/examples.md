# Integration Examples

This document provides practical examples of integrating Satox SDK into various applications.

## Core SDK Examples

### Basic Wallet Management
```cpp
#include <satox/sdk.hpp>
#include <iostream>

int main() {
    // Initialize SDK
    SatoxSDK sdk;
    Config config;
    config.network = "mainnet";
    config.quantum_enabled = true;
    
    if (!sdk.initialize(config)) {
        std::cerr << "Failed to initialize SDK" << std::endl;
        return 1;
    }
    
    // Create wallet
    Wallet wallet = sdk.createWallet();
    std::cout << "Wallet created: " << wallet.getAddress() << std::endl;
    
    // Get balance
    double balance = wallet.getBalance();
    std::cout << "Balance: " << balance << " SATOX" << std::endl;
    
    // Get assets
    auto assets = wallet.getAssets();
    std::cout << "Assets owned: " << assets.size() << std::endl;
    
    return 0;
}
```

### Asset Creation and Transfer
```cpp
#include <satox/sdk.hpp>
#include <iostream>

int main() {
    SatoxSDK sdk;
    Config config;
    config.network = "mainnet";
    sdk.initialize(config);
    
    // Create wallet
    Wallet wallet = sdk.createWallet();
    
    // Create asset
    Asset asset = sdk.createAsset("My Token", "MTK", 1000000);
    std::cout << "Asset created: " << asset.getId() << std::endl;
    
    // Transfer asset
    std::string recipient = "SXrecipient_address_here";
    bool success = sdk.transferAsset(wallet, recipient, asset, 100);
    
    if (success) {
        std::cout << "Asset transferred successfully" << std::endl;
    } else {
        std::cout << "Transfer failed" << std::endl;
    }
    
    return 0;
}
```

### Quantum Security Integration
```cpp
#include <satox/sdk.hpp>
#include <iostream>

int main() {
    SatoxSDK sdk;
    Config config;
    config.network = "mainnet";
    config.quantum_enabled = true;
    sdk.initialize(config);
    
    // Generate quantum-resistant keys
    QuantumKeyPair quantumKeys = sdk.generateQuantumKeys();
    std::cout << "Quantum keys generated" << std::endl;
    
    // Sign data with quantum keys
    std::string data = "Hello, Quantum World!";
    bool signed = sdk.signWithQuantumKey(data, quantumKeys);
    
    if (signed) {
        std::cout << "Data signed with quantum keys" << std::endl;
    }
    
    return 0;
}
```

### Database Integration
```cpp
#include <satox/sdk.hpp>
#include <iostream>

int main() {
    SatoxSDK sdk;
    Config config;
    config.network = "mainnet";
    config.database_url = "postgresql://user:pass@localhost/satox";
    sdk.initialize(config);
    
    // Connect to database
    if (sdk.connectDatabase(config.database_url)) {
        std::cout << "Connected to database" << std::endl;
        
        // Create backup
        if (sdk.backupDatabase("backup.sql")) {
            std::cout << "Database backed up" << std::endl;
        }
    }
    
    return 0;
}
```

## Mobile SDK Examples

### iOS Wallet App
```swift
import SatoxSDK
import UIKit

class WalletViewController: UIViewController {
    @IBOutlet weak var addressLabel: UILabel!
    @IBOutlet weak var balanceLabel: UILabel!
    @IBOutlet weak var createAssetButton: UIButton!
    
    var sdk: SatoxSDK!
    var wallet: Wallet!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupSDK()
    }
    
    func setupSDK() {
        let config = Config()
        config.network = "mainnet"
        config.quantumEnabled = true
        
        sdk = SatoxSDK()
        sdk.initialize(config: config) { [weak self] success in
            if success {
                self?.createWallet()
            }
        }
    }
    
    func createWallet() {
        sdk.createWallet { [weak self] wallet in
            guard let wallet = wallet else { return }
            self?.wallet = wallet
            
            DispatchQueue.main.async {
                self?.addressLabel.text = wallet.address
                self?.updateBalance()
            }
        }
    }
    
    func updateBalance() {
        wallet.getBalance { [weak self] balance in
            DispatchQueue.main.async {
                self?.balanceLabel.text = "\(balance) SATOX"
            }
        }
    }
    
    @IBAction func createAssetTapped(_ sender: Any) {
        sdk.createAsset(name: "My Token", symbol: "MTK", supply: 1000000) { asset in
            if let asset = asset {
                print("Asset created: \(asset.id)")
            }
        }
    }
}
```

### Android Asset Manager
```kotlin
import com.satox.sdk.*
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    private lateinit var sdk: SatoxSDK
    private lateinit var wallet: Wallet
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        setupSDK()
        setupUI()
    }
    
    private fun setupSDK() {
        val config = Config().apply {
            network = "mainnet"
            quantumEnabled = true
        }
        
        sdk = SatoxSDK()
        sdk.initialize(config) { success ->
            if (success) {
                createWallet()
            }
        }
    }
    
    private fun setupUI() {
        createAssetButton.setOnClickListener {
            createAsset()
        }
        
        transferButton.setOnClickListener {
            transferAsset()
        }
    }
    
    private fun createWallet() {
        sdk.createWallet { wallet ->
            this.wallet = wallet
            runOnUiThread {
                addressTextView.text = wallet.address
                updateBalance()
            }
        }
    }
    
    private fun updateBalance() {
        wallet.getBalance { balance ->
            runOnUiThread {
                balanceTextView.text = "$balance SATOX"
            }
        }
    }
    
    private fun createAsset() {
        sdk.createAsset("My Token", "MTK", 1000000L) { asset ->
            asset?.let {
                Log.d("Asset", "Created: ${it.id}")
            }
        }
    }
    
    private fun transferAsset() {
        val recipient = recipientEditText.text.toString()
        val amount = amountEditText.text.toString().toLongOrNull() ?: 0L
        
        sdk.transferAsset(wallet, recipient, assetId, amount) { success ->
            if (success) {
                Log.d("Transfer", "Success")
            }
        }
    }
}
```

## Game SDK Examples

### Unity Asset Trading Game
```csharp
using Satox.GameSDK;
using UnityEngine;
using UnityEngine.UI;

public class GameManager : MonoBehaviour
{
    [Header("UI References")]
    public Text walletAddressText;
    public Text balanceText;
    public Button createAssetButton;
    public Button tradeButton;
    
    private SatoxGameSDK sdk;
    private PlayerWallet playerWallet;
    private GameAsset gameToken;
    
    void Start()
    {
        InitializeSDK();
        SetupUI();
    }
    
    async void InitializeSDK()
    {
        try
        {
            var config = new Config
            {
                Network = "mainnet",
                QuantumEnabled = true
            };

            sdk = new SatoxGameSDK();
            await sdk.Initialize(config);
            
            Debug.Log("SDK initialized");
            CreatePlayerWallet();
        }
        catch (System.Exception e)
        {
            Debug.LogError($"SDK initialization failed: {e.Message}");
        }
    }
    
    void SetupUI()
    {
        createAssetButton.onClick.AddListener(CreateGameAsset);
        tradeButton.onClick.AddListener(TradeAsset);
    }
    
    async void CreatePlayerWallet()
    {
        try
        {
            string playerId = SystemInfo.deviceUniqueIdentifier;
            playerWallet = await sdk.CreatePlayerWallet(playerId);
            
            walletAddressText.text = $"Wallet: {playerWallet.Address}";
            UpdateBalance();
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Wallet creation failed: {e.Message}");
        }
    }
    
    async void UpdateBalance()
    {
        try
        {
            var balance = await playerWallet.GetBalance();
            balanceText.text = $"Balance: {balance} SATOX";
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Balance update failed: {e.Message}");
        }
    }
    
    async void CreateGameAsset()
    {
        try
        {
            gameToken = await sdk.CreateGameAsset("Game Token", "GT", 1000000);
            Debug.Log($"Game asset created: {gameToken.Id}");
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Asset creation failed: {e.Message}");
        }
    }
    
    async void TradeAsset()
    {
        try
        {
            // Simulate trading with another player
            string otherPlayerAddress = "SXother_player_address";
            var transaction = await sdk.TransferAsset(
                playerWallet, 
                otherPlayerAddress, 
                gameToken.Id, 
                100
            );
            
            Debug.Log($"Trade completed: {transaction.Id}");
        }
        catch (System.Exception e)
        {
            Debug.LogError($"Trade failed: {e.Message}");
        }
    }
}
```

### Unreal Engine NFT Game
```cpp
#include "SatoxGameSDK.h"
#include "Engine/Engine.h"
#include "Components/TextRenderComponent.h"

ASatoxGameManager::ASatoxGameManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASatoxGameManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeSDK();
}

void ASatoxGameManager::InitializeSDK()
{
    FConfig Config;
    Config.Network = TEXT("mainnet");
    Config.QuantumEnabled = true;
    
    SDK = NewObject<USatoxGameSDK>();
    SDK->Initialize(Config, FSDKInitializeComplete::CreateUObject(this, &ASatoxGameManager::OnSDKInitialized));
}

void ASatoxGameManager::OnSDKInitialized(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SDK initialized successfully"));
        CreatePlayerWallet();
    }
}

void ASatoxGameManager::CreatePlayerWallet()
{
    FString PlayerId = FPlatformMisc::GetDeviceId();
    SDK->CreatePlayerWallet(PlayerId, FWalletCreated::CreateUObject(this, &ASatoxGameManager::OnWalletCreated));
}

void ASatoxGameManager::OnWalletCreated(UPlayerWallet* Wallet)
{
    if (Wallet)
    {
        PlayerWallet = Wallet;
        UE_LOG(LogTemp, Log, TEXT("Player wallet created: %s"), *Wallet->GetAddress());
        
        // Update UI
        if (WalletAddressText)
        {
            WalletAddressText->SetText(FText::FromString(Wallet->GetAddress()));
        }
        
        CreateNFTAsset();
    }
}

void ASatoxGameManager::CreateNFTAsset()
{
    // Create a unique NFT for the player
    FString AssetName = FString::Printf(TEXT("Player NFT %s"), *FDateTime::Now().ToString());
    SDK->CreateGameAsset(AssetName, TEXT("NFT"), 1, 
        FAssetCreated::CreateUObject(this, &ASatoxGameManager::OnNFTCreated));
}

void ASatoxGameManager::OnNFTCreated(UGameAsset* Asset)
{
    if (Asset)
    {
        UE_LOG(LogTemp, Log, TEXT("NFT created: %s"), *Asset->GetId());
        
        // Store NFT data
        NFTAsset = Asset;
        
        // Update UI
        if (NFTStatusText)
        {
            NFTStatusText->SetText(FText::FromString(TEXT("NFT Created!")));
        }
    }
}

void ASatoxGameManager::TransferNFT(FString RecipientAddress)
{
    if (NFTAsset && PlayerWallet)
    {
        SDK->TransferAsset(PlayerWallet, RecipientAddress, NFTAsset->GetId(), 1,
            FTransferComplete::CreateUObject(this, &ASatoxGameManager::OnTransferComplete));
    }
}

void ASatoxGameManager::OnTransferComplete(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("NFT transferred successfully"));
        
        if (NFTStatusText)
        {
            NFTStatusText->SetText(FText::FromString(TEXT("NFT Transferred!")));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("NFT transfer failed"));
    }
}
```

## Web Application Examples

### React.js Integration
```javascript
import React, { useState, useEffect } from 'react';
import SatoxSDK from '@satox/web-sdk';

const SatoxApp = () => {
  const [sdk, setSdk] = useState(null);
  const [wallet, setWallet] = useState(null);
  const [balance, setBalance] = useState(0);
  const [assets, setAssets] = useState([]);

  useEffect(() => {
    initializeSDK();
  }, []);

  const initializeSDK = async () => {
    try {
      const config = {
        network: 'mainnet',
        quantumEnabled: true,
      };
      
      const sdkInstance = new SatoxSDK();
      await sdkInstance.initialize(config);
      setSdk(sdkInstance);
      
      console.log('SDK initialized successfully');
    } catch (error) {
      console.error('Failed to initialize SDK:', error);
    }
  };

  const createWallet = async () => {
    try {
      const newWallet = await sdk.createWallet();
      setWallet(newWallet);
      console.log('Wallet created:', newWallet.address);
      updateBalance();
    } catch (error) {
      console.error('Failed to create wallet:', error);
    }
  };

  const updateBalance = async () => {
    if (wallet) {
      try {
        const walletBalance = await wallet.getBalance();
        setBalance(walletBalance);
      } catch (error) {
        console.error('Failed to get balance:', error);
      }
    }
  };

  const createAsset = async () => {
    try {
      const asset = await sdk.createAsset('My Token', 'MTK', 1000000);
      console.log('Asset created:', asset.id);
      updateAssets();
    } catch (error) {
      console.error('Failed to create asset:', error);
    }
  };

  const updateAssets = async () => {
    if (wallet) {
      try {
        const walletAssets = await wallet.getAssets();
        setAssets(walletAssets);
      } catch (error) {
        console.error('Failed to get assets:', error);
      }
    }
  };

  return (
    <div className="satox-app">
      <h1>Satox SDK Demo</h1>
      
      {!wallet ? (
        <button onClick={createWallet}>Create Wallet</button>
      ) : (
        <div>
          <h2>Wallet: {wallet.address}</h2>
          <p>Balance: {balance} SATOX</p>
          
          <button onClick={createAsset}>Create Asset</button>
          
          <h3>Assets:</h3>
          <ul>
            {assets.map(asset => (
              <li key={asset.id}>
                {asset.name} ({asset.symbol}): {asset.balance}
              </li>
            ))}
          </ul>
        </div>
      )}
    </div>
  );
};

export default SatoxApp;
```

### Node.js Backend Integration
```javascript
const express = require('express');
const SatoxSDK = require('@satox/node-sdk');

const app = express();
app.use(express.json());

// Initialize SDK
const sdk = new SatoxSDK();
const config = {
  network: 'mainnet',
  quantumEnabled: true,
  databaseUrl: 'postgresql://user:pass@localhost/satox'
};

sdk.initialize(config).then(() => {
  console.log('SDK initialized successfully');
}).catch(error => {
  console.error('Failed to initialize SDK:', error);
});

// API Routes
app.post('/api/wallet', async (req, res) => {
  try {
    const wallet = await sdk.createWallet();
    res.json({
      success: true,
      wallet: {
        address: wallet.address,
        balance: await wallet.getBalance()
      }
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error.message
    });
  }
});

app.post('/api/asset', async (req, res) => {
  try {
    const { name, symbol, supply } = req.body;
    const asset = await sdk.createAsset(name, symbol, supply);
    
    res.json({
      success: true,
      asset: {
        id: asset.id,
        name: asset.name,
        symbol: asset.symbol,
        supply: asset.supply
      }
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error.message
    });
  }
});

app.post('/api/transfer', async (req, res) => {
  try {
    const { fromAddress, toAddress, assetId, amount } = req.body;
    
    // Import wallet from address (in production, use proper authentication)
    const wallet = await sdk.importWallet(fromAddress);
    
    const success = await sdk.transferAsset(wallet, toAddress, assetId, amount);
    
    res.json({
      success: true,
      transferred: success
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error.message
    });
  }
});

app.get('/api/wallet/:address', async (req, res) => {
  try {
    const { address } = req.params;
    const wallet = await sdk.importWallet(address);
    
    const balance = await wallet.getBalance();
    const assets = await wallet.getAssets();
    
    res.json({
      success: true,
      wallet: {
        address: wallet.address,
        balance,
        assets
      }
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error.message
    });
  }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
```

## Resources
- [Integration Overview](overview.md)
- [API Reference](api-reference.md)
- [Platform-Specific Guides](platform-specific.md)
- [Troubleshooting](../troubleshooting/common-issues.md) 