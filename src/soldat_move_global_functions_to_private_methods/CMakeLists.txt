include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatMoveGlobalFunctionsToPrivateMethods MODULE "")
target_compile_options(SoldatMoveGlobalFunctionsToPrivateMethods PRIVATE -fno-rtti)
target_include_directories(SoldatMoveGlobalFunctionsToPrivateMethods
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatMoveGlobalFunctionsToPrivateMethods PRIVATE pch.hpp)
target_sources(SoldatMoveGlobalFunctionsToPrivateMethods
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_move_global_functions_to_private_methods.cpp
)

create_clang_tidy_test(SoldatMoveGlobalFunctionsToPrivateMethods
    soldat-move-global-functions-to-private-methods
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_move_global_functions_to_private_methods.cpp
)