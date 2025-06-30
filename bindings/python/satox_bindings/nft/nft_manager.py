from typing import Dict, List, Optional, Any, Union
from ..core.satox_error import SatoxError
import json
import logging
from datetime import datetime
import ipfshttpclient
import hashlib
import os

class NFTManager:
    """Manages NFT operations and IPFS integration."""
    
    def __init__(self, ipfs_api_url: str = "/ip4/127.0.0.1/tcp/5001"):
        """Initialize the NFT manager.
        
        Args:
            ipfs_api_url: IPFS API URL
        """
        self.ipfs_client = ipfshttpclient.connect(ipfs_api_url)
        self.nfts: Dict[str, Dict[str, Any]] = {}  # nft_id -> nft_data
        self.logger = logging.getLogger(__name__)
        
    def create_nft(self, 
                  name: str,
                  description: str,
                  creator: str,
                  metadata: Dict[str, Any],
                  asset_data: Union[str, bytes],
                  asset_type: str) -> str:
        """Create a new NFT.
        
        Args:
            name: NFT name
            description: NFT description
            creator: Creator's address
            metadata: Additional metadata
            asset_data: Asset data (file path or bytes)
            asset_type: Type of asset (e.g., 'image', 'video', 'audio')
            
        Returns:
            NFT ID
            
        Raises:
            SatoxError: If NFT creation fails
        """
        try:
            # Generate NFT ID
            nft_id = hashlib.sha256(
                f"{name}{description}{creator}{datetime.now().isoformat()}".encode()
            ).hexdigest()
            
            # Upload asset to IPFS
            if isinstance(asset_data, str):
                with open(asset_data, 'rb') as f:
                    asset_hash = self.ipfs_client.add_bytes(f.read())
            else:
                asset_hash = self.ipfs_client.add_bytes(asset_data)
                
            # Create NFT data
            nft_data = {
                'id': nft_id,
                'name': name,
                'description': description,
                'creator': creator,
                'created_at': datetime.now().isoformat(),
                'asset_hash': asset_hash,
                'asset_type': asset_type,
                'metadata': metadata
            }
            
            # Store NFT data
            self.nfts[nft_id] = nft_data
            
            return nft_id
            
        except Exception as e:
            raise SatoxError(f"Failed to create NFT: {str(e)}")
            
    def get_nft(self, nft_id: str) -> Dict[str, Any]:
        """Get NFT data.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            NFT data
            
        Raises:
            SatoxError: If NFT does not exist
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        return self.nfts[nft_id].copy()
        
    def update_nft(self, nft_id: str, updates: Dict[str, Any]) -> None:
        """Update NFT data.
        
        Args:
            nft_id: NFT identifier
            updates: Data to update
            
        Raises:
            SatoxError: If NFT does not exist
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        # Update NFT data
        self.nfts[nft_id].update(updates)
        
    def delete_nft(self, nft_id: str) -> None:
        """Delete an NFT.
        
        Args:
            nft_id: NFT identifier
            
        Raises:
            SatoxError: If NFT does not exist
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        try:
            # Remove from IPFS (optional)
            # self.ipfs_client.pin_rm(self.nfts[nft_id]['asset_hash'])
            
            # Remove NFT data
            del self.nfts[nft_id]
            
        except Exception as e:
            raise SatoxError(f"Failed to delete NFT: {str(e)}")
            
    def list_nfts(self, creator: Optional[str] = None) -> List[Dict[str, Any]]:
        """List NFTs.
        
        Args:
            creator: Filter by creator
            
        Returns:
            List of NFT data
        """
        if creator:
            return [
                nft.copy() for nft in self.nfts.values()
                if nft['creator'] == creator
            ]
        return [nft.copy() for nft in self.nfts.values()]
        
    def get_nft_asset(self, nft_id: str) -> bytes:
        """Get NFT asset data.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            Asset data
            
        Raises:
            SatoxError: If NFT does not exist
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        try:
            return self.ipfs_client.cat(self.nfts[nft_id]['asset_hash'])
            
        except Exception as e:
            raise SatoxError(f"Failed to get NFT asset: {str(e)}")
            
    def search_nfts(self, query: str) -> List[Dict[str, Any]]:
        """Search NFTs by name or description.
        
        Args:
            query: Search query
            
        Returns:
            List of matching NFT data
        """
        query = query.lower()
        return [
            nft.copy() for nft in self.nfts.values()
            if query in nft['name'].lower() or query in nft['description'].lower()
        ]
        
    def get_nft_history(self, nft_id: str) -> List[Dict[str, Any]]:
        """Get NFT transaction history.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            List of transactions
            
        Raises:
            SatoxError: If NFT does not exist
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        # TODO: Implement transaction history
        return []
        
    def transfer_nft(self, nft_id: str, from_address: str, to_address: str) -> None:
        """Transfer NFT ownership.
        
        Args:
            nft_id: NFT identifier
            from_address: Current owner's address
            to_address: New owner's address
            
        Raises:
            SatoxError: If transfer fails
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        # TODO: Implement transfer logic
        pass
        
    def verify_nft(self, nft_id: str) -> bool:
        """Verify NFT integrity.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            True if NFT is valid
            
        Raises:
            SatoxError: If verification fails
        """
        if nft_id not in self.nfts:
            raise SatoxError(f"NFT {nft_id} does not exist")
            
        try:
            # Verify asset exists in IPFS
            self.ipfs_client.cat(self.nfts[nft_id]['asset_hash'])
            return True
            
        except Exception as e:
            self.logger.error(f"NFT verification failed: {str(e)}")
            return False 