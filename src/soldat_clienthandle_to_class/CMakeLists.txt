include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatClienthandleToClass MODULE "")
target_compile_options(SoldatClienthandleToClass PRIVATE -fno-rtti)
target_include_directories(SoldatClienthandleToClass
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_precompile_headers(SoldatClienthandleToClass PRIVATE pch.hpp)
target_sources(SoldatClienthandleToClass
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_clienthandle_to_class.cpp
)

create_clang_tidy_test(SoldatClienthandleToClass
    soldat-clienthandle-to-class
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_clienthandle_to_class.cpp
)
