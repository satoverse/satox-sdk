/**
 * @file game_manager.hpp
 * @brief Satox satox-sdk - Game Manager
 * @details This header provides Game Manager functionality for the Satox satox-sdk.
 *
 * @defgroup game game Component
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
 * @file game_manager.hpp
 * @brief Satox satox-sdk - Game Manager
 * @details This header provides Game Manager functionality for the Satox satox-sdk.
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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

/** @ingroup game */
namespace satox {
/** @ingroup game */
namespace game {

// Forward declarations
/** @ingroup game */
class Player;
/** @ingroup game */
class GameSession;
/** @ingroup game */
class GameAsset;
/** @ingroup game */
class GameState;

/**
 * @brief Main game manager for the Satox Game SDK
 * 
 * The GameManager is the central coordinator for all game-related operations.
 * It manages players, sessions, assets, and game state.
 */
/** @ingroup game */
class GameManager {
public:
    /**
     * @brief Constructor
     */
    GameManager();

    /**
     * @brief Destructor
     */
    ~GameManager();

    /**
     * @brief Create a new player
     * @param player_id Unique player identifier
     * @return Shared pointer to the created player
     */
    std::shared_ptr<Player> create_player(const std::string& player_id);

    /**
     * @brief Get an existing player
     * @param player_id Player identifier
     * @return Shared pointer to the player, or nullptr if not found
     */
    std::shared_ptr<Player> get_player(const std::string& player_id);

    /**
     * @brief Create a new game session
     * @param player Player to create session for
     * @return Shared pointer to the created session
     */
    std::shared_ptr<GameSession> create_session(std::shared_ptr<Player> player);

    /**
     * @brief Create a new game asset
     * @param asset_id Unique asset identifier
     * @param asset_type Type of asset
     * @param value Asset value
     * @return Shared pointer to the created asset
     */
    std::shared_ptr<GameAsset> create_asset(const std::string& asset_id, 
                                           const std::string& asset_type, 
                                           double value);

    /**
     * @brief Get a game asset
     * @param asset_id Asset identifier
     * @return Shared pointer to the asset, or nullptr if not found
     */
    std::shared_ptr<GameAsset> get_asset(const std::string& asset_id);

    /**
     * @brief Create or get game state
     * @param game_id Game identifier
     * @return Shared pointer to the game state
     */
    std::shared_ptr<GameState> get_game_state(const std::string& game_id);

    /**
     * @brief Register a game event handler
     * @param event_type Type of event to handle
     * @param handler Function to handle the event
     */
    void register_event_handler(const std::string& event_type, 
                               std::function<void(const std::string&, const std::string&)> handler);

    /**
     * @brief Trigger a game event
     * @param event_type Type of event
     * @param game_id Game identifier
     * @param event_data Event data
     */
    void trigger_event(const std::string& event_type, 
                      const std::string& game_id, 
                      const std::string& event_data);

    /**
     * @brief Initialize the game manager
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Shutdown the game manager
     */
    void shutdown();

    /**
     * @brief Get the number of active players
     * @return Number of active players
     */
    size_t get_player_count() const;

    /**
     * @brief Get the number of active sessions
     * @return Number of active sessions
     */
    size_t get_session_count() const;

    /**
     * @brief Get the number of game assets
     * @return Number of game assets
     */
    size_t get_asset_count() const;

private:
/** @ingroup game */
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace game
} // namespace satox 
