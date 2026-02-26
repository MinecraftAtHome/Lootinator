#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>

#include "lootinator/constraint/constraint.h"
#include "lootinator/mc/minecraft.hpp"

namespace loot {
	/**
	 * Utility function for comparing two item attribute vectors. Returns `true`
	 * if the vectors have the exact same contents, regardless of element order.
	 */
	bool attributes_match(
		const std::vector<mc::ItemAttribute>& first, 
		const std::vector<mc::ItemAttribute>& second
	) {
		if (first.size() != second.size())
			return false;

		for (const auto& e1 : first) {
			bool found = false;
			for (const auto& e2 : second) {
				if (e1.type == e2.type && e1.level == e2.level) {
					found = true;
					break;
				}
			}
			if (!found) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Returns whether the item data of this constraint matches exactly the item data
	 * of the `other` constraint. This takes into account both the item type and item attributes.
	 */
	bool loot::Constraint::item_equal(const Constraint& other) const {
		if (item != other.item)
			return false;
		return loot::attributes_match(attributes, other.attributes);
	}

	bool loot::Constraint::operator==(const Constraint& other) const {
		return item_equal(other) && other.count_range == count_range && other.slot_id == slot_id;
	}

	bool loot::Constraint::operator!=(const Constraint& other) const {
		return !(*this == other);
	}

	std::ostream& operator<<(std::ostream& os, const Constraint& constraint) {
		return util::DebugStruct(os, "Constraint")
			.add("item", constraint.item)
			.add("count_range", constraint.count_range)
			.add("slot_id", constraint.slot_id)
			.add("attributes", constraint.attributes)
			.finish();
	}

	// -------------------------------------------------------------------------------

	// Internal utility function that merges the provided constraint into the existing
	// constraint accumulation vector `dest`.
	static void merge_into(
		std::vector<loot::Constraint>& dest, 
		const loot::Constraint& constraint,
		std::function<bool(const Constraint& a, const Constraint& b)> cmp
	) {
		for (auto& stored_constraint : dest) {
			if (cmp(stored_constraint, constraint)) {
				// have two item count ranges (min1, max1), (min2, max2)
				// the new min is min1+min2, new max is max1+max2
				stored_constraint.count_range =
					stored_constraint.count_range.merge(constraint.count_range);
				stored_constraint.slot_id = loot::SLOT_NONE;
				return;
			}
		}

		// did not find the item, create new constraint in the destination vector
		loot::Constraint to_add = constraint;
		to_add.slot_id = loot::SLOT_NONE;
		dest.push_back(to_add);
	}

	/**
	 * Merges the provided constraint vector `src` into the destination vector `dest`, using
	 * `cmp` as the constraint equality comparison function. 
	 *
	 * For each element in `src`, if the destination vector contains an element equal to 
	 * `constraint` under `cmp`, the first matching element's count range gets merged with 
	 * the count range defined by `constraint`. Otherwise, the new constraint gets added at 
	 * the back of the destination vector.
	 * Warning: the destination vector will lose all chest slot data once this function is called.
	 */
	void merge_contraints(
		const std::vector<loot::Constraint>& src, 
		std::vector<loot::Constraint>& dest,
		std::function<bool(const Constraint& a, const Constraint& b)> cmp
	) {
		for (auto& constraint : dest) {
			constraint.slot_id = loot::SLOT_NONE;
		}
		for (const auto& constraint : src) {
			merge_into(dest, constraint, cmp);
		}
	}

	/**
	 * Parses the provided json data into a list of item attributes.
	 */
	std::vector<mc::ItemAttribute> parse_attribute_json(nlohmann::json attribute_json) {
		std::vector<mc::ItemAttribute> attributes;
		for (auto json : attribute_json) {
			attributes.push_back(mc::ItemAttribute::from_json(json));
		}
		return attributes;
	}

	/**
	 * Parses the provided json data (available in file `filepath`) into a list of loot constraints.
	 */
	std::vector<loot::Constraint> parse_constraints_from_json(
		const char* filepath, 
		std::unordered_map<std::string, int>& item_map
	) {
		std::vector<loot::Constraint> constraints;
		std::ifstream f(filepath);
		nlohmann::json data = nlohmann::json::parse(f);
		for (auto con : data) {
			std::string item_name = con["item"];
			std::uint32_t item = item_map[item_name];
			std::int32_t slot_id = con["slot"];
			util::RangeInclusive<std::uint32_t> count_range =
				util::RangeInclusive<std::uint32_t>::from_json(con["range"]);
			std::vector<mc::ItemAttribute> attributes = parse_attribute_json(con["attributes"]);
			constraints.push_back({item, count_range, slot_id, attributes});
		}
		return constraints;
	}
} // namespace loot
