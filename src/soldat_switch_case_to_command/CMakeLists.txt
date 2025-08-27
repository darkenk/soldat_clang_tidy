include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatSwitchCaseToCommand MODULE "")
target_compile_options(SoldatSwitchCaseToCommand PRIVATE -fno-rtti)
target_include_directories(SoldatSwitchCaseToCommand
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatSwitchCaseToCommand PRIVATE pch.hpp)
target_sources(SoldatSwitchCaseToCommand
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_switch_case_to_command.cpp
)

create_clang_tidy_test(SoldatSwitchCaseToCommand
    soldat-switch-case-to-command
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_switch_case_to_command.cpp
)
