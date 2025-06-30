"""Tests for quantum circuit simulator."""

import pytest
import numpy as np
import asyncio
from unittest.mock import patch, MagicMock
from satox_bindings.quantum.circuit import QuantumCircuit
from satox_bindings.core.error_handling import SatoxError

@pytest.fixture
def quantum_circuit():
    """Create a quantum circuit instance for testing."""
    circuit = QuantumCircuit(num_qubits=2)
    asyncio.run(circuit.initialize())
    return circuit

@pytest.mark.asyncio
async def test_initialize(quantum_circuit):
    """Test circuit initialization."""
    assert quantum_circuit.num_qubits == 2
    assert quantum_circuit._initialized

@pytest.mark.asyncio
async def test_shutdown(quantum_circuit):
    """Test circuit shutdown."""
    assert quantum_circuit is not None

@pytest.mark.asyncio
async def test_create_circuit():
    """Test circuit creation."""
    circuit = QuantumCircuit(num_qubits=3)
    await circuit.initialize()
    assert circuit.num_qubits == 3
    assert circuit._initialized

@pytest.mark.asyncio
async def test_single_qubit_gates(quantum_circuit):
    """Test single-qubit gates."""
    # Test H gate
    await quantum_circuit.h(0)
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 1/np.sqrt(2)])
    
    # Test X gate
    await quantum_circuit.x(0)
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 1/np.sqrt(2)])
    
    # Test Y gate
    await quantum_circuit.y(0)
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 1/np.sqrt(2)])
    
    # Test Z gate
    await quantum_circuit.z(0)
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 1/np.sqrt(2)])

@pytest.mark.asyncio
async def test_two_qubit_gates(quantum_circuit):
    """Test two-qubit gates."""
    # Test CNOT gate
    await quantum_circuit.h(0)
    await quantum_circuit.cnot(0, 1)
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 0, 0, 1/np.sqrt(2)])
    
    # Test SWAP gate
    await quantum_circuit.swap(0, 1)
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 0, 0, 1/np.sqrt(2)])

@pytest.mark.asyncio
async def test_three_qubit_gates(quantum_circuit):
    """Test three-qubit gates."""
    # Create circuit with 3 qubits
    circuit = QuantumCircuit(num_qubits=3)
    await circuit.initialize()
    
    # Test Toffoli gate
    await circuit.h(0)
    await circuit.h(1)
    await circuit.cnot(0, 2)
    await circuit.cnot(1, 2)
    state = await circuit.get_state()
    assert np.allclose(np.abs(state), [1/2, 0, 0, 0, 0, 0, 0, 1/2])

@pytest.mark.asyncio
async def test_measurement(quantum_circuit):
    """Test quantum measurement."""
    # Prepare state
    await quantum_circuit.h(0)
    
    # Measure multiple times
    results = []
    for _ in range(1000):
        result = await quantum_circuit.measure(0)
        results.append(result)
    
    # Check measurement statistics
    ones = sum(results)
    assert 400 <= ones <= 600  # Should be roughly 50-50

@pytest.mark.asyncio
async def test_circuit_execution(quantum_circuit):
    """Test circuit execution."""
    # Add gates
    await quantum_circuit.h(0)
    await quantum_circuit.cnot(0, 1)
    
    # Execute circuit
    await quantum_circuit.run()
    
    # Check final state
    state = await quantum_circuit.get_state()
    assert np.allclose(np.abs(state), [1/np.sqrt(2), 0, 0, 1/np.sqrt(2)])

@pytest.mark.asyncio
async def test_circuit_optimization(quantum_circuit):
    """Test circuit optimization."""
    # Add redundant gates
    await quantum_circuit.h(0)
    await quantum_circuit.h(0)
    
    # Optimize circuit
    await quantum_circuit.optimize()
    
    # Check gate count
    assert len(quantum_circuit.gates) == 1  # Should combine the two H gates

@pytest.mark.asyncio
async def test_circuit_visualization(quantum_circuit):
    """Test circuit visualization."""
    # Add gates
    await quantum_circuit.h(0)
    await quantum_circuit.cnot(0, 1)
    
    # Generate visualization
    visualization = await quantum_circuit.visualize_circuit()
    assert visualization is not None
    assert isinstance(visualization, str)

