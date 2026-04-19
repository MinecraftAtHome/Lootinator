add_library(lootinator
	include/lootinator/assertions.h
	include/lootinator/constraint/constraint.h
	include/lootinator/data/loot_data.hpp
	include/lootinator/global_settings.hpp
	include/lootinator/kgen/bruteforce_kernel.hpp
	include/lootinator/kgen/cuda_source_gen.hpp
	include/lootinator/kgen/kernel.hpp
	include/lootinator/kgen/pipeline_generator.hpp
	include/lootinator/kgen/secondary_bruteforce_kernel.hpp
	include/lootinator/kgen/statepred_kernel.hpp
	include/lootinator/lootinator.h
	include/lootinator/mc/minecraft.hpp
	include/lootinator/probability/loot_prob.h
	include/lootinator/template/helpers.h
	include/lootinator/template/kernel/naive_bruteforce_template.h
	include/lootinator/template/kernel_template.h
	include/lootinator/template/template.h
	include/lootinator/utility/debug.h
	include/lootinator/utility/enum_bimap.hpp
	include/lootinator/utility/mth.h
	include/lootinator/utility/range.h

	src/constraint/constraint.cpp
	src/data/loot_data.cpp
	src/global_settings.cpp
	src/kgen/bruteforce_kernel.cpp
	src/kgen/cuda_source_gen.cpp
	src/kgen/kernel.cpp
	src/kgen/pipeline_generator.cpp
	src/kgen/secondary_bruteforce_kernel.cpp
	src/kgen/statepred_kernel.cpp
	src/lootinator.cpp
	src/mc/minecraft.cpp
	src/probability/loot_prob.cpp
	src/template/helpers.cpp
	src/template/kernel/naive_bruteforce_template.cpp
	src/template/kernel_template.cpp
	src/template/template.cpp
	src/utility/debug.cpp
)

target_include_directories(lootinator PUBLIC include)

set(TEST_FILES
	tests/test_kgen_config.cpp
)

# TODO: compiler flags, (-O3, warnings, etc)

# Create test driver and test source list
create_test_sourcelist(TEST_SOURCELIST CommonCxxTests.cxx ${TEST_FILES})

# Build test executable with the generated test driver
add_executable(test_driver ${TEST_SOURCELIST})
target_link_libraries(test_driver PRIVATE lootinator)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/json ${CMAKE_BINARY_DIR}/json)
target_include_directories(lootinator PUBLIC ${CMAKE_SOURCE_DIR}/external/json/single_include/)
target_link_libraries(lootinator PRIVATE nlohmann_json::nlohmann_json)

foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_test(NAME ${test_name} COMMAND test_driver tests/${test_name})
endforeach()
