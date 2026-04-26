if(EMSCRIPTEN)

    add_executable(lootinator_wasm
        ${CMAKE_CURRENT_SOURCE_DIR}/lootinator_bindings.cpp
    )

    target_link_libraries(lootinator_wasm PRIVATE lootinator)

    target_include_directories(lootinator_wasm PRIVATE
        ${CMAKE_SOURCE_DIR}
    )

    target_link_options(lootinator_wasm PRIVATE
        --bind
        -sMODULARIZE=1
        -sEXPORT_ES6=1
        -sEXPORT_NAME=Lootinator
        -sALLOW_MEMORY_GROWTH=1
        -sENVIRONMENT=web
    )

    set_target_properties(lootinator_wasm PROPERTIES
        OUTPUT_NAME "lootinator"
        SUFFIX ".js"
    )

endif()
