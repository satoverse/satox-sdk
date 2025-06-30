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

PYBIND11_MODULE(_core, m) {
    m.doc() = "Satox SDK Python Bindings"; // Optional module docstring
    
    // Add version info
    m.attr("__version__") = "0.1.0";
    m.attr("__author__") = "Satox Team";
    
    // Core Manager bindings
    py::class_<satox::core::CoreManager>(m, "CoreManager")
        .def_static("getInstance", &satox::core::CoreManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::CoreManager::initialize)
        .def("shutdown", &satox::core::CoreManager::shutdown)
        .def("isRunning", &satox::core::CoreManager::isRunning)
        .def("getLastError", &satox::core::CoreManager::getLastError)
        .def("start", &satox::core::CoreManager::start)
        .def("stop", &satox::core::CoreManager::stop)
        .def("restartComponent", &satox::core::CoreManager::restartComponent)
        .def("updateConfig", &satox::core::CoreManager::updateConfig)
        .def("getConfig", &satox::core::CoreManager::getConfig)
        .def("validateConfig", &satox::core::CoreManager::validateConfig)
        .def("getSystemInfo", &satox::core::CoreManager::getSystemInfo)
        .def("registerErrorCallback", &satox::core::CoreManager::registerErrorCallback)
        .def("registerStatusCallback", &satox::core::CoreManager::registerStatusCallback)
        .def("registerShutdownCallback", &satox::core::CoreManager::registerShutdownCallback);
    
    // Core Config bindings
    py::class_<satox::core::CoreConfig>(m, "CoreConfig")
        .def(py::init<>())
        .def_readwrite("data_dir", &satox::core::CoreConfig::data_dir)
        .def_readwrite("network", &satox::core::CoreConfig::network)
        .def_readwrite("enable_mining", &satox::core::CoreConfig::enable_mining)
        .def_readwrite("enable_sync", &satox::core::CoreConfig::enable_sync)
        .def_readwrite("sync_interval_ms", &satox::core::CoreConfig::sync_interval_ms)
        .def_readwrite("mining_threads", &satox::core::CoreConfig::mining_threads)
        .def_readwrite("rpc_endpoint", &satox::core::CoreConfig::rpc_endpoint)
        .def_readwrite("rpc_username", &satox::core::CoreConfig::rpc_username)
        .def_readwrite("rpc_password", &satox::core::CoreConfig::rpc_password)
        .def_readwrite("timeout_ms", &satox::core::CoreConfig::timeout_ms)
        .def_readwrite("database", &satox::core::CoreConfig::database)
        .def_readwrite("network_config", &satox::core::CoreConfig::network_config)
        .def_readwrite("blockchain", &satox::core::CoreConfig::blockchain)
        .def_readwrite("wallet", &satox::core::CoreConfig::wallet)
        .def_readwrite("asset", &satox::core::CoreConfig::asset)
        .def_readwrite("ipfs", &satox::core::CoreConfig::ipfs);
    
    // Database Manager bindings - commented out due to private destructor
    /*
    py::class_<satox::core::DatabaseManager>(m, "DatabaseManager")
        .def_static("getInstance", &satox::core::DatabaseManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::DatabaseManager::initialize)
        .def("shutdown", &satox::core::DatabaseManager::shutdown)
        .def("isInitialized", &satox::core::DatabaseManager::isInitialized)
        .def("setConfig", &satox::core::DatabaseManager::setConfig)
        .def("getConfig", &satox::core::DatabaseManager::getConfig)
        .def("validateConfig", &satox::core::DatabaseManager::validateConfig)
        .def("createDatabase", &satox::core::DatabaseManager::createDatabase)
        .def("deleteDatabase", &satox::core::DatabaseManager::deleteDatabase)
        .def("useDatabase", &satox::core::DatabaseManager::useDatabase)
        .def("listDatabases", &satox::core::DatabaseManager::listDatabases)
        .def("databaseExists", &satox::core::DatabaseManager::databaseExists)
        .def("createTable", &satox::core::DatabaseManager::createTable)
        .def("deleteTable", &satox::core::DatabaseManager::deleteTable)
        .def("tableExists", &satox::core::DatabaseManager::tableExists)
        .def("listTables", &satox::core::DatabaseManager::listTables)
        .def("getTableSchema", &satox::core::DatabaseManager::getTableSchema)
        .def("insert", &satox::core::DatabaseManager::insert)
        .def("update", &satox::core::DatabaseManager::update)
        .def("remove", &satox::core::DatabaseManager::remove)
        .def("find", &satox::core::DatabaseManager::find)
        .def("query", &satox::core::DatabaseManager::query)
        .def("beginTransaction", &satox::core::DatabaseManager::beginTransaction)
        .def("commitTransaction", &satox::core::DatabaseManager::commitTransaction)
        .def("rollbackTransaction", &satox::core::DatabaseManager::rollbackTransaction)
        .def("isInTransaction", &satox::core::DatabaseManager::isInTransaction)
        .def("createIndex", &satox::core::DatabaseManager::createIndex)
        .def("dropIndex", &satox::core::DatabaseManager::dropIndex)
        .def("listIndexes", &satox::core::DatabaseManager::listIndexes)
        .def("createBackup", &satox::core::DatabaseManager::createBackup)
        .def("restoreFromBackup", &satox::core::DatabaseManager::restoreFromBackup)
        .def("connect", &satox::core::DatabaseManager::connect)
        .def("disconnect", &satox::core::DatabaseManager::disconnect)
        .def("isConnected", &satox::core::DatabaseManager::isConnected)
        .def("reconnect", &satox::core::DatabaseManager::reconnect)
        .def("setMaxConnections", &satox::core::DatabaseManager::setMaxConnections)
        .def("getMaxConnections", &satox::core::DatabaseManager::getMaxConnections)
        .def("setConnectionTimeout", &satox::core::DatabaseManager::setConnectionTimeout)
        .def("getConnectionTimeout", &satox::core::DatabaseManager::getConnectionTimeout)
        .def("setDatabaseCallback", &satox::core::DatabaseManager::setDatabaseCallback)
        .def("setConnectionCallback", &satox::core::DatabaseManager::setConnectionCallback)
        .def("setHealthCallback", &satox::core::DatabaseManager::setHealthCallback)
        .def("clearCallbacks", &satox::core::DatabaseManager::clearCallbacks)
        .def("getStats", &satox::core::DatabaseManager::getStats)
        .def("resetStats", &satox::core::DatabaseManager::resetStats)
        .def("getHealth", &satox::core::DatabaseManager::getHealth)
        .def("performHealthCheck", &satox::core::DatabaseManager::performHealthCheck)
        .def("getLastErrorCode", &satox::core::DatabaseManager::getLastErrorCode)
        .def("getLastError", &satox::core::DatabaseManager::getLastError)
        .def("clearLastError", &satox::core::DatabaseManager::clearLastError)
        .def("getVersion", &satox::core::DatabaseManager::getVersion);
    
    // Database Config bindings
    py::class_<satox::core::DatabaseConfig>(m, "DatabaseConfig")
        .def(py::init<>())
        .def_readwrite("name", &satox::core::DatabaseConfig::name)
        .def_readwrite("enableLogging", &satox::core::DatabaseConfig::enableLogging)
        .def_readwrite("logPath", &satox::core::DatabaseConfig::logPath)
        .def_readwrite("maxConnections", &satox::core::DatabaseConfig::maxConnections)
        .def_readwrite("connectionTimeout", &satox::core::DatabaseConfig::connectionTimeout)
        .def_readwrite("additionalConfig", &satox::core::DatabaseConfig::additionalConfig);
    */
    
    // Security Manager bindings
    py::class_<satox::core::SecurityManager>(m, "SecurityManager")
        .def_static("getInstance", &satox::core::SecurityManager::getInstance, py::return_value_policy::reference)
        .def("initialize", &satox::core::SecurityManager::initialize)
        .def("shutdown", &satox::core::SecurityManager::shutdown)
        .def("start", &satox::core::SecurityManager::start)
        .def("stop", &satox::core::SecurityManager::stop)
        .def("performOperation", &satox::core::SecurityManager::performOperation)
        .def("executeOperation", &satox::core::SecurityManager::executeOperation)
        .def("validateOperation", &satox::core::SecurityManager::validateOperation)
        .def("getState", &satox::core::SecurityManager::getState)
        .def("isInitialized", &satox::core::SecurityManager::isInitialized)
        .def("isRunning", &satox::core::SecurityManager::isRunning)
        .def("isHealthy", &satox::core::SecurityManager::isHealthy)
        .def("getConfig", &satox::core::SecurityManager::getConfig)
        .def("updateConfig", &satox::core::SecurityManager::updateConfig)
        .def("validateConfig", &satox::core::SecurityManager::validateConfig)
        .def("getStats", &satox::core::SecurityManager::getStats)
        .def("resetStats", &satox::core::SecurityManager::resetStats)
        .def("enableStats", &satox::core::SecurityManager::enableStats)
        .def("registerSecurityCallback", &satox::core::SecurityManager::registerSecurityCallback)
        .def("registerViolationCallback", &satox::core::SecurityManager::registerViolationCallback)
        .def("unregisterSecurityCallback", &satox::core::SecurityManager::unregisterSecurityCallback)
        .def("unregisterViolationCallback", &satox::core::SecurityManager::unregisterViolationCallback)
        .def("getLastError", &satox::core::SecurityManager::getLastError)
        .def("clearLastError", &satox::core::SecurityManager::clearLastError)
        .def("healthCheck", &satox::core::SecurityManager::healthCheck)
        .def("getHealthStatus", &satox::core::SecurityManager::getHealthStatus)
        .def("generatePQCKeyPair", &satox::core::SecurityManager::generatePQCKeyPair)
        .def("signWithPQC", &satox::core::SecurityManager::signWithPQC)
        .def("verifyWithPQC", &satox::core::SecurityManager::verifyWithPQC)
        .def("encryptWithPQC", &satox::core::SecurityManager::encryptWithPQC)
        .def("decryptWithPQC", &satox::core::SecurityManager::decryptWithPQC)
        .def("validateInput", &satox::core::SecurityManager::validateInput)
        .def("sanitizeInput", &satox::core::SecurityManager::sanitizeInput)
        .def("validateEmail", &satox::core::SecurityManager::validateEmail)
        .def("validateURL", &satox::core::SecurityManager::validateURL)
        .def("validateIPAddress", &satox::core::SecurityManager::validateIPAddress)
        .def("validateJson", &satox::core::SecurityManager::validateJson)
        .def("validateToken", &satox::core::SecurityManager::validateToken)
        .def("checkRateLimit", &satox::core::SecurityManager::checkRateLimit)
        .def("setRateLimit", &satox::core::SecurityManager::setRateLimit)
        .def("resetRateLimit", &satox::core::SecurityManager::resetRateLimit)
        .def("setPerformanceConfig", &satox::core::SecurityManager::setPerformanceConfig)
        .def("getCacheSize", &satox::core::SecurityManager::getCacheSize)
        .def("getConnectionPoolSize", &satox::core::SecurityManager::getConnectionPoolSize)
        .def("getBatchSize", &satox::core::SecurityManager::getBatchSize);
    
    // Security Config bindings
    py::class_<satox::core::SecurityConfig>(m, "SecurityConfig")
        .def(py::init<>())
        .def_readwrite("name", &satox::core::SecurityConfig::name)
        .def_readwrite("enablePQC", &satox::core::SecurityConfig::enablePQC)
        .def_readwrite("enableInputValidation", &satox::core::SecurityConfig::enableInputValidation)
        .def_readwrite("enableRateLimiting", &satox::core::SecurityConfig::enableRateLimiting)
        .def_readwrite("enableLogging", &satox::core::SecurityConfig::enableLogging)
        .def_readwrite("logPath", &satox::core::SecurityConfig::logPath)
        .def_readwrite("additionalConfig", &satox::core::SecurityConfig::additionalConfig);
} 