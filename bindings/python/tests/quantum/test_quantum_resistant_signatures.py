"""Tests for quantum-resistant signatures."""

import pytest
import os
from unittest.mock import patch, MagicMock
from satox_bindings.quantum.signatures import (
    QuantumResistantSignature,
    Dilithium,
    Falcon,
    SPHINCS,
    Picnic,
    QTESLA
)
from satox_bindings.core.error_handling import SatoxError

@pytest.fixture
def test_message():
    """Create test message."""
    return b"Test quantum-resistant signature"

@pytest.mark.asyncio
async def test_dilithium():
    """Test Dilithium signature scheme."""
    dilithium = Dilithium()
    
    # Generate key pair
    public_key, private_key = await dilithium.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test signing
    message = b"Test Dilithium"
    signature = await dilithium.sign(private_key, message)
    assert signature is not None
    
    # Test verification
    is_valid = await dilithium.verify(public_key, message, signature)
    assert is_valid is True
    
    # Test invalid signature
    is_valid = await dilithium.verify(public_key, message, b"invalid_signature")
    assert is_valid is False

@pytest.mark.asyncio
async def test_falcon():
    """Test Falcon signature scheme."""
    falcon = Falcon()
    
    # Generate key pair
    public_key, private_key = await falcon.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test signing
    message = b"Test Falcon"
    signature = await falcon.sign(private_key, message)
    assert signature is not None
    
    # Test verification
    is_valid = await falcon.verify(public_key, message, signature)
    assert is_valid is True
    
    # Test invalid signature
    is_valid = await falcon.verify(public_key, message, b"invalid_signature")
    assert is_valid is False

@pytest.mark.asyncio
async def test_sphincs():
    """Test SPHINCS+ signature scheme."""
    sphincs = SPHINCS()
    
    # Generate key pair
    public_key, private_key = await sphincs.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test signing
    message = b"Test SPHINCS+"
    signature = await sphincs.sign(private_key, message)
    assert signature is not None
    
    # Test verification
    is_valid = await sphincs.verify(public_key, message, signature)
    assert is_valid is True
    
    # Test invalid signature
    is_valid = await sphincs.verify(public_key, message, b"invalid_signature")
    assert is_valid is False

@pytest.mark.asyncio
async def test_picnic():
    """Test Picnic signature scheme."""
    picnic = Picnic()
    
    # Generate key pair
    public_key, private_key = await picnic.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test signing
    message = b"Test Picnic"
    signature = await picnic.sign(private_key, message)
    assert signature is not None
    
    # Test verification
    is_valid = await picnic.verify(public_key, message, signature)
    assert is_valid is True
    
    # Test invalid signature
    is_valid = await picnic.verify(public_key, message, b"invalid_signature")
    assert is_valid is False

@pytest.mark.asyncio
async def test_qtesla():
    """Test qTESLA signature scheme."""
    qtesla = QTESLA()
    
    # Generate key pair
    public_key, private_key = await qtesla.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test signing
    message = b"Test qTESLA"
    signature = await qtesla.sign(private_key, message)
    assert signature is not None
    
    # Test verification
    is_valid = await qtesla.verify(public_key, message, signature)
    assert is_valid is True
    
    # Test invalid signature
    is_valid = await qtesla.verify(public_key, message, b"invalid_signature")
    assert is_valid is False

@pytest.mark.asyncio
async def test_signature_parameters():
    """Test signature parameter handling."""
    # Test Dilithium parameters
    dilithium = Dilithium()
    params = await dilithium.get_parameters()
    assert "security_level" in params
    assert "key_size" in params
    
    # Test Falcon parameters
    falcon = Falcon()
    params = await falcon.get_parameters()
    assert "security_level" in params
    assert "key_size" in params

@pytest.mark.asyncio
async def test_signature_performance():
    """Test signature performance."""
    # Test Dilithium performance
    dilithium = Dilithium()
    public_key, private_key = await dilithium.generate_key_pair()
    message = b"Test performance"
    
    # Measure signing time
    signature = await dilithium.sign(private_key, message)
    assert signature is not None
    
    # Measure verification time
    is_valid = await dilithium.verify(public_key, message, signature)
    assert is_valid is True

@pytest.mark.asyncio
async def test_signature_security():
    """Test signature security properties."""
    # Test Dilithium security
    dilithium = Dilithium()
    public_key, private_key = await dilithium.generate_key_pair()
    
    # Test key pair security
    assert len(public_key) >= 32  # Minimum key size
    assert len(private_key) >= 32
    
    # Test signature security
    message = b"Test security"
    signature = await dilithium.sign(private_key, message)
    assert signature != message  # Signature should be different from message

@pytest.mark.asyncio
async def test_signature_interoperability():
    """Test signature interoperability."""
    # Test Dilithium interoperability
    dilithium1 = Dilithium()
    dilithium2 = Dilithium()
    
    # Generate key pair with first instance
    public_key, private_key = await dilithium1.generate_key_pair()
    
    # Use key pair with second instance
    message = b"Test interoperability"
    signature = await dilithium2.sign(private_key, message)
    is_valid = await dilithium2.verify(public_key, message, signature)
    assert is_valid is True

@pytest.mark.asyncio
async def test_signature_error_handling():
    """Test signature error handling."""
    # Test Dilithium error handling
    dilithium = Dilithium()
    
    # Test invalid key
    with pytest.raises(SatoxError) as exc_info:
        await dilithium.sign(b"invalid_key", b"test")
    assert "Invalid key" in str(exc_info.value)
    
    # Test invalid message
    with pytest.raises(SatoxError) as exc_info:
        await dilithium.sign(None, None)
    assert "Invalid message" in str(exc_info.value)

@pytest.mark.asyncio
async def test_signature_serialization():
    """Test signature serialization."""
    # Test Dilithium serialization
    dilithium = Dilithium()
    public_key, private_key = await dilithium.generate_key_pair()
    
    # Serialize keys
    serialized_public = await dilithium.serialize_key(public_key)
    serialized_private = await dilithium.serialize_key(private_key)
    
    # Deserialize keys
    deserialized_public = await dilithium.deserialize_key(serialized_public)
    deserialized_private = await dilithium.deserialize_key(serialized_private)
    
    assert deserialized_public == public_key
    assert deserialized_private == private_key

@pytest.mark.asyncio
async def test_signature_batch_operations():
    """Test signature batch operations."""
    # Test Dilithium batch operations
    dilithium = Dilithium()
    public_key, private_key = await dilithium.generate_key_pair()
    
    # Create batch of messages
    messages = [b"Message 1", b"Message 2", b"Message 3"]
    
    # Batch sign
    signatures = await dilithium.batch_sign(private_key, messages)
    assert len(signatures) == len(messages)
    
    # Batch verify
    results = await dilithium.batch_verify(public_key, messages, signatures)
    assert all(results)  # All signatures should be valid

@pytest.mark.asyncio
async def test_signature_key_rotation():
    """Test signature key rotation."""
    # Test Dilithium key rotation
    dilithium = Dilithium()
    old_public, old_private = await dilithium.generate_key_pair()
    
    # Rotate keys
    new_public, new_private = await dilithium.rotate_keys(old_public, old_private)
    assert new_public != old_public
    assert new_private != old_private
    
    # Verify new keys work
    message = b"Test key rotation"
    signature = await dilithium.sign(new_private, message)
    is_valid = await dilithium.verify(new_public, message, signature)
    assert is_valid is True 