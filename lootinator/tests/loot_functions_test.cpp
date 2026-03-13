#include "lootinator/assertions.h"
#include "lootinator/data/loot_data.hpp"
#include "lootinator/utility/debug.h"
#include <algorithm>
#include <fstream>
#include <iostream>

static void smoke_test() {
	{
		std::ifstream f("../../lootinator/tests/data/ruined_portal.json");
		nlohmann::json loot_table_json = nlohmann::json::parse(f);
		data::LootTableRoot root = data::LootTableRoot(loot_table_json,
			"../../lootinator/tests/data/item_map_rp.txt",
			mc::VersionRange::MC_1_16_TO_1_20);
	}
	{
		std::ifstream f("../../lootinator/tests/data/end_city_1_21_8.json");
		nlohmann::json loot_table_json = nlohmann::json::parse(f);
		data::LootTableRoot root = data::LootTableRoot(loot_table_json,
			"../../lootinator/tests/data/item_map_end_city.txt",
			mc::VersionRange::MC_1_21_TO_1_21_10);
	}
}

static void correctness_test_enchant_randomly_list() {
	std::ifstream f("../../lootinator/tests/data/bastion_1_17_1.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);
	data::LootTableRoot root = data::LootTableRoot(loot_table_json,
		"../../lootinator/tests/data/item_map_bastion.txt",
		mc::VersionRange::MC_1_16_TO_1_20);

	std::vector<mc::Enchantment> target_order({mc::SOUL_SPEED});
	std::vector<int> target_shmem({3});

	data::LootTreeNode* node = root.children[0]->children[10]->children[0];
	data::LootFunctionData* func_node = dynamic_cast<data::LootFunctionData*>(node);
	data::LootFunctionData* prevent_this = nullptr;
	ASSERT_NE(func_node, prevent_this);

	ASSERT_EQ(std::equal(target_order.begin(),
				  target_order.end(),
				  func_node->enchant_randomly.enchantment_order.begin()),
		true);
	ASSERT_EQ(
		std::equal(target_shmem.begin(), target_shmem.end(), func_node->shared_mem.begin()), true);
}

static void correctness_test_enchant_randomly_natural() {
	std::ifstream f("../../lootinator/tests/data/ruined_portal.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);
	data::LootTableRoot root = data::LootTableRoot(loot_table_json,
		"../../lootinator/tests/data/item_map_rp.txt",
		mc::VersionRange::MC_1_16_TO_1_20);

	data::LootTreeNode* node = root.children[0]->children[14]->children[0];
	data::LootFunctionData* func_node = dynamic_cast<data::LootFunctionData*>(node);
	data::LootFunctionData* prevent_this = nullptr;
	ASSERT_NE(func_node, prevent_this);

	std::vector<mc::Enchantment> target_order({mc::PROTECTION,
		mc::FIRE_PROTECTION,
		mc::BLAST_PROTECTION,
		mc::PROJECTILE_PROTECTION,
		mc::RESPIRATION,
		mc::AQUA_AFFINITY,
		mc::THORNS,
		mc::UNBREAKING,
		mc::CURSE_OF_BINDING,
		mc::CURSE_OF_VANISHING,
		mc::MENDING});
	std::vector<int> target_shmem({4, 4, 4, 4, 3, 1, 3, 3, 1, 1, 1});

	ASSERT_EQ(std::equal(target_order.begin(),
				  target_order.end(),
				  func_node->enchant_randomly.enchantment_order.begin()),
		true);
	ASSERT_EQ(
		std::equal(target_shmem.begin(), target_shmem.end(), func_node->shared_mem.begin()), true);
}

static void correctness_test_enchant_with_levels() {
	std::ifstream f("../../lootinator/tests/data/end_city_1_21_8.json");
	nlohmann::json loot_table_json = nlohmann::json::parse(f);
	data::LootTableRoot root = data::LootTableRoot(loot_table_json,
		"../../lootinator/tests/data/item_map_end_city.txt",
		mc::VersionRange::MC_1_21_TO_1_21_10);

	data::LootTreeNode* node = root.children[0]->children[10]->children[0];
	data::LootFunctionData* func_node = dynamic_cast<data::LootFunctionData*>(node);
	data::LootFunctionData* prevent_this = nullptr;
	ASSERT_NE(func_node, prevent_this);

	const uint32_t target_minlevel = 20, target_maxlevel = 39;
	const uint32_t target_enchantability = 10;
	ASSERT_EQ(target_minlevel, func_node->enchant_with_levels.level.min);
	ASSERT_EQ(target_maxlevel, func_node->enchant_with_levels.level.max);
	ASSERT_EQ(target_enchantability, func_node->enchant_with_levels.enchantability);

	std::vector<int> target_shmem({0,
		1,
		1,
		1,
		1,
		3,
		3,
		3,
		3,
		3,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		7,
		7,
		7,
		7,
		7,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		6,
		4,
		4,
		4,
		4,
		4,
		2});

	std::cerr << "Got:\n";
	util::debug(std::cerr, func_node->shared_mem) << std::endl;
	std::cerr << "Expected:\n";
	util::debug(std::cerr, target_shmem) << std::endl;

	ASSERT_EQ(
		std::equal(target_shmem.begin(), target_shmem.end(), func_node->shared_mem.begin()), true);
}

int LOOTINATOR_EXTERN tests_loot_functions_test(int argc, char** const argv) {
	(void)argc;
	(void)argv;

	ASSERT_NOEXCEPT(smoke_test());
	ASSERT_NOEXCEPT(correctness_test_enchant_randomly_list());
	ASSERT_NOEXCEPT(correctness_test_enchant_randomly_natural());
	ASSERT_NOEXCEPT(correctness_test_enchant_with_levels());

	return 0;
}