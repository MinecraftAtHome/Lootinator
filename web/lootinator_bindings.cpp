#include <emscripten/bind.h>
#include "lootinator/lootinator.h"
#include "lootinator/mc/minecraft.hpp"

using namespace emscripten;

namespace {

val generate_best_pipeline_heur(
    std::string loot_table,
    std::string constraints,
    mc::VersionRange version_range,
    bool seedcracking_mode
) {
    std::string result;

    loot::LootinatorError err = loot::generate_best_pipeline_heur_from_string(
        loot_table,
        constraints,
        version_range,
        seedcracking_mode,
        &result
    );

    val error = val::object();
    error.set("kind", err.kind);
    error.set("message", err.message);

    val out = val::object();
    out.set("result", result);
    out.set("error", error);

    return out;
}

val generate_benchmark_source(
    std::string loot_table,
    std::string constraints,
    mc::VersionRange version_range,
    bool seedcracking_mode
) {
    std::string result;

    loot::LootinatorError err = loot::generate_benchmark_source_from_string(
        loot_table,
        constraints,
        version_range,
        seedcracking_mode,
        &result
    );

    val error = val::object();
    error.set("kind", err.kind);
    error.set("message", err.message);

    val out = val::object();
    out.set("result", result);
    out.set("error", error);

    return out;
}

}

EMSCRIPTEN_BINDINGS(lootinator_module) {
    function("generate_best_pipeline_heur", &generate_best_pipeline_heur);
    function("generate_benchmark_source", &generate_benchmark_source);

    enum_<loot::LootinatorErrorKind>("LootinatorErrorKind")
        .value("SUCCESS", loot::LootinatorErrorKind::SUCCESS)
        .value("USER_CONSTRAINT_TOO_WEAK", loot::LootinatorErrorKind::USER_CONSTRAINT_TOO_WEAK)
        .value("USER_CONSTRAINT_NOT_POSSIBLE", loot::LootinatorErrorKind::USER_CONSTRAINT_NOT_POSSIBLE)
        .value("BAD_CONSTRAINT_FILE", loot::LootinatorErrorKind::BAD_CONSTRAINT_FILE)
        .value("BAD_LOOT_TABLE", loot::LootinatorErrorKind::BAD_LOOT_TABLE)
        .value("RANGE_PARSE", loot::LootinatorErrorKind::RANGE_PARSE)
        .value("INTERNAL_ERROR", loot::LootinatorErrorKind::INTERNAL_ERROR);

    enum_<mc::VersionRange>("VersionRange")
    #define X(v) .value(#v, mc::v)
        VersionRangeList(X)
    #undef X
    ;
}
