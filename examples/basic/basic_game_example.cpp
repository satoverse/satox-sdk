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

#include <iostream>
#include <memory>
#include <satox/game.hpp>

using namespace satox::game;

int main() {
    std::cout << "=== Satox Game SDK Basic Example ===" << std::endl;
    
    // Initialize the Game SDK
    if (!initialize_sdk()) {
        std::cerr << "Failed to initialize Game SDK" << std::endl;
        return 1;
    }
    
    std::cout << "Game SDK initialized successfully" << std::endl;
    std::cout << "SDK Version: " << get_sdk_version() << std::endl;
    
    try {
        // Create a game manager
        GameManager game_manager;
        
        // Initialize the game manager
        if (!game_manager.initialize()) {
            std::cerr << "Failed to initialize game manager" << std::endl;
            return 1;
        }
        
        std::cout << "Game manager initialized successfully" << std::endl;
        
        // Create a player
        auto player = game_manager.create_player("player123");
        if (!player) {
            std::cerr << "Failed to create player" << std::endl;
            return 1;
        }
        
        std::cout << "Created player: " << player->get_id() << std::endl;
        
        // Set player properties
        player->set_name("Adventurer");
        player->set_level(1);
        player->set_health(100);
        player->set_max_health(100);
        player->add_gold(50.0);
        
        std::cout << "Player name: " << player->get_name() << std::endl;
        std::cout << "Player level: " << player->get_level() << std::endl;
        std::cout << "Player health: " << player->get_health() << "/" << player->get_max_health() << std::endl;
        std::cout << "Player gold: " << player->get_gold() << std::endl;
        
        // Create a game asset (sword)
        auto sword = game_manager.create_asset("sword_001", "Weapon", 25.0);
        if (!sword) {
            std::cerr << "Failed to create sword asset" << std::endl;
            return 1;
        }
        
        std::cout << "Created asset: " << sword->get_id() << std::endl;
        
        // Set sword properties
        sword->set_name("Iron Sword");
        sword->set_description("A basic iron sword");
        sword->set_rarity(GameAsset::Rarity::COMMON);
        sword->set_level_requirement(1);
        sword->set_durability(100);
        sword->set_max_durability(100);
        sword->set_tradeable(true);
        sword->set_droppable(true);
        
        std::cout << "Sword name: " << sword->get_name() << std::endl;
        std::cout << "Sword value: " << sword->get_value() << std::endl;
        std::cout << "Sword durability: " << sword->get_durability() << "/" << sword->get_max_durability() << std::endl;
        
        // Add sword to player inventory
        player->add_to_inventory(sword);
        std::cout << "Added sword to player inventory" << std::endl;
        
        // Check if player has the sword
        if (player->has_item("sword_001")) {
            std::cout << "Player has the sword in inventory" << std::endl;
        }
        
        // Create a game session
        auto session = game_manager.create_session(player);
        if (!session) {
            std::cerr << "Failed to create game session" << std::endl;
            return 1;
        }
        
        std::cout << "Created game session: " << session->get_id() << std::endl;
        
        // Start the session
        if (session->start()) {
            std::cout << "Game session started successfully" << std::endl;
        }
        
        // Create a game state
        auto game_state = game_manager.get_game_state("game_001");
        if (!game_state) {
            std::cerr << "Failed to create game state" << std::endl;
            return 1;
        }
        
        std::cout << "Created game state: " << game_state->get_id() << std::endl;
        
        // Set game state properties
        game_state->set_name("Adventure Quest");
        game_state->set_description("An epic adventure begins");
        game_state->set_version("1.0.0");
        game_state->set_max_players(4);
        
        // Add player to game state
        game_state->add_player(player);
        
        // Add asset to game state
        game_state->add_asset(sword);
        
        // Set some game data
        game_state->set_data("world_name", "Fantasy Realm");
        game_state->set_data("difficulty", "Normal");
        game_state->set_statistic("monsters_killed", 0);
        game_state->set_score(0);
        game_state->set_level(1);
        
        std::cout << "Game state configured:" << std::endl;
        std::cout << "  World: " << game_state->get_data("world_name") << std::endl;
        std::cout << "  Difficulty: " << game_state->get_data("difficulty") << std::endl;
        std::cout << "  Players: " << game_state->get_player_count() << "/" << game_state->get_max_players() << std::endl;
        std::cout << "  Assets: " << game_state->get_asset_count() << std::endl;
        
        // Start the game
        if (game_state->start()) {
            std::cout << "Game started successfully" << std::endl;
        }
        
        // Simulate some game events
        std::cout << "\n=== Simulating Game Events ===" << std::endl;
        
        // Player gains experience
        player->add_experience(100);
        std::cout << "Player gained 100 experience. Total: " << player->get_experience() << std::endl;
        
        // Player levels up
        if (player->get_experience() >= 100) {
            player->set_level(2);
            player->set_max_health(120);
            player->set_health(120);
            std::cout << "Player leveled up to level " << player->get_level() << std::endl;
            std::cout << "Player health increased to " << player->get_health() << "/" << player->get_max_health() << std::endl;
        }
        
        // Player earns gold
        player->add_gold(25.0);
        std::cout << "Player earned 25 gold. Total: " << player->get_gold() << std::endl;
        
        // Game state updates
        game_state->increment_statistic("monsters_killed", 3);
        game_state->add_score(150);
        game_state->set_level(2);
        
        std::cout << "Game progress:" << std::endl;
        std::cout << "  Monsters killed: " << game_state->get_statistics().at("monsters_killed") << std::endl;
        std::cout << "  Score: " << game_state->get_score() << std::endl;
        std::cout << "  Level: " << game_state->get_level() << std::endl;
        
        // End the game
        if (game_state->end()) {
            std::cout << "Game ended successfully" << std::endl;
        }
        
        // End the session
        if (session->end()) {
            std::cout << "Game session ended successfully" << std::endl;
        }
        
        // Save player data
        if (player->save()) {
            std::cout << "Player data saved successfully" << std::endl;
        }
        
        // Save game state
        if (game_state->save()) {
            std::cout << "Game state saved successfully" << std::endl;
        }
        
        // Get summaries
        std::cout << "\n=== Session Summary ===" << std::endl;
        std::cout << session->get_summary() << std::endl;
        
        std::cout << "\n=== Game State Summary ===" << std::endl;
        std::cout << game_state->get_summary() << std::endl;
        
        // Shutdown the game manager
        game_manager.shutdown();
        std::cout << "Game manager shut down successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    // Shutdown the Game SDK
    shutdown_sdk();
    std::cout << "Game SDK shut down successfully" << std::endl;
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    return 0;
} 