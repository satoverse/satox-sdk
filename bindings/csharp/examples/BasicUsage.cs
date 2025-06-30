using Satox.SDK;
using Satox.SDK.Models;
using System;
using System.Threading.Tasks;

class BasicUsage
{
    static async Task Main(string[] args)
    {
        try
        {
            // Initialize Blockchain Manager
            var config = new BlockchainConfig
            {
                Network = "mainnet",
                NodeUrl = "https://node.satox.com"
            };

            using var blockchainManager = new BlockchainManager(config);
            await blockchainManager.ConnectAsync();

            // Get blockchain information
            var blockHeight = await blockchainManager.GetBlockHeightAsync();
            Console.WriteLine($"Current block height: {blockHeight}");

            // Create and broadcast a transaction
            var tx = await blockchainManager.CreateTransactionAsync(
                "recipient_address",
                1.0m,
                "optional_memo"
            );
            var txId = await blockchainManager.BroadcastTransactionAsync(tx);
            Console.WriteLine($"Transaction broadcast: {txId}");

            // Initialize NFT Manager
            using var nftManager = new NFTManager(config);

            // Create an NFT
            var metadata = new NFTMetadata
            {
                Name = "My First NFT",
                Description = "A unique digital asset",
                Image = "ipfs://Qm...",
                Attributes = new[]
                {
                    new NFTAttribute { TraitType = "rarity", Value = "legendary" }
                }
            };

            var tokenId = await nftManager.CreateNFTAsync(metadata);
            Console.WriteLine($"NFT created with token ID: {tokenId}");

            // Transfer NFT
            var transferTxId = await nftManager.TransferNFTAsync(
                tokenId,
                "new_owner_address"
            );
            Console.WriteLine($"NFT transferred: {transferTxId}");

            // Initialize IPFS Manager
            using var ipfsManager = new IPFSManager(config);

            // Upload file to IPFS
            var cid = await ipfsManager.AddAsync("path/to/file.jpg");
            Console.WriteLine($"File uploaded to IPFS: {cid}");

            // Pin content
            await ipfsManager.PinAsync(cid);
            Console.WriteLine($"Content pinned: {cid}");

            // Get pinned content
            var content = await ipfsManager.GetAsync(cid);
            Console.WriteLine($"Retrieved content size: {content.Length} bytes");

            // Get NFTs by owner
            var nfts = await nftManager.GetNFTsByOwnerAsync("owner_address");
            foreach (var nft in nfts)
            {
                Console.WriteLine($"Found NFT: {nft.Name} (ID: {nft.TokenId})");
            }

            // Search NFTs
            var searchResults = await nftManager.SearchNFTsAsync("legendary");
            foreach (var nft in searchResults)
            {
                Console.WriteLine($"Found NFT in search: {nft.Name}");
            }

            // Get NFT statistics
            var stats = await nftManager.GetNFTStatsAsync(tokenId);
            Console.WriteLine($"NFT stats - Views: {stats.Views}, Likes: {stats.Likes}");

        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"Error: {ex.Message}");
            Console.Error.WriteLine(ex.StackTrace);
        }
    }
} 