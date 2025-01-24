include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatMySpriteReplacer MODULE "")
target_compile_options(SoldatMySpriteReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatMySpriteReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatMySpriteReplacer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/soldat_mysprite_replacer.cpp)

create_clang_tidy_test(SoldatMySpriteReplacer soldat-mysprite-replacer ${CMAKE_CURRENT_LIST_DIR}/Stub.cpp)