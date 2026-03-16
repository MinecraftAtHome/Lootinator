#include "lootinator/data/loot_data.hpp"
#include "lootinator/utility/debug.h"

#include <fstream>
#include <iostream>

namespace data {
	// ---------------------------------------------------------------
	// LootTreeNode - base class
	
	LootTreeNode::LootTreeNode(data::LootTreeNode* parent) {
		this->parent = parent;
		this->child_index = parent == nullptr ? 0 : parent->children.size() - 1;
	}

	/**
	 * Returns the Minecraft version range of this node's tree.
	 */
	mc::VersionRange LootTreeNode::get_version() {
		return dynamic_cast<LootTableRoot*>(get_root_node())->version;
	}

	/**
	 * Returns the pointer to the root node of the tree.
	 */
	LootTreeNode* LootTreeNode::get_root_node() {
		LootTreeNode* current = this;
		while (current->parent != nullptr) {
			current = current->parent;
		}
		return current;
	}

	LootTreeNode::~LootTreeNode() {
		for (auto child : children) {
			delete child;
		}
	}

	/**
	 * Returns the minimum number of states by which this node and all of its children can advance 
	 * the LCG. By default, returns the sum of all childrens' `get_min_lcg_advancement` outputs.
	 */
	uint32_t LootTreeNode::get_min_lcg_advancement() const {
		uint32_t total_advance = 0;
		for (auto& func : children) {
			total_advance += func->get_min_lcg_advancement();
		}
		return total_advance;
	}

	/**
	 * Returns the maximum number of states by which this node and all of its children can advance 
	 * the LCG. By default, returns the sum of all childrens' `get_max_lcg_advancement` outputs.
	 * This doesn't account for any possible extra nextInt(n) advancements by design.
	 */
	uint32_t LootTreeNode::get_max_lcg_advancement() const {
		uint32_t total_advance = 0;
		for (auto& func : children) {
			total_advance += func->get_max_lcg_advancement();
		}
		return total_advance;
	}

	/**
	 * Fetches the internal item index for the provided item name. Returns -1 if the item was not
	 * found in the item to index mapping.
	 */
	int LootTreeNode::get_item_index(const std::string& item_name) const {
		if (parent != nullptr) {
			return parent->get_item_index(item_name);
		}
		throw "illegal state in LootTreeNode::get_item_index - parent was nullptr";
	}

	/**
	 * Deletes all constraints from this loot tree node.
	 */
	void LootTreeNode::clear_constraints() {
		this->constraints.clear();
		for (auto child : this->children) {
			child->clear_constraints();
		}
	}

	// Utility for indentation
	static void indent(int indentation) {
		std::cout << '\n';
		for (int i = 0; i < indentation; i++) {
			std::cout << ' ';
		}
	}

	/**
	 * Prints the entire loot table (sub-)tree to console, treating this node as the root.
	 */
	void LootTreeNode::print(int indentation) const {
		for (const auto& child : children) {
			child->print(indentation);
			// std::cout << '\n';
		}
		print_constraints(indentation);
	}

	// Internal helper for loot tree printing. Prints the vector of constraints for the current node.
	void LootTreeNode::print_constraints(int indentation) const {
		indent(indentation);
		std::cout << "Constraints: [";
		for (auto& constraint : constraints) {
			std::cout << constraint;
			std::cout << ", ";
		}
		std::cout << ']';
	}

	uint32_t LootEntry::get_min_lcg_advancement() const {
		uint32_t total_advance = 0;
		for (auto& func : children) {
			total_advance += func->get_min_lcg_advancement();
		}
		return total_advance;
	}

	uint32_t LootEntry::get_max_lcg_advancement() const {
		uint32_t total_advance = 0;
		for (auto& func : children) {
			total_advance += func->get_max_lcg_advancement();
		}
		return total_advance;
	}

