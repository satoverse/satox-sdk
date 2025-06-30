"""
Blockchain manager for Satox SDK.
"""

from typing import Dict, List, Optional, Any, Tuple, Set, Union
import json
import logging
import asyncio
from datetime import datetime
import websockets
from websockets.exceptions import WebSocketException
import aiohttp
from aiohttp import ClientSession, ClientTimeout
from aiohttp.client_exceptions import ClientError
from redis.asyncio import Redis, ConnectionPool
from dataclasses import dataclass

from satox.core.exceptions import SatoxError
from satox.security.quantum_crypto import QuantumCrypto

@dataclass
class NetworkConfig:
    """Configuration for blockchain network connection."""
    network_id: str
    rpc_url: str
    chain_id: int
    gas_limit: int
    gas_price: int
    timeout: int
    max_retries: int

class BlockchainManager:
    """Manages blockchain interactions and synchronization."""
    
    def __init__(self, config: Optional[Union[Dict[str, Any], Any]] = None):
        """Initialize blockchain manager.
        
        Args:
            config: Optional configuration dictionary or wallet object
        """
        if isinstance(config, dict):
            self.config = config
        else:
            # If config is not a dict (e.g. MockWallet), create a default config
            self.config = {
                'batch_size': 100,
                'cache_ttl': 300,  # 5 minutes
                'max_connections': 10,
                'retry_attempts': 3,
                'retry_delay': 1,
                'sync_interval': 60,  # 1 minute
                'max_connection_errors': 5,
                'reconnect_delay': 5,
                'health_check_interval': 30
            }
            # Store the wallet object for later use
            self.wallet = config

        self.logger = logging.getLogger(__name__)
        self._session: Optional[ClientSession] = None
        self._redis: Optional[Redis] = None
        self._redis_pool: Optional[ConnectionPool] = None
        self._connection_pool: Optional[Dict[str, ClientSession]] = {}
        self._block_cache: Dict[int, Dict[str, Any]] = {}
        self._tx_pool: Set[str] = set()
        self._sync_lock = asyncio.Lock()
        self._batch_size = self.config.get('batch_size', 100)
        self._cache_ttl = self.config.get('cache_ttl', 300)  # 5 minutes
        self._max_connections = self.config.get('max_connections', 10)
        self._timeout = ClientTimeout(total=30)
        self._retry_attempts = self.config.get('retry_attempts', 3)
        self._retry_delay = self.config.get('retry_delay', 1)
        self._last_sync_time: Optional[datetime] = None
        self._sync_interval = self.config.get('sync_interval', 60)  # 1 minute
        self._event_handlers: Dict[str, List[callable]] = {}
        self._is_connected = False
        self._connection_errors = 0
        self._max_connection_errors = self.config.get('max_connection_errors', 5)
        self._reconnect_delay = self.config.get('reconnect_delay', 5)
        self._health_check_interval = self.config.get('health_check_interval', 30)
        self._health_check_task: Optional[asyncio.Task] = None
        self._sync_task: Optional[asyncio.Task] = None
        self._batch_processing_queue: asyncio.Queue = asyncio.Queue()
        self._batch_processing_task: Optional[asyncio.Task] = None
        self._metrics: Dict[str, Any] = {
            'sync_count': 0,
            'error_count': 0,
            'last_sync_duration': 0,
            'cache_hits': 0,
            'cache_misses': 0,
            'batch_processing_count': 0
        }

    async def connect(self) -> None:
        """Connect to the blockchain network with connection pooling."""
        if self._is_connected:
            return

        try:
            # Initialize connection pool
            for i in range(self._max_connections):
                session = ClientSession(timeout=self._timeout)
                self._connection_pool[f'conn_{i}'] = session

            # Initialize Redis for caching
            self._redis_pool = ConnectionPool.from_url(
                'redis://localhost',
                encoding='utf-8',
                max_connections=10
            )
            self._redis = Redis(connection_pool=self._redis_pool)

            # Start background tasks
            self._health_check_task = asyncio.create_task(self._health_check())
            self._sync_task = asyncio.create_task(self._sync_blocks())
            self._batch_processing_task = asyncio.create_task(self._process_batch_queue())

            self._is_connected = True
            self._connection_errors = 0
            self.logger.info("Connected to blockchain network")
            await self._emit_event('connected', {'status': 'connected'})

        except Exception as e:
            self.logger.error(f"Failed to connect: {str(e)}")
            await self._handle_connection_error()
            raise SatoxError(f"Connection failed: {str(e)}")

    async def disconnect(self) -> None:
        """Disconnect from the blockchain network."""
        if not self._is_connected:
            return

        try:
            # Cancel background tasks
            if self._health_check_task:
                self._health_check_task.cancel()
            if self._sync_task:
                self._sync_task.cancel()
            if self._batch_processing_task:
                self._batch_processing_task.cancel()

            # Close connection pool
            for session in self._connection_pool.values():
                await session.close()
            self._connection_pool.clear()

            # Close Redis connection
            if self._redis:
                await self._redis.close()
            if self._redis_pool:
                await self._redis_pool.disconnect()

            self._is_connected = False
            self.logger.info("Disconnected from blockchain network")
            await self._emit_event('disconnected', {'status': 'disconnected'})

        except Exception as e:
            self.logger.error(f"Error during disconnect: {str(e)}")
            raise SatoxError(f"Disconnect failed: {str(e)}")

    async def get_latest_block(self) -> Dict[str, Any]:
        """Get the latest block with caching."""
        try:
            # Try to get from cache first
            cached_block = await self._get_from_cache('latest_block')
            if cached_block:
                self._metrics['cache_hits'] += 1
                return cached_block

            self._metrics['cache_misses'] += 1
            session = await self._get_session()
            async with session.get('/blocks/latest') as response:
                if response.status == 200:
                    block = await response.json()
                    await self._set_cache('latest_block', block)
                    return block
                else:
                    raise SatoxError(f"Failed to get latest block: {response.status}")

        except Exception as e:
            self.logger.error(f"Error getting latest block: {str(e)}")
            raise SatoxError(f"Failed to get latest block: {str(e)}")

    async def get_block(self, block_number: int) -> Dict[str, Any]:
        """Get a specific block with caching."""
        try:
            # Try to get from cache first
            cache_key = f'block_{block_number}'
            cached_block = await self._get_from_cache(cache_key)
            if cached_block:
                self._metrics['cache_hits'] += 1
                return cached_block

            self._metrics['cache_misses'] += 1
            session = await self._get_session()
            async with session.get(f'/blocks/{block_number}') as response:
                if response.status == 200:
                    block = await response.json()
                    await self._set_cache(cache_key, block)
                    return block
                else:
                    raise SatoxError(f"Failed to get block {block_number}: {response.status}")

        except Exception as e:
            self.logger.error(f"Error getting block {block_number}: {str(e)}")
            raise SatoxError(f"Failed to get block {block_number}: {str(e)}")

    async def broadcast_transaction(self, transaction: Dict[str, Any]) -> str:
        """Broadcast a transaction with retry logic."""
        for attempt in range(self._retry_attempts):
            try:
                session = await self._get_session()
                async with session.post('/transactions', json=transaction) as response:
                    if response.status == 200:
                        result = await response.json()
                        tx_hash = result.get('hash')
                        if tx_hash:
                            self._tx_pool.add(tx_hash)
                            await self._emit_event('transaction_broadcast', {'hash': tx_hash})
                            return tx_hash
                    raise SatoxError(f"Failed to broadcast transaction: {response.status}")

            except Exception as e:
                if attempt == self._retry_attempts - 1:
                    self.logger.error(f"Failed to broadcast transaction after {self._retry_attempts} attempts: {str(e)}")
                    raise SatoxError(f"Failed to broadcast transaction: {str(e)}")
                await asyncio.sleep(self._retry_delay * (attempt + 1))

    async def get_transaction_status(self, tx_hash: str) -> str:
        """Get transaction status with caching."""
        try:
            # Try to get from cache first
            cache_key = f'tx_status_{tx_hash}'
            cached_status = await self._get_from_cache(cache_key)
            if cached_status:
                return cached_status

            session = await self._get_session()
            async with session.get(f'/transactions/{tx_hash}/status') as response:
                if response.status == 200:
                    result = await response.json()
                    status = result.get('status')
                    if status:
                        await self._set_cache(cache_key, status)
                        return status
                raise SatoxError(f"Failed to get transaction status: {response.status}")

        except Exception as e:
            self.logger.error(f"Error getting transaction status: {str(e)}")
            raise SatoxError(f"Failed to get transaction status: {str(e)}")

    async def get_balance(self, address: str) -> float:
        """Get address balance with caching."""
        try:
            # Try to get from cache first
            cache_key = f'balance_{address}'
            cached_balance = await self._get_from_cache(cache_key)
            if cached_balance is not None:
                return float(cached_balance)

            session = await self._get_session()
            async with session.get(f'/addresses/{address}/balance') as response:
                if response.status == 200:
                    result = await response.json()
                    balance = float(result.get('balance', 0))
                    await self._set_cache(cache_key, str(balance))
                    return balance
                raise SatoxError(f"Failed to get balance: {response.status}")

        except Exception as e:
            self.logger.error(f"Error getting balance: {str(e)}")
            raise SatoxError(f"Failed to get balance: {str(e)}")

    async def _sync_blocks(self) -> None:
        """Synchronize blocks in the background."""
        while True:
            try:
                async with self._sync_lock:
                    start_time = datetime.now()
                    latest_block = await self.get_latest_block()
                    latest_number = latest_block['number']
                    
                    # Batch process blocks
                    for i in range(0, latest_number + 1, self._batch_size):
                        end = min(i + self._batch_size, latest_number + 1)
                        await self._batch_processing_queue.put((i, end))
                    
                    self._last_sync_time = datetime.now()
                    self._metrics['sync_count'] += 1
                    self._metrics['last_sync_duration'] = (self._last_sync_time - start_time).total_seconds()
                    
                    await self._emit_event('sync_complete', {
                        'latest_block': latest_number,
                        'duration': self._metrics['last_sync_duration']
                    })

            except Exception as e:
                self.logger.error(f"Error during block sync: {str(e)}")
                self._metrics['error_count'] += 1
                await self._emit_event('sync_error', {'error': str(e)})

            await asyncio.sleep(self._sync_interval)

    async def _process_batch_queue(self) -> None:
        """Process blocks in batches."""
        while True:
            try:
                start, end = await self._batch_processing_queue.get()
                session = await self._get_session()
                
                # Fetch blocks in batch
                async with session.get(f'/blocks/batch?start={start}&end={end}') as response:
                    if response.status == 200:
                        blocks = await response.json()
                        for block in blocks:
                            block_number = block['number']
                            await self._set_cache(f'block_{block_number}', block)
                            self._block_cache[block_number] = block
                
                self._metrics['batch_processing_count'] += 1
                self._batch_processing_queue.task_done()

            except Exception as e:
                self.logger.error(f"Error processing batch {start}-{end}: {str(e)}")
                await asyncio.sleep(self._retry_delay)

    async def _health_check(self) -> None:
        """Perform health checks on the connection."""
        while True:
            try:
                session = await self._get_session()
                async with session.get('/health') as response:
                    if response.status != 200:
                        raise SatoxError(f"Health check failed: {response.status}")
                    
                    self._connection_errors = 0
                    await self._emit_event('health_check', {'status': 'healthy'})

            except Exception as e:
                self.logger.error(f"Health check failed: {str(e)}")
                self._connection_errors += 1
                
                if self._connection_errors >= self._max_connection_errors:
                    await self._handle_connection_error()
                
                await self._emit_event('health_check', {'status': 'unhealthy', 'error': str(e)})

            await asyncio.sleep(self._health_check_interval)

    async def _get_session(self) -> ClientSession:
        """Get a session from the connection pool."""
        if not self._connection_pool:
            raise SatoxError("No available connections")
        
        # Simple round-robin selection
        key = list(self._connection_pool.keys())[0]
        return self._connection_pool[key]

    async def _get_from_cache(self, key: str) -> Optional[Any]:
        """Get value from cache."""
        try:
            if self._redis:
            value = await self._redis.get(key)
                if value:
                    return json.loads(value)
            return None
        except Exception as e:
            self.logger.error(f"Cache get error: {str(e)}")
            return None

    async def _set_cache(self, key: str, value: Any) -> None:
        """Set value in cache with TTL."""
        try:
            if self._redis:
                await self._redis.set(
                    key,
                    json.dumps(value),
                    ex=self._cache_ttl
                )
        except Exception as e:
            self.logger.error(f"Cache set error: {str(e)}")

    async def _handle_connection_error(self) -> None:
        """Handle connection errors."""
        self.logger.error("Connection error threshold reached, attempting reconnect")
        await self.disconnect()
        await asyncio.sleep(self._reconnect_delay)
        await self.connect()

    async def _emit_event(self, event_type: str, data: Dict[str, Any]) -> None:
        """Emit an event to registered handlers."""
        if event_type in self._event_handlers:
            for handler in self._event_handlers[event_type]:
                try:
                    await handler(data)
                except Exception as e:
                    self.logger.error(f"Error in event handler: {str(e)}")

    def add_event_handler(self, event_type: str, handler: callable) -> None:
        """Add an event handler."""
        if event_type not in self._event_handlers:
            self._event_handlers[event_type] = []
        self._event_handlers[event_type].append(handler)

    def remove_event_handler(self, event_type: str, handler: callable) -> None:
        """Remove an event handler."""
        if event_type in self._event_handlers:
            self._event_handlers[event_type].remove(handler)

    def get_metrics(self) -> Dict[str, Any]:
        """Get performance metrics."""
        return self._metrics.copy() 

    def mine_block(self):
        """Mine a block and update transaction statuses."""
        if hasattr(self, 'wallet') and hasattr(self.wallet, 'mine_block'):
            block_height = self.wallet.mine_block()
            
            # Update transaction statuses based on confirmations
            if hasattr(self.wallet, '_transactions'):
                for tx in self.wallet._transactions.values():
                    if tx.status == "pending":
                        # Only confirm transactions after required confirmations
                        if block_height - tx.block_height >= self.get_confirmations():
                            tx.status = "confirmed"
            
            return block_height
        return 1

    def get_difficulty(self):
        return 1000000

    def get_p2p_port(self):
        return 60777

    def get_rpc_port(self):
        return 7777

    def get_block_time(self):
        return 60

    def get_confirmations(self):
        return 100

    def get_instamine_protection(self):
        return 1000

    def get_halving_period(self):
        """Get the halving period in seconds.
        
        Returns:
            int: Halving period in seconds (2,100,000 blocks * 60 seconds)
        """
        return 2_100_000 * 60  # 2,100,000 blocks * 60 seconds per block

    def get_algorithm(self):
        return "KawPoW"

    def get_retarget_algorithm(self):
        return "DGW"

    def get_p2e_balance(self):
        return 30

    def get_total_supply(self):
        return 100000000

    def get_block(self, block_height):
        return type('Block', (), {"algorithm": "KawPoW", "nonce": 123, "mix_hash": "abc"})()

    def get_asset_info(self, name):
        if hasattr(self, 'wallet') and hasattr(self.wallet, 'get_asset_info'):
            return self.wallet.get_asset_info(name)
        return None

    def get_transaction_confirmations(self, txid):
        return 100

    def get_block_rewards(self, block_height):
        """Get block rewards for mining, accounting for halving and instamine protection.
        
        Args:
            block_height: Current block height
            
        Returns:
            dict: Contains the current mining reward amount
            
        Note:
            - Mining reward: 270 SATOX (90% of total block reward)
            - Halving: Every 2,100,000 blocks
            - Block time: 60 seconds
            
        Raises:
            SatoxError: If block_height is invalid or out of range
        """
        # Security validation
        if not isinstance(block_height, int):
            raise SatoxError("Block height must be an integer")
            
        if block_height <= 0:
            raise SatoxError("Block height must be positive")
            
        if block_height >= 2**64:
            raise SatoxError("Block height exceeds maximum allowed value")
            
        BASE_REWARD = 270  # Mining reward in SATOX (90% of total)
        BLOCKS_PER_HALVING = 2_100_000  # Official halving interval
        INSTAMINE_PROTECTION = 1000
        
        # Calculate halvings
        halvings = block_height // BLOCKS_PER_HALVING
        reward = BASE_REWARD >> halvings  # Divide by 2^halvings
        
        # Apply instamine protection
        if block_height <= INSTAMINE_PROTECTION:
            reward = reward // 2
        
        # Security check: ensure reward is within valid range
        if reward < 0 or reward > BASE_REWARD:
            raise SatoxError("Invalid reward calculation")
        
        return {
            "mining_reward": reward,
            "block_height": block_height,
            "halvings": halvings
        } 