include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatGlobalObjectsReplacer MODULE "")
target_compile_options(SoldatGlobalObjectsReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatGlobalObjectsReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatGlobalObjectsReplacer
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/soldat_global_objects_replacer.cpp
)

create_clang_tidy_test(SoldatGlobalObjectsReplacer
    soldat-global-objects-replacer
    ${CMAKE_CURRENT_LIST_DIR}/stub_soldat_global_objects_replacer.cpp
    HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/Client.hpp
        ${CMAKE_CURRENT_LIST_DIR}/Stub.hpp
)