	LootEntry::LootEntry(LootTreeNode* parent, const nlohmann::json& json,
		const util::RangeInclusive<uint32_t> next_int_range, int index)
		: next_int_range(next_int_range), LootTreeNode(parent) {
		// entry parsing
		weight = json.contains("weight") ? static_cast<int>(json["weight"]) : 1;
		this->index = index;
		if (json["type"] == "minecraft:empty") {
			// empty entry
			name = "";
			type = mc::ItemType::NO_ITEM;
			item = -1;
		} else {
			// item entry
			name = json["name"];
			type = mc::string_to_item_type(name);
			item = get_item_index(name);
		}

		// loot functions
		if (json.contains("functions")) {
			for (auto& function_json : json["functions"]) {
				children.push_back(new data::LootFunctionData(this, function_json));
			}
		}
	}

	void LootEntry::print(int indentation) const {
		indent(indentation);

		util::DebugStruct(std::cout, "LootEntry")
			.add("item", item)
			.add("name", name)
			.add("type", type)
			.add("weight", weight)
			.add("min_next_int", next_int_range.min)
			.add("max_next_int", next_int_range.max)
			.finish();
		printf(" %p", (void*)this);

		LootTreeNode::print(indentation + LootTreeNode::INDENT_SIZE);
	}

	// ---------------------------------------------------------------
	// LootTableRoot

	/**
	 * Constructs the loot table tree within the LootTableRoot object.
	 * @param json the loot table json data
	 * @param item_map the item name to item index map to be used while parsing the loot table
	 * @param version the version range of the target loot table
	 */

	LootTableRoot::LootTableRoot(const nlohmann::json& json,
		const std::unordered_map<std::string, int>& item_map, mc::VersionRange version)
		: item_map(item_map), LootTreeNode(nullptr) {
		this->id_counter = 0;
		this->version = version;

		auto& pools = json["pools"];
		for (auto& pool : pools) {
			this->children.push_back(new LootPool(this, pool));
		}
	}

	void LootTableRoot::add_constraints(const std::vector<loot::Constraint>& new_constraints) {
		for (const auto& constraint : new_constraints) {
			std::vector<int> matching_pools;
			int pool_idx = 0;
			for (auto child : children) {
				LootPool* pool = dynamic_cast<LootPool*>(child);
				if (pool->matches_constraint(constraint)) {
					matching_pools.push_back(pool_idx);
				}
				pool_idx++;
			}

			if (matching_pools.empty()) {
				throw "we messed up";
			} else if (matching_pools.size() > 1) {
				constraints.push_back(constraint);
			} else {
				children[matching_pools[0]]->constraints.push_back(constraint);
			}
		}
	}

	int LootTableRoot::get_item_index(const std::string& item_name) const {
		if (item_map.find(item_name) != item_map.end()) {
			return item_map.at(item_name);
		}
		return -1;
	}

	void LootTableRoot::print(int indentation) const {
		indent(indentation);

		util::DebugStruct(std::cout, "LootTableRoot").add("version", version).finish();
		printf(" %p", (void*)this);
		LootTreeNode::print(indentation + LootTreeNode::INDENT_SIZE);
	}

	// ---------------------------------------------------------------
	// LootPool

	LootPool::LootPool(LootTreeNode* parent, const nlohmann::json& json)
		: rolls(util::RangeInclusive<std::uint32_t>::from_json(json["rolls"])),
		  LootTreeNode(parent) {

		int entry_index = 0;
		for (auto& entry : json["entries"]) {
			uint32_t entry_weight =
				(entry.contains("weight") ? static_cast<uint32_t>(entry["weight"]) : 1);
			uint32_t start_weight = static_cast<uint32_t>(entry_lookup.size());
			uint32_t end_weight =
				start_weight + entry_weight - 1; // -1 accounts for range being inclusive-inclusive

			this->children.push_back(
				new LootEntry(this, entry, {start_weight, end_weight}, entry_index++));

			for (uint32_t w = 0; w < entry_weight; w++) {
				this->entry_lookup.push_back(
					dynamic_cast<LootEntry*>(this->children.back())->index);
			}
		}
	}

