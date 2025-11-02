#include "lootinator/utility/debug.h"

namespace util {
    DebugArray::DebugArray(std::ostream& os) : os(&os) {
        os << "[";
    }

    std::ostream& DebugArray::finish() {
        return *os << "]";
    }

    DebugStruct::DebugStruct(std::ostream& os, const char* name) : os(&os) {
        os << name << "{";
    }

    std::ostream& DebugStruct::finish() {
        return *os << "}";
    }
}