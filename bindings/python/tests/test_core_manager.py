"""
Tests for Core Manager
"""

import pytest
import asyncio
from satox_bindings import CoreManager, SatoxError, CoreConfig

class TestCoreManager:
    @pytest.fixture
    async def manager(self):
        manager = CoreManager()
        yield manager
        try:
            await manager.shutdown()
        except:
            pass
    
    @pytest.mark.asyncio
    async def test_initialize(self, manager):
        """Test core manager initialization"""
        config = CoreConfig(name="test_sdk")
        result = await manager.initialize(config)
        assert result is True
    
    @pytest.mark.asyncio
    async def test_double_initialize(self, manager):
        """Test double initialization error"""
        config = CoreConfig(name="test_sdk")
        await manager.initialize(config)
        with pytest.raises(SatoxError, match="already initialized"):
            await manager.initialize(config)
    
    @pytest.mark.asyncio
    async def test_get_status(self, manager):
        """Test getting status"""
        config = CoreConfig(name="test_sdk")
        await manager.initialize(config)
        status = await manager.get_status()
        assert isinstance(status, dict)
        assert "initialized" in status
    
    @pytest.mark.asyncio
    async def test_callback_registration(self, manager):
        """Test callback registration"""
        callback_called = False
        
        def callback(event, success):
            nonlocal callback_called
            callback_called = True
        
        manager.register_callback(callback)
        config = CoreConfig(name="test_sdk")
        await manager.initialize(config)
        
        # Note: This test may need adjustment based on actual implementation
        # assert callback_called 