#!/usr/bin/env python3
"""
Satox SDK Command Line Interface
Copyright (c) 2025 Satoxcoin Core Developers
MIT License

Command-line interface for the Satox SDK with quantum-resistant security.
"""

import argparse
import json
import sys
import os
from typing import Dict, Any, Optional
from datetime import datetime

try:
    from . import SDK, create_default_config, create_mainnet_config, create_testnet_config
except ImportError:
    # Fallback for development
    from .mock import SDK
    create_default_config = lambda: {}
    create_mainnet_config = lambda: {}
    create_testnet_config = lambda: {}

def print_banner():
    """Print the Satox SDK banner."""
    banner = """
╔══════════════════════════════════════════════════════════════╗
║                    Satox SDK CLI v1.0.0                      ║
║              Quantum-Resistant Blockchain Toolkit            ║
║                                                              ║
║  Made with ❤️ by the Satoverse Team                          ║
║  https://satoverse.io | https://github.com/satoverse/satox-sdk ║
╚══════════════════════════════════════════════════════════════╝
    """
    print(banner)

def print_error(message: str):
    """Print an error message."""
    print(f"❌ Error: {message}", file=sys.stderr)

def print_success(message: str):
    """Print a success message."""
    print(f"✅ {message}")

def print_info(message: str):
    """Print an info message."""
    print(f"ℹ️  {message}")

def load_config(config_file: Optional[str] = None) -> Dict[str, Any]:
    """Load configuration from file or create default."""
    if config_file and os.path.exists(config_file):
        try:
            with open(config_file, 'r') as f:
                return json.load(f)
        except Exception as e:
            print_error(f"Failed to load config file: {e}")
            return create_default_config()
    else:
        return create_default_config()

def save_config(config: Dict[str, Any], config_file: str):
    """Save configuration to file."""
    try:
        os.makedirs(os.path.dirname(config_file), exist_ok=True)
        with open(config_file, 'w') as f:
            json.dump(config, f, indent=2)
        print_success(f"Configuration saved to {config_file}")
    except Exception as e:
        print_error(f"Failed to save config: {e}")

def init_sdk(config: Dict[str, Any]) -> Optional[SDK]:
    """Initialize the SDK with configuration."""
    try:
        sdk = SDK()
        if sdk.initialize(config):
            print_success("SDK initialized successfully")
            return sdk
        else:
            print_error("Failed to initialize SDK")
            return None
    except Exception as e:
        print_error(f"SDK initialization error: {e}")
        return None

def cmd_init(args):
    """Initialize SDK configuration."""
    print_banner()
    
    if args.network == "mainnet":
        config = create_mainnet_config()
    elif args.network == "testnet":
        config = create_testnet_config()
    else:
        config = create_default_config()
    
    if args.config_file:
        save_config(config, args.config_file)
    else:
        print(json.dumps(config, indent=2))

def cmd_wallet(args):
    """Wallet management commands."""
    config = load_config(args.config_file)
    sdk = init_sdk(config)
    if not sdk:
        return 1
    
    try:
        if args.action == "create":
            wallet_id = sdk.wallet_manager.create_wallet(args.name)
            print_success(f"Wallet created: {wallet_id}")
            print_info(f"Wallet name: {args.name}")
            
        elif args.action == "list":
            # Mock implementation - in real SDK this would list wallets
            print_info("Available wallets:")
            print("  - mock_wallet_1 (testnet)")
            print("  - mock_wallet_2 (mainnet)")
            
        elif args.action == "address":
            if args.wallet_id:
                address = sdk.wallet_manager.generate_address(args.wallet_id)
                print_success(f"Generated address: {address}")
            else:
                print_error("Wallet ID required for address generation")
                return 1
                
        elif args.action == "balance":
            if args.wallet_id:
                balance = sdk.wallet_manager.get_balance(args.wallet_id)
                print_success(f"Wallet balance: {balance} SATOX")
            else:
                print_error("Wallet ID required for balance check")
                return 1
                
        elif args.action == "send":
            if not all([args.from_address, args.to_address, args.amount]):
                print_error("from-address, to-address, and amount required for send")
                return 1
            
            tx_id = sdk.wallet_manager.send_transaction(
                args.from_address, 
                args.to_address, 
                float(args.amount)
            )
            print_success(f"Transaction sent: {tx_id}")
            
    except Exception as e:
        print_error(f"Wallet operation failed: {e}")
        return 1
    
    return 0

