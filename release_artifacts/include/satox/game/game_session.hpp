/**
 * @file game_session.hpp
 * @brief Satox satox-sdk - Game Session
 * @details This header provides Game Session functionality for the Satox satox-sdk.
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
 * @file game_session.hpp
 * @brief Satox satox-sdk - Game Session
 * @details This header provides Game Session functionality for the Satox satox-sdk.
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
#include <functional>

/** @ingroup game */
namespace satox {
/** @ingroup game */
namespace game {

/** @ingroup game */
class Player;
/** @ingroup game */
class GameState;

/**
 * @brief Represents a game session
 * 
 * The GameSession class manages an active game session for a player,
 * including game state, events, and session-specific data.
 */
/** @ingroup game */
class GameSession {
public:
    /**
     * @brief Session states
     */
/** @ingroup game */
    enum class State {
        CREATED,
        ACTIVE,
        PAUSED,
        ENDED,
        ERROR
    };

    /**
     * @brief Constructor
     * @param session_id Unique session identifier
     * @param player Player associated with this session
     */
    GameSession(const std::string& session_id, std::shared_ptr<Player> player);

    /**
     * @brief Destructor
     */
    ~GameSession();

    /**
     * @brief Get the session ID
     * @return Session identifier
     */
    const std::string& get_id() const;

    /**
     * @brief Get the associated player
     * @return Player associated with this session
     */
    std::shared_ptr<Player> get_player() const;

    /**
     * @brief Get the session state
     * @return Current session state
     */
    State get_state() const;

    /**
     * @brief Set the session state
     * @param state New session state
     */
    void set_state(State state);

    /**
     * @brief Get the session creation time
     * @return Creation timestamp
     */
    std::chrono::system_clock::time_point get_created_at() const;

    /**
     * @brief Get the session start time
     * @return Start timestamp
     */
    std::chrono::system_clock::time_point get_started_at() const;

    /**
     * @brief Get the session end time
     * @return End timestamp, or epoch if not ended
     */
    std::chrono::system_clock::time_point get_ended_at() const;

    /**
     * @brief Get the session duration
     * @return Session duration in seconds
     */
    std::chrono::seconds get_duration() const;

    /**
     * @brief Start the session
     * @return true if session started successfully
     */
    bool start();

    /**
     * @brief Pause the session
     * @return true if session paused successfully
     */
    bool pause();

    /**
     * @brief Resume the session
     * @return true if session resumed successfully
     */
    bool resume();

    /**
     * @brief End the session
     * @return true if session ended successfully
     */
    bool end();

    /**
     * @brief Get the game state
     * @return Game state for this session
     */
    std::shared_ptr<GameState> get_game_state() const;

    /**
     * @brief Set the game state
     * @param state New game state
     */
    void set_game_state(std::shared_ptr<GameState> state);

    /**
     * @brief Get session data
     * @param key Data key
     * @return Data value, or empty string if not found
     */
    std::string get_data(const std::string& key) const;

    /**
     * @brief Set session data
     * @param key Data key
     * @param value Data value
     */
    void set_data(const std::string& key, const std::string& value);

    /**
     * @brief Check if session has data
     * @param key Data key
     * @return true if data exists
     */
    bool has_data(const std::string& key) const;

    /**
     * @brief Remove session data
     * @param key Data key to remove
     * @return true if data was removed
     */
    bool remove_data(const std::string& key);

    /**
     * @brief Get all session data
     * @return Map of all session data
     */
    std::unordered_map<std::string, std::string> get_all_data() const;

    /**
     * @brief Register an event handler
     * @param event_type Type of event to handle
     * @param handler Function to handle the event
     */
    void register_event_handler(const std::string& event_type, 
                               std::function<void(const std::string&)> handler);

    /**
     * @brief Trigger an event
     * @param event_type Type of event
     * @param event_data Event data
     */
    void trigger_event(const std::string& event_type, const std::string& event_data);

    /**
     * @brief Get session events
     * @return Vector of session events
     */
    std::vector<std::pair<std::string, std::string>> get_events() const;

    /**
     * @brief Clear session events
     */
    void clear_events();

    /**
     * @brief Get session statistics
     * @return Map of session statistics
     */
    std::unordered_map<std::string, int> get_statistics() const;

    /**
     * @brief Set session statistic
     * @param stat_name Statistic name
     * @param value Statistic value
     */
    void set_statistic(const std::string& stat_name, int value);

    /**
     * @brief Increment session statistic
     * @param stat_name Statistic name
     * @param increment Amount to increment
     */
    void increment_statistic(const std::string& stat_name, int increment = 1);

    /**
     * @brief Get session score
     * @return Session score
     */
    int get_score() const;

    /**
     * @brief Set session score
     * @param score New session score
     */
    void set_score(int score);

    /**
     * @brief Add to session score
     * @param points Points to add
     */
    void add_score(int points);

    /**
     * @brief Get session level
     * @return Session level
     */
    int get_level() const;

    /**
     * @brief Set session level
     * @param level New session level
     */
    void set_level(int level);

    /**
     * @brief Check if session is active
     * @return true if session is active
     */
    bool is_active() const;

    /**
     * @brief Check if session is paused
     * @return true if session is paused
     */
    bool is_paused() const;

    /**
     * @brief Check if session is ended
     * @return true if session is ended
     */
    bool is_ended() const;

    /**
     * @brief Save session data
     * @return true if save successful
     */
    bool save();

    /**
     * @brief Load session data
     * @return true if load successful
     */
    bool load();

    /**
     * @brief Get session summary
     * @return Session summary as string
     */
    std::string get_summary() const;

private:
/** @ingroup game */
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace game
} // namespace satox 
