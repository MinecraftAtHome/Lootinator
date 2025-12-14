import glob
import pathlib

path_pfx = pathlib.Path(__file__).parent.resolve()


def get_file_list(dirname, flag):
    file_list = list(filter(lambda x: '.' in x, glob.glob(str(dirname), recursive=True)))
    result = ''
    list_len = len(file_list)
    i = 0
    for filename in file_list:
        i += 1
        result += '\t' + filename.replace(str(path_pfx), '').replace("""\\""", '/')[1:]
        if i != list_len or flag:
            result += '\n'
    return result


includes = get_file_list(path_pfx / pathlib.Path('include') / pathlib.Path('**'), flag=True)
sources = get_file_list(path_pfx / pathlib.Path('src') / pathlib.Path('**'), flag=False)

cmake = \
"""add_library(lootinator
@INCLUDES
@SOURCES
)

target_include_directories(lootinator PUBLIC include)

set(TEST_FILES
    tests/constraint_test.cpp
    tests/constraint_json_test.cpp
    tests/filter_test.cpp
    tests/loot_functions_test.cpp
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
"""

final_cmake = cmake.replace('@INCLUDES', includes).replace('@SOURCES', sources)
cmake_filepath = path_pfx / pathlib.Path('CMakeLists.txt')
print(cmake_filepath)
     
with open(cmake_filepath, "w") as f:
    f.write(final_cmake)
