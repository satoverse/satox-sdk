"""
Mock wallet implementation for testing Satoxcoin features.
This simulates the mainnet environment since Satoxcoin only operates on mainnet.
Supports both current (9007) and legacy (1669) SLIP-44 coin types.
"""

import json
import time
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass
from datetime import datetime
from enum import Enum

# SLIP-44 coin types for Satoxcoin
class CoinType(Enum):
    CURRENT = 9007  # Official current coin type
    LEGACY = 1669   # Legacy coin type

@dataclass
class Transaction:
    """Represents a Satoxcoin transaction."""
    txid: str
    sender: str
    recipient: str
    amount: float
    asset_name: Optional[str] = None
    timestamp: float = time.time()
    status: str = "pending"
    block_height: Optional[int] = None
    signature: Optional[str] = None
    metadata: Dict[str, Any] = None
    coin_type: CoinType = CoinType.CURRENT

@dataclass
class Asset:
    """Represents a Satoxcoin asset."""
    name: str
    type: str = "asset"
    amount: float = 0.0
    owner: str = ""
    metadata: Dict[str, Any] = None
    created_at: datetime = datetime.now()
    updated_at: datetime = datetime.now()
    ipfs_hash: Optional[str] = None
    coin_type: CoinType = CoinType.CURRENT
    is_nft: bool = False

