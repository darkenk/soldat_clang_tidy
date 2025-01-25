include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatSpriteIsMyspriteReplacer MODULE "")
target_compile_options(SoldatSpriteIsMyspriteReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatSpriteIsMyspriteReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatSpriteIsMyspriteReplacer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_sprite_is_mysprite_replacer.cpp
)

create_clang_tidy_test(SoldatSpriteIsMyspriteReplacer
    soldat-sprite-is-mysprite-replacer
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_sprite_is_mysprite_replacer.cpp
)