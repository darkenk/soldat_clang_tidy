include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatInlineInitializerList MODULE "")
target_compile_options(SoldatInlineInitializerList PRIVATE -fno-rtti)
target_include_directories(SoldatInlineInitializerList
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatInlineInitializerList PRIVATE pch.hpp)
target_sources(SoldatInlineInitializerList
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_inline_initializer_list.cpp
)

create_clang_tidy_test(SoldatInlineInitializerList
    soldat-inline-initializer-list
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_inline_initializer_list.cpp
)