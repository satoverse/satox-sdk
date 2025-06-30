"""Mock Wallet implementation for development and testing."""

import uuid
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
from datetime import datetime
from satox_bindings.core.error_handling import SatoxError

@dataclass
class MockAccount:
    """Mock account representation."""
    id: str
    address: str
    balance: float
    created_at: datetime
    updated_at: datetime

@dataclass
class MockTransaction:
    """Mock transaction representation."""
    id: str
    sender: str
    receiver: str
    amount: float
    fee: float
    status: str
    created_at: datetime
    updated_at: datetime
    data: Optional[Dict] = None

class MockWallet:
    """Mock wallet for development and testing."""

    def __init__(self):
        """Initialize the mock wallet."""
        self.accounts: Dict[str, MockAccount] = {}
        self.transactions: Dict[str, MockTransaction] = {}
        self.transaction_history: Dict[str, List[str]] = {}  # account_id -> [transaction_ids]

    def create_account(self, initial_balance: float = 0.0) -> MockAccount:
        """Create a new mock account.
        
        Args:
            initial_balance: Initial balance for the account
            
        Returns:
            MockAccount: Created account
        """
        account_id = str(uuid.uuid4())
        address = f"mock_{account_id[:8]}"
        now = datetime.utcnow()
        
        account = MockAccount(
            id=account_id,
            address=address,
            balance=initial_balance,
            created_at=now,
            updated_at=now
        )
        
        self.accounts[account_id] = account
        self.transaction_history[account_id] = []
        return account

    def get_account(self, account_id: str) -> MockAccount:
        """Get account by ID.
        
        Args:
            account_id: Account identifier
            
        Returns:
            MockAccount: Account information
            
        Raises:
            SatoxError: If account not found
        """
        if account_id not in self.accounts:
            raise SatoxError(f"Account not found: {account_id}")
        return self.accounts[account_id]

    def get_account_by_address(self, address: str) -> MockAccount:
        """Get account by address.
        
        Args:
            address: Account address
            
        Returns:
            MockAccount: Account information
            
        Raises:
            SatoxError: If account not found
        """
        for account in self.accounts.values():
            if account.address == address:
                return account
        raise SatoxError(f"Account not found with address: {address}")

    def fund_account(self, account_id: str, amount: float) -> MockAccount:
        """Fund an account with mock coins.
        
        Args:
            account_id: Account identifier
            amount: Amount to fund
            
        Returns:
            MockAccount: Updated account
            
        Raises:
            SatoxError: If account not found or amount is negative
        """
        if amount < 0:
            raise SatoxError("Amount must be positive")
            
        account = self.get_account(account_id)
        account.balance += amount
        account.updated_at = datetime.utcnow()
        return account

    def get_balance(self, account_id: str) -> float:
        """Get account balance.
        
        Args:
            account_id: Account identifier
            
        Returns:
            float: Account balance
            
        Raises:
            SatoxError: If account not found
        """
        account = self.get_account(account_id)
        return account.balance

    def create_transaction(
        self,
        sender_id: str,
        receiver_id: str,
        amount: float,
        fee: float = 0.0,
        data: Optional[Dict] = None
    ) -> MockTransaction:
        """Create a new transaction.
        
        Args:
            sender_id: Sender account identifier
            receiver_id: Receiver account identifier
            amount: Transaction amount
            fee: Transaction fee
            data: Additional transaction data
            
        Returns:
            MockTransaction: Created transaction
            
        Raises:
            SatoxError: If accounts not found or insufficient balance
        """
        sender = self.get_account(sender_id)
        receiver = self.get_account(receiver_id)
        
        if sender.balance < (amount + fee):
            raise SatoxError("Insufficient balance")
            
        now = datetime.utcnow()
        tx_id = str(uuid.uuid4())
        
        transaction = MockTransaction(
            id=tx_id,
            sender=sender.address,
            receiver=receiver.address,
            amount=amount,
            fee=fee,
            status="pending",
            created_at=now,
            updated_at=now,
            data=data
        )
        
        self.transactions[tx_id] = transaction
        return transaction

    def send_transaction(self, transaction_id: str) -> MockTransaction:
        """Send a transaction.
        
        Args:
            transaction_id: Transaction identifier
            
        Returns:
            MockTransaction: Updated transaction
            
        Raises:
            SatoxError: If transaction not found or invalid
        """
        if transaction_id not in self.transactions:
            raise SatoxError(f"Transaction not found: {transaction_id}")
            
        transaction = self.transactions[transaction_id]
        sender = self.get_account_by_address(transaction.sender)
        receiver = self.get_account_by_address(transaction.receiver)
        
        if sender.balance < (transaction.amount + transaction.fee):
            raise SatoxError("Insufficient balance")
            
        # Update balances
        sender.balance -= (transaction.amount + transaction.fee)
        receiver.balance += transaction.amount
        
        # Update transaction
        transaction.status = "completed"
        transaction.updated_at = datetime.utcnow()
        
        # Update transaction history
        self.transaction_history[sender.id].append(transaction_id)
        self.transaction_history[receiver.id].append(transaction_id)
        
        return transaction

    def get_transaction(self, transaction_id: str) -> MockTransaction:
        """Get transaction by ID.
        
        Args:
            transaction_id: Transaction identifier
            
        Returns:
            MockTransaction: Transaction information
            
        Raises:
            SatoxError: If transaction not found
        """
        if transaction_id not in self.transactions:
            raise SatoxError(f"Transaction not found: {transaction_id}")
        return self.transactions[transaction_id]

    def get_account_transactions(
        self,
        account_id: str,
        limit: int = 10,
        offset: int = 0
    ) -> List[MockTransaction]:
        """Get account transaction history.
        
        Args:
            account_id: Account identifier
            limit: Maximum number of transactions to return
            offset: Number of transactions to skip
            
        Returns:
            List[MockTransaction]: List of transactions
            
        Raises:
            SatoxError: If account not found
        """
        if account_id not in self.transaction_history:
            raise SatoxError(f"Account not found: {account_id}")
            
        transaction_ids = self.transaction_history[account_id]
        start = offset
        end = offset + limit
        
        return [
            self.transactions[tx_id]
            for tx_id in transaction_ids[start:end]
        ]

    def reset(self) -> None:
        """Reset the mock wallet state."""
        self.accounts.clear()
        self.transactions.clear()
        self.transaction_history.clear()
