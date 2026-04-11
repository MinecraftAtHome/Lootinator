#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>

#include "lootinator/constraint/constraint.h"
#include "lootinator/mc/minecraft.hpp"

namespace loot {
	// -------------------------------------------------------------------------------
	// Constraint

	bool attributes_match(
		const std::vector<mc::ItemAttribute>& first, const std::vector<mc::ItemAttribute>& second) {
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

	bool loot::Constraint::item_equal(const Constraint& other) const {
		if (item != other.item || attributes.size() != other.attributes.size())
			return false;

		// all item attributes must match
		return loot::attributes_match(attributes, other.attributes);
	}

	Constraint Constraint::truncate_attribute(int attribute_prefix_length) const {
		std::vector<mc::ItemAttribute> new_attrs;

		if (attribute_prefix_length >= 1 && !attributes.empty()) {
			new_attrs.push_back(attributes[0]);
			new_attrs[0].level = -1; // truncation unsets the level
		}
		if (attribute_prefix_length >= 2 && !attributes.empty()) {
			new_attrs[0].level = attributes[0].level; // set the level
		}

		return Constraint{item, count_range, slot_id, new_attrs};
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

	static void merge_into(std::vector<loot::Constraint>& dest, const loot::Constraint& constraint,
		std::function<bool(const Constraint& a, const Constraint& b)> cmp) {
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

	// accumulates the per-slot constraints into per-item-type ones (used by seedfinding kernels)
	// the acculumation takes into accout item enchantments
	void merge_contraints(const std::vector<loot::Constraint>& src,
		std::vector<loot::Constraint>& dest,
		std::function<bool(const Constraint& a, const Constraint& b)> cmp) {
		for (auto& constraint : dest) {
			constraint.slot_id = loot::SLOT_NONE;
		}
		for (const auto& constraint : src) {
			merge_into(dest, constraint, cmp);
		}
	}

	std::vector<mc::ItemAttribute> parse_attribute_json(nlohmann::json attribute_json) {
		std::vector<mc::ItemAttribute> attributes;
		for (auto json : attribute_json) {
			attributes.push_back(mc::ItemAttribute::from_json(json));
		}
		return attributes;
	}

	std::vector<loot::Constraint> parse_constraints_from_json(
		std::string constraint_string, std::unordered_map<std::string, int>& item_map) {
		try {
			std::vector<loot::Constraint> constraints;
			nlohmann::json data = nlohmann::json::parse(constraint_string);

			for (auto con : data) {
				std::string item_name = con["item"];
				std::int32_t item = -1;
				for (auto& value : item_map) {
					if (value.first == item_name) {
						item = item_map[item_name];
					}
				}
				if (item < 0) {
					throw loot::LootinatorError(
						loot::LootinatorErrorKind::USER_CONSTRAINT_NOT_POSSIBLE);
				}
				std::int32_t slot_id = con["slot"];
				try {
					util::RangeInclusive<std::uint32_t> count_range =
						util::RangeInclusive<std::uint32_t>::from_json(con["range"]);
					std::vector<mc::ItemAttribute> attributes =
						parse_attribute_json(con["attributes"]);
					constraints.push_back(
						{static_cast<uint32_t>(item), count_range, slot_id, attributes});
				} catch (loot::LootinatorError& e) {
					e.message += " at parse_constaint_from_json";
					throw e;
				}
			}
			return constraints;
		} catch (std::exception& e) {
			throw loot::LootinatorError(loot::LootinatorErrorKind::BAD_CONSTRAINT_FILE);
		}
	}
} // namespace loot
