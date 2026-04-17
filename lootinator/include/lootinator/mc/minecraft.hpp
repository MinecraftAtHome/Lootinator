#ifndef LOOTINATOR_MINECRAFT_H
#define LOOTINATOR_MINECRAFT_H

#include "nlohmann/json.hpp"
#include <map>
#include <string>
#include <vector>

namespace mc {
#define VersionRangeList(X)                                                                        \
	X(MC_UNDEFINED)                                                                                \
	X(MC_1_13)                                                                                     \
	X(MC_1_14_TO_1_15)                                                                             \
	X(MC_1_16_TO_1_20)                                                                             \
	X(MC_1_21_TO_1_21_10)                                                                          \
	X(MC_1_21_11_TO_26_1)

	enum VersionRange {
#define X(v) v,
		VersionRangeList(X)
#undef X
#define X(v) 1 +
			MC_LATEST = VersionRangeList(X) - 1
#undef X
	};

	/*
		NO_ATTRIBUTE: 0
		ENCHANTMENT: 1 - 1000
		OTHER_ATTRIBUTES: 1000 - ...
	*/
	enum Enchantment {
		NO_ENCHANTMENT = 1,

		// armor
		PROTECTION = 2,
		FIRE_PROTECTION = 3,
		BLAST_PROTECTION = 4,
		PROJECTILE_PROTECTION = 5,
		RESPIRATION = 6,
		AQUA_AFFINITY = 7,
		THORNS = 8,
		SWIFT_SNEAK = 9,
		FEATHER_FALLING = 10,
		DEPTH_STRIDER = 11,
		FROST_WALKER = 12,
		SOUL_SPEED = 13,

		// swords
		SHARPNESS = 14,
		SMITE = 15,
		BANE_OF_ARTHROPODS = 16,
		KNOCKBACK = 17,
		FIRE_ASPECT = 18,
		LOOTING = 19,
		SWEEPING_EDGE = 20,

		// tools
		EFFICIENCY = 21,
		SILK_TOUCH = 22,
		FORTUNE = 23,

		// fishing rods
		LUCK_OF_THE_SEA = 24,
		LURE = 25,

		// bows
		POWER = 26,
		PUNCH = 27,
		FLAME = 28,
		INFINITY_ENCHANTMENT = 29,

		// crossbows
		QUICK_CHARGE = 30,
		MULTISHOT = 31,
		PIERCING = 32,

		// tridents
		IMPALING = 33,
		RIPTIDE = 34,
		LOYALTY = 35,
		CHANNELING = 36,

		// maces
		DENSITY = 37,
		BREACH = 38,
		WIND_BURST = 39,

		// general
		MENDING = 40,
		UNBREAKING = 41,
		CURSE_OF_VANISHING = 42,
		CURSE_OF_BINDING = 43,
		LUNGE = 44,
	};

	enum ItemType {
		NO_ITEM,
		HELMET,
		CHESTPLATE,
		LEGGINGS,
		BOOTS,
		SWORD,
		PICKAXE,
		SHOVEL,
		AXE,
		HOE,
		FISHING_ROD,
		BOW,
		CROSSBOW,
		TRIDENT,
		MACE,
		BOOK,
		SPEAR
	};

	enum AttributeCategory {
		NO_ATTRIBUTE,
		ENCHANTMENT_ATTRIBUTE,
		DURABILITY_ATTRIBUTE,
		POTION_EFFECT_ATTRIBUTE,
	};

	// represents an additional attribute of an item, such as an enchantment
	// or type of music disc
	struct ItemAttribute {
		std::uint32_t type;
		std::int32_t level;

		mc::AttributeCategory get_category() const;
		static ItemAttribute from_json(nlohmann::json json);
		bool operator==(const ItemAttribute& other) const;
		bool operator!=(const ItemAttribute& other) const;
		friend std::ostream& operator<<(std::ostream& os, const ItemAttribute& attribute);
	};

	// ---------------------------------------------------------------------------------------

	std::string strip_prefix(const std::string& str);

	bool is_treasure_enchantment(mc::Enchantment enchantment);
	int count_unique_groups(const std::vector<mc::Enchantment>& enchantments);
	bool is_enchantment_applicable(
		mc::Enchantment enchantment, mc::ItemType item_type, bool extra_enchants);
	bool is_enchantment_available_at_level(
		mc::Enchantment enchantment, int enchantment_level, int level);
	uint32_t get_max_level(mc::Enchantment enchantment);
	uint32_t get_enchantability(const std::string& item_name);

	std::vector<mc::Enchantment> get_enchantments_for_version(const mc::VersionRange version_range);
	// NOTE: when we have new item attribute types, they need to be added here!
	mc::Enchantment get_enchantment_from_attribute(const ItemAttribute& attr);

	std::string item_type_to_string(mc::ItemType type);
	mc::ItemType string_to_item_type(const std::string& item_type_string);
	std::string enchantment_to_string(mc::Enchantment type);
	mc::Enchantment string_to_enchantment(const std::string& enchantment_string);
	mc::VersionRange parse_version(std::string vstring);
	std::string get_version_from_enum(mc::VersionRange version);
} // namespace mc

#endif