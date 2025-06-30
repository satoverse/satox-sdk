/**
 * @file game_asset.hpp
 * @brief Satox satox-sdk - Game Asset
 * @details This header provides Game Asset functionality for the Satox satox-sdk.
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
 * @file game_asset.hpp
 * @brief Satox satox-sdk - Game Asset
 * @details This header provides Game Asset functionality for the Satox satox-sdk.
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
#include <chrono>

/** @ingroup game */
namespace satox {
/** @ingroup game */
namespace game {

/**
 * @brief Represents a game asset (item, weapon, etc.)
 * 
 * The GameAsset class represents any item or asset in the game
 * that can be owned, traded, or used by players.
 */
/** @ingroup game */
class GameAsset {
public:
    /**
     * @brief Asset types
     */
/** @ingroup game */
    enum class Type {
        WEAPON,
        ARMOR,
        CONSUMABLE,
        MATERIAL,
        CURRENCY,
        QUEST_ITEM,
        CRAFTING_ITEM,
        DECORATION,
        VEHICLE,
        PET,
        SKILL_BOOK,
        ENCHANTMENT,
        GEM,
        POTION,
        SCROLL,
        KEY,
        CONTAINER,
        TOOL,
        INGREDIENT,
        RECIPE,
        OTHER
    };

    /**
     * @brief Asset rarity levels
     */
/** @ingroup game */
    enum class Rarity {
        COMMON,
        UNCOMMON,
        RARE,
        EPIC,
        LEGENDARY,
        MYTHIC
    };

    /**
     * @brief Constructor
     * @param asset_id Unique asset identifier
     * @param asset_type Type of asset
     * @param value Asset value
     */
    GameAsset(const std::string& asset_id, 
              const std::string& asset_type, 
              double value);

    /**
     * @brief Destructor
     */
    ~GameAsset();

    /**
     * @brief Get the asset ID
     * @return Asset identifier
     */
    const std::string& get_id() const;

    /**
     * @brief Get the asset name
     * @return Asset name
     */
    const std::string& get_name() const;

    /**
     * @brief Set the asset name
     * @param name New asset name
     */
    void set_name(const std::string& name);

    /**
     * @brief Get the asset type
     * @return Asset type
     */
    const std::string& get_type() const;

    /**
     * @brief Set the asset type
     * @param type New asset type
     */
    void set_type(const std::string& type);

    /**
     * @brief Get the asset value
     * @return Asset value
     */
    double get_value() const;

    /**
     * @brief Set the asset value
     * @param value New asset value
     */
    void set_value(double value);

    /**
     * @brief Get the asset rarity
     * @return Asset rarity
     */
    Rarity get_rarity() const;

    /**
     * @brief Set the asset rarity
     * @param rarity New asset rarity
     */
    void set_rarity(Rarity rarity);

    /**
     * @brief Get the asset level requirement
     * @return Level requirement
     */
    int get_level_requirement() const;

    /**
     * @brief Set the asset level requirement
     * @param level Level requirement
     */
    void set_level_requirement(int level);

    /**
     * @brief Get the asset durability
     * @return Current durability
     */
    int get_durability() const;

    /**
     * @brief Set the asset durability
     * @param durability New durability value
     */
    void set_durability(int durability);

    /**
     * @brief Get the maximum durability
     * @return Maximum durability
     */
    int get_max_durability() const;

    /**
     * @brief Set the maximum durability
     * @param max_durability New maximum durability
     */
    void set_max_durability(int max_durability);

    /**
     * @brief Get the asset description
     * @return Asset description
     */
    const std::string& get_description() const;

    /**
     * @brief Set the asset description
     * @param description New asset description
     */
    void set_description(const std::string& description);

    /**
     * @brief Get the asset icon
     * @return Asset icon identifier
     */
    const std::string& get_icon() const;

    /**
     * @brief Set the asset icon
     * @param icon New asset icon identifier
     */
    void set_icon(const std::string& icon);

    /**
     * @brief Get the asset model
     * @return Asset model identifier
     */
    const std::string& get_model() const;

    /**
     * @brief Set the asset model
     * @param model New asset model identifier
     */
    void set_model(const std::string& model);

    /**
     * @brief Get asset properties
     * @return Map of asset properties
     */
    std::unordered_map<std::string, std::string> get_properties() const;

    /**
     * @brief Set asset property
     * @param key Property key
     * @param value Property value
     */
    void set_property(const std::string& key, const std::string& value);

    /**
     * @brief Get asset property
     * @param key Property key
     * @return Property value, or empty string if not found
     */
    std::string get_property(const std::string& key) const;

    /**
     * @brief Check if asset has property
     * @param key Property key
     * @return true if property exists
     */
    bool has_property(const std::string& key) const;

    /**
     * @brief Get asset tags
     * @return Vector of asset tags
     */
    std::vector<std::string> get_tags() const;

    /**
     * @brief Add tag to asset
     * @param tag Tag to add
     */
    void add_tag(const std::string& tag);

    /**
     * @brief Remove tag from asset
     * @param tag Tag to remove
     * @return true if tag was removed
     */
    bool remove_tag(const std::string& tag);

    /**
     * @brief Check if asset has tag
     * @param tag Tag to check
     * @return true if asset has the tag
     */
    bool has_tag(const std::string& tag) const;

    /**
     * @brief Get asset creation time
     * @return Creation timestamp
     */
    std::chrono::system_clock::time_point get_created_at() const;

    /**
     * @brief Get asset last modified time
     * @return Last modified timestamp
     */
    std::chrono::system_clock::time_point get_modified_at() const;

    /**
     * @brief Get asset owner
     * @return Owner player ID, or empty string if unowned
     */
    const std::string& get_owner() const;

    /**
     * @brief Set asset owner
     * @param owner_id Owner player ID
     */
    void set_owner(const std::string& owner_id);

    /**
     * @brief Check if asset is owned
     * @return true if asset has an owner
     */
    bool is_owned() const;

    /**
     * @brief Get asset stack size
     * @return Stack size
     */
    int get_stack_size() const;

    /**
     * @brief Set asset stack size
     * @param stack_size New stack size
     */
    void set_stack_size(int stack_size);

    /**
     * @brief Get maximum stack size
     * @return Maximum stack size
     */
    int get_max_stack_size() const;

    /**
     * @brief Set maximum stack size
     * @param max_stack_size New maximum stack size
     */
    void set_max_stack_size(int max_stack_size);

    /**
     * @brief Check if asset is stackable
     * @return true if asset can be stacked
     */
    bool is_stackable() const;

    /**
     * @brief Check if asset is tradeable
     * @return true if asset can be traded
     */
    bool is_tradeable() const;

    /**
     * @brief Set tradeable status
     * @param tradeable New tradeable status
     */
    void set_tradeable(bool tradeable);

    /**
     * @brief Check if asset is droppable
     * @return true if asset can be dropped
     */
    bool is_droppable() const;

    /**
     * @brief Set droppable status
     * @param droppable New droppable status
     */
    void set_droppable(bool droppable);

    /**
     * @brief Save asset data
     * @return true if save successful
     */
    bool save();

    /**
     * @brief Load asset data
     * @return true if load successful
     */
    bool load();

    /**
     * @brief Clone the asset
     * @return New asset instance with same properties
     */
    std::shared_ptr<GameAsset> clone() const;

private:
/** @ingroup game */
    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace game
} // namespace satox 
