#ifndef LOOTINATOR_UTILITY_RANGE_H
#define LOOTINATOR_UTILITY_RANGE_H

#include "lootinator/utility/debug.h"
#include <nlohmann/json.hpp>

namespace util {
	template <class T> struct RangeInclusive {
		T min;
		T max;

		RangeInclusive(T min, T max) : min(min), max(max) {}

		bool operator==(RangeInclusive other) const { return min == other.min && max == other.max; }

		bool operator!=(RangeInclusive other) const { return !(*this == other); }

		RangeInclusive merge(RangeInclusive other) const {
			return {min + other.min, max + other.max};
		}

		bool contains(T value) const { return value >= min && value <= max; }

		static RangeInclusive from_json(const nlohmann::json& json) {
			if (json.is_number()) {
				return {json, json};
			} else {
				return {json["min"], json["max"]};
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const RangeInclusive& range) {
			return DebugStruct(os, "RangeInclusive")
				.add("min", range.min)
				.add("max", range.max)
				.finish();
		}
	};
} // namespace util

#endif
