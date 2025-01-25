include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatMySpriteIsValidReplacer MODULE "")
target_compile_options(SoldatMySpriteIsValidReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatMySpriteIsValidReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatMySpriteIsValidReplacer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/soldat_mysprite_is_valid_replacer.cpp)

create_clang_tidy_test(SoldatMySpriteIsValidReplacer soldat-mysprite-is-valid-replacer ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_mysprite_is_valid_replacer.cpp)