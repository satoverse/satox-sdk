# API Reference

This document provides a comprehensive reference for the Satox SDK APIs.

## Core SDK API (C++)

### Initialization

#### `SatoxSDK::initialize(const Config& config)`
Initialize the SDK with configuration.

**Parameters:**
- `config` (Config): SDK configuration object

**Returns:** `bool` - Success status

**Example:**
```cpp
Config config;
config.network = "mainnet";
config.quantum_enabled = true;
config.database_url = "postgresql://user:pass@localhost/satox";

SatoxSDK sdk;
bool success = sdk.initialize(config);
```

### Wallet Management

#### `Wallet SatoxSDK::createWallet()`
Create a new wallet with quantum-resistant keys.

**Returns:** `Wallet` - New wallet instance

**Example:**
```cpp
Wallet wallet = sdk.createWallet();
std::string address = wallet.getAddress();
```

#### `Wallet SatoxSDK::importWallet(const std::string& privateKey)`
Import an existing wallet from private key.

**Parameters:**
- `privateKey` (string): Private key in WIF format

**Returns:** `Wallet` - Imported wallet instance

**Example:**
```cpp
Wallet wallet = sdk.importWallet("5KJvsngHeMpm884wtkJNzQGaCErckhHJBGFsvd3VyK5qMZXj3hS");
```

#### `std::string Wallet::getAddress()`
Get the wallet's public address.

**Returns:** `string` - Public address

#### `double Wallet::getBalance()`
Get the wallet's SATOX balance.

**Returns:** `double` - Balance in SATOX

#### `std::vector<Asset> Wallet::getAssets()`
Get all assets owned by the wallet.

**Returns:** `vector<Asset>` - List of owned assets

### Asset Management

#### `Asset SatoxSDK::createAsset(const std::string& name, const std::string& symbol, uint64_t supply)`
Create a new digital asset.

**Parameters:**
- `name` (string): Asset name
- `symbol` (string): Asset symbol (3-5 characters)
- `supply` (uint64_t): Total supply

**Returns:** `Asset` - Created asset instance

**Example:**
```cpp
Asset asset = sdk.createAsset("My Token", "MTK", 1000000);
std::string assetId = asset.getId();
```

#### `Asset SatoxSDK::getAsset(const std::string& assetId)`
Get asset information by ID.

**Parameters:**
- `assetId` (string): Asset identifier

**Returns:** `Asset` - Asset instance

#### `bool SatoxSDK::transferAsset(const Wallet& from, const std::string& to, const Asset& asset, uint64_t amount)`
Transfer assets between wallets.

**Parameters:**
- `from` (Wallet): Source wallet
- `to` (string): Destination address
- `asset` (Asset): Asset to transfer
- `amount` (uint64_t): Amount to transfer

**Returns:** `bool` - Success status

**Example:**
```cpp
bool success = sdk.transferAsset(wallet, "SXaddress", asset, 100);
```

### Transaction Management

#### `Transaction SatoxSDK::sendSATOX(const Wallet& from, const std::string& to, double amount)`
Send SATOX to another address.

**Parameters:**
- `from` (Wallet): Source wallet
- `to` (string): Destination address
- `amount` (double): Amount in SATOX

**Returns:** `Transaction` - Transaction instance

#### `Transaction SatoxSDK::getTransaction(const std::string& txId)`
Get transaction information by ID.

**Parameters:**
- `txId` (string): Transaction ID

**Returns:** `Transaction` - Transaction instance

#### `std::vector<Transaction> Wallet::getTransactionHistory()`
Get wallet's transaction history.

**Returns:** `vector<Transaction>` - Transaction history

### Security

#### `QuantumKeyPair SatoxSDK::generateQuantumKeys()`
Generate quantum-resistant key pair.

**Returns:** `QuantumKeyPair` - Quantum key pair

#### `bool SatoxSDK::signWithQuantumKey(const std::string& data, const QuantumKeyPair& keys)`
Sign data with quantum-resistant keys.

**Parameters:**
- `data` (string): Data to sign
- `keys` (QuantumKeyPair): Quantum key pair

**Returns:** `bool` - Success status

### Database Operations

#### `bool SatoxSDK::connectDatabase(const std::string& url)`
Connect to database.

**Parameters:**
- `url` (string): Database connection URL

**Returns:** `bool` - Success status

#### `bool SatoxSDK::backupDatabase(const std::string& path)`
Create database backup.

**Parameters:**
- `path` (string): Backup file path

**Returns:** `bool` - Success status

## Mobile SDK API

### iOS (Swift)

#### `SatoxSDK.initialize(config: Config)`
Initialize the mobile SDK.

**Parameters:**
- `config` (Config): SDK configuration

**Returns:** `Bool` - Success status

#### `Wallet SatoxSDK.createWallet()`
Create a new wallet.

**Returns:** `Wallet` - New wallet instance

#### `Asset SatoxSDK.createAsset(name: String, symbol: String, supply: UInt64)`
Create a new asset.

**Parameters:**
- `name` (String): Asset name
- `symbol` (String): Asset symbol
- `supply` (UInt64): Total supply