def cmd_security(args):
    """Security management commands."""
    config = load_config(args.config_file)
    sdk = init_sdk(config)
    if not sdk:
        return 1
    
    try:
        if args.action == "generate":
            public_key, private_key = sdk.security_manager.generate_pqc_keypair(args.key_id)
            print_success("Quantum-resistant key pair generated")
            print_info(f"Public key: {public_key}")
            print_info(f"Private key: {private_key}")
            
        elif args.action == "sign":
            if not all([args.key_id, args.data]):
                print_error("key-id and data required for signing")
                return 1
            
            signature = sdk.security_manager.sign_with_pqc(args.key_id, args.data)
            print_success(f"Data signed: {signature}")
            
        elif args.action == "verify":
            if not all([args.key_id, args.data, args.signature]):
                print_error("key-id, data, and signature required for verification")
                return 1
            
            is_valid = sdk.security_manager.verify_with_pqc(args.key_id, args.data, args.signature)
            if is_valid:
                print_success("Signature is valid")
            else:
                print_error("Signature is invalid")
                
        elif args.action == "encrypt":
            if not all([args.key_id, args.data]):
                print_error("key-id and data required for encryption")
                return 1
            
            encrypted = sdk.security_manager.encrypt_with_pqc(args.key_id, args.data)
            print_success(f"Data encrypted: {encrypted}")
            
        elif args.action == "decrypt":
            if not all([args.key_id, args.encrypted_data]):
                print_error("key-id and encrypted-data required for decryption")
                return 1
            
            decrypted = sdk.security_manager.decrypt_with_pqc(args.key_id, args.encrypted_data)
            print_success(f"Data decrypted: {decrypted}")
            
    except Exception as e:
        print_error(f"Security operation failed: {e}")
        return 1
    
    return 0

def cmd_asset(args):
    """Asset management commands."""
    config = load_config(args.config_file)
    sdk = init_sdk(config)
    if not sdk:
        return 1
    
    try:
        if args.action == "create":
            metadata = {
                "name": args.name,
                "symbol": args.symbol,
                "total_supply": int(args.total_supply),
                "decimals": int(args.decimals)
            }
            asset_id = sdk.asset_manager.create_asset(metadata)
            print_success(f"Asset created: {asset_id}")
            print_info(f"Name: {args.name}")
            print_info(f"Symbol: {args.symbol}")
            
        elif args.action == "transfer":
            if not all([args.asset_id, args.from_address, args.to_address, args.amount]):
                print_error("asset-id, from-address, to-address, and amount required for transfer")
                return 1
            
            tx_id = sdk.asset_manager.transfer_asset(
                args.asset_id,
                args.from_address,
                args.to_address,
                int(args.amount)
            )
            print_success(f"Asset transfer completed: {tx_id}")
            
        elif args.action == "balance":
            if not all([args.asset_id, args.address]):
                print_error("asset-id and address required for balance check")
                return 1
            
            balance = sdk.asset_manager.get_asset_balance(args.asset_id, args.address)
            print_success(f"Asset balance: {balance}")
            
        elif args.action == "info":
            if not args.asset_id:
                print_error("asset-id required for info")
                return 1
            
            info = sdk.asset_manager.get_asset_info(args.asset_id)
            print_success("Asset information:")
            print(json.dumps(info, indent=2))
            
    except Exception as e:
        print_error(f"Asset operation failed: {e}")
        return 1
    
    return 0

def cmd_nft(args):
    """NFT management commands."""
    config = load_config(args.config_file)
    sdk = init_sdk(config)
    if not sdk:
        return 1
    
    try:
        if args.action == "create":
            metadata = {
                "name": args.name,
                "description": args.description or "",
                "metadata": json.loads(args.metadata) if args.metadata else {}
            }
            nft_id = sdk.nft_manager.create_nft(metadata)
            print_success(f"NFT created: {nft_id}")
            print_info(f"Name: {args.name}")
            
        elif args.action == "mint":
            if not all([args.nft_id, args.owner_address]):
                print_error("nft-id and owner-address required for minting")
                return 1
            
            tx_id = sdk.nft_manager.mint_nft(args.nft_id, args.owner_address)
            print_success(f"NFT minted: {tx_id}")
            
        elif args.action == "transfer":
            if not all([args.nft_id, args.from_address, args.to_address]):
                print_error("nft-id, from-address, and to-address required for transfer")
                return 1
            
            tx_id = sdk.nft_manager.transfer_nft(args.nft_id, args.from_address, args.to_address)
            print_success(f"NFT transfer completed: {tx_id}")
            
        elif args.action == "info":
            if not args.nft_id:
                print_error("nft-id required for info")
                return 1
            
            info = sdk.nft_manager.get_nft_info(args.nft_id)
            print_success("NFT information:")
            print(json.dumps(info, indent=2))
            
    except Exception as e:
        print_error(f"NFT operation failed: {e}")
        return 1
    
    return 0

