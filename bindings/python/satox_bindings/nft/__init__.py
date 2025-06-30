"""
Satox SDK NFT Management Module
"""

from typing import Dict, List, Optional, Any
from datetime import datetime
import json
import hashlib
from enum import Enum

class NFTStandard(Enum):
    """NFT standards"""
    ERC721 = "ERC721"
    ERC1155 = "ERC1155"

class NFTMetadata:
    """Represents NFT metadata"""
    
    def __init__(self,
                 name: str,
                 description: str,
                 image_url: str,
                 attributes: Optional[List[Dict[str, Any]]] = None,
                 external_url: Optional[str] = None,
                 animation_url: Optional[str] = None,
                 background_color: Optional[str] = None):
        self.name = name
        self.description = description
        self.image_url = image_url
        self.attributes = attributes or []
        self.external_url = external_url
        self.animation_url = animation_url
        self.background_color = background_color
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert metadata to dictionary"""
        return {
            'name': self.name,
            'description': self.description,
            'image_url': self.image_url,
            'attributes': self.attributes,
            'external_url': self.external_url,
            'animation_url': self.animation_url,
            'background_color': self.background_color
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'NFTMetadata':
        """Create metadata from dictionary"""
        return cls(
            name=data['name'],
            description=data['description'],
            image_url=data['image_url'],
            attributes=data.get('attributes'),
            external_url=data.get('external_url'),
            animation_url=data.get('animation_url'),
            background_color=data.get('background_color')
        )

class NFT:
    """Represents a non-fungible token"""
    
    def __init__(self,
                 token_id: str,
                 contract_address: str,
                 owner: str,
                 standard: NFTStandard,
                 metadata: NFTMetadata,
                 created_at: Optional[datetime] = None,
                 creator: Optional[str] = None,
                 royalty_percentage: Optional[float] = None):
        self.token_id = token_id
        self.contract_address = contract_address
        self.owner = owner
        self.standard = standard
        self.metadata = metadata
        self.created_at = created_at or datetime.utcnow()
        self.creator = creator or owner
        self.royalty_percentage = royalty_percentage or 0.0
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert NFT to dictionary"""
        return {
            'token_id': self.token_id,
            'contract_address': self.contract_address,
            'owner': self.owner,
            'standard': self.standard.value,
            'metadata': self.metadata.to_dict(),
            'created_at': self.created_at.isoformat(),
            'creator': self.creator,
            'royalty_percentage': self.royalty_percentage
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'NFT':
        """Create NFT from dictionary"""
        return cls(
            token_id=data['token_id'],
            contract_address=data['contract_address'],
            owner=data['owner'],
            standard=NFTStandard(data['standard']),
            metadata=NFTMetadata.from_dict(data['metadata']),
            created_at=datetime.fromisoformat(data['created_at']),
            creator=data.get('creator'),
            royalty_percentage=data.get('royalty_percentage')
        )

class NFTManager:
    """Manages NFT operations"""
    
    def __init__(self):
        self._nfts: Dict[str, NFT] = {}
        self._last_error = ""
    
    def mint_nft(self,
                contract_address: str,
                owner: str,
                standard: NFTStandard,
                metadata: NFTMetadata,
                creator: Optional[str] = None,
                royalty_percentage: Optional[float] = None) -> Optional[NFT]:
        """
        Mint a new NFT
        
        Args:
            contract_address: NFT contract address
            owner: Owner address
            standard: NFT standard
            metadata: NFT metadata
            creator: Creator address
            royalty_percentage: Royalty percentage
            
        Returns:
            Optional[NFT]: Created NFT if successful, None otherwise
        """
        try:
            # Validate parameters
            if not self._validate_nft_parameters(contract_address, owner, metadata, royalty_percentage):
                return None
            
            # Generate token ID
            token_id = self._generate_token_id(contract_address, metadata)
            
            # Create NFT
            nft = NFT(
                token_id=token_id,
                contract_address=contract_address,
                owner=owner,
                standard=standard,
                metadata=metadata,
                creator=creator,
                royalty_percentage=royalty_percentage
            )
            
            self._nfts[token_id] = nft
            return nft
            
        except Exception as e:
            self._last_error = f"Failed to mint NFT: {str(e)}"
            return None
    
    def get_nft(self, token_id: str) -> Optional[NFT]:
        """
        Get NFT by token ID
        
        Args:
            token_id: Token ID
            
        Returns:
            Optional[NFT]: NFT if found, None otherwise
        """
        return self._nfts.get(token_id)
    
    def get_nfts_by_owner(self, owner: str) -> List[NFT]:
        """
        Get all NFTs owned by an address
        
        Args:
            owner: Owner address
            
        Returns:
            List[NFT]: List of owned NFTs
        """
        return [nft for nft in self._nfts.values() if nft.owner == owner]
    
    def get_nfts_by_contract(self, contract_address: str) -> List[NFT]:
        """
        Get all NFTs from a contract
        
        Args:
            contract_address: Contract address
            
        Returns:
            List[NFT]: List of NFTs from contract
        """
        return [nft for nft in self._nfts.values() if nft.contract_address == contract_address]
    
    def transfer_nft(self,
                    token_id: str,
                    from_address: str,
                    to_address: str) -> bool:
        """
        Transfer NFT ownership
        
        Args:
            token_id: Token ID
            from_address: Current owner address
            to_address: New owner address
            
        Returns:
            bool: True if transfer successful, False otherwise
        """
        try:
            nft = self.get_nft(token_id)
            if not nft:
                self._last_error = f"NFT not found: {token_id}"
                return False
            
            if nft.owner != from_address:
                self._last_error = "Invalid owner address"
                return False
            
            nft.owner = to_address
            return True
            
        except Exception as e:
            self._last_error = f"Failed to transfer NFT: {str(e)}"
            return False
    
    def _generate_token_id(self, contract_address: str, metadata: NFTMetadata) -> str:
        """Generate unique token ID"""
        data = f"{contract_address}{metadata.name}{metadata.description}{datetime.utcnow().isoformat()}"
        return hashlib.sha256(data.encode()).hexdigest()
    
    def _validate_nft_parameters(self,
                               contract_address: str,
                               owner: str,
                               metadata: NFTMetadata,
                               royalty_percentage: Optional[float]) -> bool:
        """
        Validate NFT parameters
        
        Args:
            contract_address: Contract address
            owner: Owner address
            metadata: NFT metadata
            royalty_percentage: Royalty percentage
            
        Returns:
            bool: True if parameters are valid, False otherwise
        """
        if not contract_address or not owner:
            self._last_error = "Invalid contract address or owner"
            return False
        
        if not metadata.name or not metadata.description or not metadata.image_url:
            self._last_error = "Invalid metadata"
            return False
        
        if royalty_percentage is not None and (royalty_percentage < 0 or royalty_percentage > 100):
            self._last_error = "Royalty percentage must be between 0 and 100"
            return False
        
        return True
    
    def get_last_error(self) -> str:
        """
        Get the last error message
        
        Returns:
            str: Last error message
        """
        return self._last_error
    
    def clear_last_error(self) -> None:
        """Clear the last error message"""
        self._last_error = ""

# Export the manager instance
nft_manager = NFTManager()
