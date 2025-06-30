"""
Tests for the database module
"""

import pytest
import asyncio
from pathlib import Path
from satox_bindings.database import DatabaseManager, DatabaseModel

class TestModel(DatabaseModel):
    """Test database model"""
    
    def __init__(self, id: int, name: str):
        super().__init__("test_table")
        self.id = id
        self.name = name
    
    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name
        }
    
    @classmethod
    def from_dict(cls, data):
        return cls(data["id"], data["name"])

@pytest.fixture
async def db_manager(tmp_path):
    """Create database manager instance"""
    db_path = tmp_path / "test.db"
    manager = DatabaseManager(str(db_path))
    assert await manager.initialize()
    return manager

@pytest.mark.asyncio
async def test_initialize(db_manager):
    """Test database manager initialization"""
    assert db_manager.get_last_error() == ""

@pytest.mark.asyncio
async def test_create_table(db_manager):
    """Test table creation"""
    columns = {
        "id": "INTEGER",
        "name": "TEXT"
    }
    result = await db_manager.create_table(
        "test_table",
        columns,
        primary_key="id"
    )
    assert result is True

@pytest.mark.asyncio
async def test_insert_data(db_manager):
    """Test data insertion"""
    # Create table
    await db_manager.create_table(
        "test_table",
        {"id": "INTEGER", "name": "TEXT"},
        primary_key="id"
    )
    
    # Insert data
    data = {"id": 1, "name": "Test"}
    result = await db_manager.insert("test_table", data)
    assert result is True

@pytest.mark.asyncio
async def test_update_data(db_manager):
    """Test data update"""
    # Create table and insert data
    await db_manager.create_table(
        "test_table",
        {"id": "INTEGER", "name": "TEXT"},
        primary_key="id"
    )
    await db_manager.insert("test_table", {"id": 1, "name": "Test"})
    
    # Update data
    data = {"name": "Updated"}
    where = {"id": 1}
    result = await db_manager.update("test_table", data, where)
    assert result is True

@pytest.mark.asyncio
async def test_delete_data(db_manager):
    """Test data deletion"""
    # Create table and insert data
    await db_manager.create_table(
        "test_table",
        {"id": "INTEGER", "name": "TEXT"},
        primary_key="id"
    )
    await db_manager.insert("test_table", {"id": 1, "name": "Test"})
    
    # Delete data
    where = {"id": 1}
    result = await db_manager.delete("test_table", where)
    assert result is True

@pytest.mark.asyncio
async def test_query_data(db_manager):
    """Test data querying"""
    # Create table and insert data
    await db_manager.create_table(
        "test_table",
        {"id": "INTEGER", "name": "TEXT"},
        primary_key="id"
    )
    await db_manager.insert("test_table", {"id": 1, "name": "Test"})
    
    # Query data
    results = await db_manager.query(
        "test_table",
        columns=["id", "name"],
        where={"id": 1}
    )
    assert len(results) == 1
    assert results[0]["id"] == 1
    assert results[0]["name"] == "Test"

@pytest.mark.asyncio
async def test_error_handling(db_manager):
    """Test error handling"""
    # Test with non-existent table
    result = await db_manager.insert("non_existent", {"id": 1})
    assert result is False
    assert "no such table" in db_manager.get_last_error().lower()
    
    # Test with invalid data
    await db_manager.create_table(
        "test_table",
        {"id": "INTEGER", "name": "TEXT"},
        primary_key="id"
    )
    result = await db_manager.insert("test_table", {"invalid": "data"})
    assert result is False
    assert "no such column" in db_manager.get_last_error().lower()

@pytest.mark.asyncio
async def test_model_operations(db_manager):
    """Test database model operations"""
    # Create table
    await db_manager.create_table(
        "test_table",
        {"id": "INTEGER", "name": "TEXT"},
        primary_key="id"
    )
    
    # Create and save model
    model = TestModel(1, "Test")
    result = await db_manager.insert("test_table", model.to_dict())
    assert result is True
    
    # Query and recreate model
    results = await db_manager.query("test_table", where={"id": 1})
    assert len(results) == 1
    recreated_model = TestModel.from_dict(results[0])
    assert recreated_model.id == model.id
    assert recreated_model.name == model.name 