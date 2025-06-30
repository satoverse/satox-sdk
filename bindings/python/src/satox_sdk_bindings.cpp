#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

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

PYBIND11_MODULE(_satox_sdk, m) {
    m.doc() = "Satox SDK Python bindings"; // Optional module docstring

    // Module information
    m.attr("__version__") = "0.9.0";
    m.attr("__author__") = "Satox Team";

    // Core SDK functions
    m.def("initialize", &satox_sdk_initialize, "Initialize the Satox SDK");
    m.def("shutdown", &satox_sdk_shutdown, "Shutdown the Satox SDK");
    m.def("get_version", &satox_sdk_get_version, "Get the SDK version");

    // Core Manager bindings
    py::class_<CoreManager>(m, "CoreManager")
        .def(py::init<const std::string&>())
        .def("initialize", &CoreManager::initialize)
        .def("shutdown", &CoreManager::shutdown)
        .def("get_status", &CoreManager::get_status)
        .def("get_config", &CoreManager::get_config)
        .def("set_config", &CoreManager::set_config);

    // Wallet Manager bindings
    py::class_<WalletManager>(m, "WalletManager")
        .def(py::init<const std::string&>())
        .def("create_wallet", &WalletManager::create_wallet)
        .def("import_wallet", &WalletManager::import_wallet)
        .def("export_wallet", &WalletManager::export_wallet)
        .def("get_balance", &WalletManager::get_balance)
        .def("send_transaction", &WalletManager::send_transaction)
        .def("get_transaction_history", &WalletManager::get_transaction_history)
        .def("get_address", &WalletManager::get_address)
        .def("validate_address", &WalletManager::validate_address);

    // Asset Manager bindings
    py::class_<AssetManager>(m, "AssetManager")
        .def(py::init<const std::string&>())
        .def("create_asset", &AssetManager::create_asset)
        .def("get_asset_info", &AssetManager::get_asset_info)
        .def("transfer_asset", &AssetManager::transfer_asset)
        .def("get_asset_balance", &AssetManager::get_asset_balance)
        .def("reissue_asset", &AssetManager::reissue_asset)
        .def("burn_asset", &AssetManager::burn_asset)
        .def("list_assets", &AssetManager::list_assets)
        .def("get_asset_transactions", &AssetManager::get_asset_transactions);

    // Blockchain Manager bindings
    py::class_<BlockchainManager>(m, "BlockchainManager")
        .def(py::init<const std::string&>())
        .def("get_block_height", &BlockchainManager::get_block_height)
        .def("get_block_info", &BlockchainManager::get_block_info)
        .def("get_transaction_info", &BlockchainManager::get_transaction_info)
        .def("get_best_block_hash", &BlockchainManager::get_best_block_hash)
        .def("get_block_hash", &BlockchainManager::get_block_hash)
        .def("get_difficulty", &BlockchainManager::get_difficulty)
        .def("get_network_hashrate", &BlockchainManager::get_network_hashrate)
        .def("validate_transaction", &BlockchainManager::validate_transaction);

    // Database Manager bindings
    py::class_<DatabaseManager>(m, "DatabaseManager")
        .def(py::init<const std::string&>())
        .def("create_database", &DatabaseManager::create_database)
        .def("delete_database", &DatabaseManager::delete_database)
        .def("list_databases", &DatabaseManager::list_databases)
        .def("create_collection", &DatabaseManager::create_collection)
        .def("delete_collection", &DatabaseManager::delete_collection)
        .def("insert_document", &DatabaseManager::insert_document)
        .def("update_document", &DatabaseManager::update_document)
        .def("delete_document", &DatabaseManager::delete_document)
        .def("find_documents", &DatabaseManager::find_documents)
        .def("get_document", &DatabaseManager::get_document);

    // Security Manager bindings
    py::class_<SecurityManager>(m, "SecurityManager")
        .def(py::init<>())
        .def("generate_keypair", &SecurityManager::generate_keypair)
        .def("sign_data", &SecurityManager::sign_data)
        .def("verify_signature", &SecurityManager::verify_signature)
        .def("sha256", &SecurityManager::sha256)
        .def("sha512", &SecurityManager::sha512)
        .def("encrypt_data", &SecurityManager::encrypt_data)
        .def("decrypt_data", &SecurityManager::decrypt_data)
        .def("generate_random_bytes", &SecurityManager::generate_random_bytes)
        .def("hash_password", &SecurityManager::hash_password)
        .def("verify_password", &SecurityManager::verify_password);

    // Network Manager bindings
    py::class_<NetworkManager>(m, "NetworkManager")
        .def(py::init<const std::string&>())
        .def("start", &NetworkManager::start)
        .def("stop", &NetworkManager::stop)
        .def("get_peers", &NetworkManager::get_peers)
        .def("get_stats", &NetworkManager::get_stats)
        .def("add_peer", &NetworkManager::add_peer)
        .def("remove_peer", &NetworkManager::remove_peer)
        .def("send_message", &NetworkManager::send_message)
        .def("broadcast_message", &NetworkManager::broadcast_message)
        .def("get_connection_count", &NetworkManager::get_connection_count);

    // IPFS Manager bindings
    py::class_<IPFSManager>(m, "IPFSManager")
        .def(py::init<const std::string&>())
        .def("upload_file", &IPFSManager::upload_file)
        .def("upload_data", &IPFSManager::upload_data)
        .def("download_file", &IPFSManager::download_file)
        .def("get_file_info", &IPFSManager::get_file_info)
        .def("pin_file", &IPFSManager::pin_file)
        .def("unpin_file", &IPFSManager::unpin_file)
        .def("get_gateway_url", &IPFSManager::get_gateway_url)
        .def("list_pinned_files", &IPFSManager::list_pinned_files)
        .def("get_storage_stats", &IPFSManager::get_storage_stats);

    // Error handling
    py::register_exception<SatoxException>(m, "SatoxException");
    py::register_exception<InitializationError>(m, "InitializationError");
    py::register_exception<NetworkError>(m, "NetworkError");
    py::register_exception<DatabaseError>(m, "DatabaseError");
    py::register_exception<SecurityError>(m, "SecurityError");
    py::register_exception<AssetError>(m, "AssetError");
    py::register_exception<BlockchainError>(m, "BlockchainError");
    py::register_exception<IPFSError>(m, "IPFSError");
} 