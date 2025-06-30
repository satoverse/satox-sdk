/**
 * @file game_state.hpp
 * @brief Satox satox-sdk - Game State
 * @details This header provides Game State functionality for the Satox satox-sdk.
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
 * @file game_state.hpp
 * @brief Satox satox-sdk - Game State
 * @details This header provides Game State functionality for the Satox satox-sdk.
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
#include <vector>
#include <unordered_map>
#include <chrono>
#include <any>

/** @ingroup game */
namespace satox {
/** @ingroup game */
namespace game {

/** @ingroup game */
class Player;
/** @ingroup game */
class GameAsset;

/**
 * @brief Represents the state of a game
 * 
 * The GameState class manages the current state of a game,
 * including world state, player positions, and game data.
 */
/** @ingroup game */
class GameState {
public:
    /**
     * @brief Game state types
     */
/** @ingroup game */
    enum class Type {
        SINGLE_PLAYER,
        MULTIPLAYER,
        COOPERATIVE,
        COMPETITIVE,
        TOURNAMENT,
        PRACTICE
    };

    /**
     * @brief Constructor
     * @param game_id Unique game identifier
     * @param game_type Type of game
     */
    GameState(const std::string& game_id, Type game_type);

    /**
     * @brief Destructor
     */
    ~GameState();

    /**
     * @brief Get the game ID
     * @return Game identifier
     */
    const std::string& get_id() const;

    /**
     * @brief Get the game type
     * @return Game type
     */
    Type get_type() const;

    /**
     * @brief Get the game name
     * @return Game name
     */
    const std::string& get_name() const;

    /**
     * @brief Set the game name
     * @param name New game name
     */
    void set_name(const std::string& name);

    /**
     * @brief Get the game description
     * @return Game description
     */
    const std::string& get_description() const;

    /**
     * @brief Set the game description
     * @param description New game description
     */
    void set_description(const std::string& description);

    /**
     * @brief Get the game version
     * @return Game version
     */
    const std::string& get_version() const;

    /**
     * @brief Set the game version
     * @param version New game version
     */
    void set_version(const std::string& version);

    /**
     * @brief Get the game creation time
     * @return Creation timestamp
     */
    std::chrono::system_clock::time_point get_created_at() const;

    /**
     * @brief Get the game start time
     * @return Start timestamp
     */
    std::chrono::system_clock::time_point get_started_at() const;

    /**
     * @brief Get the game end time
     * @return End timestamp, or epoch if not ended
     */
    std::chrono::system_clock::time_point get_ended_at() const;

    /**
     * @brief Get the game duration
     * @return Game duration in seconds
     */
    std::chrono::seconds get_duration() const;

    /**
     * @brief Start the game
     * @return true if game started successfully
     */
    bool start();

    /**
     * @brief End the game
     * @return true if game ended successfully
     */
    bool end();

    /**
     * @brief Check if game is active
     * @return true if game is active
     */
    bool is_active() const;

    /**
     * @brief Check if game is ended
     * @return true if game is ended
     */
    bool is_ended() const;

    /**
     * @brief Get all players in the game
     * @return Vector of players
     */
    std::vector<std::shared_ptr<Player>> get_players() const;

    /**
     * @brief Add player to the game
     * @param player Player to add
     * @return true if player added successfully
     */
    bool add_player(std::shared_ptr<Player> player);

    /**
     * @brief Remove player from the game
     * @param player_id Player ID to remove
     * @return true if player removed successfully
     */
    bool remove_player(const std::string& player_id);

    /**
     * @brief Get player by ID
     * @param player_id Player ID
     * @return Player, or nullptr if not found
     */
    std::shared_ptr<Player> get_player(const std::string& player_id) const;

    /**
     * @brief Check if player is in the game
     * @param player_id Player ID
     * @return true if player is in the game
     */
    bool has_player(const std::string& player_id) const;

    /**
     * @brief Get player count
     * @return Number of players in the game
     */
    size_t get_player_count() const;

