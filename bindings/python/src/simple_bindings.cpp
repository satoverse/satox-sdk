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

// Include the C API header
#include "satox_sdk.h"

namespace py = pybind11;

// Opaque structs for manager handles
struct SecurityManagerHandle {
    void* handle;
    SecurityManagerHandle(void* h) : handle(h) {}
};

struct AssetManagerHandle {
    void* handle;
    AssetManagerHandle(void* h) : handle(h) {}
};

struct NFTManagerHandle {
    void* handle;
    NFTManagerHandle(void* h) : handle(h) {}
};

PYBIND11_MODULE(_core, m) {
    m.doc() = "Satox SDK Python Bindings"; // Optional module docstring
    
    // Add version info
    m.attr("__version__") = "0.1.0";
    m.attr("__author__") = "Satox Team";
    
    // SDK initialization functions
    m.def("initialize", []() -> bool {
        return satox_sdk_initialize() == 0;
    }, "Initialize the Satox SDK");
    
    m.def("shutdown", []() {
        satox_sdk_shutdown();
    }, "Shutdown the Satox SDK");
    
    m.def("get_version", []() -> std::string {
        const char* version = satox_sdk_get_version();
        return version ? std::string(version) : "unknown";
    }, "Get the SDK version");
    
    // Security Manager bindings
    py::class_<SecurityManagerHandle>(m, "SecurityManager")
        .def_static("create", []() -> SecurityManagerHandle {
            return SecurityManagerHandle(satox_security_manager_create());
        })
        .def("destroy", [](SecurityManagerHandle& self) {
            if (self.handle) satox_security_manager_destroy(self.handle);
            self.handle = nullptr;
        })
        .def("generate_keypair", [](SecurityManagerHandle& self) -> py::tuple {
            if (!self.handle) return py::make_tuple("", "");
            
            char* public_key = nullptr;
            char* private_key = nullptr;
            int result = satox_security_manager_generate_keypair(self.handle, &public_key, &private_key);
            
            if (result == 0 && public_key && private_key) {
                std::string pub_key(public_key);
                std::string priv_key(private_key);
                // Free the C strings
                free(public_key);
                free(private_key);
                return py::make_tuple(pub_key, priv_key);
            }
            return py::make_tuple("", "");
        })
        .def("sign_data", [](SecurityManagerHandle& self, const std::string& data, const std::string& private_key) -> std::string {
            if (!self.handle) return "";
            
            char* result = satox_security_manager_sign_data(self.handle, data.c_str(), private_key.c_str());
            if (result) {
                std::string signature(result);
                free(result);
                return signature;
            }
            return "";
        })
        .def("verify_signature", [](SecurityManagerHandle& self, const std::string& data, const std::string& signature, const std::string& public_key) -> bool {
            if (!self.handle) return false;
            
            return satox_security_manager_verify_signature(self.handle, data.c_str(), signature.c_str(), public_key.c_str()) == 1;
        })
        .def("sha256", [](SecurityManagerHandle& self, const std::string& data) -> std::string {
            if (!self.handle) return "";
            
            char* result = satox_security_manager_sha256(self.handle, data.c_str());
            if (result) {
                std::string hash(result);
                free(result);
                return hash;
            }
            return "";
        });
    
    // Asset Manager bindings
    py::class_<AssetManagerHandle>(m, "AssetManager")
        .def_static("create", []() -> AssetManagerHandle {
            return AssetManagerHandle(satox_asset_manager_create());
        })
        .def("destroy", [](AssetManagerHandle& self) {
            if (self.handle) satox_asset_manager_destroy(self.handle);
            self.handle = nullptr;
        })
        .def("create_asset", [](AssetManagerHandle& self, const std::string& config_json, const std::string& owner_address) -> std::string {
            if (!self.handle) return "";
            
            char* result = satox_asset_manager_create_asset(self.handle, config_json.c_str(), owner_address.c_str());
            if (result) {
                std::string asset_info(result);
                free(result);
                return asset_info;
            }
            return "";
        })
        .def("get_asset_info", [](AssetManagerHandle& self, const std::string& asset_id) -> std::string {
            if (!self.handle) return "";
            
            char* result = satox_asset_manager_get_asset_info(self.handle, asset_id.c_str());
            if (result) {
                std::string asset_info(result);
                free(result);
                return asset_info;
            }
            return "";
        });
    
    // NFT Manager bindings
    py::class_<NFTManagerHandle>(m, "NFTManager")
        .def_static("create", []() -> NFTManagerHandle {
            return NFTManagerHandle(satox_nft_manager_create());
        })
        .def("destroy", [](NFTManagerHandle& self) {
            if (self.handle) satox_nft_manager_destroy(self.handle);
            self.handle = nullptr;
        })
        .def("create_nft", [](NFTManagerHandle& self, const std::string& asset_id, const std::string& metadata_json, const std::string& owner_address) -> std::string {
            if (!self.handle) return "";
            
            char* result = satox_nft_manager_create_nft(self.handle, asset_id.c_str(), metadata_json.c_str(), owner_address.c_str());
            if (result) {
                std::string nft_info(result);
                free(result);
                return nft_info;
            }
            return "";
        })
        .def("get_nft_info", [](NFTManagerHandle& self, const std::string& nft_id) -> std::string {
            if (!self.handle) return "";
            
            char* result = satox_nft_manager_get_nft_info(self.handle, nft_id.c_str());
            if (result) {
                std::string nft_info(result);
                free(result);
                return nft_info;
            }
            return "";
        });
} 