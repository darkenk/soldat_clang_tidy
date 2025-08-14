include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatReplaceGlobalFunctionCallsWithThisCalls MODULE "")
target_compile_options(SoldatReplaceGlobalFunctionCallsWithThisCalls PRIVATE -fno-rtti)
target_include_directories(SoldatReplaceGlobalFunctionCallsWithThisCalls
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatReplaceGlobalFunctionCallsWithThisCalls PRIVATE pch.hpp)
target_sources(SoldatReplaceGlobalFunctionCallsWithThisCalls
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_replace_global_function_calls_with_this_calls.cpp
)

create_clang_tidy_test(SoldatReplaceGlobalFunctionCallsWithThisCalls
    soldat-replace-global-function-calls-with-this-calls
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_replace_global_function_calls_with_this_calls.cpp
)