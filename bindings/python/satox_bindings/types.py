"""
Type definitions for Satox SDK
"""

from typing import Dict, List, Optional, Any
from enum import Enum
from dataclasses import dataclass
from datetime import datetime

class DatabaseType(Enum):
    SQLITE = "sqlite"
    POSTGRESQL = "postgresql"
    MYSQL = "mysql"
    REDIS = "redis"
    MONGODB = "mongodb"
    SUPABASE = "supabase"
    FIREBASE = "firebase"
    AWS = "aws"
    AZURE = "azure"
    GOOGLE_CLOUD = "google_cloud"

class AssetType(Enum):
    TOKEN = "token"
    NFT = "nft"
    CURRENCY = "currency"

class NetworkType(Enum):
    MAINNET = "mainnet"
    TESTNET = "testnet"
    REGTEST = "regtest"

@dataclass
class CoreConfig:
    name: str = "satox_sdk"
    enable_logging: bool = True
    log_level: str = "info"
    log_path: Optional[str] = None
    max_threads: int = 4
    timeout: int = 30

@dataclass
class CoreStatus:
    initialized: bool
    version: str
    uptime: float
    memory_usage: Dict[str, Any]
    thread_count: int

@dataclass
class DatabaseConfig:
    type: DatabaseType
    host: Optional[str] = None
    port: Optional[int] = None
    database: Optional[str] = None
    username: Optional[str] = None
    password: Optional[str] = None
    connection_string: Optional[str] = None
    max_connections: int = 10
    timeout: int = 30

@dataclass
class QueryResult:
    success: bool
    rows: List[Dict[str, Any]]
    error: str = ""
    affected_rows: int = 0
    last_insert_id: Optional[int] = None

@dataclass
class BlockInfo:
    hash: str
    height: int
    timestamp: int
    transactions: List[str]
    difficulty: float

@dataclass
class TransactionInfo:
    txid: str
    block_hash: str
    confirmations: int
    amount: float
    fee: float

@dataclass
class Asset:
    id: str
    name: str
    symbol: str
    type: AssetType
    decimals: int
    total_supply: int
    metadata: Dict[str, Any]

@dataclass
class NFT:
    id: str
    name: str
    description: str
    image_url: str
    metadata: Dict[str, Any]
    owner: str
    created_at: datetime

@dataclass
class Wallet:
    address: str
    balance: Dict[str, float]
    assets: List[Asset]
    nfts: List[NFT]
    created_at: datetime
    last_updated: datetime 