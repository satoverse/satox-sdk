"""
Tests for the IPFS module
"""

import os
import pytest
from pathlib import Path
from satox_bindings.ipfs import IPFSManager, IPFSContent

@pytest.fixture
def ipfs_manager():
    """Create IPFS manager instance"""
    manager = IPFSManager()
    manager.initialize()
    return manager

@pytest.fixture
def test_file(tmp_path):
    """Create test file"""
    file_path = tmp_path / "test.txt"
    file_path.write_text("Test content")
    return file_path

def test_initialize(ipfs_manager):
    """Test IPFS manager initialization"""
    assert ipfs_manager.initialize() is True
    assert ipfs_manager.get_last_error() == ""

def test_add_file(ipfs_manager, test_file):
    """Test adding file to IPFS"""
    content = ipfs_manager.add_file(str(test_file))
    assert content is not None
    assert isinstance(content, IPFSContent)
    assert content.cid is not None
    assert content.name == test_file.name
    assert content.size > 0
    assert content.content_type == "text/plain"

def test_get_file(ipfs_manager, test_file, tmp_path):
    """Test getting file from IPFS"""
    # First add the file
    content = ipfs_manager.add_file(str(test_file))
    assert content is not None
    
    # Then get the file
    output_path = tmp_path / "downloaded.txt"
    result = ipfs_manager.get_file(content.cid, str(output_path))
    assert result is True
    assert output_path.exists()
    assert output_path.read_text() == "Test content"

def test_pin_content(ipfs_manager, test_file):
    """Test pinning content in IPFS"""
    # First add the file
    content = ipfs_manager.add_file(str(test_file))
    assert content is not None
    
    # Then pin the content
    result = ipfs_manager.pin_content(content.cid)
    assert result is True

def test_unpin_content(ipfs_manager, test_file):
    """Test unpinning content from IPFS"""
    # First add the file
    content = ipfs_manager.add_file(str(test_file))
    assert content is not None
    
    # Then pin and unpin the content
    assert ipfs_manager.pin_content(content.cid) is True
    assert ipfs_manager.unpin_content(content.cid) is True

def test_error_handling(ipfs_manager):
    """Test error handling"""
    # Test with non-existent file
    content = ipfs_manager.add_file("non_existent.txt")
    assert content is None
    assert "File not found" in ipfs_manager.get_last_error()
    
    # Test with invalid CID
    result = ipfs_manager.get_file("invalid_cid", "output.txt")
    assert result is False
    assert "Invalid CID" in ipfs_manager.get_last_error()

def test_content_metadata(ipfs_manager, test_file):
    """Test content metadata handling"""
    # Add file with metadata
    metadata = {"description": "Test file", "tags": ["test", "ipfs"]}
    content = ipfs_manager.add_file(str(test_file), metadata=metadata)
    assert content is not None
    assert content.metadata == metadata

def test_content_serialization(ipfs_manager, test_file):
    """Test content serialization"""
    content = ipfs_manager.add_file(str(test_file))
    assert content is not None
    
    # Convert to dictionary
    content_dict = content.to_dict()
    assert isinstance(content_dict, dict)
    assert content_dict["cid"] == content.cid
    assert content_dict["name"] == content.name
    
    # Create from dictionary
    new_content = IPFSContent.from_dict(content_dict)
    assert new_content.cid == content.cid
    assert new_content.name == content.name 