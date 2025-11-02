#ifndef LOOTINATOR_UTILITY_MTH_H
#define LOOTINATOR_UTILITY_MTH_H

#include <cinttypes>

template<typename T>
constexpr T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
constexpr T max(T a, T b) {
    return a > b ? a : b;
}

constexpr uint32_t ceil_div(uint32_t a, uint32_t b) {
    return (a + b - 1) / b;
}

#endif