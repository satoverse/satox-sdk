import hashlib
import time
import json
import random
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass
from enum import Enum

# SLIP-0044 coin type for Satoxcoin
SATOX_COIN_TYPE = 9007

# Kawpow constants
KAWPOW_EPOCH_LENGTH = 7500
KAWPOW_DAG_SIZE = 1073741824  # 1GB
KAWPOW_CACHE_SIZE = 1073741824  # 1GB
KAWPOW_DIFFICULTY_ADJUSTMENT = 2016  # blocks

class TransactionStatus(Enum):
    PENDING = "pending"
    CONFIRMED = "confirmed"
    FAILED = "failed"
    CANCELLED = "cancelled"

class AssetType(Enum):
    TOKEN = "token"
    NFT = "nft"
    SUB_TOKEN = "sub_token"
    RESTRICTED_TOKEN = "restricted_token"
    QUALIFIER_TOKEN = "qualifier_token"
    SUB_QUALIFIER_TOKEN = "sub_qualifier_token"

@dataclass
class MockTransaction:
    hash: str
    from_address: str
    to_address: str
    value: int
    data: str
    timestamp: int
    status: TransactionStatus
    block_hash: Optional[str] = None
    block_number: Optional[int] = None
    gas_used: Optional[int] = None
    asset_type: Optional[AssetType] = None
    asset_id: Optional[str] = None
    asset_amount: Optional[int] = None
    asset_metadata: Optional[Dict] = None

@dataclass
class MockBlock:
    hash: str
    previous_hash: str
    timestamp: int
    transactions: List[MockTransaction]
    nonce: int
    number: int
    metadata: Dict = None
    epoch: int = 0
    difficulty: int = 0
    total_difficulty: int = 0

@dataclass
class MockAsset:
    id: str
    type: AssetType
    owner: str
    metadata: Dict
    created_at: int
    updated_at: int
    supply: Optional[int] = None
    decimals: Optional[int] = None
    reissuable: bool = False
    ipfs_hash: Optional[str] = None
    parent_asset: Optional[str] = None
    restricted: bool = False
    qualifier: Optional[str] = None

