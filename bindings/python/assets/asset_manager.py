"""
Asset manager for Satox SDK.
"""

from typing import Dict, List, Optional, Any, Tuple
import json
import logging
import asyncio
from datetime import datetime
import hashlib
import base58

from satox.core.exceptions import SatoxError
from satox.security.quantum_crypto import QuantumCrypto
from satox.blockchain.blockchain_manager import BlockchainManager

class AssetManager:
    """Manages asset operations."""
    
    def __init__(self, blockchain_manager: BlockchainManager, config: Optional[Dict] = None):
        """
        Initialize asset manager.
        
        Args:
            blockchain_manager: Blockchain manager instance
            config: Optional configuration dictionary
        """
        self.logger = logging.getLogger(__name__)
        self.config = config or {
            'max_assets': 1000,
            'max_metadata_size': 1024 * 1024,  # 1MB
            'supported_asset_types': ['token', 'nft', 'collectible'],
            'ipfs_gateway': 'https://ipfs.satox.com/ipfs/',
            'verification_required': True
        }
        
        self.blockchain_manager = blockchain_manager
        self.security_manager = QuantumCrypto()
        self._assets: Dict[str, Dict[str, Any]] = {}
        self._metadata_cache: Dict[str, Dict[str, Any]] = {}
        self._permissions: Dict[str, Dict[str, List[str]]] = {}
        self._event_handlers: Dict[str, List[callable]] = {}
    
    async def create_asset(self, 
                          name: str,
                          symbol: str,
                          asset_type: str,
                          total_supply: int,
                          decimals: int = 18,
                          metadata: Optional[Dict[str, Any]] = None,
                          owner: Optional[str] = None,
                          ipfs_hash: Optional[str] = None) -> str:
        """
        Create new asset.
        
        Args:
            name: Asset name
            symbol: Asset symbol
            asset_type: Asset type (token, nft, collectible)
            total_supply: Total supply
            decimals: Number of decimals
            metadata: Optional asset metadata
            owner: Optional asset owner address
            ipfs_hash: Optional IPFS hash for metadata
            
        Returns:
            str: Asset ID
        """
        try:
            # Validate parameters
            if asset_type not in self.config['supported_asset_types']:
                raise SatoxError(f"Unsupported asset type: {asset_type}")
            
            if total_supply <= 0:
                raise SatoxError("Total supply must be positive")
            
            if decimals < 0 or decimals > 18:
                raise SatoxError("Decimals must be between 0 and 18")
            
            # Generate asset ID
            asset_id = self._generate_asset_id(name, symbol)
            
            # Create asset data
            asset_data = {
                'id': asset_id,
                'name': name,
                'symbol': symbol,
                'type': asset_type,
                'total_supply': total_supply,
                'decimals': decimals,
                'owner': owner,
                'created_at': datetime.utcnow().isoformat(),
                'metadata': metadata or {},
                'ipfs_hash': ipfs_hash,
                'status': 'active'
            }
            
            # Store asset data
            self._assets[asset_id] = asset_data
            
            # Store metadata in cache if present
            if metadata:
                self._metadata_cache[asset_id] = metadata
            
            # Initialize permissions
            self._permissions[asset_id] = {
                'owners': [owner] if owner else [],
                'operators': [],
                'blacklist': []
            }
            
            # Notify event handlers
            if 'assetCreated' in self._event_handlers:
                for handler in self._event_handlers['assetCreated']:
                    asyncio.create_task(handler(asset_data))
            
            return asset_id
            
        except Exception as e:
            self.logger.error(f"Failed to create asset: {e}")
            raise SatoxError(f"Failed to create asset: {e}")
    
    async def get_asset(self, asset_id: str) -> Dict[str, Any]:
        """
        Get asset by ID.
        
        Args:
            asset_id: Asset ID
            
        Returns:
            Dict: Asset data
        """
        if asset_id not in self._assets:
            raise SatoxError(f"Asset {asset_id} not found")
        
        return self._assets[asset_id]
    
    async def get_asset_balance(self, asset_id: str, address: str) -> int:
        """
        Get asset balance for address.
        
        Args:
            asset_id: Asset ID
            address: Wallet address
            
        Returns:
            int: Asset balance
        """
        try:
            # Request balance from blockchain
            await self.blockchain_manager._ws.send(json.dumps({
                'method': 'getAssetBalance',
                'params': [asset_id, address],
                'id': 1
            }))
            
            response = await self.blockchain_manager._ws.recv()
            data = json.loads(response)
            
            if 'error' in data:
                raise SatoxError(f"Failed to get asset balance: {data['error']}")
            
            return int(data['result'])
            
        except Exception as e:
            self.logger.error(f"Failed to get asset balance: {e}")
            raise SatoxError(f"Failed to get asset balance: {e}")
    
    async def transfer_asset(self,
                           asset_id: str,
                           from_address: str,
                           to_address: str,
                           amount: int,
                           metadata: Optional[Dict[str, Any]] = None) -> str:
        """
        Transfer asset.
        
        Args:
            asset_id: Asset ID
            from_address: Sender address
            to_address: Recipient address
            amount: Transfer amount
            metadata: Optional transfer metadata
            
        Returns:
            str: Transaction hash
        """
        try:
            # Validate parameters
            if asset_id not in self._assets:
                raise SatoxError(f"Asset {asset_id} not found")
            
            if amount <= 0:
                raise SatoxError("Transfer amount must be positive")
            
            # Check permissions
            if not self._check_transfer_permission(asset_id, from_address):
                raise SatoxError(f"Address {from_address} not authorized to transfer asset")
            
            # Create transfer transaction
            transaction = {
                'type': 'asset_transfer',
                'asset_id': asset_id,
                'from': from_address,
                'to': to_address,
                'amount': amount,
                'metadata': metadata or {},
                'timestamp': datetime.utcnow().isoformat()
            }
            
            # Sign transaction
            signature = self.security_manager.sign_transaction(transaction, from_address)
            transaction['signature'] = signature.hex()
            
            # Broadcast transaction
            tx_hash = await self.blockchain_manager.broadcast_transaction(transaction)
            
            # Notify event handlers
            if 'assetTransferred' in self._event_handlers:
                for handler in self._event_handlers['assetTransferred']:
                    asyncio.create_task(handler(transaction))
            
            return tx_hash
            
        except Exception as e:
            self.logger.error(f"Failed to transfer asset: {e}")
            raise SatoxError(f"Failed to transfer asset: {e}")
    
    async def update_asset_metadata(self,
                                  asset_id: str,
                                  metadata: Dict[str, Any],
                                  owner: str,
                                  ipfs_hash: Optional[str] = None) -> bool:
        """
        Update asset metadata.
        
        Args:
            asset_id: Asset ID
            metadata: New metadata
            owner: Asset owner address
            ipfs_hash: Optional IPFS hash for metadata
            
        Returns:
            bool: True if successful
        """
        try:
            # Validate parameters
            if asset_id not in self._assets:
                raise SatoxError(f"Asset {asset_id} not found")
            
            if not self._check_owner_permission(asset_id, owner):
                raise SatoxError(f"Address {owner} not authorized to update asset")
            
            # Update metadata
            self._assets[asset_id]['metadata'] = metadata
            self._assets[asset_id]['ipfs_hash'] = ipfs_hash
            self._metadata_cache[asset_id] = metadata
            
            # Notify event handlers
            if 'assetMetadataUpdated' in self._event_handlers:
                for handler in self._event_handlers['assetMetadataUpdated']:
                    asyncio.create_task(handler({
                        'asset_id': asset_id,
                        'metadata': metadata,
                        'ipfs_hash': ipfs_hash,
                        'updated_by': owner
                    }))
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to update asset metadata: {e}")
            raise SatoxError(f"Failed to update asset metadata: {e}")
    
    async def verify_asset(self, asset_id: str) -> bool:
        """
        Verify asset.
        
        Args:
            asset_id: Asset ID
            
        Returns:
            bool: True if asset is valid
        """
        try:
            # Get asset data
            asset = await self.get_asset(asset_id)
            
            # Verify asset data
            if not self._verify_asset_data(asset):
                return False
            
            # Verify metadata if present
            if asset.get('metadata'):
                if not self._verify_metadata(asset['metadata']):
                    return False
            
            # Verify IPFS hash if present
            if asset.get('ipfs_hash'):
                if not await self._verify_ipfs_hash(asset['ipfs_hash']):
                    return False
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to verify asset: {e}")
            return False
    
    def add_event_handler(self, event: str, handler: callable) -> None:
        """
        Add event handler.
        
        Args:
            event: Event name
            handler: Event handler function
        """
        if event not in self._event_handlers:
            self._event_handlers[event] = []
        self._event_handlers[event].append(handler)
    
    def remove_event_handler(self, event: str, handler: callable) -> None:
        """
        Remove event handler.
        
        Args:
            event: Event name
            handler: Event handler function
        """
        if event in self._event_handlers and handler in self._event_handlers[event]:
            self._event_handlers[event].remove(handler)
    
    def _generate_asset_id(self, name: str, symbol: str) -> str:
        """
        Generate asset ID.
        
        Args:
            name: Asset name
            symbol: Asset symbol
            
        Returns:
            str: Asset ID
        """
        data = f"{name}:{symbol}:{datetime.utcnow().isoformat()}"
        return base58.b58encode(hashlib.sha256(data.encode()).digest()).decode()
    
    def _check_transfer_permission(self, asset_id: str, address: str) -> bool:
        """
        Check transfer permission.
        
        Args:
            asset_id: Asset ID
            address: Wallet address
            
        Returns:
            bool: True if address has permission
        """
        if asset_id not in self._permissions:
            return False
        
        permissions = self._permissions[asset_id]
        return (address in permissions['owners'] or
                address in permissions['operators']) and \
               address not in permissions['blacklist']
    
    def _check_owner_permission(self, asset_id: str, address: str) -> bool:
        """
        Check owner permission.
        
        Args:
            asset_id: Asset ID
            address: Wallet address
            
        Returns:
            bool: True if address is owner
        """
        if asset_id not in self._permissions:
            return False
        
        return address in self._permissions[asset_id]['owners']
    
    def _verify_asset_data(self, asset: Dict[str, Any]) -> bool:
        """
        Verify asset data.
        
        Args:
            asset: Asset data
            
        Returns:
            bool: True if asset data is valid
        """
        required_fields = ['id', 'name', 'symbol', 'type', 'total_supply', 'decimals']
        return all(field in asset for field in required_fields)
    
    def _verify_metadata(self, metadata: Dict[str, Any]) -> bool:
        """
        Verify metadata.
        
        Args:
            metadata: Asset metadata
            
        Returns:
            bool: True if metadata is valid
        """
        # Check metadata size
        metadata_size = len(json.dumps(metadata).encode())
        if metadata_size > self.config['max_metadata_size']:
            return False
        
        return True
    
    async def _verify_ipfs_hash(self, ipfs_hash: str) -> bool:
        """
        Verify IPFS hash.
        
        Args:
            ipfs_hash: IPFS hash
            
        Returns:
            bool: True if IPFS hash is valid
        """
        try:
            # Request IPFS hash verification
            await self.blockchain_manager._ws.send(json.dumps({
                'method': 'verifyIpfsHash',
                'params': [ipfs_hash],
                'id': 1
            }))
            
            response = await self.blockchain_manager._ws.recv()
            data = json.loads(response)
            
            if 'error' in data:
                return False
            
            return data['result']
            
        except Exception as e:
            self.logger.error(f"Failed to verify IPFS hash: {e}")
            return False 