class MockWallet:
    """Mock wallet for testing Satoxcoin features.
    This simulates the mainnet environment since Satoxcoin only operates on mainnet.
    Supports both current (9007) and legacy (1669) SLIP-44 coin types.
    """
    
    def __init__(self):
        """Initialize the mock wallet."""
        self._addresses: Dict[str, Dict[str, float]] = {}  # address -> asset_name -> balance
        self._transactions: Dict[str, Transaction] = {}
        self._assets: Dict[str, Asset] = {}
        self._ipfs_pins = set()
        self._block_height = 0
        self._connected = False
        self._coin_type = CoinType.CURRENT  # Default to current coin type
        self._confirmations_required = 100
    
    def connect(self) -> bool:
        """Connect to the Satoxcoin mainnet."""
        self._connected = True
        return True
    
    def disconnect(self) -> bool:
        """Disconnect from the Satoxcoin mainnet."""
        self._connected = False
        return True
    
    def is_connected(self) -> bool:
        """Check if connected to mainnet."""
        return self._connected
    
    def set_coin_type(self, coin_type: CoinType) -> None:
        """Set the coin type for address generation and transactions."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        self._coin_type = coin_type
    
    def get_coin_type(self) -> CoinType:
        """Get the current coin type."""
        return self._coin_type
    
    def create_address(self) -> str:
        """Create a new address using the current coin type."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        address = f"mock_address_{len(self._addresses)}_{self._coin_type.value}"
        self._addresses[address] = {}
        return address
    
    def get_balance(self, address: str, asset_name: Optional[str] = None) -> float:
        """Get balance for an address and optional asset."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if address not in self._addresses:
            return 0.0
        
        if asset_name is None:
            # Return total balance across all assets
            return sum(self._addresses[address].values())
        
        return self._addresses[address].get(asset_name, 0.0)
    
    def send_transaction(self, from_address: str, to_address: str, amount: float, 
                        asset_name: Optional[str] = None, metadata: Optional[Dict[str, Any]] = None) -> str:
        """Send a transaction."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if from_address not in self._addresses:
            raise ValueError(f"Sender address {from_address} not found")
        
        if to_address not in self._addresses:
            raise ValueError(f"Recipient address {to_address} not found")
        
        # Check balance
        current_balance = self.get_balance(from_address, asset_name)
        if current_balance < amount:
            raise ValueError(f"Insufficient balance for {asset_name or 'SATOX'}")

        # Create transaction
        txid = f"mock_tx_{len(self._transactions)}"
        tx = Transaction(
            txid=txid,
            sender=from_address,
            recipient=to_address,
            amount=amount,
            asset_name=asset_name,
            metadata=metadata or {},
            coin_type=self._coin_type
        )
        
        # Update balances
        self._update_balance(from_address, asset_name, -amount)
        self._update_balance(to_address, asset_name, amount)
        
        # Store transaction
        self._transactions[txid] = tx
        
        return txid
    
    def get_transaction(self, txid: str) -> Optional[Transaction]:
        """Get transaction by ID."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        return self._transactions.get(txid)
    
    def list_transactions(self, address: Optional[str] = None, coin_type: Optional[CoinType] = None) -> List[Transaction]:
        """List transactions, optionally filtered by address and coin type."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        transactions = self._transactions.values()
        
        if address is not None:
            transactions = [tx for tx in transactions if tx.sender == address or tx.recipient == address]
        
        if coin_type is not None:
            transactions = [tx for tx in transactions if tx.coin_type == coin_type]
        
        return list(transactions)
    
    def create_asset(self, name: str, amount: float, owner: str, ipfs_hash: Optional[str] = None, metadata: Optional[Dict[str, Any]] = None, type: str = "asset", is_nft: bool = False) -> Asset:
        """Create a new asset."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if name in self._assets:
            raise ValueError(f"Asset {name} already exists")
        if metadata is None:
            metadata = {}
        asset = Asset(
            name=name,
            type=type,
            amount=amount,
            owner=owner,
            metadata=metadata,
            ipfs_hash=ipfs_hash,
            coin_type=self._coin_type,
            is_nft=is_nft
        )
        self._assets[name] = asset
        self._update_balance(owner, name, amount)
        return asset

    def transfer_asset(self, name: str, from_owner: str, to_owner: str, amount: float) -> None:
        """Transfer an asset between owners."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if name not in self._assets:
            raise ValueError(f"Asset {name} does not exist")
        
        # Check if sender has enough balance
        current_balance = self.get_balance(from_owner, name)
        if current_balance < amount:
            raise ValueError(f"Insufficient balance for {name}")
        
        # Update balances
        self._update_balance(from_owner, name, -amount)
        self._update_balance(to_owner, name, amount)
        
        # Update asset owner if all amount is transferred
        if current_balance == amount:
            self._assets[name].owner = to_owner
    
    def get_asset_info(self, name: str) -> Optional[Asset]:
        """Get information about an asset."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        return self._assets.get(name)
    
    def list_assets(self, owner: Optional[str] = None, coin_type: Optional[CoinType] = None) -> List[Asset]:
        """List assets, optionally filtered by owner and coin type."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        assets = self._assets.values()
        
        if owner is not None:
            assets = [asset for asset in assets if asset.owner == owner]
        
        if coin_type is not None:
            assets = [asset for asset in assets if asset.coin_type == coin_type]
        
        return list(assets)
    
    def store_asset_metadata(self, name: str, metadata: Dict[str, Any]) -> str:
        """Store asset metadata and return IPFS hash."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if name not in self._assets:
            raise ValueError(f"Asset {name} does not exist")
        
        # In a real implementation, this would store the metadata in IPFS
        # For now, we'll just update the asset's metadata
        self._assets[name].metadata.update(metadata)
        self._assets[name].updated_at = datetime.now()
        
        # Generate a dummy IPFS hash
        ipfs_hash = f"ipfs://{hash(str(metadata))}"
        self._assets[name].ipfs_hash = ipfs_hash
        return ipfs_hash
    
    def get_asset_metadata(self, ipfs_hash: str) -> Dict[str, Any]:
        """Get asset metadata from IPFS hash."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        # Find asset with matching IPFS hash
        for asset in self._assets.values():
            if asset.ipfs_hash == ipfs_hash:
                metadata = dict(asset.metadata) if asset.metadata else {}
                metadata["ipfs_hash"] = ipfs_hash
                return metadata
        return {"ipfs_hash": ipfs_hash, "timestamp": datetime.now().isoformat()}

    def verify_ipfs_metadata(self, ipfs_hash: str) -> bool:
        """Verify IPFS metadata integrity."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        # Check if any asset has this IPFS hash
        return any(asset.ipfs_hash == ipfs_hash for asset in self._assets.values())

    def pin_ipfs_metadata(self, ipfs_hash: str) -> bool:
        """Pin IPFS metadata."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if self.verify_ipfs_metadata(ipfs_hash):
            self._ipfs_pins.add(ipfs_hash)
            return True
        return False

    def unpin_ipfs_metadata(self, ipfs_hash: str) -> bool:
        """Unpin IPFS metadata."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if ipfs_hash in self._ipfs_pins:
            self._ipfs_pins.remove(ipfs_hash)
            return True
        return False

    def get_ipfs_pin_status(self, ipfs_hash: str) -> Dict[str, bool]:
        """Get IPFS pin status."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        return {"is_pinned": ipfs_hash in self._ipfs_pins}
    
    def _update_balance(self, address: str, asset_name: Optional[str], amount: float) -> None:
        """Update the balance of an address."""
        if address not in self._addresses:
            self._addresses[address] = {}
        
        asset_name = asset_name or "SATOX"
        current_balance = self._addresses[address].get(asset_name, 0.0)
        new_balance = current_balance + amount
        
        if new_balance < 0:
            raise ValueError(f"Negative balance not allowed for {asset_name}")
        
        self._addresses[address][asset_name] = new_balance
    
    def mine_block(self) -> int:
        """Simulate mining a block."""
        BLOCK_TIME = 60
        time.sleep(BLOCK_TIME)
        self._block_height += 1
        # Update transaction statuses
        for tx in self._transactions.values():
            if tx.status == "pending" and tx.block_height is not None:
                if self._block_height - tx.block_height >= self._confirmations_required:
                tx.status = "confirmed"
        return self._block_height
    
    def get_block_height(self) -> int:
        """Get current block height."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        return self._block_height
    
    def export_wallet(self) -> str:
        """Export wallet data as JSON."""
        data = {
            "addresses": self._addresses,
            "transactions": {
                txid: {
                    "sender": tx.sender,
                    "recipient": tx.recipient,
                    "amount": tx.amount,
                    "asset_name": tx.asset_name,
                    "timestamp": tx.timestamp,
                    "status": tx.status,
                    "block_height": tx.block_height,
                    "signature": tx.signature,
                    "metadata": tx.metadata,
                    "coin_type": tx.coin_type.value
                }
                for txid, tx in self._transactions.items()
            },
            "assets": {
                name: {
                    "type": asset.type,
                    "amount": asset.amount,
                    "owner": asset.owner,
                    "metadata": asset.metadata,
                    "created_at": asset.created_at.isoformat(),
                    "updated_at": asset.updated_at.isoformat(),
                    "ipfs_hash": asset.ipfs_hash,
                    "coin_type": asset.coin_type.value
                }
                for name, asset in self._assets.items()
            },
            "block_height": self._block_height,
            "coin_type": self._coin_type.value
        }
        return json.dumps(data, indent=2)
    
    def import_wallet(self, data: str) -> None:
        """Import wallet data from JSON."""
        data = json.loads(data)
        
        # Import addresses
        self._addresses = data["addresses"]
        
        # Import transactions
        self._transactions = {
            txid: Transaction(
                txid=txid,
                sender=tx["sender"],
                recipient=tx["recipient"],
                amount=tx["amount"],
                asset_name=tx["asset_name"],
                timestamp=tx["timestamp"],
                status=tx["status"],
                block_height=tx["block_height"],
                signature=tx.get("signature"),
                metadata=tx.get("metadata", {}),
                coin_type=CoinType(tx.get("coin_type", CoinType.CURRENT.value))
            )
            for txid, tx in data["transactions"].items()
        }
        
        # Import assets
        self._assets = {
            name: Asset(
                name=name,
                type=asset["type"],
                amount=asset["amount"],
                owner=asset["owner"],
                metadata=asset["metadata"],
                created_at=datetime.fromisoformat(asset["created_at"]),
                updated_at=datetime.fromisoformat(asset["updated_at"]),
                ipfs_hash=asset.get("ipfs_hash"),
                coin_type=CoinType(asset.get("coin_type", CoinType.CURRENT.value)),
                is_nft=asset.get("is_nft", False)
            )
            for name, asset in data["assets"].items()
        }
        
        # Import block height and coin type
        self._block_height = data["block_height"]
        self._coin_type = CoinType(data.get("coin_type", CoinType.CURRENT.value)) 

    def update_asset_metadata(self, name: str, ipfs_hash: str) -> None:
        """Update asset's IPFS hash."""
        if not self._connected:
            raise RuntimeError("Not connected to mainnet")
        if name not in self._assets:
            raise ValueError(f"Asset {name} does not exist")
        self._assets[name].ipfs_hash = ipfs_hash
        self._assets[name].updated_at = datetime.now() 