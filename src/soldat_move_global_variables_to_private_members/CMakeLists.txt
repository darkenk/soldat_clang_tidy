include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatMoveGlobalVariablesToPrivateMembers MODULE "")
target_compile_options(SoldatMoveGlobalVariablesToPrivateMembers PRIVATE -fno-rtti)
target_include_directories(SoldatMoveGlobalVariablesToPrivateMembers
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatMoveGlobalVariablesToPrivateMembers PRIVATE pch.hpp)
target_sources(SoldatMoveGlobalVariablesToPrivateMembers
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_move_global_variables_to_private_members.cpp
)

create_clang_tidy_test(SoldatMoveGlobalVariablesToPrivateMembers
    soldat-move-global-variables-to-private-members
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_move_global_variables_to_private_members.cpp
)