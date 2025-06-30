/**
 * @file blockchain.hpp
 * @brief Satox satox-sdk - Blockchain
 * @details This header provides Blockchain functionality for the Satox satox-sdk.
 *
 * @defgroup blockchain blockchain Component
 * @ingroup satox-sdk
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 *
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 *
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

/**
 * @file blockchain.hpp
 * @brief Satox satox-sdk - Blockchain
 * @details This header provides Blockchain functionality for the Satox satox-sdk.
 * 
 * @author Satox Development Team
 * @date 2025-06-23
 * @version 1.0.0
 * @copyright Copyright (c) 2024 Satox
 * 
 * @note This file is part of the Satox satox-sdk
 * @warning This is a public API header - changes may break compatibility
 * 
 * @see satox_sdk.h
 * @see https://github.com/satoverse/satox-satox-sdk
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

/** @ingroup blockchain */
namespace satox {
/** @ingroup blockchain */
namespace blockchain {

/** @ingroup blockchain */
class Blockchain {
public:
    Blockchain();
    ~Blockchain();

    // Core blockchain functionality
    bool initialize();
    bool addBlock(const std::string& data);
    bool validateChain();
    std::string getLatestBlockHash() const;

private:
/** @ingroup blockchain */
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace blockchain
} // namespace satox 
