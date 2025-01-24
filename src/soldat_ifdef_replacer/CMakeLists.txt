include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/clang_tidy.cmake)

add_library(SoldatIfdefReplacer MODULE "")
target_compile_options(SoldatIfdefReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatIfdefReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatIfdefReplacer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/soldat_ifdef_replacer.cpp)

create_clang_tidy_test(SoldatIfdefReplacer soldat-ifdef-replacer ${CMAKE_CURRENT_LIST_DIR}/Stub.cpp)