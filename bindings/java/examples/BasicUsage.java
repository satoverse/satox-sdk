import com.satox.sdk.BlockchainManager;
import com.satox.sdk.NFTManager;
import com.satox.sdk.IPFSManager;
import com.satox.sdk.models.BlockchainConfig;
import com.satox.sdk.models.NFTMetadata;
import com.satox.sdk.models.Transaction;

public class BasicUsage {
    public static void main(String[] args) {
        try {
            // Initialize Blockchain Manager
            BlockchainConfig config = new BlockchainConfig.Builder()
                .setNetwork("mainnet")
                .setNodeUrl("https://node.satox.com")
                .build();
            
            BlockchainManager blockchainManager = new BlockchainManager(config);
            blockchainManager.connect();

            // Get blockchain information
            long blockHeight = blockchainManager.getBlockHeight();
            System.out.println("Current block height: " + blockHeight);

            // Create and broadcast a transaction
            Transaction tx = blockchainManager.createTransaction(
                "recipient_address",
                1.0,
                "optional_memo"
            );
            String txId = blockchainManager.broadcastTransaction(tx);
            System.out.println("Transaction broadcast: " + txId);

            // Initialize NFT Manager
            NFTManager nftManager = new NFTManager(config);

            // Create an NFT
            NFTMetadata metadata = new NFTMetadata.Builder()
                .setName("My First NFT")
                .setDescription("A unique digital asset")
                .setImage("ipfs://Qm...")
                .addAttribute("rarity", "legendary")
                .build();

            String tokenId = nftManager.createNFT(metadata);
            System.out.println("NFT created with token ID: " + tokenId);

            // Transfer NFT
            String transferTxId = nftManager.transferNFT(
                tokenId,
                "new_owner_address"
            );
            System.out.println("NFT transferred: " + transferTxId);

            // Initialize IPFS Manager
            IPFSManager ipfsManager = new IPFSManager(config);

            // Upload file to IPFS
            String cid = ipfsManager.add("path/to/file.jpg");
            System.out.println("File uploaded to IPFS: " + cid);

            // Pin content
            ipfsManager.pin(cid);
            System.out.println("Content pinned: " + cid);

            // Get pinned content
            byte[] content = ipfsManager.get(cid);
            System.out.println("Retrieved content size: " + content.length + " bytes");

            // Cleanup
            blockchainManager.disconnect();
            nftManager.disconnect();
            ipfsManager.disconnect();

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
} 