class MockWallet:
    def __init__(self):
        self.accounts: Dict[str, int] = {}  # address -> balance
        self.transactions: Dict[str, MockTransaction] = {}  # tx_hash -> transaction
        self.blocks: List[MockBlock] = []
        self.pending_transactions: List[MockTransaction] = []
        self.assets: Dict[str, MockAsset] = {}  # asset_id -> asset
        self._last_block_hash = "0" * 64
        self._block_number = 0
        self._kawpow_difficulty = 0x00000000ffff0000000000000000000000000000000000000000000000000000
        self._total_difficulty = 0
        self._epoch = 0
        self._cache = {}  # Simulated DAG cache
        self._performance_metrics = {
            "block_times": [],
            "difficulty_adjustments": [],
            "asset_operations": [],
            "transaction_times": []
        }

    def _calculate_epoch(self, block_number: int) -> int:
        """Calculate the current Kawpow epoch."""
        return block_number // KAWPOW_EPOCH_LENGTH

    def _adjust_difficulty(self, block: MockBlock) -> int:
        """Adjust difficulty based on block time."""
        if len(self.blocks) < KAWPOW_DIFFICULTY_ADJUSTMENT:
            return self._kawpow_difficulty

        target_time = 60  # 1 minute target
        actual_time = block.timestamp - self.blocks[-KAWPOW_DIFFICULTY_ADJUSTMENT].timestamp
        time_ratio = actual_time / (target_time * KAWPOW_DIFFICULTY_ADJUSTMENT)

        if time_ratio > 1.5:
            new_difficulty = int(self._kawpow_difficulty * 1.5)
        elif time_ratio < 0.5:
            new_difficulty = int(self._kawpow_difficulty * 0.5)
        else:
            new_difficulty = int(self._kawpow_difficulty * time_ratio)

        self._performance_metrics["difficulty_adjustments"].append({
            "block_number": block.number,
            "old_difficulty": self._kawpow_difficulty,
            "new_difficulty": new_difficulty,
            "time_ratio": time_ratio
        })

        return new_difficulty

    def _simulate_kawpow(self, block_data: str, nonce: int) -> Tuple[str, int]:
        """Simulate Kawpow mining algorithm."""
        start_time = time.time()
        
        # Simulate DAG access
        dag_index = (int(block_data, 16) + nonce) % KAWPOW_DAG_SIZE
        if dag_index not in self._cache:
            self._cache[dag_index] = hashlib.sha256(f"{dag_index}".encode()).hexdigest()
        
        # Simulate Kawpow mixing
        mix = self._cache[dag_index]
        for _ in range(64):  # Simulate 64 rounds of mixing
            mix = hashlib.sha256(f"{mix}{nonce}".encode()).hexdigest()
        
        # Calculate final hash
        final_hash = hashlib.sha256(f"{mix}{block_data}{nonce}".encode()).hexdigest()
        
        # Record performance
        self._performance_metrics["block_times"].append({
            "block_number": self._block_number + 1,
            "mining_time": time.time() - start_time,
            "difficulty": self._kawpow_difficulty
        })
        
        return final_hash, int(final_hash, 16)

    def create_account(self, address: str, initial_balance: int = 0) -> None:
        """Create a new account with the specified address and initial balance."""
        if address in self.accounts:
            raise ValueError(f"Account {address} already exists")
        self.accounts[address] = initial_balance

    def get_balance(self, address: str) -> int:
        """Get the balance of an account."""
        return self.accounts.get(address, 0)

    def create_transaction(self, from_address: str, to_address: str, value: int, 
                         data: str = "0x", asset_type: Optional[AssetType] = None,
                         asset_id: Optional[str] = None, asset_amount: Optional[int] = None,
                         asset_metadata: Optional[Dict] = None) -> MockTransaction:
        """Create a new transaction."""
        if from_address not in self.accounts:
            raise ValueError(f"Account {from_address} does not exist")
        if to_address not in self.accounts:
            raise ValueError(f"Account {to_address} does not exist")
        if self.accounts[from_address] < value:
            raise ValueError("Insufficient balance")

        # Create transaction hash
        tx_data = f"{from_address}{to_address}{value}{data}{int(time.time())}"
        tx_hash = hashlib.sha256(tx_data.encode()).hexdigest()

        # Create transaction
        transaction = MockTransaction(
            hash=tx_hash,
            from_address=from_address,
            to_address=to_address,
            value=value,
            data=data,
            timestamp=int(time.time()),
            status=TransactionStatus.PENDING,
            asset_type=asset_type,
            asset_id=asset_id,
            asset_amount=asset_amount,
            asset_metadata=asset_metadata
        )

        self.pending_transactions.append(transaction)
        
        # Record performance
        self._performance_metrics["transaction_times"].append({
            "timestamp": time.time(),
            "type": "create",
            "asset_type": asset_type.value if asset_type else None
        })
        
        return transaction

    def create_asset(self, owner: str, asset_type: AssetType, metadata: Dict,
                    supply: Optional[int] = None, decimals: Optional[int] = None,
                    reissuable: bool = False, ipfs_hash: Optional[str] = None,
                    parent_asset: Optional[str] = None, restricted: bool = False,
                    qualifier: Optional[str] = None) -> MockAsset:
        """Create a new asset."""
        if owner not in self.accounts:
            raise ValueError(f"Account {owner} does not exist")

        # Validate asset type and parameters
        if asset_type in [AssetType.SUB_TOKEN, AssetType.SUB_QUALIFIER_TOKEN] and not parent_asset:
            raise ValueError(f"Parent asset required for {asset_type.value}")
        if asset_type in [AssetType.RESTRICTED_TOKEN, AssetType.QUALIFIER_TOKEN, 
                         AssetType.SUB_QUALIFIER_TOKEN] and not restricted:
            raise ValueError(f"Restricted flag required for {asset_type.value}")

        # Generate asset ID
        asset_data = f"{owner}{asset_type.value}{json.dumps(metadata)}{int(time.time())}"
        asset_id = hashlib.sha256(asset_data.encode()).hexdigest()

        # Create asset
        asset = MockAsset(
            id=asset_id,
            type=asset_type,
            owner=owner,
            metadata=metadata,
            created_at=int(time.time()),
            updated_at=int(time.time()),
            supply=supply,
            decimals=decimals,
            reissuable=reissuable,
            ipfs_hash=ipfs_hash,
            parent_asset=parent_asset,
            restricted=restricted,
            qualifier=qualifier
        )

        self.assets[asset_id] = asset
        
        # Record performance
        self._performance_metrics["asset_operations"].append({
            "timestamp": time.time(),
            "operation": "create",
            "asset_type": asset_type.value,
            "restricted": restricted
        })
        
        return asset

    def reissue_asset(self, asset_id: str, owner: str, amount: int) -> None:
        """Reissue an existing asset."""
        if asset_id not in self.assets:
            raise ValueError(f"Asset {asset_id} does not exist")
        if owner != self.assets[asset_id].owner:
            raise ValueError("Only asset owner can reissue")
        if not self.assets[asset_id].reissuable:
            raise ValueError("Asset is not reissuable")

        self.assets[asset_id].supply += amount
        self.assets[asset_id].updated_at = int(time.time())
        
        # Record performance
        self._performance_metrics["asset_operations"].append({
            "timestamp": time.time(),
            "operation": "reissue",
            "asset_id": asset_id,
            "amount": amount
        })

    def transfer_asset(self, asset_id: str, from_address: str, to_address: str, amount: int) -> None:
        """Transfer an asset between addresses."""
        if asset_id not in self.assets:
            raise ValueError(f"Asset {asset_id} does not exist")
        if from_address != self.assets[asset_id].owner:
            raise ValueError("Only asset owner can transfer")
        if self.assets[asset_id].restricted:
            raise ValueError("Cannot transfer restricted asset")

        self.assets[asset_id].owner = to_address
        self.assets[asset_id].updated_at = int(time.time())
        
        # Record performance
        self._performance_metrics["asset_operations"].append({
            "timestamp": time.time(),
            "operation": "transfer",
            "asset_id": asset_id,
            "from": from_address,
            "to": to_address,
            "amount": amount
        })

    def mine_block(self) -> MockBlock:
        """Mine a new block with pending transactions using Kawpow algorithm."""
        if not self.pending_transactions:
            raise ValueError("No pending transactions to mine")

        start_time = time.time()
        
        # Calculate epoch
        epoch = self._calculate_epoch(self._block_number + 1)
        if epoch != self._epoch:
            self._epoch = epoch
            self._cache.clear()  # Clear DAG cache on epoch change

        # Create block
        block_data = f"{self._last_block_hash}{int(time.time())}"
        block_hash = "0" * 64
        nonce = 0
        
        # Mine block
        while True:
            block_hash, hash_value = self._simulate_kawpow(block_data, nonce)
            if hash_value < self._kawpow_difficulty:
                break
            nonce += 1

        # Create block
        block = MockBlock(
            hash=block_hash,
            previous_hash=self._last_block_hash,
            timestamp=int(time.time()),
            transactions=self.pending_transactions.copy(),
            nonce=nonce,
            number=self._block_number + 1,
            epoch=epoch,
            difficulty=self._kawpow_difficulty,
            total_difficulty=self._total_difficulty + self._kawpow_difficulty,
            metadata={
                "difficulty": self._kawpow_difficulty,
                "algorithm": "kawpow",
                "coin_type": SATOX_COIN_TYPE,
                "epoch": epoch,
                "dag_size": KAWPOW_DAG_SIZE,
                "cache_size": KAWPOW_CACHE_SIZE
            }
        )

        # Update transaction statuses and balances
        for tx in self.pending_transactions:
            tx.status = TransactionStatus.CONFIRMED
            tx.block_hash = block_hash
            tx.block_number = block.number
            
            # Update balances
            self.accounts[tx.from_address] -= tx.value
            self.accounts[tx.to_address] += tx.value

            # Update asset ownership if applicable
            if tx.asset_type and tx.asset_id:
                if tx.asset_id in self.assets:
                    self.assets[tx.asset_id].owner = tx.to_address
                    self.assets[tx.asset_id].updated_at = int(time.time())

            self.transactions[tx.hash] = tx

        # Update state
        self.blocks.append(block)
        self._last_block_hash = block_hash
        self._block_number = block.number
        self._total_difficulty += self._kawpow_difficulty
        self._kawpow_difficulty = self._adjust_difficulty(block)
        self.pending_transactions.clear()

        return block

    def get_performance_metrics(self) -> Dict:
        """Get performance metrics."""
        return self._performance_metrics

    def get_transaction(self, tx_hash: str) -> Optional[MockTransaction]:
        """Get a transaction by its hash."""
        return self.transactions.get(tx_hash)

    def get_block_by_hash(self, block_hash: str) -> Optional[MockBlock]:
        """Get a block by its hash."""
        for block in self.blocks:
            if block.hash == block_hash:
                return block
        return None

    def get_block_by_number(self, block_number: int) -> Optional[MockBlock]:
        """Get a block by its number."""
        for block in self.blocks:
            if block.number == block_number:
                return block
        return None

    def get_latest_block(self) -> Optional[MockBlock]:
        """Get the latest block."""
        return self.blocks[-1] if self.blocks else None

    def get_pending_transactions(self) -> List[MockTransaction]:
        """Get all pending transactions."""
        return self.pending_transactions.copy()

    def cancel_transaction(self, tx_hash: str) -> bool:
        """Cancel a pending transaction."""
        for i, tx in enumerate(self.pending_transactions):
            if tx.hash == tx_hash:
                tx.status = TransactionStatus.CANCELLED
                self.pending_transactions.pop(i)
                return True
        return False

    def get_asset(self, asset_id: str) -> Optional[MockAsset]:
        """Get an asset by its ID."""
        return self.assets.get(asset_id)

    def set_mining_difficulty(self, difficulty: int) -> None:
        """Set the mining difficulty."""
        if difficulty < 1:
            raise ValueError("Difficulty must be at least 1")
        self._kawpow_difficulty = difficulty

    def get_mining_difficulty(self) -> int:
        """Get the current mining difficulty."""
        return self._kawpow_difficulty

    def get_block_count(self) -> int:
        """Get the total number of blocks."""
        return len(self.blocks)

    def get_transaction_count(self) -> int:
        """Get the total number of transactions."""
        return len(self.transactions)

    def get_account_count(self) -> int:
        """Get the total number of accounts."""
        return len(self.accounts)

    def get_asset_count(self) -> int:
        """Get the total number of assets."""
        return len(self.assets)

    def reset(self) -> None:
        """Reset the wallet state."""
        self.accounts.clear()
        self.transactions.clear()
        self.blocks.clear()
        self.pending_transactions.clear()
        self.assets.clear()
        self._last_block_hash = "0" * 64
        self._block_number = 0
        self._total_difficulty = 0
        self._epoch = 0
        self._cache.clear()
        self._performance_metrics = {
            "block_times": [],
            "difficulty_adjustments": [],
            "asset_operations": [],
            "transaction_times": []
        } 