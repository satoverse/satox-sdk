/**
 * @file player.hpp
 * @brief Satox satox-sdk - Player
 * @details This header provides Player functionality for the Satox satox-sdk.
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
 * @file player.hpp
 * @brief Satox satox-sdk - Player
 * @details This header provides Player functionality for the Satox satox-sdk.
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

/** @ingroup game */
namespace satox {
/** @ingroup game */
namespace game {

/** @ingroup game */
class GameAsset;
/** @ingroup game */
class GameSession;

/**
 * @brief Represents a player in the game
 * 
 * The Player class manages player data, inventory, and game sessions.
 */
/** @ingroup game */
class Player {
public:
    /**
     * @brief Constructor
     * @param player_id Unique player identifier
     */
    explicit Player(const std::string& player_id);

    /**
     * @brief Destructor
     */
    ~Player();

    /**
     * @brief Get the player ID
     * @return Player identifier
     */
    const std::string& get_id() const;

    /**
     * @brief Get the player name
     * @return Player name
     */
    const std::string& get_name() const;

    /**
     * @brief Set the player name
     * @param name New player name
     */
    void set_name(const std::string& name);

    /**
     * @brief Get player level
     * @return Player level
     */
    int get_level() const;

    /**
     * @brief Set player level
     * @param level New player level
     */
    void set_level(int level);

    /**
     * @brief Get player experience points
     * @return Experience points
     */
    int get_experience() const;

    /**
     * @brief Add experience points
     * @param exp Experience points to add
     */
    void add_experience(int exp);

    /**
     * @brief Get player health
     * @return Current health
     */
    int get_health() const;

    /**
     * @brief Set player health
     * @param health New health value
     */
    void set_health(int health);

    /**
     * @brief Get player maximum health
     * @return Maximum health
     */
    int get_max_health() const;

    /**
     * @brief Set player maximum health
     * @param max_health New maximum health
     */
    void set_max_health(int max_health);

    /**
     * @brief Get player gold/currency
     * @return Current gold amount
     */
    double get_gold() const;

    /**
     * @brief Add gold to player
     * @param amount Gold amount to add
     */
    void add_gold(double amount);

    /**
     * @brief Remove gold from player
     * @param amount Gold amount to remove
     * @return true if successful, false if insufficient funds
     */
    bool remove_gold(double amount);

    /**
     * @brief Add item to inventory
     * @param asset Game asset to add
     */
    void add_to_inventory(std::shared_ptr<GameAsset> asset);

    /**
     * @brief Remove item from inventory
     * @param asset_id Asset identifier to remove
     * @return true if successful, false if item not found
     */
    bool remove_from_inventory(const std::string& asset_id);

    /**
     * @brief Get inventory items
     * @return Vector of inventory assets
     */
    std::vector<std::shared_ptr<GameAsset>> get_inventory() const;

    /**
     * @brief Check if player has item in inventory
     * @param asset_id Asset identifier
     * @return true if player has the item
     */
    bool has_item(const std::string& asset_id) const;

    /**
     * @brief Get player statistics
     * @return Map of player statistics
     */
    std::unordered_map<std::string, int> get_statistics() const;

    /**
     * @brief Set player statistic
     * @param stat_name Statistic name
     * @param value Statistic value
     */
    void set_statistic(const std::string& stat_name, int value);

    /**
     * @brief Get player achievements
     * @return Vector of achievement names
     */
    std::vector<std::string> get_achievements() const;

    /**
     * @brief Add achievement to player
     * @param achievement_name Achievement name
     */
    void add_achievement(const std::string& achievement_name);

    /**
     * @brief Check if player has achievement
     * @param achievement_name Achievement name
     * @return true if player has the achievement
     */
    bool has_achievement(const std::string& achievement_name) const;

    /**
     * @brief Get player creation time
     * @return Creation timestamp
     */
    std::chrono::system_clock::time_point get_created_at() const;

    /**
     * @brief Get last login time
     * @return Last login timestamp
     */
    std::chrono::system_clock::time_point get_last_login() const;

    /**
     * @brief Update last login time
     */
    void update_last_login();

    /**
     * @brief Get active sessions
     * @return Vector of active game sessions
     */
    std::vector<std::shared_ptr<GameSession>> get_active_sessions() const;

    /**
     * @brief Add active session
     * @param session Game session to add
     */
    void add_active_session(std::shared_ptr<GameSession> session);

    /**
     * @brief Remove active session
     * @param session Game session to remove
     */
    void remove_active_session(std::shared_ptr<GameSession> session);

    /**
     * @brief Save player data
     * @return true if save successful
     */
    bool save();

    /**
     * @brief Load player data
     * @return true if load successful
     */
    bool load();

private:
/** @ingroup game */
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace game
} // namespace satox 