@pytest.mark.asyncio
async def test_circuit_serialization(quantum_circuit):
    """Test circuit serialization."""
    # Add gates
    await quantum_circuit.h(0)
    await quantum_circuit.cnot(0, 1)
    
    # Serialize circuit
    serialized = await quantum_circuit.serialize_circuit()
    assert serialized is not None
    
    # Create new circuit and deserialize
    new_circuit = QuantumCircuit(num_qubits=2)
    await new_circuit.initialize()
    await new_circuit.deserialize_circuit(serialized)
    
    # Compare states
    original_state = await quantum_circuit.get_state()
    deserialized_state = await new_circuit.get_state()
    assert np.allclose(original_state, deserialized_state)

@pytest.mark.asyncio
async def test_circuit_error_handling(quantum_circuit):
    """Test circuit error handling."""
    # Test invalid qubit index
    with pytest.raises(ValueError) as exc_info:
        await quantum_circuit.h(2)  # Only 2 qubits in circuit
    assert "Qubit index out of range" in str(exc_info.value)
    
    # Test invalid gate
    with pytest.raises(ValueError) as exc_info:
        await quantum_circuit.add_gate("INVALID", [0])
    assert "Invalid gate type" in str(exc_info.value)

@pytest.mark.asyncio
async def test_circuit_performance(quantum_circuit):
    """Test circuit performance."""
    import time
    
    # Create large circuit
    circuit = QuantumCircuit(num_qubits=10)
    await circuit.initialize()
    
    # Add many gates
    start_time = time.time()
    for i in range(100):
        await circuit.h(i % 10)
        await circuit.cnot(i % 10, (i + 1) % 10)
    
    # Execute circuit
    await circuit.run()
    end_time = time.time()
    
    # Check execution time
    execution_time = end_time - start_time
    assert execution_time < 1.0  # Should complete within 1 second

@pytest.mark.asyncio
async def test_circuit_parallel_execution(quantum_circuit):
    """Test parallel circuit execution."""
    import asyncio
    
    async def execute_circuit():
        circuit = QuantumCircuit(num_qubits=2)
        await circuit.initialize()
        await circuit.h(0)
        await circuit.cnot(0, 1)
        await circuit.run()
        return await circuit.get_state()
    
    # Execute multiple circuits in parallel
    tasks = [execute_circuit() for _ in range(10)]
    results = await asyncio.gather(*tasks)
    
    # Check results
    for result in results:
        assert result is not None
        assert np.allclose(np.abs(result), [1/np.sqrt(2), 0, 0, 1/np.sqrt(2)])

@pytest.mark.asyncio
async def test_circuit_quantum_teleportation(quantum_circuit):
    """Test quantum teleportation circuit."""
    # Create circuit with 3 qubits
    circuit = QuantumCircuit(num_qubits=3)
    await circuit.initialize()
    
    # Prepare Bell state
    await circuit.h(1)
    await circuit.cnot(1, 2)
    
    # Prepare state to teleport
    await circuit.h(0)
    
    # Perform teleportation
    await circuit.cnot(0, 1)
    await circuit.h(0)
    
    # Measure and apply corrections
    m1 = await circuit.measure(0)
    m2 = await circuit.measure(1)
    
    if m2:
        await circuit.x(2)
    if m1:
        await circuit.z(2)
    
    # Check final state
    state = await circuit.get_state()
    assert np.allclose(np.abs(state), [0, 0, 0, 0, 0, 0, 1, 0])  # Should be |1⟩ on qubit 2

@pytest.mark.asyncio
async def test_circuit_quantum_fourier_transform(quantum_circuit):
    """Test quantum Fourier transform circuit."""
    # Create circuit with 3 qubits
    circuit = QuantumCircuit(num_qubits=3)
    await circuit.initialize()
    
    # Apply QFT
    for i in range(3):
        await circuit.h(i)
        for j in range(i + 1, 3):
            await circuit.cnot(i, j)
    
    # Check final state
    state = await circuit.get_state()
    assert np.allclose(np.abs(state), [1/8] * 8)  # Equal superposition

@pytest.mark.asyncio
async def test_circuit_quantum_phase_estimation(quantum_circuit):
    """Test quantum phase estimation circuit."""
    # Create circuit with 4 qubits
    circuit = QuantumCircuit(num_qubits=4)
    await circuit.initialize()
    
    # Prepare initial state
    await circuit.h(0)
    await circuit.h(1)
    await circuit.h(2)
    
    # Apply controlled operations
    await circuit.cnot(0, 3)
    await circuit.cnot(1, 3)
    await circuit.cnot(2, 3)
    
    # Apply inverse QFT
    await circuit.h(2)
    await circuit.cnot(1, 2)
    await circuit.h(1)
    await circuit.cnot(0, 1)
    await circuit.h(0)
    
    # Check final state
    state = await circuit.get_state()
    assert np.allclose(np.abs(state), [1/4] * 16)  # Equal superposition 