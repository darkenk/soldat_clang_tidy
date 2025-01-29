include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatSpriteSystemGetReplacer MODULE "")
target_compile_options(SoldatSpriteSystemGetReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatSpriteSystemGetReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatSpriteSystemGetReplacer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_sprite_system_get_replacer.cpp
)

create_clang_tidy_test(SoldatSpriteSystemGetReplacer
    soldat-sprite-system-get-replacer
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_sprite_system_get_replacer.cpp
)