	bool LootPool::matches_constraint(const loot::Constraint& constraint) const {
		bool matches = false;
		for (auto child : children) {
			LootEntry* entry = dynamic_cast<LootEntry*>(child);
			if (entry->matches_constraint(constraint)) {
				entry->constraints.push_back(constraint);
				matches = true;
			}
		}
		return matches;
	}

	uint32_t LootPool::get_total_weight() const {
		return static_cast<uint32_t>(entry_lookup.size());
	}

	uint32_t LootPool::get_min_lcg_advancement() const {
		uint32_t min_among_children = 1000;
		for (auto& child : children) {
			min_among_children = std::min(min_among_children, child->get_min_lcg_advancement());
		}

		uint32_t item_choice_advance = (children.size() == 1 ? 0 : 1);
		uint32_t roll_count_advance = (rolls.min == rolls.max ? 0 : 1);

		return roll_count_advance + rolls.min * (min_among_children + item_choice_advance);
	}

	uint32_t LootPool::get_max_lcg_advancement() const {
		uint32_t max_among_children = 0;
		for (auto& child : children) {
			max_among_children = std::max(max_among_children, child->get_max_lcg_advancement());
		}

		uint32_t item_choice_advance = (children.size() == 1 ? 0 : 1);
		uint32_t roll_count_advance = (rolls.min == rolls.max ? 0 : 1);

		return roll_count_advance + rolls.max * (max_among_children + item_choice_advance);
	}

	void LootPool::print(int indentation) const {
		indent(indentation);

		util::DebugStruct(std::cout, "LootPool")
			.add("min_rolls", rolls.min)
			.add("max_rolls", rolls.max)
			.add("total_weight", get_total_weight())
			.finish();
		printf(" %p", (void*)this);
		LootTreeNode::print(indentation + LootTreeNode::INDENT_SIZE);
	}


	// ---------------------------------------------------------------
	// LootEntry

	uint32_t LootEntry::get_min_lcg_advancement() const {
		// TODO remove, deprecated
		return LootTreeNode::get_min_lcg_advancement();
	}

	uint32_t LootEntry::get_max_lcg_advancement() const {
		// TODO remove, deprecated
		return LootTreeNode::get_max_lcg_advancement();
	}

	LootEntry::LootEntry(LootTreeNode* parent, const nlohmann::json& json,
		const util::RangeInclusive<uint32_t> next_int_range)
		: next_int_range(next_int_range) {
		this->parent = parent;

		// entry parsing
		weight = json.contains("weight") ? static_cast<int>(json["weight"]) : 1;

		if (json["type"] == "minecraft:empty") {
			// empty entry
			name = "";
			type = mc::ItemType::NO_ITEM;
			item = -1;
		} else {
			// item entry
			name = json["name"];
			type = mc::string_to_item_type(name);
			item = get_item_index(name);
		}

		// loot functions
		if (json.contains("functions")) {
			for (auto& function_json : json["functions"]) {
				children.push_back(new data::LootFunctionData(this, function_json));
			}
		}
	}

	bool LootEntry::matches_constraint(const loot::Constraint& constraint) const {
		if (item < 0 || static_cast<uint32_t>(item) != constraint.item) {
			return false;
		}

		for (auto& attribute : constraint.attributes) {
			bool any_matched = false;
			for (auto& child : children) {
				LootFunctionData* func = dynamic_cast<LootFunctionData*>(child);

				if (attribute.get_category() == mc::AttributeCategory::ENCHANTMENT_ATTRIBUTE) {
					if (func->type == LootFunctionType::ENCHANT_WITH_LEVELS) {
						any_matched = true;
					} else if (func->type == LootFunctionType::ENCHANT_RANDOMLY) {
						auto& vec = func->enchant_randomly.enchantment_order;
						if (std::find(vec.begin(),
								vec.end(),
								mc::get_enchantment_from_attribute(attribute)) != vec.end()) {
							any_matched = true;
						}
					}
				}
			}
			if (!any_matched) {
				return false;
			}
		}

		return true;
	}

