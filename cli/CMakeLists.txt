set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")
add_compile_options(-Os)

add_executable(lootinator-cli
	src/main.cpp
)


add_subdirectory(${CMAKE_SOURCE_DIR}/external/clipp ${CMAKE_BINARY_DIR}/clipp)
target_include_directories(lootinator-cli PUBLIC ${CMAKE_SOURCE_DIR}/external/clipp/include/)
target_link_libraries(lootinator-cli PRIVATE clipp::clipp)

target_link_libraries(lootinator-cli PRIVATE lootinator)

