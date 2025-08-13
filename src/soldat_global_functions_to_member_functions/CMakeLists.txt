include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatGlobalFunctionsToMemberFunctions MODULE "")
target_compile_options(SoldatGlobalFunctionsToMemberFunctions PRIVATE -fno-rtti)
target_include_directories(SoldatGlobalFunctionsToMemberFunctions
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatGlobalFunctionsToMemberFunctions PRIVATE pch.hpp)
target_sources(SoldatGlobalFunctionsToMemberFunctions
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_global_functions_to_member_functions.cpp
)

create_clang_tidy_test(SoldatGlobalFunctionsToMemberFunctions
    soldat-global-functions-to-member-functions
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_global_functions_to_member_functions.cpp
    HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/Stub.hpp
)