	util::RangeInclusive<uint32_t> LootEntry::get_count_range() const {
		for (auto child : children) {
			CAST_CHILD(lf, data::LootFunctionData, child);
			if (lf->type == data::SET_COUNT) {
				return lf->set_count;
			}
		}
		return {1, 1};
	}

	void LootEntry::print(int indentation) const {
		indent(indentation);

		util::DebugStruct(std::cout, "LootEntry")
			.add("item", item)
			.add("name", name)
			.add("type", type)
			.add("weight", weight)
			.add("min_next_int", next_int_range.min)
			.add("max_next_int", next_int_range.max)
			.finish();
		printf(" %p", (void*)this);

		LootTreeNode::print(indentation + LootTreeNode::INDENT_SIZE);
	}


	// ---------------------------------------------------------------
	// LootFunctionData

	LootFunctionData::LootFunctionData(LootTreeNode* parent, const nlohmann::json& json)
		: LootTreeNode(parent) {

		LootTableRoot* root = dynamic_cast<LootTableRoot*>(get_root_node());
		id = root->id_counter;
		(root->id_counter)++;
		type = LootFunctionType::IGNORED;

		this->children = std::vector<LootTreeNode*>();

		// function parsing
		std::string function_name = mc::strip_prefix(json["function"]);

		if (function_name == "enchant_randomly") {
			// some versions define available enchantments as "options", others use "enchantments"
			if (json.contains("options") && json["options"] != "#minecraft:on_random_loot") {
				create_list_enchant_randomly(json["options"]);
			} else if (json.contains("enchantments")) {
				create_list_enchant_randomly(json["enchantments"]);
			} else {
				// defaulting to the full list if enchantments undefined
				create_enchant_randomly(json);
			}
		} else if (function_name == "enchant_with_levels") {
			create_enchant_with_levels(json);
		} else if (function_name == "set_count") {
			type = data::LootFunctionType::SET_COUNT;
			set_count = util::RangeInclusive<uint32_t>::from_json(json["count"]);
		} else if (function_name == "set_damage") {
			type = data::LootFunctionType::APPLY_DAMAGE;
		}
	}

	void LootFunctionData::print(int indentation) const {
		indent(indentation);

		static const char* function_names[] = {
			"enchant_randomly", "enchant_with_levels", "set_count", "apply_damage", "NULL"};
		util::DebugStruct(std::cout, "LootFunction")
			.add("function_type", function_names[type])
			.finish();
		printf(" %p", (void*)this);
		indent(indentation + LootTreeNode::INDENT_SIZE);
		std::cout << "SharedMemory: ";
		util::debug(std::cout, shared_mem);

		indent(indentation + LootTreeNode::INDENT_SIZE);
		if (type == data::LootFunctionType::ENCHANT_RANDOMLY) {
			std::cout << "Enchantments: ";
			util::debug(std::cout, enchant_randomly.enchantment_order);
		} else {
			util::DebugStruct extra_data(std::cout, "ExtraData");

			switch (type) {
				case data::LootFunctionType::ENCHANT_WITH_LEVELS:
					extra_data.add("enchantability", enchant_with_levels.enchantability);
					extra_data.add("min_level", enchant_with_levels.level.min);
					extra_data.add("max_level", enchant_with_levels.level.max);
					break;
				case data::LootFunctionType::SET_COUNT:
					extra_data.add("min_count", set_count.min);
					extra_data.add("max_count", set_count.max);
					break;
				default:
					break;
			}

			extra_data.finish();
		}
		LootTreeNode::print(indentation + LootTreeNode::INDENT_SIZE);
	}

