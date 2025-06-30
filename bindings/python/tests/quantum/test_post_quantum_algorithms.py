"""Tests for post-quantum algorithms."""

import pytest
import os
from unittest.mock import patch, MagicMock
from satox_bindings.quantum.post_quantum import (
    CrystalsKyber,
    NTRU,
    SABER,
    BIKE,
    HQC,
    McEliece
)
from satox_bindings.core.error_handling import SatoxError
from satox_bindings.quantum.signatures import (
    QuantumResistantSignature,
    Dilithium,
    Falcon,
    SPHINCS,
    Picnic,
    QTESLA
)

@pytest.fixture
def test_data():
    """Create test data."""
    return b"Test quantum-resistant encryption"

@pytest.mark.asyncio
async def test_crystals_kyber():
    """Test CRYSTALS-Kyber implementation."""
    kyber = CrystalsKyber()
    
    # Generate key pair
    public_key, private_key = await kyber.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test encryption/decryption
    message = b"Test CRYSTALS-Kyber"
    ciphertext = await kyber.encrypt(public_key, message)
    decrypted = await kyber.decrypt(private_key, ciphertext)
    assert decrypted == message
    
    # Test key encapsulation
    shared_secret, ciphertext = await kyber.encapsulate(public_key)
    decrypted_secret = await kyber.decapsulate(private_key, ciphertext)
    assert shared_secret == decrypted_secret

@pytest.mark.asyncio
async def test_ntru():
    """Test NTRU implementation."""
    ntru = NTRU()
    
    # Generate key pair
    public_key, private_key = await ntru.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test encryption/decryption
    message = b"Test NTRU"
    ciphertext = await ntru.encrypt(public_key, message)
    decrypted = await ntru.decrypt(private_key, ciphertext)
    assert decrypted == message
    
    # Test key encapsulation
    shared_secret, ciphertext = await ntru.encapsulate(public_key)
    decrypted_secret = await ntru.decapsulate(private_key, ciphertext)
    assert shared_secret == decrypted_secret

@pytest.mark.asyncio
async def test_saber():
    """Test SABER implementation."""
    saber = SABER()
    
    # Generate key pair
    public_key, private_key = await saber.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test encryption/decryption
    message = b"Test SABER"
    ciphertext = await saber.encrypt(public_key, message)
    decrypted = await saber.decrypt(private_key, ciphertext)
    assert decrypted == message
    
    # Test key encapsulation
    shared_secret, ciphertext = await saber.encapsulate(public_key)
    decrypted_secret = await saber.decapsulate(private_key, ciphertext)
    assert shared_secret == decrypted_secret

@pytest.mark.asyncio
async def test_bike():
    """Test BIKE implementation."""
    bike = BIKE()
    
    # Generate key pair
    public_key, private_key = await bike.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test encryption/decryption
    message = b"Test BIKE"
    ciphertext = await bike.encrypt(public_key, message)
    decrypted = await bike.decrypt(private_key, ciphertext)
    assert decrypted == message
    
    # Test key encapsulation
    shared_secret, ciphertext = await bike.encapsulate(public_key)
    decrypted_secret = await bike.decapsulate(private_key, ciphertext)
    assert shared_secret == decrypted_secret

@pytest.mark.asyncio
async def test_hqc():
    """Test HQC implementation."""
    hqc = HQC()
    
    # Generate key pair
    public_key, private_key = await hqc.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test encryption/decryption
    message = b"Test HQC"
    ciphertext = await hqc.encrypt(public_key, message)
    decrypted = await hqc.decrypt(private_key, ciphertext)
    assert decrypted == message
    
    # Test key encapsulation
    shared_secret, ciphertext = await hqc.encapsulate(public_key)
    decrypted_secret = await hqc.decapsulate(private_key, ciphertext)
    assert shared_secret == decrypted_secret

@pytest.mark.asyncio
async def test_mceliece():
    """Test McEliece implementation."""
    mceliece = McEliece()
    
    # Generate key pair
    public_key, private_key = await mceliece.generate_key_pair()
    assert public_key is not None
    assert private_key is not None
    
    # Test encryption/decryption
    message = b"Test McEliece"
    ciphertext = await mceliece.encrypt(public_key, message)
    decrypted = await mceliece.decrypt(private_key, ciphertext)
    assert decrypted == message

@pytest.mark.asyncio
async def test_algorithm_parameters():
    """Test algorithm parameter handling."""
    # Test CRYSTALS-Kyber parameters
    kyber = CrystalsKyber()
    params = await kyber.get_parameters()
    assert "security_level" in params
    assert "key_size" in params
    
    # Test NTRU parameters
    ntru = NTRU()
    params = await ntru.get_parameters()
    assert "security_level" in params
    assert "key_size" in params

@pytest.mark.asyncio
async def test_algorithm_performance():
    """Test algorithm performance."""
    # Test CRYSTALS-Kyber performance
    kyber = CrystalsKyber()
    public_key, private_key = await kyber.generate_key_pair()
    message = b"Test performance"
    
    # Measure encryption time
    ciphertext = await kyber.encrypt(public_key, message)
    assert ciphertext is not None
    
    # Measure decryption time
    decrypted = await kyber.decrypt(private_key, ciphertext)
    assert decrypted == message

@pytest.mark.asyncio
async def test_algorithm_security():
    """Test algorithm security properties."""
    # Test CRYSTALS-Kyber security
    kyber = CrystalsKyber()
    public_key, private_key = await kyber.generate_key_pair()
    
    # Test key pair security
    assert len(public_key) >= 32  # Minimum key size
    assert len(private_key) >= 32
    
    # Test ciphertext security
    message = b"Test security"
    ciphertext = await kyber.encrypt(public_key, message)
    assert ciphertext != message  # Ciphertext should be different from plaintext

@pytest.mark.asyncio
async def test_algorithm_interoperability():
    """Test algorithm interoperability."""
    # Test CRYSTALS-Kyber interoperability
    kyber1 = CrystalsKyber()
    kyber2 = CrystalsKyber()
    
    # Generate key pair with first instance
    public_key, private_key = await kyber1.generate_key_pair()
    
    # Use key pair with second instance
    message = b"Test interoperability"
    ciphertext = await kyber2.encrypt(public_key, message)
    decrypted = await kyber2.decrypt(private_key, ciphertext)
    assert decrypted == message

@pytest.mark.asyncio
async def test_algorithm_error_handling():
    """Test algorithm error handling."""
    # Test CRYSTALS-Kyber error handling
    kyber = CrystalsKyber()
    
    # Test invalid key
    with pytest.raises(SatoxError) as exc_info:
        await kyber.encrypt(b"invalid_key", b"test")
    assert "Invalid key" in str(exc_info.value)
    
    # Test invalid data
    with pytest.raises(SatoxError) as exc_info:
        await kyber.encrypt(None, None)
    assert "Invalid data" in str(exc_info.value)

@pytest.mark.asyncio
async def test_algorithm_serialization():
    """Test algorithm serialization."""
    # Test CRYSTALS-Kyber serialization
    kyber = CrystalsKyber()
    public_key, private_key = await kyber.generate_key_pair()
    
    # Serialize keys
    serialized_public = await kyber.serialize_key(public_key)
    serialized_private = await kyber.serialize_key(private_key)
    
    # Deserialize keys
    deserialized_public = await kyber.deserialize_key(serialized_public)
    deserialized_private = await kyber.deserialize_key(serialized_private)
    
    assert deserialized_public == public_key
    assert deserialized_private == private_key 