    /**
     * @brief Get maximum player count
     * @return Maximum number of players allowed
     */
    size_t get_max_players() const;

    /**
     * @brief Set maximum player count
     * @param max_players New maximum player count
     */
    void set_max_players(size_t max_players);

    /**
     * @brief Get all assets in the game
     * @return Vector of game assets
     */
    std::vector<std::shared_ptr<GameAsset>> get_assets() const;

    /**
     * @brief Add asset to the game
     * @param asset Asset to add
     * @return true if asset added successfully
     */
    bool add_asset(std::shared_ptr<GameAsset> asset);

    /**
     * @brief Remove asset from the game
     * @param asset_id Asset ID to remove
     * @return true if asset removed successfully
     */
    bool remove_asset(const std::string& asset_id);

    /**
     * @brief Get asset by ID
     * @param asset_id Asset ID
     * @return Asset, or nullptr if not found
     */
    std::shared_ptr<GameAsset> get_asset(const std::string& asset_id) const;

    /**
     * @brief Check if asset is in the game
     * @param asset_id Asset ID
     * @return true if asset is in the game
     */
    bool has_asset(const std::string& asset_id) const;

    /**
     * @brief Get asset count
     * @return Number of assets in the game
     */
    size_t get_asset_count() const;

    /**
     * @brief Get game data
     * @param key Data key
     * @return Data value, or empty string if not found
     */
    std::string get_data(const std::string& key) const;

    /**
     * @brief Set game data
     * @param key Data key
     * @param value Data value
     */
    void set_data(const std::string& key, const std::string& value);

    /**
     * @brief Check if game has data
     * @param key Data key
     * @return true if data exists
     */
    bool has_data(const std::string& key) const;

    /**
     * @brief Remove game data
     * @param key Data key to remove
     * @return true if data was removed
     */
    bool remove_data(const std::string& key);

    /**
     * @brief Get all game data
     * @return Map of all game data
     */
    std::unordered_map<std::string, std::string> get_all_data() const;

    /**
     * @brief Get game state value
     * @param key State key
     * @return State value as any type
     */
    std::any get_state_value(const std::string& key) const;

    /**
     * @brief Set game state value
     * @param key State key
     * @param value State value
     */
    void set_state_value(const std::string& key, const std::any& value);

    /**
     * @brief Check if game has state value
     * @param key State key
     * @return true if state value exists
     */
    bool has_state_value(const std::string& key) const;

    /**
     * @brief Remove game state value
     * @param key State key to remove
     * @return true if state value was removed
     */
    bool remove_state_value(const std::string& key);

    /**
     * @brief Get all state values
     * @return Map of all state values
     */
    std::unordered_map<std::string, std::any> get_all_state_values() const;

    /**
     * @brief Get game statistics
     * @return Map of game statistics
     */
    std::unordered_map<std::string, int> get_statistics() const;

    /**
     * @brief Set game statistic
     * @param stat_name Statistic name
     * @param value Statistic value
     */
    void set_statistic(const std::string& stat_name, int value);

    /**
     * @brief Increment game statistic
     * @param stat_name Statistic name
     * @param increment Amount to increment
     */
    void increment_statistic(const std::string& stat_name, int increment = 1);

    /**
     * @brief Get game score
     * @return Game score
     */
    int get_score() const;

    /**
     * @brief Set game score
     * @param score New game score
     */
    void set_score(int score);

    /**
     * @brief Add to game score
     * @param points Points to add
     */
    void add_score(int points);

    /**
     * @brief Get game level
     * @return Game level
     */
    int get_level() const;

    /**
     * @brief Set game level
     * @param level New game level
     */
    void set_level(int level);

    /**
     * @brief Save game state
     * @return true if save successful
     */
    bool save();

    /**
     * @brief Load game state
     * @return true if load successful
     */
    bool load();

    /**
     * @brief Get game state summary
     * @return Game state summary as string
     */
    std::string get_summary() const;

    /**
     * @brief Reset game state
     * @return true if reset successful
     */
    bool reset();

private:
/** @ingroup game */
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace game
} // namespace satox 
