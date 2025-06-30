/*
 * MIT License
 *
 * Copyright (c) 2025 Satoxcoin Core Developer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>

// Include C++ SDK headers
#include "satox/satox.hpp"
#include "satox/core/core_manager.hpp"
#include "satox/core/database_manager.hpp"
#include "satox/core/blockchain_manager.hpp"
#include "satox/core/asset_manager.hpp"
#include "satox/core/nft_manager.hpp"
#include "satox/core/ipfs_manager.hpp"
#include "satox/core/network_manager.hpp"
#include "satox/core/security_manager.hpp"
#include "satox/core/wallet_manager.hpp"

namespace py = pybind11;

// Helper function to convert C++ exceptions to Python exceptions
void translate_exception(const std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

PYBIND11_MODULE(satox_native, m) {
    m.doc() = "Satox SDK Python Native Bindings"; // Optional module docstring
    
    // Add version info
    m.attr("__version__") = "1.0.0";
    m.attr("__author__") = "Satoxcoin Core Developer";
    
    // Register exception translator
    py::register_exception_translator<std::exception>(translate_exception);
    
    // SDK class bindings
    py::class_<satox::SDK>(m, "SDK")
        .def_static("getInstance", &satox::SDK::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::SDK::initialize, py::arg("config") = py::dict())
        .def("shutdown", &satox::SDK::shutdown)
        .def("getVersion", &satox::SDK::getVersion)
        .def("isInitialized", &satox::SDK::isInitialized)
        .def("getCoreManager", &satox::SDK::getCoreManager, py::return_value_policy::reference)
        .def("getDatabaseManager", &satox::SDK::getDatabaseManager, py::return_value_policy::reference)
        .def("getBlockchainManager", &satox::SDK::getBlockchainManager, py::return_value_policy::reference)
        .def("getAssetManager", &satox::SDK::getAssetManager, py::return_value_policy::reference)
        .def("getNFTManager", &satox::SDK::getNFTManager, py::return_value_policy::reference)
        .def("getIPFSManager", &satox::SDK::getIPFSManager, py::return_value_policy::reference)
        .def("getNetworkManager", &satox::SDK::getNetworkManager, py::return_value_policy::reference)
        .def("getSecurityManager", &satox::SDK::getSecurityManager, py::return_value_policy::reference)
        .def("getWalletManager", &satox::SDK::getWalletManager, py::return_value_policy::reference);
    
    // Core Manager bindings
    py::class_<satox::core::CoreManager>(m, "CoreManager")
        .def_static("getInstance", &satox::core::CoreManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::CoreManager::initialize)
        .def("shutdown", &satox::core::CoreManager::shutdown)
        .def("isInitialized", &satox::core::CoreManager::isInitialized)
        .def("getStats", &satox::core::CoreManager::getStats)
        .def("isHealthy", &satox::core::CoreManager::isHealthy);
    
    // Database Manager bindings
    py::class_<satox::core::DatabaseManager>(m, "DatabaseManager")
        .def_static("getInstance", &satox::core::DatabaseManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::DatabaseManager::initialize)
        .def("shutdown", &satox::core::DatabaseManager::shutdown)
        .def("isInitialized", &satox::core::DatabaseManager::isInitialized)
        .def("createDatabase", &satox::core::DatabaseManager::createDatabase)
        .def("deleteDatabase", &satox::core::DatabaseManager::deleteDatabase)
        .def("listDatabases", &satox::core::DatabaseManager::listDatabases)
        .def("getStats", &satox::core::DatabaseManager::getStats)
        .def("isHealthy", &satox::core::DatabaseManager::isHealthy);
    
    // Blockchain Manager bindings
    py::class_<satox::core::BlockchainManager>(m, "BlockchainManager")
        .def_static("getInstance", &satox::core::BlockchainManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::BlockchainManager::initialize)
        .def("shutdown", &satox::core::BlockchainManager::shutdown)
        .def("isInitialized", &satox::core::BlockchainManager::isInitialized)
        .def("getStats", &satox::core::BlockchainManager::getStats)
        .def("isHealthy", &satox::core::BlockchainManager::isHealthy);
    
    // Asset Manager bindings
    py::class_<satox::core::AssetManager>(m, "AssetManager")
        .def_static("getInstance", &satox::core::AssetManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::AssetManager::initialize)
        .def("shutdown", &satox::core::AssetManager::shutdown)
        .def("isInitialized", &satox::core::AssetManager::isInitialized)
        .def("createAsset", &satox::core::AssetManager::createAsset)
        .def("getStats", &satox::core::AssetManager::getStats)
        .def("isHealthy", &satox::core::AssetManager::isHealthy);
    
    // NFT Manager bindings
    py::class_<satox::core::NFTManager>(m, "NFTManager")
        .def_static("getInstance", &satox::core::NFTManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::NFTManager::initialize)
        .def("shutdown", &satox::core::NFTManager::shutdown)
        .def("isInitialized", &satox::core::NFTManager::isInitialized)
        .def("createNFT", &satox::core::NFTManager::createNFT)
        .def("getStats", &satox::core::NFTManager::getStats)
        .def("isHealthy", &satox::core::NFTManager::isHealthy);
    
    // IPFS Manager bindings
    py::class_<satox::core::IPFSManager>(m, "IPFSManager")
        .def_static("getInstance", &satox::core::IPFSManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::IPFSManager::initialize)
        .def("shutdown", &satox::core::IPFSManager::shutdown)
        .def("isInitialized", &satox::core::IPFSManager::isInitialized)
        .def("getStats", &satox::core::IPFSManager::getStats)
        .def("isHealthy", &satox::core::IPFSManager::isHealthy);
    
    // Network Manager bindings
    py::class_<satox::core::NetworkManager>(m, "NetworkManager")
        .def_static("getInstance", &satox::core::NetworkManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::NetworkManager::initialize)
        .def("shutdown", &satox::core::NetworkManager::shutdown)
        .def("isInitialized", &satox::core::NetworkManager::isInitialized)
        .def("getStats", &satox::core::NetworkManager::getStats)
        .def("isHealthy", &satox::core::NetworkManager::isHealthy);
    
    // Security Manager bindings
    py::class_<satox::core::SecurityManager>(m, "SecurityManager")
        .def_static("getInstance", &satox::core::SecurityManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::SecurityManager::initialize)
        .def("shutdown", &satox::core::SecurityManager::shutdown)
        .def("isInitialized", &satox::core::SecurityManager::isInitialized)
        .def("generatePQCKeyPair", &satox::core::SecurityManager::generatePQCKeyPair)
        .def("signWithPQC", &satox::core::SecurityManager::signWithPQC)
        .def("verifyWithPQC", &satox::core::SecurityManager::verifyWithPQC)
        .def("encryptWithPQC", &satox::core::SecurityManager::encryptWithPQC)
        .def("decryptWithPQC", &satox::core::SecurityManager::decryptWithPQC)
        .def("getStats", &satox::core::SecurityManager::getStats)
        .def("isHealthy", &satox::core::SecurityManager::isHealthy);
    
    // Wallet Manager bindings
    py::class_<satox::core::WalletManager>(m, "WalletManager")
        .def_static("getInstance", &satox::core::WalletManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::WalletManager::initialize)
        .def("shutdown", &satox::core::WalletManager::shutdown)
        .def("isInitialized", &satox::core::WalletManager::isInitialized)
        .def("createWallet", &satox::core::WalletManager::createWallet)
        .def("importWallet", &satox::core::WalletManager::importWallet)
        .def("getStats", &satox::core::WalletManager::getStats)
        .def("isHealthy", &satox::core::WalletManager::isHealthy);
    
    // Configuration classes - simplified versions
    py::class_<satox::core::CoreConfig>(m, "CoreConfig")
        .def(py::init<>());
    
    py::class_<satox::core::DatabaseConfig>(m, "DatabaseConfig")
        .def(py::init<>());
    
    py::class_<satox::core::BlockchainConfig>(m, "BlockchainConfig")
        .def(py::init<>());
    
    py::class_<satox::core::AssetConfig>(m, "AssetConfig")
        .def(py::init<>());
    
    py::class_<satox::core::NftConfig>(m, "NftConfig")
        .def(py::init<>());
    
    py::class_<satox::core::IPFSConfig>(m, "IPFSConfig")
        .def(py::init<>());
    
    py::class_<satox::core::NetworkConfig>(m, "NetworkConfig")
        .def(py::init<>());
    
    py::class_<satox::core::SecurityConfig>(m, "SecurityConfig")
        .def(py::init<>());
    
    py::class_<satox::core::WalletConfig>(m, "WalletConfig")
        .def(py::init<>());
} 