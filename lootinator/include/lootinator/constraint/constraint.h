#ifndef LOOTINATOR_CONSTRAINT_CONSTRAINT_H
#define LOOTINATOR_CONSTRAINT_CONSTRAINT_H

#include "lootinator/mc/minecraft.hpp"
#include "lootinator/utility/debug.h"
#include "lootinator/utility/range.h"

#include <cstdint>
#include <functional>

namespace loot {
	// Used to mark an unused slot index constraint (`Constraint::slot_id`).
	constexpr int32_t SLOT_NONE = -1;

	
	bool attributes_match(
		const std::vector<mc::ItemAttribute>& first, 
		const std::vector<mc::ItemAttribute>& second
	);

	/**
	 * Represents a single constraint on a loot table.
	 */
	struct Constraint {
		// The internal item index for which the constraint is specified.
		std::uint32_t item;
		// an inclusive range of item counts required by the constraint.
		util::RangeInclusive<std::uint32_t> count_range;
		// (optional) The slot index of the item stack inside the target chest's layout. 0 = top left corner.
		std::int32_t slot_id;
		// A list of all attributes required by the constraint.
		std::vector<mc::ItemAttribute> attributes; // TODO: make this not a vector...

		bool item_equal(const Constraint& other) const;

		bool operator==(const Constraint& other) const;
		bool operator!=(const Constraint& other) const;

		friend std::ostream& operator<<(std::ostream& os, const Constraint& constraint);
	};

	void merge_contraints(
		const std::vector<loot::Constraint>& src,
		std::vector<loot::Constraint>& dest,
		std::function<bool(const Constraint& a, const Constraint& b)> cmp
	);

	std::vector<loot::Constraint> parse_constraints_from_json(
		const char* filepath, 
		std::unordered_map<std::string, int>& item_map
	);

} // namespace loot

#endif
