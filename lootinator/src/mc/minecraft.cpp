#include "lootinator/mc/minecraft.hpp"
#include "lootinator/utility/debug.h"
#include "lootinator/utility/enum_bimap.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace mc {
	std::set<mc::Enchantment> TREASURE_ENCHANTS({CURSE_OF_BINDING,
		CURSE_OF_VANISHING,
		FROST_WALKER,
		MENDING,
		SOUL_SPEED,
		SWIFT_SNEAK,
		WIND_BURST});

	std::map<mc::ItemType, std::vector<Enchantment>> ITEM_ENCHANTMENTS(
		{{CHESTPLATE,
			 {PROTECTION,
				 FIRE_PROTECTION,
				 BLAST_PROTECTION,
				 PROJECTILE_PROTECTION,
				 THORNS,
				 CURSE_OF_BINDING}},
			{HELMET,
				{PROTECTION,
					FIRE_PROTECTION,
					BLAST_PROTECTION,
					PROJECTILE_PROTECTION,
					RESPIRATION,
					AQUA_AFFINITY,
					CURSE_OF_BINDING}},
			{LEGGINGS,
				{PROTECTION,
					FIRE_PROTECTION,
					BLAST_PROTECTION,
					PROJECTILE_PROTECTION,
					CURSE_OF_BINDING,
					SWIFT_SNEAK}},
			{BOOTS,
				{PROTECTION,
					FIRE_PROTECTION,
					BLAST_PROTECTION,
					PROJECTILE_PROTECTION,
					FEATHER_FALLING,
					DEPTH_STRIDER,
					FROST_WALKER,
					SOUL_SPEED,
					CURSE_OF_BINDING}},

			{SWORD,
				{SHARPNESS,
					SMITE,
					BANE_OF_ARTHROPODS,
					LOOTING,
					KNOCKBACK,
					FIRE_ASPECT,
					SWEEPING_EDGE}},
			{MACE, {DENSITY, BREACH, WIND_BURST, SMITE, BANE_OF_ARTHROPODS, FIRE_ASPECT}},
			{BOW, {POWER, PUNCH, FLAME, INFINITY_ENCHANTMENT}},
			{CROSSBOW, {PIERCING, MULTISHOT, QUICK_CHARGE}},
			{TRIDENT, {IMPALING, RIPTIDE, LOYALTY, CHANNELING}},
			{SPEAR, {LUNGE, SHARPNESS, SMITE, BANE_OF_ARTHROPODS, KNOCKBACK, FIRE_ASPECT, LOOTING}},
			{PICKAXE, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
			{AXE, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
			{SHOVEL, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
			{HOE, {EFFICIENCY, SILK_TOUCH, FORTUNE}},
			{FISHING_ROD, {LUCK_OF_THE_SEA, LURE}}});

	std::map<Enchantment, uint32_t> ENCHANTMENT_MAX_LEVEL({
		{EFFICIENCY, 5},
		{SHARPNESS, 5},
		{SMITE, 5},
		{BANE_OF_ARTHROPODS, 5},
		{POWER, 5},
		{IMPALING, 5},
		{DENSITY, 5},
		{PROTECTION, 4},
		{FIRE_PROTECTION, 4},
		{PROJECTILE_PROTECTION, 4},
		{BLAST_PROTECTION, 4},
		{FEATHER_FALLING, 4},
		{BREACH, 4},
		{PIERCING, 4},
		{FORTUNE, 3},
		{LUCK_OF_THE_SEA, 3},
		{LURE, 3},
		{UNBREAKING, 3},
		{LOYALTY, 3},
		{THORNS, 3},
		{WIND_BURST, 3},
		{LOOTING, 3},
		{RESPIRATION, 3},
		{QUICK_CHARGE, 3},
		{RIPTIDE, 3},
		{SWIFT_SNEAK, 3},
		{DEPTH_STRIDER, 3},
		{SOUL_SPEED, 3},
		{SWEEPING_EDGE, 3},
		{FROST_WALKER, 2},
		{PUNCH, 2},
		{KNOCKBACK, 2},
		{FIRE_ASPECT, 2},
		{MENDING, 1},
		{CURSE_OF_BINDING, 1},
		{CURSE_OF_VANISHING, 1},
		{MULTISHOT, 1},
		{SILK_TOUCH, 1},
		{INFINITY_ENCHANTMENT, 1},
		{FLAME, 1},
		{CHANNELING, 1},
		{AQUA_AFFINITY, 1},
		{LUNGE, 3},
	});

	// enchantability data

	constexpr uint32_t BASE_ENCHANTABILITY = 1;
	constexpr uint32_t STONE_ENCHANTABILITY = 5;
	constexpr uint32_t TURTLE_ENCHANTABILITY = 9;
	constexpr uint32_t DIAMOND_ENCHANTABILITY = 10;
	constexpr uint32_t CHAINMAIL_ENCHANTABILITY = 12;
	constexpr uint32_t WOOD_ENCHANTABILITY = 15;
	constexpr uint32_t LEATHER_ENCHANTABILITY = 15;
	constexpr uint32_t NETHERITE_ENCHANTABILITY = 15;
	constexpr uint32_t MACE_ENCHANTABILITY = 15;

	constexpr uint32_t COPPER_ENCHANTABILITY_TOOLS = 13;
	constexpr uint32_t COPPER_ENCHANTABILITY_ARMOR = 8;
	constexpr uint32_t IRON_ENCHANTABILITY_TOOLS = 14;
	constexpr uint32_t IRON_ENCHANTABILITY_ARMOR = 9;
	constexpr uint32_t GOLD_ENCHANTABILITY_TOOLS = 22;
	constexpr uint32_t GOLD_ENCHANTABILITY_ARMOR = 25;

	std::map<std::string, uint32_t> ITEM_NAME_TO_ENCHANTABILITY({
		{"leather_chestplate", LEATHER_ENCHANTABILITY},
		{"chainmail_chestplate", CHAINMAIL_ENCHANTABILITY},
		{"copper_chestplate", COPPER_ENCHANTABILITY_ARMOR},
		{"iron_chestplate", IRON_ENCHANTABILITY_ARMOR},
		{"golden_chestplate", GOLD_ENCHANTABILITY_ARMOR},
		{"diamond_chestplate", DIAMOND_ENCHANTABILITY},
		{"netherite_chestplate", NETHERITE_ENCHANTABILITY},

		{"leather_helmet", LEATHER_ENCHANTABILITY},
		{"chainmail_helmet", CHAINMAIL_ENCHANTABILITY},
		{"copper_helmet", COPPER_ENCHANTABILITY_ARMOR},
		{"iron_helmet", IRON_ENCHANTABILITY_ARMOR},
		{"golden_helmet", GOLD_ENCHANTABILITY_ARMOR},
		{"diamond_helmet", DIAMOND_ENCHANTABILITY},
		{"netherite_helmet", NETHERITE_ENCHANTABILITY},
		{"turtle_helmet", TURTLE_ENCHANTABILITY},

		{"leather_leggings", LEATHER_ENCHANTABILITY},
		{"chainmail_leggings", CHAINMAIL_ENCHANTABILITY},
		{"copper_leggings", COPPER_ENCHANTABILITY_ARMOR},
		{"iron_leggings", IRON_ENCHANTABILITY_ARMOR},
		{"golden_leggings", GOLD_ENCHANTABILITY_ARMOR},
		{"diamond_leggings", DIAMOND_ENCHANTABILITY},
		{"netherite_leggings", NETHERITE_ENCHANTABILITY},

		{"leather_boots", LEATHER_ENCHANTABILITY},
		{"chainmail_boots", CHAINMAIL_ENCHANTABILITY},
		{"copper_boots", COPPER_ENCHANTABILITY_ARMOR},
		{"iron_boots", IRON_ENCHANTABILITY_ARMOR},
		{"golden_boots", GOLD_ENCHANTABILITY_ARMOR},
		{"diamond_boots", DIAMOND_ENCHANTABILITY},
		{"netherite_boots", NETHERITE_ENCHANTABILITY},

		{"wooden_sword", WOOD_ENCHANTABILITY},
		{"stone_sword", STONE_ENCHANTABILITY},
		{"copper_sword", COPPER_ENCHANTABILITY_TOOLS},
		{"iron_sword", IRON_ENCHANTABILITY_TOOLS},
		{"golden_sword", GOLD_ENCHANTABILITY_TOOLS},
		{"diamond_sword", DIAMOND_ENCHANTABILITY},
		{"netherite_sword", NETHERITE_ENCHANTABILITY},

		{"mace", MACE_ENCHANTABILITY},
		{"bow", BASE_ENCHANTABILITY},
		{"crossbow", BASE_ENCHANTABILITY},
		{"trident", BASE_ENCHANTABILITY},

		{"wooden_pickaxe", WOOD_ENCHANTABILITY},
		{"stone_pickaxe", STONE_ENCHANTABILITY},
		{"copper_pickaxe", COPPER_ENCHANTABILITY_TOOLS},
		{"iron_pickaxe", IRON_ENCHANTABILITY_TOOLS},
		{"golden_pickaxe", GOLD_ENCHANTABILITY_TOOLS},
		{"diamond_pickaxe", DIAMOND_ENCHANTABILITY},
		{"netherite_pickaxe", NETHERITE_ENCHANTABILITY},

		{"wooden_axe", WOOD_ENCHANTABILITY},
		{"stone_axe", STONE_ENCHANTABILITY},
		{"copper_axe", COPPER_ENCHANTABILITY_TOOLS},
		{"iron_axe", IRON_ENCHANTABILITY_TOOLS},
		{"golden_axe", GOLD_ENCHANTABILITY_TOOLS},
		{"diamond_axe", DIAMOND_ENCHANTABILITY},
		{"netherite_axe", NETHERITE_ENCHANTABILITY},

		{"wooden_shovel", WOOD_ENCHANTABILITY},
		{"stone_shovel", STONE_ENCHANTABILITY},
		{"copper_shovel", COPPER_ENCHANTABILITY_TOOLS},
		{"iron_shovel", IRON_ENCHANTABILITY_TOOLS},
		{"golden_shovel", GOLD_ENCHANTABILITY_TOOLS},
		{"diamond_shovel", DIAMOND_ENCHANTABILITY},
		{"netherite_shovel", NETHERITE_ENCHANTABILITY},

		{"wooden_hoe", WOOD_ENCHANTABILITY},
		{"stone_hoe", STONE_ENCHANTABILITY},
		{"copper_hoe", COPPER_ENCHANTABILITY_TOOLS},
		{"iron_hoe", IRON_ENCHANTABILITY_TOOLS},
		{"golden_hoe", GOLD_ENCHANTABILITY_TOOLS},
		{"diamond_hoe", DIAMOND_ENCHANTABILITY},
		{"netherite_hoe", NETHERITE_ENCHANTABILITY},

		{"fishing_rod", BASE_ENCHANTABILITY},
		{"book", BASE_ENCHANTABILITY},
		{"enchanted_book", BASE_ENCHANTABILITY},

		{"wooden_spear", WOOD_ENCHANTABILITY},
		{"golden_spear", GOLD_ENCHANTABILITY_TOOLS},
		{"stone_spear", STONE_ENCHANTABILITY},
		{"copper_spear", COPPER_ENCHANTABILITY_TOOLS},
		{"iron_spear", IRON_ENCHANTABILITY_TOOLS},
		{"diamond_spear", DIAMOND_ENCHANTABILITY},
		{"netherite_spear", NETHERITE_ENCHANTABILITY},
	});

	// item name to enum translation (and reverse for debugging purposes)

	// enum VersionRange {
	// 	MC_1_13,
	// 	MC_1_14_TO_1_15,
	// 	MC_1_16_TO_1_20,
	// 	MC_1_21_TO_1_21_10,
	// 	MC_1_21_11_TO_26_1
	// };

	std::map<std::string, mc::VersionRange> STRING_TO_VERSION({
		{"latest", mc::VersionRange::MC_LATEST},
		{"1.13", mc::VersionRange::MC_1_13},
		{"1.14", mc::VersionRange::MC_1_14_TO_1_15},
		{"1.15", mc::VersionRange::MC_1_14_TO_1_15},
		{"1.16", mc::VersionRange::MC_1_16_TO_1_20},
		{"1.17", mc::VersionRange::MC_1_16_TO_1_20},
		{"1.18", mc::VersionRange::MC_1_16_TO_1_20},
		{"1.19", mc::VersionRange::MC_1_16_TO_1_20},
		{"1.20", mc::VersionRange::MC_1_16_TO_1_20},
		{"1.21", mc::VersionRange::MC_1_21_TO_1_21_10},
		{"1.21.10", mc::VersionRange::MC_1_21_TO_1_21_10},
		{"1.21.11", mc::VersionRange::MC_1_21_11_TO_26_1},
		{"26.1", mc::VersionRange::MC_1_21_11_TO_26_1},
	});

	std::map<mc::ItemType, std::string> ITEM_TYPE_TO_NAME({
		{CHESTPLATE, "chestplate"},
		{HELMET, "helmet"},
		{LEGGINGS, "leggings"},
		{BOOTS, "boots"},
		{SWORD, "sword"},
		{MACE, "mace"},
		{BOW, "bow"},
		{CROSSBOW, "crossbow"},
		{TRIDENT, "trident"},
		{PICKAXE, "pickaxe"},
		{AXE, "axe"},
		{SHOVEL, "shovel"},
		{HOE, "hoe"},
		{FISHING_ROD, "fishing_rod"},
		{BOOK, "book"},
		{SPEAR, "spear"},
	});

	std::map<std::string, mc::ItemType> ITEM_NAME_TO_ITEM_TYPE({{"chestplate", CHESTPLATE},
		{"leather_chestplate", CHESTPLATE},
		{"chainmail_chestplate", CHESTPLATE},
		{"copper_chestplate", CHESTPLATE},
		{"iron_chestplate", CHESTPLATE},
		{"golden_chestplate", CHESTPLATE},
		{"diamond_chestplate", CHESTPLATE},
		{"netherite_chestplate", CHESTPLATE},

		{"helmet", HELMET},
		{"leather_helmet", HELMET},
		{"chainmail_helmet", HELMET},
		{"copper_helmet", HELMET},
		{"iron_helmet", HELMET},
		{"golden_helmet", HELMET},
		{"diamond_helmet", HELMET},
		{"netherite_helmet", HELMET},
		{"turtle_helmet", HELMET},

		{"leggings", LEGGINGS},
		{"leather_leggings", LEGGINGS},
		{"chainmail_leggings", LEGGINGS},
		{"copper_leggings", LEGGINGS},
		{"iron_leggings", LEGGINGS},
		{"golden_leggings", LEGGINGS},
		{"diamond_leggings", LEGGINGS},
		{"netherite_leggings", LEGGINGS},

		{"boots", BOOTS},
		{"leather_boots", BOOTS},
		{"chainmail_boots", BOOTS},
		{"copper_boots", BOOTS},
		{"iron_boots", BOOTS},
		{"golden_boots", BOOTS},
		{"diamond_boots", BOOTS},
		{"netherite_boots", BOOTS},

		{"sword", SWORD},
		{"wooden_sword", SWORD},
		{"stone_sword", SWORD},
		{"copper_sword", SWORD},
		{"iron_sword", SWORD},
		{"golden_sword", SWORD},
		{"diamond_sword", SWORD},
		{"netherite_sword", SWORD},

		{"mace", MACE},
		{"bow", BOW},
		{"crossbow", CROSSBOW},
		{"trident", TRIDENT},

		{"pickaxe", PICKAXE},
		{"wooden_pickaxe", PICKAXE},
		{"stone_pickaxe", PICKAXE},
		{"copper_pickaxe", PICKAXE},
		{"iron_pickaxe", PICKAXE},
		{"golden_pickaxe", PICKAXE},
		{"diamond_pickaxe", PICKAXE},
		{"netherite_pickaxe", PICKAXE},

		{"axe", AXE},
		{"wooden_axe", AXE},
		{"stone_axe", AXE},
		{"copper_axe", AXE},
		{"iron_axe", AXE},
		{"golden_axe", AXE},
		{"diamond_axe", AXE},
		{"netherite_axe", AXE},

		{"shovel", SHOVEL},
		{"wooden_shovel", SHOVEL},
		{"stone_shovel", SHOVEL},
		{"copper_shovel", SHOVEL},
		{"iron_shovel", SHOVEL},
		{"golden_shovel", SHOVEL},
		{"diamond_shovel", SHOVEL},
		{"netherite_shovel", SHOVEL},

		{"hoe", HOE},
		{"wooden_hoe", HOE},
		{"stone_hoe", HOE},
		{"copper_hoe", HOE},
		{"iron_hoe", HOE},
		{"golden_hoe", HOE},
		{"diamond_hoe", HOE},
		{"netherite_hoe", HOE},

		{"wooden_spear", SPEAR},
		{"golden_spear", SPEAR},
		{"stone_spear", SPEAR},
		{"copper_spear", SPEAR},
		{"iron_spear", SPEAR},
		{"diamond_spear", SPEAR},
		{"netherite_spear", SPEAR},

		{"fishing_rod", FISHING_ROD},
		{"book", BOOK},
		{"enchanted_book", BOOK}});

	// bidirectional map for enchantment name <-> enum translation

	util::EnumToStringBimap<mc::Enchantment> ENCHANTMENT_TO_NAME({
		{EFFICIENCY, "efficiency"},
		{SHARPNESS, "sharpness"},
		{SMITE, "smite"},
		{BANE_OF_ARTHROPODS, "bane_of_arthropods"},
		{POWER, "power"},
		{IMPALING, "impaling"},
		{DENSITY, "density"},
		{PROTECTION, "protection"},
		{FIRE_PROTECTION, "fire_protection"},
		{PROJECTILE_PROTECTION, "projectile_protection"},
		{BLAST_PROTECTION, "blast_protection"},
		{FEATHER_FALLING, "feather_falling"},
		{BREACH, "breach"},
		{PIERCING, "piercing"},
		{FORTUNE, "fortune"},
		{LUCK_OF_THE_SEA, "luck_of_the_sea"},
		{LURE, "lure"},
		{UNBREAKING, "unbreaking"},
		{LOYALTY, "loyalty"},
		{THORNS, "thorns"},
		{WIND_BURST, "wind_burst"},
		{LOOTING, "looting"},
		{RESPIRATION, "respiration"},
		{QUICK_CHARGE, "quick_charge"},
		{RIPTIDE, "riptide"},
		{SWIFT_SNEAK, "swift_sneak"},
		{DEPTH_STRIDER, "depth_strider"},
		{SOUL_SPEED, "soul_speed"},
		{SWEEPING_EDGE, "sweeping_edge"},
		{FROST_WALKER, "frost_walker"},
		{PUNCH, "punch"},
		{KNOCKBACK, "knockback"},
		{FIRE_ASPECT, "fire_aspect"},
		{MENDING, "mending"},
		{CURSE_OF_BINDING, "curse_of_binding"},
		{CURSE_OF_VANISHING, "curse_of_vanishing"},
		{MULTISHOT, "multishot"},
		{SILK_TOUCH, "silk_touch"},
		{INFINITY_ENCHANTMENT, "infinity"},
		{FLAME, "flame"},
		{CHANNELING, "channeling"},
		{AQUA_AFFINITY, "aqua_affinity"},
		{LUNGE, "lunge"},
		{NO_ENCHANTMENT, "no_enchantment"},
	});

	// validation of enchantment level i and effective level n for enchant_with_levels
	std::map<mc::Enchantment, std::function<bool(int, int)>> ENCHANT_LEVEL_VALIDATORS(
		{{NO_ENCHANTMENT,
			 [](int i, int n) {
				 (void)i;
				 (void)n;
				 return false;
			 }},
			{PROTECTION,
				[](int i, int n) { return n >= 1 + (i - 1) * 11 && n <= 1 + (i - 1) * 11 + 11; }},
			{FIRE_PROTECTION,
				[](int i, int n) { return n >= 10 + (i - 1) * 8 && n <= 10 + (i - 1) * 8 + 8; }},
			{FEATHER_FALLING,
				[](int i, int n) { return n >= 5 + (i - 1) * 6 && n <= 5 + (i - 1) * 6 + 6; }},
			{BLAST_PROTECTION,
				[](int i, int n) { return n >= 5 + (i - 1) * 8 && n <= 5 + (i - 1) * 8 + 8; }},
			{PROJECTILE_PROTECTION,
				[](int i, int n) { return n >= 3 + (i - 1) * 6 && n <= 3 + (i - 1) * 6 + 6; }},
			{RESPIRATION, [](int i, int n) { return n >= 10 * i && n <= 10 * i + 30; }},
			{AQUA_AFFINITY,
				[](int i, int n) {
					(void)i;
					return n >= 1 && n <= 41;
				}},
			{THORNS,
				[](int i, int n) { return n >= 10 + 20 * (i - 1) && n <= 10 + 20 * (i - 1) + 50; }},
			{DEPTH_STRIDER, [](int i, int n) { return n >= i * 10 && n <= i * 10 + 15; }},
			{FROST_WALKER, [](int i, int n) { return n >= i * 10 && n <= i * 10 + 15; }},
			{CURSE_OF_BINDING,
				[](int i, int n) {
					(void)i;
					return n >= 25 && n <= 50;
				}},
			{SOUL_SPEED, [](int i, int n) { return n >= i * 10 && n <= i * 10 + 15; }},
			{SHARPNESS,
				[](int i, int n) { return n >= 1 + (i - 1) * 11 && n <= 1 + (i - 1) * 11 + 20; }},
			{SMITE, [](int i, int n) { return n >= 5 + (i - 1) * 8 && n <= 5 + (i - 1) * 8 + 20; }},
			{BANE_OF_ARTHROPODS,
				[](int i, int n) { return n >= 5 + (i - 1) * 8 && n <= 5 + (i - 1) * 8 + 20; }},
			{KNOCKBACK, [](int i, int n) { return n >= 5 + 20 * (i - 1) && n <= 1 + i * 10 + 50; }},
			{FIRE_ASPECT,
				[](int i, int n) { return n >= 10 + 20 * (i - 1) && n <= 1 + i * 10 + 50; }},
			{LOOTING, [](int i, int n) { return n >= 15 + (i - 1) * 9 && n <= 1 + i * 10 + 50; }},
			{SWEEPING_EDGE,
				[](int i, int n) { return n >= 5 + (i - 1) * 9 && n <= 5 + (i - 1) * 9 + 15; }},
			{EFFICIENCY,
				[](int i, int n) { return n >= 1 + 10 * (i - 1) && n <= 1 + i * 10 + 50; }},
			{SILK_TOUCH, [](int i, int n) { return n >= 15 && n <= 1 + i * 10 + 50; }},
			{UNBREAKING, [](int i, int n) { return n >= 5 + (i - 1) * 8 && n <= 1 + i * 10 + 50; }},
			{FORTUNE, [](int i, int n) { return n >= 15 + (i - 1) * 9 && n <= 1 + i * 10 + 50; }},
			{POWER,
				[](int i, int n) { return n >= 1 + (i - 1) * 10 && n <= 1 + (i - 1) * 10 + 15; }},
			{PUNCH,
				[](int i, int n) { return n >= 12 + (i - 1) * 20 && n <= 12 + (i - 1) * 20 + 25; }},
			{FLAME,
				[](int i, int n) {
					(void)i;
					return n >= 20 && n <= 50;
				}},
			{INFINITY_ENCHANTMENT,
				[](int i, int n) {
					(void)i;
					return n >= 20 && n <= 50;
				}},
			{LUCK_OF_THE_SEA,
				[](int i, int n) { return n >= 15 + (i - 1) * 9 && n <= 1 + i * 10 + 50; }},
			{LURE, [](int i, int n) { return n >= 15 + (i - 1) * 9 && n <= 1 + i * 10 + 50; }},
			{LOYALTY, [](int i, int n) { return n >= 5 + i * 7 && n <= 50; }},
			{IMPALING,
				[](int i, int n) { return n >= 1 + (i - 1) * 8 && n <= 1 + (i - 1) * 8 + 20; }},
			{RIPTIDE, [](int i, int n) { return n >= 10 + i * 7 && n <= 50; }},
			{CHANNELING,
				[](int i, int n) {
					(void)i;
					return n >= 25 && n <= 50;
				}},
			{MULTISHOT,
				[](int i, int n) {
					(void)i;
					return n >= 20 && n <= 50;
				}},
			{QUICK_CHARGE, [](int i, int n) { return n >= 12 + (i - 1) * 20 && n <= 50; }},
			{PIERCING, [](int i, int n) { return n >= 1 + (i - 1) * 10 && n <= 50; }},
			{MENDING, [](int i, int n) { return n >= i * 25 && n <= i * 25 + 50; }},
			{CURSE_OF_VANISHING,
				[](int i, int n) {
					(void)i;
					return n >= 25 && n <= 50;
				}},
			{DENSITY, [](int i, int n) { return n >= 5 + (i - 1) * 8 && n <= 25 + (i - 1) * 8; }},
			{LUNGE, [](int i, int n) { return n >= 5 + (i - 1) * 8 && n <= 25 + (i - 1) * 8; }},
			{BREACH, [](int i, int n) { return n >= 15 + (i - 1) * 9 && n <= 65 + (i - 1) * 9; }},
			{WIND_BURST,
				[](int i, int n) { return n >= 15 + (i - 1) * 9 && n <= 65 + (i - 1) * 9; }}});

	std::map<mc::Enchantment, int> ENCHANTMENT_GROUPS({
		{NO_ENCHANTMENT, 0},

		{PROTECTION, 1},
		{FIRE_PROTECTION, 1},
		{BLAST_PROTECTION, 1},
		{PROJECTILE_PROTECTION, 1},

		{RESPIRATION, 2},
		{AQUA_AFFINITY, 3},
		{THORNS, 4},
		{SWIFT_SNEAK, 5},
		{FEATHER_FALLING, 6},

		{DEPTH_STRIDER, 7},
		{FROST_WALKER, 7},

		{SOUL_SPEED, 8},

		{SHARPNESS, 9},
		{SMITE, 9},
		{BANE_OF_ARTHROPODS, 9},
		{DENSITY, 9},
		{BREACH, 9},

		{KNOCKBACK, 10},
		{FIRE_ASPECT, 11},
		{LOOTING, 12},
		{SWEEPING_EDGE, 13},
		{EFFICIENCY, 14},

		{SILK_TOUCH, 15},
		{FORTUNE, 15},

		{LUCK_OF_THE_SEA, 16},
		{LURE, 17},
		{POWER, 18},
		{PUNCH, 19},
		{FLAME, 20},

		{INFINITY_ENCHANTMENT, 21},
		{MENDING, 21},

		{QUICK_CHARGE, 22},

		{MULTISHOT, 23},
		{PIERCING, 23},

		{IMPALING, 24},

		// FIXME this is technically wrong! This group of 3 enchantments
		// can become a single exclusion group if riptide is chosen first, and
		// two exclusion groups if one of the other enchants is chosen first.
		// Lootinator's enchant_with_levels handling doesn't account for the 2
		// options, so if level-enchanted tridents are ever introduced in some
		// loot table, their handling will occasionally be incorrect.
		{CHANNELING, 25},
		{RIPTIDE, 26},
		{LOYALTY, 26},

		{WIND_BURST, 27},
		{UNBREAKING, 28},
		{CURSE_OF_VANISHING, 29},
		{CURSE_OF_BINDING, 30},

		{LUNGE, 31},
	});

	// --------------------------------------------------------------------------------------------------
	// public api

	std::string strip_prefix(const std::string& str) {
		if (str.find("minecraft:") == 0) {
			return str.substr(10);
		}
		return str;
	}

	bool is_treasure_enchantment(mc::Enchantment enchantment) {
		return TREASURE_ENCHANTS.find(enchantment) != TREASURE_ENCHANTS.end();
	}

	/**
	 * @return The number of groups of mutually exclusive enchantments present in
	 *         the provided list of enchantments.
	 */
	int count_unique_groups(const std::vector<mc::Enchantment>& enchantments) {
		uint64_t group_flags = 0;
		for (const auto& ench : enchantments) {
			group_flags |= 1ULL << ENCHANTMENT_GROUPS.at(ench);
		}

		int unique_groups = 0;
		group_flags >>= 1; // exclude the NO_ENCHANTMENT group
		for (int i = 0; i < 63; i++) {
			if (group_flags & (1ULL << i)) {
				unique_groups++; // could just use popcount but whatever
			}
		}

		return unique_groups;
	}

	/**
	 * @return whether the provided enchantment can be obtained for the given item type
	 * in an enchanting table. When extra_enchants=true, the applicability is extended to
	 * all anvil-placeable enchantments for the given item type (thorns for all armor
	 * pieces, sharpness & smite & bane of arth. for axes).
	 */
	bool is_enchantment_applicable(
		mc::Enchantment enchantment, mc::ItemType item_type, bool extra_enchants) {
		if (item_type == ItemType::NO_ITEM || enchantment == NO_ENCHANTMENT) {
			return false; // safeguard
		}
		if (item_type == ItemType::BOOK || enchantment == MENDING || enchantment == UNBREAKING ||
			enchantment == CURSE_OF_VANISHING) {
			return true;
		}

		const auto& enchant_vec = ITEM_ENCHANTMENTS.at(item_type);
		for (const auto& ench : enchant_vec) {
			if (ench == enchantment) {
				return true;
			}
		}

		if (extra_enchants) {
			if (enchantment == THORNS &&
				(item_type == HELMET || item_type == LEGGINGS || item_type == BOOTS)) {
				return true;
			}
			if (item_type == AXE && (enchantment == SHARPNESS || enchantment == SMITE ||
										enchantment == BANE_OF_ARTHROPODS)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @return whether the given enchantment can be applied by the `enchant_with_levels` loot
	 * function for the enchantment level `enchantment_level`, and effective enchanting level
	 * `level`.
	 */
	bool is_enchantment_available_at_level(
		mc::Enchantment enchantment, int enchantment_level, int level) {
		return ENCHANT_LEVEL_VALIDATORS.at(enchantment)(enchantment_level, level);
	}

	/**
	 * @return the maximum level of the provided enchantment or 0 if the enchantment is invalid.
	 */
	uint32_t get_max_level(mc::Enchantment enchantment) {
		if (ENCHANTMENT_MAX_LEVEL.find(enchantment) != ENCHANTMENT_MAX_LEVEL.end()) {
			return ENCHANTMENT_MAX_LEVEL.at(enchantment);
		} else {
			return 0;
		}
	}

	/**
	 * @return the enchantability for the provided item name or 0 if the item is invalid.
	 */
	uint32_t get_enchantability(const std::string& item_name) {
		std::string stripped_name = mc::strip_prefix(item_name);
		if (ITEM_NAME_TO_ENCHANTABILITY.find(stripped_name) != ITEM_NAME_TO_ENCHANTABILITY.end()) {
			return ITEM_NAME_TO_ENCHANTABILITY.at(stripped_name);
		} else {
			return 0;
		}
	}

	/**
	 * @return the order of enchantments for a provided version range.
	 */
	std::vector<mc::Enchantment> get_enchantments_for_version(
		const mc::VersionRange version_range) {
		if (version_range == mc::VersionRange::MC_1_13) {
			return std::vector<mc::Enchantment>({{PROTECTION,
				FIRE_PROTECTION,
				FEATHER_FALLING,
				BLAST_PROTECTION,
				PROJECTILE_PROTECTION,
				RESPIRATION,
				AQUA_AFFINITY,
				THORNS,
				DEPTH_STRIDER,
				FROST_WALKER,
				CURSE_OF_BINDING,
				SHARPNESS,
				SMITE,
				BANE_OF_ARTHROPODS,
				KNOCKBACK,
				FIRE_ASPECT,
				LOOTING,
				SWEEPING_EDGE,
				EFFICIENCY,
				SILK_TOUCH,
				UNBREAKING,
				FORTUNE,
				POWER,
				PUNCH,
				FLAME,
				INFINITY_ENCHANTMENT,
				LUCK_OF_THE_SEA,
				LURE,
				LOYALTY,
				IMPALING,
				RIPTIDE,
				CHANNELING,
				MENDING,
				CURSE_OF_VANISHING}});
		} else if (version_range == mc::VersionRange::MC_1_14_TO_1_15 ||
				   version_range == mc::VersionRange::MC_1_16_TO_1_20) {
			return std::vector<mc::Enchantment>({{PROTECTION,
				FIRE_PROTECTION,
				FEATHER_FALLING,
				BLAST_PROTECTION,
				PROJECTILE_PROTECTION,
				RESPIRATION,
				AQUA_AFFINITY,
				THORNS,
				DEPTH_STRIDER,
				FROST_WALKER,
				CURSE_OF_BINDING,
				SHARPNESS,
				SMITE,
				BANE_OF_ARTHROPODS,
				KNOCKBACK,
				FIRE_ASPECT,
				LOOTING,
				SWEEPING_EDGE,
				EFFICIENCY,
				SILK_TOUCH,
				UNBREAKING,
				FORTUNE,
				POWER,
				PUNCH,
				FLAME,
				INFINITY_ENCHANTMENT,
				LUCK_OF_THE_SEA,
				LURE,
				LOYALTY,
				IMPALING,
				RIPTIDE,
				CHANNELING,
				MULTISHOT,
				QUICK_CHARGE,
				PIERCING,
				MENDING,
				CURSE_OF_VANISHING}});
		} else if (version_range == mc::VersionRange::MC_1_21_TO_1_21_10) {
			return std::vector<mc::Enchantment>({{PROTECTION,
				FIRE_PROTECTION,
				FEATHER_FALLING,
				BLAST_PROTECTION,
				PROJECTILE_PROTECTION,
				RESPIRATION,
				AQUA_AFFINITY,
				THORNS,
				DEPTH_STRIDER,
				SHARPNESS,
				SMITE,
				BANE_OF_ARTHROPODS,
				KNOCKBACK,
				FIRE_ASPECT,
				LOOTING,
				SWEEPING_EDGE,
				EFFICIENCY,
				SILK_TOUCH,
				UNBREAKING,
				FORTUNE,
				POWER,
				PUNCH,
				FLAME,
				INFINITY_ENCHANTMENT,
				LUCK_OF_THE_SEA,
				LURE,
				LOYALTY,
				IMPALING,
				RIPTIDE,
				CHANNELING,
				MULTISHOT,
				QUICK_CHARGE,
				PIERCING,
				DENSITY,
				BREACH,
				CURSE_OF_BINDING,
				CURSE_OF_VANISHING,
				FROST_WALKER,
				MENDING}});
		} else if (version_range == mc::VersionRange::MC_1_21_11_TO_26_1) {
			return std::vector<mc::Enchantment>({{
				PROTECTION,
				FIRE_PROTECTION,
				FEATHER_FALLING,
				BLAST_PROTECTION,
				PROJECTILE_PROTECTION,
				RESPIRATION,
				AQUA_AFFINITY,
				THORNS,
				DEPTH_STRIDER,
				SHARPNESS,
				SMITE,
				BANE_OF_ARTHROPODS,
				KNOCKBACK,
				FIRE_ASPECT,
				LOOTING,
				SWEEPING_EDGE,
				EFFICIENCY,
				SILK_TOUCH,
				UNBREAKING,
				FORTUNE,
				POWER,
				PUNCH,
				FLAME,
				INFINITY_ENCHANTMENT,
				LUCK_OF_THE_SEA,
				LURE,
				LOYALTY,
				IMPALING,
				RIPTIDE,
				CHANNELING,
				MULTISHOT,
				QUICK_CHARGE,
				PIERCING,
				DENSITY,
				BREACH,
				LUNGE,
				CURSE_OF_BINDING,
				CURSE_OF_VANISHING,
				FROST_WALKER,
				MENDING,
			}});
		} else {
			std::cerr << "minecraft.cpp: get_enchantments_for_version(): Bad version range: "
					  << version_range << '\n';
			return std::vector<mc::Enchantment>();
		}
	}

	// -----------------------------------------------------------------------------------

	/**
	 * Converts the given ItemType enum value to a human-readable text representation of that item
	 * type.
	 * @return the string representation or `lootinator::null` if the given item type is unknown.
	 */
	std::string item_type_to_string(const mc::ItemType type) {
		if (ITEM_TYPE_TO_NAME.find(type) != ITEM_TYPE_TO_NAME.end()) {
			return ITEM_TYPE_TO_NAME.at(type);
		} else {
			return "lootinator::null";
		}
	}

	/**
	 * Converts the given Minecraft item name to an ItemType representing the item's category.
	 * The provided name must either contain the `minecraft:` prefix or have no prefix at all.
	 * Example mappings: `minecraft:diamond_sword` -> `ItemType::SWORD`, `iron_helmet` ->
	 * `ItemType::HELMET`
	 * @return the item type or `ItemType::NO_ITEM` if the item name is unknown.
	 */
	mc::ItemType string_to_item_type(const std::string& item_type_string) {
		std::string stripped_name = mc::strip_prefix(item_type_string);
		if (ITEM_NAME_TO_ITEM_TYPE.find(stripped_name) != ITEM_NAME_TO_ITEM_TYPE.end()) {
			return ITEM_NAME_TO_ITEM_TYPE.at(stripped_name);
		} else {
			return mc::ItemType::NO_ITEM;
		}
	}

	std::string enchantment_to_string(const mc::Enchantment type) {
		if (ENCHANTMENT_TO_NAME.contains_enum(type)) {
			return ENCHANTMENT_TO_NAME.lookup_enum(type);
		} else {
			return "lootinator::null";
		}
	}

	mc::Enchantment string_to_enchantment(const std::string& enchantment_string) {
		std::string stripped_name = mc::strip_prefix(enchantment_string);
		if (ENCHANTMENT_TO_NAME.contains_string(stripped_name)) {
			return ENCHANTMENT_TO_NAME.lookup_string(stripped_name);
		} else {
			return mc::Enchantment::NO_ENCHANTMENT;
		}
	}

	mc::Enchantment get_enchantment_from_attribute(const ItemAttribute& attr) {
		return (mc::Enchantment)attr.type;
	}

	mc::AttributeCategory ItemAttribute::get_category() const {
		return this->type == 0 ? mc::AttributeCategory::NO_ATTRIBUTE
							   : (this->type >= 1 && this->type < 1000
										 ? mc::AttributeCategory::ENCHANTMENT_ATTRIBUTE
										 : mc::AttributeCategory::NO_ATTRIBUTE);
	}

	// -------------------------------------------------------------------------------
	// ItemAttribute

	mc::ItemAttribute mc::ItemAttribute::from_json(nlohmann::json json) {
		std::string type_str = json["type"];
		mc::Enchantment ench_enum = ENCHANTMENT_TO_NAME.lookup_string(type_str);
		if (json.contains("level")) {
			return {static_cast<uint32_t>(ench_enum), json["level"]};
		}
		return {static_cast<uint32_t>(ench_enum), -1};
	}

	bool mc::ItemAttribute::operator==(const ItemAttribute& other) const {
		return type == other.type && level == other.level;
	}

	bool mc::ItemAttribute::operator!=(const ItemAttribute& other) const {
		return !(*this == other);
	}

	std::ostream& operator<<(std::ostream& os, const ItemAttribute& attribute) {
		return util::DebugStruct(os, "ItemAttribute")
			.add("type", attribute.type)
			.add("level", attribute.level)
			.finish();
	}

	mc::VersionRange parse_version(std::string vstring) {
		if (STRING_TO_VERSION.find(vstring) != STRING_TO_VERSION.end()) {
			return STRING_TO_VERSION[vstring];
		}
		return MC_UNDEFINED;
	}
} // namespace mc