	uint32_t LootFunctionData::get_min_lcg_advancement() const {
		switch (type) {
			case APPLY_DAMAGE:
				return 1;
			case SET_COUNT:
				return set_count.min == set_count.max ? 0 : 1;
			case ENCHANT_RANDOMLY:
				return get_enchant_randomly_advancement(std::min, true);
			case ENCHANT_WITH_LEVELS:
				return enchant_with_levels.level.min == enchant_with_levels.level.max ? 6 : 6 + 1;
			default:
				return 0;
		}
	}

	uint32_t LootFunctionData::get_max_lcg_advancement() const {
		switch (type) {
			case APPLY_DAMAGE:
				return 1;
			case SET_COUNT:
				return set_count.min == set_count.max ? 0 : 1;
			case ENCHANT_RANDOMLY:
				return get_enchant_randomly_advancement(std::max, false);
			case ENCHANT_WITH_LEVELS:
				return get_enchant_with_levels_max_advancement();
			default:
				return 0;
		}
	}

	// function parsing helpers

	uint32_t LootFunctionData::get_enchant_with_levels_max_advancement() const {
		uint32_t max_unamplified =
			enchant_with_levels.level.max + 1 + (enchant_with_levels.enchantability / 4) * 2;
		uint32_t effective_max = static_cast<uint32_t>(std::ceil(1.15f * max_unamplified));
		uint32_t min_unamplified = enchant_with_levels.level.min + 1;
		uint32_t effective_min = static_cast<uint32_t>(std::floor(0.85f * min_unamplified));

		uint32_t max_groups = 0; // enchantment groups for max case
		for (uint32_t lvl = effective_min; lvl <= effective_max; lvl++) {
			max_groups = std::max(max_groups, static_cast<uint32_t>(shared_mem[lvl]));
		}

		uint32_t base_calls =
			enchant_with_levels.level.min == enchant_with_levels.level.max ? 5 : 6;
		return base_calls + (max_groups - 1) * 2 + 1;
	}

	uint32_t LootFunctionData::get_enchant_randomly_advancement(
		const uint32_t& (*compare_func)(const uint32_t&, const uint32_t&), bool is_min) const {
		uint32_t total_advance = 1;
		uint32_t extreme_level = is_min ? 10 : 0;
		for (auto enchantment : enchant_randomly.enchantment_order) {
			uint32_t level = mc::get_max_level(enchantment);
			extreme_level = compare_func(extreme_level, level);
		}

		return total_advance + (extreme_level > 1 ? 1 : 0);
	}

	/**
	 * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`,
	 * where `max_i` is the maximum level of the i-th enchantment.
	 * Enchantment indices follow the natural enchantment order inside the loot function, not enum
	 * values. The order is stored outside of shared memory and used by the compiler to map
	 * enchantment contraints to function-specific indices. This variant of `enchant_randomly` uses
	 * a user-provided list of applicable enchantments.
	 */
	void LootFunctionData::create_list_enchant_randomly(const nlohmann::json& list) {
		for (auto& entry : list) {
			if (!entry.is_string()) {
				std::fprintf(stderr,
					"create_list_enchant_randomly_vector(): got non-string enchant "
					"list element, skipped.\n");
				continue;
			}
			mc::Enchantment ench = mc::string_to_enchantment(mc::strip_prefix(entry));
			if (ench == mc::Enchantment::NO_ENCHANTMENT) {
				std::fprintf(stderr,
					"create_list_enchant_randomly_vector(): got unrecognized enchantment, "
					"skipped.\n");
				continue;
			}
			int max_level = mc::get_max_level(ench);
			shared_mem.push_back(max_level);
			enchant_randomly.enchantment_order.push_back(ench);
		}
		type = data::LootFunctionType::ENCHANT_RANDOMLY;
	}

