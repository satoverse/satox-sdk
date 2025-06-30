import pytest
from satox.core.satox_manager import SatoxManager, IComponent, SatoxConfig

class MockComponent(IComponent):
    def __init__(self, name):
        self.name = name
        self.initialized = False
        self.shutdown = False

    def initialize(self) -> bool:
        self.initialized = True
        return True

    def shutdown(self) -> bool:
        self.shutdown = True
        return True

    def get_name(self) -> str:
        return self.name

def test_satox_manager_initialization(satox_manager):
    assert satox_manager is not None
    assert satox_manager.get_config() is not None
    assert satox_manager.get_component_count() == 0

def test_component_registration(satox_manager):
    component = MockComponent("test")
    satox_manager.register_component(component)
    assert satox_manager.get_component_count() == 1
    assert "test" in satox_manager.get_component_names()

def test_component_unregistration(satox_manager):
    component = MockComponent("test")
    satox_manager.register_component(component)
    satox_manager.unregister_component("test")
    assert satox_manager.get_component_count() == 0
    assert "test" not in satox_manager.get_component_names()

def test_component_initialization(satox_manager):
    component = MockComponent("test")
    satox_manager.register_component(component)
    satox_manager.initialize_components()
    assert component.initialized

def test_component_shutdown(satox_manager):
    component = MockComponent("test")
    satox_manager.register_component(component)
    satox_manager.initialize_components()
    satox_manager.shutdown_components()
    assert component.shutdown

def test_config_update(satox_manager, satox_config):
    new_config = SatoxConfig(
        network_id="mainnet",
        api_endpoint="http://mainnet:8545",
        debug_mode=False,
        max_retries=5,
        timeout=60,
        custom_config={"test": "value"}
    )
    satox_manager.update_config(new_config)
    updated_config = satox_manager.get_config()
    assert updated_config.network_id == "mainnet"
    assert updated_config.api_endpoint == "http://mainnet:8545"
    assert not updated_config.debug_mode
    assert updated_config.max_retries == 5
    assert updated_config.timeout == 60
    assert updated_config.custom_config["test"] == "value"

def test_duplicate_component_registration(satox_manager):
    component1 = MockComponent("test")
    component2 = MockComponent("test")
    satox_manager.register_component(component1)
    with pytest.raises(ValueError):
        satox_manager.register_component(component2)

def test_nonexistent_component_unregistration(satox_manager):
    with pytest.raises(ValueError):
        satox_manager.unregister_component("nonexistent")

def test_component_initialization_failure(satox_manager):
    class FailingComponent(MockComponent):
        def initialize(self) -> bool:
            return False

    component = FailingComponent("test")
    satox_manager.register_component(component)
    with pytest.raises(RuntimeError):
        satox_manager.initialize_components()

def test_component_shutdown_failure(satox_manager):
    class FailingComponent(MockComponent):
        def shutdown(self) -> bool:
            return False

    component = FailingComponent("test")
    satox_manager.register_component(component)
    satox_manager.initialize_components()
    with pytest.raises(RuntimeError):
        satox_manager.shutdown_components() 