#!/usr/bin/env python3
"""
Benchmark Tests for Wallet Operations

This module contains benchmark tests for various wallet operations
in the Satox SDK.
"""

import pytest
from satox import SatoxManager
from satox.wallet import Wallet
from tests.benchmarks.benchmark_framework import BenchmarkFramework, benchmark_decorator

@pytest.fixture
def sdk():
    """Initialize SDK for testing"""
    return SatoxManager()

@pytest.fixture
def benchmark():
    """Initialize benchmark framework"""
    return BenchmarkFramework()

def test_wallet_creation_benchmark(sdk, benchmark):
    """Benchmark wallet creation"""
    result = benchmark.measure(
        operation="wallet_creation",
        func=lambda: sdk.create_wallet(),
        iterations=100,
        metadata={"network": "testnet"}
    )
    assert result.avg_time < 1.0  # Should take less than 1 second

def test_wallet_import_benchmark(sdk, benchmark):
    """Benchmark wallet import from private key"""
    # Create a wallet first to get a valid private key
    wallet = sdk.create_wallet()
    private_key = wallet.private_key

    result = benchmark.measure(
        operation="wallet_import_private_key",
        func=lambda: sdk.import_wallet(private_key=private_key),
        iterations=100,
        metadata={"import_type": "private_key"}
    )
    assert result.avg_time < 1.0

def test_wallet_balance_benchmark(sdk, benchmark):
    """Benchmark wallet balance check"""
    wallet = sdk.create_wallet()
    
    result = benchmark.measure(
        operation="wallet_balance_check",
        func=lambda: wallet.get_balance(),
        iterations=100,
        metadata={"network": "testnet"}
    )
    assert result.avg_time < 0.5  # Should be relatively fast

def test_wallet_signature_benchmark(sdk, benchmark):
    """Benchmark message signing"""
    wallet = sdk.create_wallet()
    message = "Hello, Satox!"
    
    result = benchmark.measure(
        operation="wallet_sign_message",
        func=lambda: wallet.sign_message(message),
        iterations=100,
        metadata={"message_length": len(message)}
    )
    assert result.avg_time < 0.1  # Should be very fast

def test_wallet_verification_benchmark(sdk, benchmark):
    """Benchmark signature verification"""
    wallet = sdk.create_wallet()
    message = "Hello, Satox!"
    signature = wallet.sign_message(message)
    
    result = benchmark.measure(
        operation="wallet_verify_signature",
        func=lambda: wallet.verify_signature(message, signature),
        iterations=100,
        metadata={"message_length": len(message)}
    )
    assert result.avg_time < 0.1

@benchmark_decorator(iterations=50)
def test_wallet_backup_benchmark(sdk):
    """Benchmark wallet backup"""
    wallet = sdk.create_wallet()
    return wallet.backup()

@benchmark_decorator(iterations=50)
def test_wallet_restore_benchmark(sdk):
    """Benchmark wallet restore"""
    wallet = sdk.create_wallet()
    backup_data = wallet.backup()
    return sdk.restore_wallet(backup_data)

def test_wallet_manager_benchmark(sdk, benchmark):
    """Benchmark wallet manager operations"""
    wallet_manager = sdk.wallet_manager
    wallet = sdk.create_wallet()
    
    # Test adding wallet
    add_result = benchmark.measure(
        operation="wallet_manager_add",
        func=lambda: wallet_manager.add_wallet(wallet, "test"),
        iterations=100
    )
    assert add_result.avg_time < 0.1
    
    # Test getting wallet
    get_result = benchmark.measure(
        operation="wallet_manager_get",
        func=lambda: wallet_manager.get_wallet("test"),
        iterations=100
    )
    assert get_result.avg_time < 0.1
    
    # Test listing wallets
    list_result = benchmark.measure(
        operation="wallet_manager_list",
        func=lambda: wallet_manager.list_wallets(),
        iterations=100
    )
    assert list_result.avg_time < 0.1

def test_wallet_transaction_history_benchmark(sdk, benchmark):
    """Benchmark transaction history retrieval"""
    wallet = sdk.create_wallet()
    
    result = benchmark.measure(
        operation="wallet_transaction_history",
        func=lambda: wallet.get_transaction_history(),
        iterations=50,
        metadata={"network": "testnet"}
    )
    assert result.avg_time < 1.0  # Network operation, might take longer

def run_all_benchmarks():
    """Run all wallet benchmarks and generate report"""
    framework = BenchmarkFramework()
    
    # Run all benchmark tests
    test_wallet_creation_benchmark(SatoxManager(), framework)
    test_wallet_import_benchmark(SatoxManager(), framework)
    test_wallet_balance_benchmark(SatoxManager(), framework)
    test_wallet_signature_benchmark(SatoxManager(), framework)
    test_wallet_verification_benchmark(SatoxManager(), framework)
    test_wallet_manager_benchmark(SatoxManager(), framework)
    test_wallet_transaction_history_benchmark(SatoxManager(), framework)
    
    # Save results
    framework.save_results()
    
    # Generate and print report
    report = framework.generate_report()
    print(report)

if __name__ == "__main__":
    run_all_benchmarks() 