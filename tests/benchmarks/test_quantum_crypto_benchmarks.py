"""Benchmark tests for quantum-resistant cryptography operations."""
import pytest
import time
import statistics
from satox.security.quantum_crypto import QuantumCrypto

@pytest.fixture
def crypto():
    """Create a QuantumCrypto instance for benchmarking."""
    return QuantumCrypto()

def benchmark_operation(operation, *args, iterations=100):
    """Run a benchmark for a specific operation."""
    times = []
    for _ in range(iterations):
        start_time = time.perf_counter()
        operation(*args)
        end_time = time.perf_counter()
        times.append(end_time - start_time)
    
    return {
        'min': min(times),
        'max': max(times),
        'mean': statistics.mean(times),
        'median': statistics.median(times),
        'std_dev': statistics.stdev(times) if len(times) > 1 else 0
    }

def test_key_generation_benchmark(crypto):
    """Benchmark key pair generation."""
    results = benchmark_operation(crypto.generate_keypair)
    print("\nKey Generation Benchmark:")
    print(f"Min: {results['min']*1000:.2f}ms")
    print(f"Max: {results['max']*1000:.2f}ms")
    print(f"Mean: {results['mean']*1000:.2f}ms")
    print(f"Median: {results['median']*1000:.2f}ms")
    print(f"Std Dev: {results['std_dev']*1000:.2f}ms")

def test_wallet_encryption_benchmark(crypto):
    """Benchmark wallet encryption and decryption."""
    # Generate test data
    wallet_data = b"test wallet data" * 1000  # ~16KB
    password = "test_password_123"
    
    # Benchmark encryption
    encrypted_wallet = crypto.encrypt_wallet(wallet_data, password)
    enc_results = benchmark_operation(crypto.encrypt_wallet, wallet_data, password)
    
    print("\nWallet Encryption Benchmark:")
    print(f"Min: {enc_results['min']*1000:.2f}ms")
    print(f"Max: {enc_results['max']*1000:.2f}ms")
    print(f"Mean: {enc_results['mean']*1000:.2f}ms")
    print(f"Median: {enc_results['median']*1000:.2f}ms")
    print(f"Std Dev: {enc_results['std_dev']*1000:.2f}ms")
    
    # Benchmark decryption
    dec_results = benchmark_operation(crypto.decrypt_wallet, encrypted_wallet, password)
    
    print("\nWallet Decryption Benchmark:")
    print(f"Min: {dec_results['min']*1000:.2f}ms")
    print(f"Max: {dec_results['max']*1000:.2f}ms")
    print(f"Mean: {dec_results['mean']*1000:.2f}ms")
    print(f"Median: {dec_results['median']*1000:.2f}ms")
    print(f"Std Dev: {dec_results['std_dev']*1000:.2f}ms")

def test_transaction_signing_benchmark(crypto):
    """Benchmark transaction signing and verification."""
    # Generate key pair
    private_key, public_key = crypto.generate_keypair()
    
    # Generate test data
    transaction = b"test transaction data" * 100  # ~2KB
    
    # Benchmark signing
    signature = crypto.sign_transaction(transaction, private_key)
    sign_results = benchmark_operation(crypto.sign_transaction, transaction, private_key)
    
    print("\nTransaction Signing Benchmark:")
    print(f"Min: {sign_results['min']*1000:.2f}ms")
    print(f"Max: {sign_results['max']*1000:.2f}ms")
    print(f"Mean: {sign_results['mean']*1000:.2f}ms")
    print(f"Median: {sign_results['median']*1000:.2f}ms")
    print(f"Std Dev: {sign_results['std_dev']*1000:.2f}ms")
    
    # Benchmark verification
    verify_results = benchmark_operation(crypto.verify_signature, transaction, signature, public_key)
    
    print("\nSignature Verification Benchmark:")
    print(f"Min: {verify_results['min']*1000:.2f}ms")
    print(f"Max: {verify_results['max']*1000:.2f}ms")
    print(f"Mean: {verify_results['mean']*1000:.2f}ms")
    print(f"Median: {verify_results['median']*1000:.2f}ms")
    print(f"Std Dev: {verify_results['std_dev']*1000:.2f}ms")

def test_message_encryption_benchmark(crypto):
    """Benchmark message encryption and decryption."""
    # Generate key pair
    private_key, public_key = crypto.generate_keypair()
    
    # Generate test data
    message = b"test message" * 1000  # ~12KB
    
    # Benchmark encryption
    encrypted_message = crypto.encrypt_message(message, public_key)
    enc_results = benchmark_operation(crypto.encrypt_message, message, public_key)
    
    print("\nMessage Encryption Benchmark:")
    print(f"Min: {enc_results['min']*1000:.2f}ms")
    print(f"Max: {enc_results['max']*1000:.2f}ms")
    print(f"Mean: {enc_results['mean']*1000:.2f}ms")
    print(f"Median: {enc_results['median']*1000:.2f}ms")
    print(f"Std Dev: {enc_results['std_dev']*1000:.2f}ms")
    
    # Benchmark decryption
    dec_results = benchmark_operation(crypto.decrypt_message, encrypted_message, private_key)
    
    print("\nMessage Decryption Benchmark:")
    print(f"Min: {dec_results['min']*1000:.2f}ms")
    print(f"Max: {dec_results['max']*1000:.2f}ms")
    print(f"Mean: {dec_results['mean']*1000:.2f}ms")
    print(f"Median: {dec_results['median']*1000:.2f}ms")
    print(f"Std Dev: {dec_results['std_dev']*1000:.2f}ms")

def test_concurrent_operations_benchmark(crypto):
    """Benchmark concurrent cryptographic operations."""
    import asyncio
    
    async def perform_operations():
        private_key, public_key = crypto.generate_keypair()
        message = b"test message" * 100
        tasks = []
        
        # Create multiple concurrent operations
        for _ in range(10):
            tasks.append(crypto.encrypt_message(message, public_key))
            tasks.append(crypto.sign_transaction(message, private_key))
        
        # Execute all operations concurrently
        return await asyncio.gather(*tasks)
    
    # Benchmark concurrent operations
    start_time = time.perf_counter()
    results = asyncio.run(perform_operations())
    end_time = time.perf_counter()
    
    total_time = end_time - start_time
    print("\nConcurrent Operations Benchmark:")
    print(f"Total time for 20 operations: {total_time*1000:.2f}ms")
    print(f"Average time per operation: {(total_time/20)*1000:.2f}ms")

def test_memory_usage_benchmark(crypto):
    """Benchmark memory usage for different operations."""
    import psutil
    import os
    
    process = psutil.Process(os.getpid())
    
    def measure_memory(operation, *args):
        initial_memory = process.memory_info().rss
        operation(*args)
        final_memory = process.memory_info().rss
        return final_memory - initial_memory
    
    # Generate key pair
    private_key, public_key = crypto.generate_keypair()
    
    # Test data
    message = b"test message" * 1000
    
    # Measure memory for different operations
    key_gen_memory = measure_memory(crypto.generate_keypair)
    encryption_memory = measure_memory(crypto.encrypt_message, message, public_key)
    signing_memory = measure_memory(crypto.sign_transaction, message, private_key)
    
    print("\nMemory Usage Benchmark:")
    print(f"Key Generation: {key_gen_memory/1024:.2f}KB")
    print(f"Message Encryption: {encryption_memory/1024:.2f}KB")
    print(f"Transaction Signing: {signing_memory/1024:.2f}KB") 