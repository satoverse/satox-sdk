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

// Include C API header
#include "satox_sdk.h"

namespace py = pybind11;

// Helper function to convert C++ exceptions to Python exceptions
void translate_exception(const std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

// Helper function to check C API return codes
void check_return_code(int result, const std::string& operation) {
    if (result != 0) {
        throw std::runtime_error("Operation failed: " + operation + " (error code: " + std::to_string(result) + ")");
    }
}

// Helper function to convert C string to Python string
py::str c_string_to_python(const char* c_str) {
    if (c_str == nullptr) {
        return py::str("");
    }
    return py::str(c_str);
}

// Helper function to convert Python string to C string
std::string python_string_to_c(const py::str& py_str) {
    return py_str.cast<std::string>();
}

// Helper function to free C string
void free_c_string(char* c_str) {
    if (c_str) {
        free(c_str);
    }
}

PYBIND11_MODULE(satox_native, m) {
    m.doc() = "Satox SDK Python Native Bindings"; // Optional module docstring
    
    // Add version info
    m.attr("__version__") = "1.0.0";
    m.attr("__author__") = "Satoxcoin Core Developer";
    
    // Register exception translator
    py::register_exception_translator([](std::exception_ptr p) {
        try {
            if (p) std::rethrow_exception(p);
        } catch (const std::exception& e) {
            PyErr_SetString(PyExc_RuntimeError, e.what());
        }
    });
    
    // SDK initialization and shutdown
    m.def("initialize", []() {
        int result = satox_sdk_initialize();
        check_return_code(result, "SDK initialization");
    }, "Initialize the Satox SDK");
    
    m.def("shutdown", []() {
        satox_sdk_shutdown();
    }, "Shutdown the Satox SDK");
    
    m.def("get_version", []() {
        const char* version = satox_sdk_get_version();
        return c_string_to_python(version);
    }, "Get the SDK version");
    
    // Database Manager bindings
    py::class_<void>(m, "DatabaseManager")
        .def_static("create", [](const py::str& config_json) {
            std::string c_config = python_string_to_c(config_json);
            return satox_database_manager_create(c_config.c_str());
        })
        .def("destroy", [](void* handle) {
            satox_database_manager_destroy(handle);
        })
        .def("create_database", [](void* handle, const py::str& name) {
            std::string c_name = python_string_to_c(name);
            int result = satox_database_manager_create_database(handle, c_name.c_str());
            check_return_code(result, "Create database");
        })
        .def("list_databases", [](void* handle) {
            char* result_str = satox_database_manager_list_databases(handle);
            py::str py_result = c_string_to_python(result_str);
            free_c_string(result_str);
            return py_result;
        });
    
    // Security Manager bindings
    py::class_<void>(m, "SecurityManager")
        .def_static("create", []() {
            return satox_security_manager_create();
        })
        .def("destroy", [](void* handle) {
            satox_security_manager_destroy(handle);
        })
        .def("generate_keypair", [](void* handle) {
            char* public_key = nullptr;
            char* private_key = nullptr;
            int result = satox_security_manager_generate_keypair(handle, &public_key, &private_key);
            check_return_code(result, "Generate keypair");
            
            py::dict keypair;
            keypair["public_key"] = c_string_to_python(public_key);
            keypair["private_key"] = c_string_to_python(private_key);
            
            free_c_string(public_key);
            free_c_string(private_key);
            
            return keypair;
        })
        .def("sign_data", [](void* handle, const py::str& data, const py::str& private_key) {
            std::string c_data = python_string_to_c(data);
            std::string c_private_key = python_string_to_c(private_key);
            char* signature = satox_security_manager_sign_data(handle, c_data.c_str(), c_private_key.c_str());
            py::str py_signature = c_string_to_python(signature);
            free_c_string(signature);
            return py_signature;
        })
        .def("verify_signature", [](void* handle, const py::str& data, const py::str& signature, const py::str& public_key) {
            std::string c_data = python_string_to_c(data);
            std::string c_signature = python_string_to_c(signature);
            std::string c_public_key = python_string_to_c(public_key);
            int result = satox_security_manager_verify_signature(handle, c_data.c_str(), c_signature.c_str(), c_public_key.c_str());
            return result == 0;
        })
        .def("sha256", [](void* handle, const py::str& data) {
            std::string c_data = python_string_to_c(data);
            char* hash = satox_security_manager_sha256(handle, c_data.c_str());
            py::str py_hash = c_string_to_python(hash);
            free_c_string(hash);
            return py_hash;
        })
        .def("encrypt_data", [](void* handle, const py::str& data, const py::str& public_key) {
            std::string c_data = python_string_to_c(data);
            std::string c_public_key = python_string_to_c(public_key);
            char* encrypted = satox_security_manager_encrypt_data(handle, c_data.c_str(), c_public_key.c_str());
            py::str py_encrypted = c_string_to_python(encrypted);
            free_c_string(encrypted);
            return py_encrypted;
        })
        .def("decrypt_data", [](void* handle, const py::str& encrypted_data, const py::str& private_key) {
            std::string c_encrypted_data = python_string_to_c(encrypted_data);
            std::string c_private_key = python_string_to_c(private_key);
            char* decrypted = satox_security_manager_decrypt_data(handle, c_encrypted_data.c_str(), c_private_key.c_str());
            py::str py_decrypted = c_string_to_python(decrypted);
            free_c_string(decrypted);
            return py_decrypted;
        });
    
    // Asset Manager bindings
    py::class_<void>(m, "AssetManager")
        .def_static("create", []() {
            return satox_asset_manager_create();
        })
        .def("destroy", [](void* handle) {
            satox_asset_manager_destroy(handle);
        })
        .def("create_asset", [](void* handle, const py::str& config_json, const py::str& owner_address) {
            std::string c_config = python_string_to_c(config_json);
            std::string c_owner_address = python_string_to_c(owner_address);
            char* asset_id = satox_asset_manager_create_asset(handle, c_config.c_str(), c_owner_address.c_str());
            py::str py_asset_id = c_string_to_python(asset_id);
            free_c_string(asset_id);
            return py_asset_id;
        })
        .def("get_asset_info", [](void* handle, const py::str& asset_id) {
            std::string c_asset_id = python_string_to_c(asset_id);
            char* info = satox_asset_manager_get_asset_info(handle, c_asset_id.c_str());
            py::str py_info = c_string_to_python(info);
            free_c_string(info);
            return py_info;
        })
        .def("transfer_asset", [](void* handle, const py::str& asset_id, const py::str& from_address, const py::str& to_address, uint64_t amount) {
            std::string c_asset_id = python_string_to_c(asset_id);
            std::string c_from_address = python_string_to_c(from_address);
            std::string c_to_address = python_string_to_c(to_address);
            char* txid = satox_asset_manager_transfer_asset(handle, c_asset_id.c_str(), c_from_address.c_str(), c_to_address.c_str(), amount);
            py::str py_txid = c_string_to_python(txid);
            free_c_string(txid);
            return py_txid;
        })
        .def("get_asset_balance", [](void* handle, const py::str& asset_id, const py::str& address) {
            std::string c_asset_id = python_string_to_c(asset_id);
            std::string c_address = python_string_to_c(address);
            unsigned long long balance = 0;
            int result = satox_asset_manager_get_asset_balance(handle, c_asset_id.c_str(), c_address.c_str(), &balance);
            check_return_code(result, "Get asset balance");
            return balance;
        })
        .def("reissue_asset", [](void* handle, const py::str& asset_id, const py::str& owner_address, uint64_t amount) {
            std::string c_asset_id = python_string_to_c(asset_id);
            std::string c_owner_address = python_string_to_c(owner_address);
            char* txid = satox_asset_manager_reissue_asset(handle, c_asset_id.c_str(), c_owner_address.c_str(), amount);
            py::str py_txid = c_string_to_python(txid);
            free_c_string(txid);
            return py_txid;
        })
        .def("burn_asset", [](void* handle, const py::str& asset_id, const py::str& owner_address, uint64_t amount) {
            std::string c_asset_id = python_string_to_c(asset_id);
            std::string c_owner_address = python_string_to_c(owner_address);
            char* txid = satox_asset_manager_burn_asset(handle, c_asset_id.c_str(), c_owner_address.c_str(), amount);
            py::str py_txid = c_string_to_python(txid);
            free_c_string(txid);
            return py_txid;
        });
    
    // NFT Manager bindings
    py::class_<void>(m, "NFTManager")
        .def_static("create", []() {
            return satox_nft_manager_create();
        })
        .def("destroy", [](void* handle) {
            satox_nft_manager_destroy(handle);
        })
        .def("create_nft", [](void* handle, const py::str& asset_id, const py::str& metadata_json, const py::str& owner_address) {
            std::string c_asset_id = python_string_to_c(asset_id);
            std::string c_metadata_json = python_string_to_c(metadata_json);
            std::string c_owner_address = python_string_to_c(owner_address);
            char* nft_id = satox_nft_manager_create_nft(handle, c_asset_id.c_str(), c_metadata_json.c_str(), c_owner_address.c_str());
            py::str py_nft_id = c_string_to_python(nft_id);
            free_c_string(nft_id);
            return py_nft_id;
        })
        .def("get_nft_info", [](void* handle, const py::str& nft_id) {
            std::string c_nft_id = python_string_to_c(nft_id);
            char* info = satox_nft_manager_get_nft_info(handle, c_nft_id.c_str());
            py::str py_info = c_string_to_python(info);
            free_c_string(info);
            return py_info;
        })
        .def("transfer_nft", [](void* handle, const py::str& nft_id, const py::str& from_address, const py::str& to_address) {
            std::string c_nft_id = python_string_to_c(nft_id);
            std::string c_from_address = python_string_to_c(from_address);
            std::string c_to_address = python_string_to_c(to_address);
            char* txid = satox_nft_manager_transfer_nft(handle, c_nft_id.c_str(), c_from_address.c_str(), c_to_address.c_str());
            py::str py_txid = c_string_to_python(txid);
            free_c_string(txid);
            return py_txid;
        })
        .def("get_nfts_by_owner", [](void* handle, const py::str& address) {
            std::string c_address = python_string_to_c(address);
            char* nfts = satox_nft_manager_get_nfts_by_owner(handle, c_address.c_str());
            py::str py_nfts = c_string_to_python(nfts);
            free_c_string(nfts);
            return py_nfts;
        })
        .def("update_metadata", [](void* handle, const py::str& nft_id, const py::str& metadata_json, const py::str& owner_address) {
            std::string c_nft_id = python_string_to_c(nft_id);
            std::string c_metadata_json = python_string_to_c(metadata_json);
            std::string c_owner_address = python_string_to_c(owner_address);
            char* txid = satox_nft_manager_update_metadata(handle, c_nft_id.c_str(), c_metadata_json.c_str(), c_owner_address.c_str());
            py::str py_txid = c_string_to_python(txid);
            free_c_string(txid);
            return py_txid;
        });
    
    // Blockchain Manager bindings
    py::class_<void>(m, "BlockchainManager")
        .def_static("create", [](const py::str& config_json) {
            std::string c_config = python_string_to_c(config_json);
            return satox_blockchain_manager_create(c_config.c_str());
        })
        .def("destroy", [](void* handle) {
            satox_blockchain_manager_destroy(handle);
        })
        .def("get_block_height", [](void* handle) {
            unsigned long long height = 0;
            int result = satox_blockchain_manager_get_block_height(handle, &height);
            check_return_code(result, "Get block height");
            return height;
        })
        .def("get_block_info", [](void* handle, const py::str& hash) {
            std::string c_hash = python_string_to_c(hash);
            char* info = satox_blockchain_manager_get_block_info(handle, c_hash.c_str());
            py::str py_info = c_string_to_python(info);
            free_c_string(info);
            return py_info;
        })
        .def("get_transaction_info", [](void* handle, const py::str& txid) {
            std::string c_txid = python_string_to_c(txid);
            char* info = satox_blockchain_manager_get_transaction_info(handle, c_txid.c_str());
            py::str py_info = c_string_to_python(info);
            free_c_string(info);
            return py_info;
        })
        .def("get_best_block_hash", [](void* handle) {
            char* hash = satox_blockchain_manager_get_best_block_hash(handle);
            py::str py_hash = c_string_to_python(hash);
            free_c_string(hash);
            return py_hash;
        })
        .def("get_block_hash", [](void* handle, uint64_t height) {
            char* hash = satox_blockchain_manager_get_block_hash(handle, height);
            py::str py_hash = c_string_to_python(hash);
            free_c_string(hash);
            return py_hash;
        });
    
    // Network Manager bindings
    py::class_<void>(m, "NetworkManager")
        .def_static("create", [](const py::str& config_json) {
            std::string c_config = python_string_to_c(config_json);
            return satox_network_manager_create(c_config.c_str());
        })
        .def("destroy", [](void* handle) {
            satox_network_manager_destroy(handle);
        })
        .def("start", [](void* handle) {
            int result = satox_network_manager_start(handle);
            check_return_code(result, "Start network manager");
        })
        .def("stop", [](void* handle) {
            int result = satox_network_manager_stop(handle);
            check_return_code(result, "Stop network manager");
        })
        .def("get_peers", [](void* handle) {
            char* peers = satox_network_manager_get_peers(handle);
            py::str py_peers = c_string_to_python(peers);
            free_c_string(peers);
            return py_peers;
        })
        .def("get_stats", [](void* handle) {
            char* stats = satox_network_manager_get_stats(handle);
            py::str py_stats = c_string_to_python(stats);
            free_c_string(stats);
            return py_stats;
        })
        .def("add_peer", [](void* handle, const py::str& address, uint16_t port) {
            std::string c_address = python_string_to_c(address);
            int result = satox_network_manager_add_peer(handle, c_address.c_str(), port);
            check_return_code(result, "Add peer");
        })
        .def("remove_peer", [](void* handle, const py::str& address, uint16_t port) {
            std::string c_address = python_string_to_c(address);
            int result = satox_network_manager_remove_peer(handle, c_address.c_str(), port);
            check_return_code(result, "Remove peer");
        })
        .def("send_message", [](void* handle, const py::str& address, uint16_t port, const py::str& message) {
            std::string c_address = python_string_to_c(address);
            std::string c_message = python_string_to_c(message);
            int result = satox_network_manager_send_message(handle, c_address.c_str(), port, c_message.c_str());
            check_return_code(result, "Send message");
        });
    
    // IPFS Manager bindings
    py::class_<void>(m, "IPFSManager")
        .def_static("create", [](const py::str& config_json) {
            std::string c_config = python_string_to_c(config_json);
            return satox_ipfs_manager_create(c_config.c_str());
        })
        .def("destroy", [](void* handle) {
            satox_ipfs_manager_destroy(handle);
        })
        .def("upload_file", [](void* handle, const py::str& file_path) {
            std::string c_file_path = python_string_to_c(file_path);
            char* hash = satox_ipfs_manager_upload_file(handle, c_file_path.c_str());
            py::str py_hash = c_string_to_python(hash);
            free_c_string(hash);
            return py_hash;
        })
        .def("upload_data", [](void* handle, const py::bytes& data, const py::str& filename) {
            std::string c_data = data.cast<std::string>();
            std::string c_filename = python_string_to_c(filename);
            char* hash = satox_ipfs_manager_upload_data(handle, reinterpret_cast<const unsigned char*>(c_data.c_str()), c_data.length(), c_filename.c_str());
            py::str py_hash = c_string_to_python(hash);
            free_c_string(hash);
            return py_hash;
        })
        .def("download_file", [](void* handle, const py::str& hash, const py::str& output_path) {
            std::string c_hash = python_string_to_c(hash);
            std::string c_output_path = python_string_to_c(output_path);
            int result = satox_ipfs_manager_download_file(handle, c_hash.c_str(), c_output_path.c_str());
            check_return_code(result, "Download file");
        })
        .def("get_file_info", [](void* handle, const py::str& hash) {
            std::string c_hash = python_string_to_c(hash);
            char* info = satox_ipfs_manager_get_file_info(handle, c_hash.c_str());
            py::str py_info = c_string_to_python(info);
            free_c_string(info);
            return py_info;
        })
        .def("pin_file", [](void* handle, const py::str& hash) {
            std::string c_hash = python_string_to_c(hash);
            int result = satox_ipfs_manager_pin_file(handle, c_hash.c_str());
            check_return_code(result, "Pin file");
        })
        .def("unpin_file", [](void* handle, const py::str& hash) {
            std::string c_hash = python_string_to_c(hash);
            int result = satox_ipfs_manager_unpin_file(handle, c_hash.c_str());
            check_return_code(result, "Unpin file");
        })
        .def("get_gateway_url", [](void* handle, const py::str& hash) {
            std::string c_hash = python_string_to_c(hash);
            char* url = satox_ipfs_manager_get_gateway_url(handle, c_hash.c_str());
            py::str py_url = c_string_to_python(url);
            free_c_string(url);
            return py_url;
        });
}
