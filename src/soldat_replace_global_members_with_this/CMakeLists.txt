include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatReplaceGlobalMembersWithThis MODULE "")
target_compile_options(SoldatReplaceGlobalMembersWithThis PRIVATE -fno-rtti)
target_include_directories(SoldatReplaceGlobalMembersWithThis
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatReplaceGlobalMembersWithThis PRIVATE pch.hpp)
target_sources(SoldatReplaceGlobalMembersWithThis
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_replace_global_members_with_this.cpp
)

create_clang_tidy_test(SoldatReplaceGlobalMembersWithThis
    soldat-replace-global-members-with-this
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_replace_global_members_with_this.cpp
)