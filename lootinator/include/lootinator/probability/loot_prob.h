#ifndef LOOTINATOR_PROBABILITY_LOOT_PROB_H
#define LOOTINATOR_PROBABILITY_LOOT_PROB_H

#include "lootinator/data/loot_data.hpp"
#include "lootinator/kgen/kernel.hpp"
#include <vector>

namespace prob {
	struct LootEntry {
		int item_id;
		double weight;
		int count_min;
		int count_max;
	};

	struct LootPool {
		int rolls_min;
		int rolls_max;
		std::vector<LootEntry> entries;
	};

	struct LootTable {
		std::vector<LootPool> pools;
	};

	struct TargetItem {
		int item_id;
		int item_count;
		bool need_exact_count;
	};

	LootTable get_probability_table(const data::LootTableRoot& loot_table);

	double get_probability_of_config(kgen::KernelGenConfig& config);
	double get_loot_probability(
		const LootTable& loot_table, const std::vector<prob::TargetItem>& target_items);
} // namespace prob

#endif