**Returns:** `Asset` - Created asset

### Android (Kotlin)

#### `SatoxSDK.initialize(config: Config)`
Initialize the mobile SDK.

**Parameters:**
- `config` (Config): SDK configuration

**Returns:** `Boolean` - Success status

#### `Wallet SatoxSDK.createWallet()`
Create a new wallet.

**Returns:** `Wallet` - New wallet instance

#### `Asset SatoxSDK.createAsset(name: String, symbol: String, supply: Long)`
Create a new asset.

**Parameters:**
- `name` (String): Asset name
- `symbol` (String): Asset symbol
- `supply` (Long): Total supply

**Returns:** `Asset` - Created asset

## Game SDK API

### Unity (C#)

#### `SatoxGameSDK.Initialize(Config config)`
Initialize the game SDK.

**Parameters:**
- `config` (Config): SDK configuration

**Returns:** `bool` - Success status

#### `PlayerWallet SatoxGameSDK.CreatePlayerWallet(string playerId)`
Create a wallet for a player.

**Parameters:**
- `playerId` (string): Player identifier

**Returns:** `PlayerWallet` - Player wallet instance

#### `GameAsset SatoxGameSDK.CreateGameAsset(string name, string symbol, long supply)`
Create a game asset.

**Parameters:**
- `name` (string): Asset name
- `symbol` (string): Asset symbol
- `supply` (long): Total supply

**Returns:** `GameAsset` - Created game asset

## Data Types

### Config
```cpp
struct Config {
    std::string network;
    bool quantum_enabled;
    std::string database_url;
    std::string redis_url;
    bool hsm_enabled;
    bool mfa_required;
    int session_timeout;
    int max_login_attempts;
};
```

### Wallet
```cpp
class Wallet {
public:
    std::string getAddress();
    double getBalance();
    std::vector<Asset> getAssets();
    std::vector<Transaction> getTransactionHistory();
    bool backup(const std::string& path);
    bool restore(const std::string& path);
};
```

### Asset
```cpp
class Asset {
public:
    std::string getId();
    std::string getName();
    std::string getSymbol();
    uint64_t getSupply();
    uint64_t getBalance(const Wallet& wallet);
    bool isNFT();
    std::string getMetadata();
};
```

### Transaction
```cpp
class Transaction {
public:
    std::string getId();
    std::string getFrom();
    std::string getTo();
    double getAmount();
    std::string getAssetId();
    std::string getStatus();
    time_t getTimestamp();
    std::string getBlockHash();
    int getConfirmations();
};
```

### QuantumKeyPair
```cpp
class QuantumKeyPair {
public:
    std::string getPublicKey();
    std::string getPrivateKey();
    bool isValid();
    time_t getCreatedAt();
    time_t getExpiresAt();
};
```

## Error Handling

### Error Codes
- `SUCCESS` (0): Operation completed successfully
- `INVALID_CONFIG` (1): Invalid configuration
- `DATABASE_ERROR` (2): Database operation failed
- `NETWORK_ERROR` (3): Network communication failed
- `SECURITY_ERROR` (4): Security operation failed
- `INSUFFICIENT_FUNDS` (5): Insufficient funds for transaction
- `INVALID_ADDRESS` (6): Invalid wallet address
- `ASSET_NOT_FOUND` (7): Asset not found
- `TRANSACTION_FAILED` (8): Transaction failed
- `QUANTUM_ERROR` (9): Quantum operation failed

### Error Handling Example
```cpp
try {
    Wallet wallet = sdk.createWallet();
    Asset asset = sdk.createAsset("MyAsset", "MA", 1000000);
    bool success = sdk.transferAsset(wallet, "address", asset, 100);
    
    if (!success) {
        Error error = sdk.getLastError();
        std::cout << "Error: " << error.getMessage() << std::endl;
    }
} catch (const SatoxException& e) {
    std::cout << "Exception: " << e.what() << std::endl;
}
```

## Rate Limits

### API Limits
- **Wallet creation**: 10 per minute
- **Asset creation**: 5 per minute
- **Transactions**: 100 per minute
- **Database queries**: 1000 per minute

### Rate Limit Handling
```cpp
// Check rate limits before operations
if (sdk.checkRateLimit("create_wallet")) {
    Wallet wallet = sdk.createWallet();
} else {
    // Wait or handle rate limit
    std::cout << "Rate limit exceeded" << std::endl;
}
```

## Authentication

### API Keys
```cpp
// Set API key for authenticated requests
sdk.setApiKey("your-api-key");

// Check if API key is valid
if (sdk.isApiKeyValid()) {
    // Proceed with operations
}
```

### Multi-Factor Authentication
```cpp
// Enable MFA for wallet
sdk.enableMFA(wallet, MFA_TYPE_TOTP);

// Verify MFA token
if (sdk.verifyMFAToken(wallet, token)) {
    // Proceed with sensitive operations
}
```

## Resources
- [Integration Overview](overview.md)
- [Platform-Specific Guides](platform-specific.md)
- [Examples](examples.md)
- [Troubleshooting](../troubleshooting/common-issues.md) 