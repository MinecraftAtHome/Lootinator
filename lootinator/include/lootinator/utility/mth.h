#ifndef LOOTINATOR_UTILITY_MTH_H
#define LOOTINATOR_UTILITY_MTH_H

constexpr int min(int a, int b) {
    return a < b ? a : b;
}

constexpr int max(int a, int b) {
    return a > b ? a : b;
}

constexpr int floor_div(int a, int b) {
    return (a < 0 ? (a - b + 1) / b : a / b);
}

constexpr int ceil_div(int a, int b) {
    return (a < 0 ? (a + b - 1) / b : a / b);
}

#endif