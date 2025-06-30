/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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
#include <nlohmann/json.hpp>
#include "satox/asset/asset_manager.hpp"

namespace py = pybind11;
using namespace satox::asset;

// Helper: Convert AssetMetadata <-> dict
AssetManager::AssetMetadata dict_to_metadata(const py::dict& d) {
    AssetManager::AssetMetadata m;
    m.name = d["name"].cast<std::string>();
    m.symbol = d["symbol"].cast<std::string>();
    m.description = d["description"].cast<std::string>();
    m.ipfs_hash = d["ipfs_hash"].cast<std::string>();
    m.decimals = d["decimals"].cast<uint8_t>();
    m.total_supply = d["total_supply"].cast<uint64_t>();
    m.reissuable = d["reissuable"].cast<bool>();
    if (d.contains("custom_data"))
        m.custom_data = d["custom_data"].cast<std::map<std::string, std::string>>();
    return m;
}

py::dict metadata_to_dict(const AssetManager::AssetMetadata& m) {
    py::dict d;
    d["name"] = m.name;
    d["symbol"] = m.symbol;
    d["description"] = m.description;
    d["ipfs_hash"] = m.ipfs_hash;
    d["decimals"] = m.decimals;
    d["total_supply"] = m.total_supply;
    d["reissuable"] = m.reissuable;
    d["custom_data"] = m.custom_data;
    return d;
}

PYBIND11_MODULE(satox_asset, m) {
    m.doc() = "Python bindings for Satox AssetManager";

    py::enum_<AssetManager::AssetType>(m, "AssetType")
        .value("MAIN_ASSET", AssetManager::AssetType::MAIN_ASSET)
        .value("SUB_ASSET", AssetManager::AssetType::SUB_ASSET)
        .value("UNIQUE_ASSET", AssetManager::AssetType::UNIQUE_ASSET)
        .value("RESTRICTED_ASSET", AssetManager::AssetType::RESTRICTED_ASSET);

    py::class_<AssetManager>(m, "AssetManager")
        .def_static("getInstance", &AssetManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &AssetManager::initialize)
        .def("isInitialized", &AssetManager::isInitialized)
        .def("getLastError", &AssetManager::getLastError)
        .def("createAsset", [](AssetManager& self, py::dict req) {
            AssetManager::AssetCreationRequest r;
            r.owner_address = req["owner_address"].cast<std::string>();
            r.type = req["type"].cast<AssetManager::AssetType>();
            r.metadata = dict_to_metadata(req["metadata"].cast<py::dict>());
            r.initial_supply = req["initial_supply"].cast<uint64_t>();
            r.reissuable = req["reissuable"].cast<bool>();
            r.admin_addresses = req["admin_addresses"].cast<std::vector<std::string>>();
            r.restricted_addresses = req["restricted_addresses"].cast<std::vector<std::string>>();
            return self.createAsset(r);
        })
        .def("getAssetMetadata", [](AssetManager& self, const std::string& name) {
            AssetManager::AssetMetadata m;
            if (!self.getAssetMetadata(name, m))
                throw std::runtime_error(self.getLastError());
            return metadata_to_dict(m);
        });
} 