	/**
	 * Shared memory structure: `[max_1, max_2, max_3, ..., max_n]`,
	 * where `max_i` is the maximum level of the i-th enchantment.
	 * Enchantment indices follow the natural enchantment order inside the loot function, not enum
	 * values. The order is stored outside of shared memory and used by the compiler to map
	 * enchantment contraints to function-specific indices. This variant of `enchant_randomly` uses
	 * the standard Minecraft enchantment order for the appropriate version range.
	 */
	void LootFunctionData::create_enchant_randomly(const nlohmann::json& function) {
		data::LootEntry* entry = dynamic_cast<data::LootEntry*>(parent);
		std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(get_version());
		bool allow_treasure = function.contains("treasure") ? (bool)function["treasure"] : true;

		// add enchantments in natural order
		for (auto ench : all_enchants) {
			if (!allow_treasure && mc::is_treasure_enchantment(ench)) {
				continue;
			}
			if (mc::is_enchantment_applicable(ench, entry->type, true)) {
				int max_level = mc::get_max_level(ench);
				shared_mem.push_back(max_level);
				enchant_randomly.enchantment_order.push_back(ench);
			}
		}
		type = data::LootFunctionType::ENCHANT_RANDOMLY;
	}

	/**
	 * LootFunctionData::create_enchant_with_levels helper. Returns the number of unique enchantment
	 * groups for a given item and a enchant_with_levels effective level.
	 */
	static int get_enchant_with_levels_groups(const std::vector<mc::Enchantment>& enchants,
		int level, mc::ItemType item_type, bool allow_treasure) {
		std::vector<mc::Enchantment> applicable;
		for (const auto& ench : enchants) {
			if (!mc::is_enchantment_applicable(ench, item_type, false)) {
				continue;
			}
			if (!allow_treasure && mc::is_treasure_enchantment(ench)) {
				continue;
			}

			for (int ench_level = mc::get_max_level(ench); ench_level >= 1; ench_level--) {
				if (!mc::is_enchantment_available_at_level(ench, ench_level, level)) {
					continue;
				}
				applicable.push_back(ench);
			}
		}
		return mc::count_unique_groups(applicable);
	}

	/**
	 * Shared memory structure: `[groups_1, groups_2, groups_3, ..., groups_n]`,
	 * where `count_i` is the number of mutually-exclusive enchantment groups among all applicable
	 * enchantments, e.g. `{"fortune", "silk_touch"}`, for an effective enchanting level `i`.
	 * Enchantment indices are not stored; `enchant_with_levels` output filtering is currently
	 * unsupported.
	 */
	void LootFunctionData::create_enchant_with_levels(const nlohmann::json& function) {
		data::LootEntry* entry = dynamic_cast<data::LootEntry*>(parent);
		std::vector<mc::Enchantment> all_enchants = mc::get_enchantments_for_version(get_version());
		bool allow_treasure = function.contains("treasure") ? (bool)function["treasure"] : true;

		if (!function.contains("levels")) {
			fprintf(stderr,
				"create_skip_enchant_with_levels_vector(): levels undefined in loot "
				"function, parsing skipped.\n");
			return;
		}
		enchant_with_levels.level = util::RangeInclusive<uint32_t>::from_json(function["levels"]);

		enchant_with_levels.enchantability = mc::get_enchantability(entry->name);
		uint32_t max_unamplified =
			enchant_with_levels.level.max + 1 + (enchant_with_levels.enchantability / 4) * 2;
		uint32_t effective_max = static_cast<uint32_t>(std::ceil(1.15f * max_unamplified));
		// uint32_t min_unamplified = enchant_with_levels.level.min + 1;
		// uint32_t effective_min = static_cast<uint32_t>(std::floor(0.85f * min_unamplified));

		for (uint32_t level = 0; level <= effective_max; level++) {
			shared_mem.push_back(
				get_enchant_with_levels_groups(all_enchants, level, entry->type, allow_treasure));
		}

		type = data::LootFunctionType::ENCHANT_WITH_LEVELS;
	}
} // namespace data
