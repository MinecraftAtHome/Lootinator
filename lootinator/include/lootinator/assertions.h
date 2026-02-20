#ifndef LOOTINATOR_ASSERTIONS_H
#define LOOTINATOR_ASSERTIONS_H

#include "lootinator/utility/debug.h"
#include <iostream>

#ifdef _WIN32
#define LOOTINATOR_EXTERN __cdecl
#else
#define LOOTINATOR_EXTERN
#endif

namespace loot {
	template <class T, class U>
	void assert_compare_fail(const char* file, int line, const char* operator_symbol,
		const T& first, const U& second, const char* first_string, const char* second_string) {
		std::cerr << file << ":" << line << "\nAssertion \'" << operator_symbol << "\' failed!\n";
		util::debug(std::cerr << first_string << " : ", first) << "\n";
		util::debug(std::cerr << second_string << " : ", second) << "\n";
		std::terminate();
	}

	inline void assert_noexcept_fail(const char* file, int line, const char* message) {
		std::cerr << file << ":" << line << "No-exception assertion failed!\n"
				  << message << std::endl;
		std::terminate();
	}

#define ASSERT_COMPARE(first, operator_symbol, second)                                             \
	do {                                                                                           \
		const auto& first_var = first;                                                             \
		const auto& second_var = second;                                                           \
		if (!(first_var operator_symbol second_var)) {                                             \
			loot::assert_compare_fail(                                                             \
				__FILE__, __LINE__, #operator_symbol, first_var, second_var, #first, #second);     \
		}                                                                                          \
	} while (false)

	// ----------------------------------------------

#define ASSERT_EQ(first, second) ASSERT_COMPARE(first, ==, second)
#define ASSERT_NE(first, second) ASSERT_COMPARE(first, !=, second)
#define ASSERT_GT(first, second) ASSERT_COMPARE(first, >, second)
#define ASSERT_LT(first, second) ASSERT_COMPARE(first, <, second)
#define ASSERT_GE(first, second) ASSERT_COMPARE(first, >=, second)
#define ASSERT_LE(first, second) ASSERT_COMPARE(first, <=, second)

#define ASSERT_NOEXCEPT(code)                                                                      \
	do {                                                                                           \
		try {                                                                                      \
			code;                                                                                  \
		} catch (const std::exception& any) {                                                      \
			loot::assert_noexcept_fail(__FILE__, __LINE__, any.what());                            \
		}                                                                                          \
	} while (false)
} // namespace loot

#endif
