from dataclasses import dataclass
from typing import Dict, List, Optional, Any
from abc import ABC, abstractmethod
from satox.wallet.wallet_manager import WalletManager

@dataclass
class SatoxConfig:
    network_id: str
    api_endpoint: str
    api_key: str = "test_api_key"
    debug_mode: bool = False
    max_retries: int = 3
    timeout: int = 30
    custom_config: Dict[str, Any] = None

class IComponent(ABC):
    @abstractmethod
    def initialize(self) -> bool:
        pass

    @abstractmethod
    def shutdown(self) -> bool:
        pass

    @abstractmethod
    def get_name(self) -> str:
        pass

class SatoxManager:
    def __init__(self):
        self._components: Dict[str, IComponent] = {}
        self._config: Optional[SatoxConfig] = None
        self._initialized: bool = False
        self.wallet_manager = WalletManager()

    def initialize(self, config: SatoxConfig) -> None:
        if not config.network_id or not config.api_endpoint:
            raise ValueError("Invalid configuration")
        self._config = config
        self._initialized = True

    def register_component(self, component: IComponent) -> None:
        if not self._initialized:
            raise RuntimeError("SatoxManager not initialized")
        if component.get_name() in self._components:
            raise ValueError(f"Component {component.get_name()} already registered")
        self._components[component.get_name()] = component

    def unregister_component(self, component_name: str) -> None:
        if not self._initialized:
            raise RuntimeError("SatoxManager not initialized")
        if component_name not in self._components:
            raise ValueError(f"Component {component_name} not found")
        del self._components[component_name]

    def initialize_components(self) -> None:
        if not self._initialized:
            raise RuntimeError("SatoxManager not initialized")
        for component in self._components.values():
            try:
                if not component.initialize():
                    raise RuntimeError(f"Failed to initialize component {component.get_name()}")
            except Exception as e:
                raise RuntimeError(f"Error initializing component {component.get_name()}: {str(e)}")

    def shutdown_components(self) -> None:
        if not self._initialized:
            raise RuntimeError("SatoxManager not initialized")
        for component in self._components.values():
            try:
                if not component.shutdown():
                    raise RuntimeError(f"Failed to shutdown component {component.get_name()}")
            except Exception as e:
                raise RuntimeError(f"Error shutting down component {component.get_name()}: {str(e)}")

    def get_component_count(self) -> int:
        return len(self._components)

    def get_component_names(self) -> List[str]:
        return list(self._components.keys())

    def get_config(self) -> Optional[SatoxConfig]:
        return self._config

    def update_config(self, config: SatoxConfig) -> None:
        if not self._initialized:
            raise RuntimeError("SatoxManager not initialized")
        self._config = config 

    def create_wallet(self, *args, **kwargs):
        return self.wallet_manager.create_wallet(*args, **kwargs) 