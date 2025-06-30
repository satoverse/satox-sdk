# Satox SDK RPC Proxy Component

## Overview
The RPC Proxy component provides a thread-safe, configurable, and monitored proxy for forwarding JSON-RPC requests to blockchain nodes or other services. It is implemented as a Satox SDK core component, following all template and quality rules.

## Features
- Singleton pattern (`getInstance()`)
- Thread-safe operations
- Configurable endpoint, credentials, and timeouts
- Structured logging to `logs/components/rpc_proxy/`
- Health checks and statistics
- Callback/event support for errors and health
- Full test coverage

## Usage
```
#include "satox/rpc_proxy/rpc_proxy_manager.hpp"

satox::rpc_proxy::RpcProxyConfig config;
config.endpoint = "http://localhost:8545";
config.username = "user";
config.password = "pass";
config.timeout_ms = 1000;
config.enableLogging = true;
config.logPath = "logs/components/rpc_proxy/";

auto& proxy = satox::rpc_proxy::RpcProxyManager::getInstance();
proxy.initialize(config);

nlohmann::json req = {{"method", "ping"}, {"id", "1"}};
nlohmann::json resp;
if (proxy.sendRpcRequest(req, resp)) {
    // handle response
}
proxy.shutdown();
```

## Configuration
- `endpoint`: Target RPC server URL
- `username`, `password`: Optional credentials
- `timeout_ms`: Request timeout
- `enableLogging`: Enable/disable logging
- `logPath`: Log file directory

## Compliance
- Follows Satox SDK `component_templates.md`
- MIT License
- Copyright (c) 2025 Satoxcoin Core Developer 

## ⚠️ Limitations

### **Blockchain Compatibility**

- **No Smart Contract Support:** Satox is based on Ravencoin and does **not** support smart contracts or EVM compatibility
- **Asset-Based:** Focuses on asset creation, management, and transfer rather than programmable contracts
- **UTXO Model:** Uses Unspent Transaction Output (UTXO) model like Bitcoin/Ravencoin, not account-based like Ethereum

### **Platform Support**

- **Desktop Focus:** Primarily designed for desktop/server environments
- **Mobile Limitations:** Mobile SDK provides limited functionality compared to core SDK
- **Browser Support:** WebAssembly bindings have performance limitations for complex operations

### **Network Requirements**

- **Internet Connection:** Requires active internet connection for blockchain operations
- **Node Synchronization:** Full node operations require significant storage and bandwidth
- **API Dependencies:** Some features depend on external API services

### **Development Considerations**

- **Learning Curve:** Ravencoin/UTXO model differs from Ethereum/smart contract development
- **Testing Environment:** Testnet required for development and testing
- **Asset Management:** Asset creation and management have specific requirements and limitations

