#ifndef LOOTINATOR_UTILITY_MTH_H
#define LOOTINATOR_UTILITY_MTH_H

#include <cinttypes>
#include <cmath>

namespace util {
	uint64_t fact(uint64_t n) {
		if (n <= 0) {
			return 1;
		}
		return n * fact(n - 1);
	}

	uint64_t choose(uint64_t n, uint64_t r) {
		return fact(n) / (fact(r) * fact(n - r));
	}

	template <typename T> constexpr T min(T a, T b) {
		return a < b ? a : b;
	}

	template <typename T> constexpr T max(T a, T b) {
		return a > b ? a : b;
	}

	constexpr uint32_t ceil_div(uint32_t a, uint32_t b) {
		return (a + b - 1) / b;
	}
} // namespace util

#endif