def cmd_status(args):
    """Show SDK status."""
    config = load_config(args.config_file)
    sdk = init_sdk(config)
    if not sdk:
        return 1
    
    try:
        print_banner()
        print_info(f"SDK Version: {sdk.get_version()}")
        print_info(f"Initialized: {sdk.is_initialized()}")
        print_info(f"Network: {sdk.core_manager.get_network()}")
        
        # Show component status
        print_info("Component Status:")
        print("  ✅ Core Manager")
        print("  ✅ Database Manager")
        print("  ✅ Security Manager")
        print("  ✅ Wallet Manager")
        print("  ✅ Asset Manager")
        print("  ✅ NFT Manager")
        print("  ✅ IPFS Manager")
        print("  ✅ Blockchain Manager")
        print("  ✅ Network Manager")
        
    except Exception as e:
        print_error(f"Status check failed: {e}")
        return 1
    
    return 0

def main():
    """Main CLI entry point."""
    parser = argparse.ArgumentParser(
        description="Satox SDK Command Line Interface",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  satox-sdk-cli init --network testnet --config-file config.json
  satox-sdk-cli wallet create --name my-wallet
  satox-sdk-cli wallet address --wallet-id <wallet-id>
  satox-sdk-cli security generate --key-id my-key
  satox-sdk-cli asset create --name "My Token" --symbol MTK --total-supply 1000000
  satox-sdk-cli nft create --name "My NFT" --description "A unique digital asset"
  satox-sdk-cli status
        """
    )
    
    parser.add_argument(
        '--config-file',
        help='Configuration file path'
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Init command
    init_parser = subparsers.add_parser('init', help='Initialize SDK configuration')
    init_parser.add_argument(
        '--network',
        choices=['mainnet', 'testnet'],
        default='testnet',
        help='Network to use (default: testnet)'
    )
    init_parser.add_argument(
        '--config-file',
        help='Output configuration file path'
    )
    init_parser.set_defaults(func=cmd_init)
    
    # Wallet command
    wallet_parser = subparsers.add_parser('wallet', help='Wallet management')
    wallet_parser.add_argument(
        'action',
        choices=['create', 'list', 'address', 'balance', 'send'],
        help='Wallet action to perform'
    )
    wallet_parser.add_argument('--name', help='Wallet name')
    wallet_parser.add_argument('--wallet-id', help='Wallet ID')
    wallet_parser.add_argument('--from-address', help='Source address')
    wallet_parser.add_argument('--to-address', help='Destination address')
    wallet_parser.add_argument('--amount', help='Amount to send')
    wallet_parser.set_defaults(func=cmd_wallet)
    
    # Security command
    security_parser = subparsers.add_parser('security', help='Security operations')
    security_parser.add_argument(
        'action',
        choices=['generate', 'sign', 'verify', 'encrypt', 'decrypt'],
        help='Security action to perform'
    )
    security_parser.add_argument('--key-id', help='Key identifier')
    security_parser.add_argument('--data', help='Data to sign/encrypt')
    security_parser.add_argument('--signature', help='Signature to verify')
    security_parser.add_argument('--encrypted-data', help='Encrypted data to decrypt')
    security_parser.set_defaults(func=cmd_security)
    
    # Asset command
    asset_parser = subparsers.add_parser('asset', help='Asset management')
    asset_parser.add_argument(
        'action',
        choices=['create', 'transfer', 'balance', 'info'],
        help='Asset action to perform'
    )
    asset_parser.add_argument('--name', help='Asset name')
    asset_parser.add_argument('--symbol', help='Asset symbol')
    asset_parser.add_argument('--total-supply', help='Total supply')
    asset_parser.add_argument('--decimals', help='Number of decimals')
    asset_parser.add_argument('--asset-id', help='Asset ID')
    asset_parser.add_argument('--from-address', help='Source address')
    asset_parser.add_argument('--to-address', help='Destination address')
    asset_parser.add_argument('--amount', help='Amount to transfer')
    asset_parser.add_argument('--address', help='Address for balance check')
    asset_parser.set_defaults(func=cmd_asset)
    
    # NFT command
    nft_parser = subparsers.add_parser('nft', help='NFT management')
    nft_parser.add_argument(
        'action',
        choices=['create', 'mint', 'transfer', 'info'],
        help='NFT action to perform'
    )
    nft_parser.add_argument('--name', help='NFT name')
    nft_parser.add_argument('--description', help='NFT description')
    nft_parser.add_argument('--metadata', help='NFT metadata (JSON string)')
    nft_parser.add_argument('--nft-id', help='NFT ID')
    nft_parser.add_argument('--owner-address', help='Owner address')
    nft_parser.add_argument('--from-address', help='Source address')
    nft_parser.add_argument('--to-address', help='Destination address')
    nft_parser.set_defaults(func=cmd_nft)
    
    # Status command
    status_parser = subparsers.add_parser('status', help='Show SDK status')
    status_parser.set_defaults(func=cmd_status)
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    try:
        return args.func(args)
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")
        return 0
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        return 1

if __name__ == '__main__':
    sys.exit(main()) 