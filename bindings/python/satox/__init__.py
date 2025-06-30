"""
Satox SDK Python Bindings
"""

from satox_bindings.core import (
    BlockProcessor,
    TransactionProcessor,
    NetworkManager,
    SecurityManager,
    ConfigManager
)

from satox.quantum import (
    QuantumManager,
    HybridEncryption,
    KeyStorage,
    PostQuantumAlgorithms
)

# satox.game is for future satox-game-sdk integration
try:
    from satox.game import (
        GameManager,
        GameAsset,
        GameWallet,
        GameTransaction,
        GameNetwork,
        GameSecurity,
        GameIPFS,
        GameNFT,
        GameAPI,
        GameDatabase,
        GameCore,
        GamePlugin,
        GameEvent,
        GameConfig,
        GameCache,
        GamePerformance,
        GameLogging,
    )
except ImportError:
    pass

__version__ = "1.0.0"
__all__ = [
    # Core components
    "BlockProcessor",
    "TransactionProcessor",
    "NetworkManager",
    "SecurityManager",
    "ConfigManager",
    
    # Quantum components
    "QuantumManager",
    "HybridEncryption",
    "KeyStorage",
    "PostQuantumAlgorithms",
    
    # Game components
    "GameManager",
    "AssetManager",
    "PlayerManager",
    "WorldManager"
] 