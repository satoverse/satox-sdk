from dataclasses import dataclass
from typing import Dict, List, Optional, Any
import time

@dataclass
class TransactionConfig:
    gas_limit: int
    gas_price: int
    nonce: int
    value: int
    data: str
    timeout: int
    max_retries: int

@dataclass
class Transaction:
    hash: str
    from_address: str
    to_address: str
    value: int
    data: str
    gas_limit: int
    gas_price: int
    status: str
    timestamp: int

@dataclass
class TransactionResult:
    hash: str
    status: str
    block_hash: Optional[str] = None
    block_num: Optional[int] = None
    gas_used: Optional[int] = None
    timestamp: int = 0
    metadata: Optional[Dict[str, Any]] = None

class TransactionManager:
    def __init__(self):
        self._config: Optional[TransactionConfig] = None
        self._pending_transactions: Dict[str, Transaction] = {}
        self._last_error: Optional[str] = None

    def initialize(self, config: TransactionConfig) -> None:
        self._config = config

    def create_transaction(self, from_address: str, to_address: str, value: int, 
                         data: str = "0x", gas_limit: Optional[int] = None,
                         gas_price: Optional[int] = None, config: Optional[TransactionConfig] = None) -> Transaction:
        if not self._config and not config:
            raise RuntimeError("Transaction configuration not set")
        if value < 0:
            raise ValueError("Transaction value cannot be negative")
        if not from_address or not to_address:
            raise ValueError("Invalid address")

        tx_hash = f"0x{hash(f'{from_address}{to_address}{value}{data}{time.time()}')}"
        tx = Transaction(
            hash=tx_hash,
            from_address=from_address,
            to_address=to_address,
            value=value,
            data=data,
            gas_limit=gas_limit or (config or self._config).gas_limit,
            gas_price=gas_price or (config or self._config).gas_price,
            status="pending",
            timestamp=int(time.time())
        )
        self._pending_transactions[tx_hash] = tx
        return tx

    def send_transaction(self, transaction: Transaction) -> TransactionResult:
        if not self._config:
            raise RuntimeError("Transaction configuration not set")
        
        tx_hash = transaction.hash
        if tx_hash not in self._pending_transactions:
            raise ValueError(f"Transaction {tx_hash} not found")

        result = TransactionResult(
            hash=tx_hash,
            status="confirmed",
            block_hash="0x" + "0" * 64,
            block_num=1,
            gas_used=transaction.gas_limit,
            timestamp=int(time.time())
        )
        
        del self._pending_transactions[tx_hash]
        return result

    def get_transaction_status(self, tx_hash: str) -> str:
        if tx_hash in self._pending_transactions:
            return self._pending_transactions[tx_hash].status
        return "failed"

    def cancel_transaction(self, tx_hash: str) -> bool:
        if tx_hash not in self._pending_transactions:
            raise ValueError(f"Transaction {tx_hash} not found")
        
        self._pending_transactions[tx_hash].status = "cancelled"
        return True

    def get_pending_transactions(self) -> List[Transaction]:
        return list(self._pending_transactions.values())

    def get_pending_transaction_count(self) -> int:
        return len(self._pending_transactions)

    def get_config(self) -> Optional[TransactionConfig]:
        return self._config

    def update_transaction_config(self, config: TransactionConfig) -> None:
        if config.gas_limit <= 0 or config.gas_price <= 0 or config.nonce < 0 or config.value < 0:
            raise ValueError("Invalid transaction configuration")
        self._config = config

    def get_last_error(self) -> Optional[str]:
        return self._last_error

    def send_transaction_batch(self, transactions: List[Transaction]) -> List[TransactionResult]:
        results = []
        for tx in transactions:
            try:
                result = self.send_transaction(tx)
                results.append(result)
            except Exception as e:
                self._last_error = str(e)
                results.append(TransactionResult(
                    hash=tx.hash,
                    status="failed",
                    timestamp=int(time.time())
                ))
        return results 