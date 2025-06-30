# Satox SDK C# Usage Examples

## Basic Usage

### Initializing the SDK
```csharp
using Satox.SDK;

// Create configuration
var config = new Config
{
    Network = "mainnet",
    ApiEndpoint = "https://api.satox.com",
    DatabaseConnection = "connection_string"
};

// Initialize SDK
var satox = new SatoxManager(config);
await satox.Initialize();

// Get components
var blockchainManager = satox.GetComponent<BlockchainManager>();
var walletManager = satox.GetComponent<WalletManager>();
```

### Creating and Managing Wallets
```csharp
// Create a new wallet
var wallet = await walletManager.CreateWallet();
Console.WriteLine($"New wallet address: {wallet.Address}");

// Import existing wallet
var importedWallet = await walletManager.ImportWallet("private_key_here");

// Backup wallet
var backup = await walletManager.BackupWallet(wallet.Address);

// Restore wallet
var restoredWallet = await walletManager.RestoreWallet(backup);
```

### Managing Assets
```csharp
// Get asset manager
var assetManager = satox.GetComponent<AssetManager>();

// Create new asset
var assetData = new AssetData
{
    Name = "My Token",
    Symbol = "MTK",
    Decimals = 18,
    TotalSupply = 1000000
};
var asset = await assetManager.CreateAsset(assetData);

// Transfer asset
var transferResult = await assetManager.TransferAsset(
    "from_address",
    "to_address",
    asset.Id,
    100
);

// Get asset balance
var balance = await assetManager.GetAssetBalance("address", asset.Id);
```

### Managing NFTs
```csharp
// Get NFT manager
var nftManager = satox.GetComponent<NFTManager>();

// Create new NFT
var nftData = new NFTData
{
    Name = "My NFT",
    Description = "A unique digital asset",
    ImageUrl = "https://example.com/image.png",
    Metadata = new Dictionary<string, string>
    {
        { "key", "value" }
    }
};
var nft = await nftManager.CreateNFT(nftData);

// Transfer NFT
var transferResult = await nftManager.TransferNFT(
    "from_address",
    "to_address",
    nft.Id
);

// Get NFT owner
var owner = await nftManager.GetNFTOwner(nft.Id);
```

### IPFS Operations
```csharp
// Get IPFS manager
var ipfsManager = satox.GetComponent<IPFSManager>();

// Upload content
var content = Encoding.UTF8.GetBytes("Hello, IPFS!");
var uploadResult = await ipfsManager.UploadContent(content);

// Download content
var downloadedContent = await ipfsManager.DownloadContent(uploadResult.Hash);

// Pin content
var pinResult = await ipfsManager.PinContent(uploadResult.Hash);

// Search content
var searchResults = await ipfsManager.SearchContent("query");
```

### Database Operations
```csharp
// Get database manager
var dbManager = satox.GetComponent<DatabaseManager>();

// Connect to database
await dbManager.Connect("connection_string");

// Execute query
var query = "SELECT * FROM users WHERE id = @id";
var parameters = new { id = 1 };
var result = await dbManager.Query<User>(query, parameters);

// Execute command
var command = "INSERT INTO users (name, email) VALUES (@name, @email)";
var commandParams = new { name = "John", email = "john@example.com" };
var affectedRows = await dbManager.Execute(command, commandParams);
```

### API Operations
```csharp
// Get API manager
var apiManager = satox.GetComponent<APIManager>();

// Send GET request
var response = await apiManager.Get("/api/v1/users");

// Send POST request
var data = new { name = "John", email = "john@example.com" };
var postResponse = await apiManager.Post("/api/v1/users", data);

// Send PUT request
var updateData = new { name = "John Doe" };
var putResponse = await apiManager.Put("/api/v1/users/1", updateData);

// Send DELETE request
var deleteResponse = await apiManager.Delete("/api/v1/users/1");
```

### Security Operations
```csharp
// Get security manager
var securityManager = satox.GetComponent<SecurityManager>();

// Validate request
var request = new Request { /* ... */ };
var isValid = await securityManager.ValidateRequest(request);

// Check permissions
var hasPermission = await securityManager.CheckPermissions("user_id", "resource");

// Generate token
var token = await securityManager.GenerateToken("user_id");
```

### Error Handling
```csharp
try
{
    var result = await blockchainManager.GetBlock("block_hash");
}
catch (SatoxError ex)
{
    Console.WriteLine($"Error code: {ex.Code}");
    Console.WriteLine($"Error details: {ex.Details}");
}
catch (Exception ex)
{
    Console.WriteLine($"Unexpected error: {ex.Message}");
}
```

### Cleanup
```csharp
// Shutdown SDK
await satox.Shutdown();
``` 