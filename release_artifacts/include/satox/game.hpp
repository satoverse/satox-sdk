/**
 * @file game.hpp
 * @brief Satox satox-sdk - Game
 * @details This header provides Game functionality for the Satox satox-sdk.
 *
 * @defgroup satox-sdk satox-sdk Component
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
 * @file game.hpp
 * @brief Satox satox-sdk - Game
 * @details This header provides Game functionality for the Satox satox-sdk.
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

/**
 * @brief Satox Game SDK - Main Include Header
 * 
 * This header includes all the main components of the Satox Game SDK.
 * Include this header to get access to all game functionality.
 */

// Core game components
#include "satox/game/game_manager.hpp"
#include "satox/game/player.hpp"
#include "satox/game/game_asset.hpp"
#include "satox/game/game_session.hpp"
#include "satox/game/game_state.hpp"

// Game systems (to be added as they are implemented)
// #include "satox/game/multiplayer.hpp"
// #include "satox/game/economy.hpp"
// #include "satox/game/quest.hpp"
// #include "satox/game/combat.hpp"
// #include "satox/game/guild.hpp"
// #include "satox/game/crafting.hpp"
// #include "satox/game/inventory.hpp"

/** @ingroup satox-sdk */
namespace satox {
/** @ingroup satox-sdk */
namespace game {

/**
 * @brief Initialize the Game SDK
 * 
 * This function must be called before using any game functionality.
 * It initializes the game SDK and sets up necessary components.
 * 
 * @return true if initialization successful
 */
bool initialize_sdk();

/**
 * @brief Shutdown the Game SDK
 * 
 * This function should be called when the game SDK is no longer needed.
 * It cleans up resources and shuts down components.
 */
void shutdown_sdk();

/**
 * @brief Get the Game SDK version
 * 
 * @return SDK version string
 */
const char* get_sdk_version();

/**
 * @brief Get the Game SDK build information
 * 
 * @return Build information string
 */
const char* get_sdk_build_info();

} // namespace game
} // namespace satox 
