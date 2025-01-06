add_library(SoldatIfdefReplacer MODULE "")
target_compile_options(SoldatIfdefReplacer PRIVATE -fno-rtti)
target_include_directories(SoldatIfdefReplacer
    PRIVATE
        ${CLANG_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_sources(SoldatIfdefReplacer PRIVATE ${CMAKE_CURRENT_LIST_DIR}/soldat_ifdef_replacer.cpp)

function(create_clang_tidy_test TARGET_NAME CHECK_NAME SOURCE_FILE)
    set(TEST_LOCATION ${CMAKE_BINARY_DIR}/${TARGET_NAME}_Test)
    set(TEST_PLUGIN $<TARGET_FILE:${TARGET_NAME}>)

    file(MAKE_DIRECTORY ${TEST_LOCATION})
    file(WRITE ${TEST_LOCATION}/CMakeLists.txt
        "cmake_minimum_required(VERSION 3.20 FATAL_ERROR)\n"
        "project(${TARGET_NAME}_StubTest LANGUAGES CXX)\n"
        "add_library(${TARGET_NAME}_StubTest \${CMAKE_CURRENT_LIST_DIR}/Stub.cpp)\n"
    )

    add_custom_target(${TARGET_NAME}_Test
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE} ${TEST_LOCATION}/Stub.cpp
        COMMAND ${CMAKE_COMMAND} -B ${TEST_LOCATION}/build -S ${TEST_LOCATION} -DEXPORT_COMPILE_COMMANDS=ON
        COMMAND clang-tidy --fix --format-style=file --checks="-*,${CHECK_NAME}" --load=${TEST_PLUGIN} -p ${TEST_LOCATION}/build ${TEST_LOCATION}/Stub.cpp
        #COMMAND diff ${TEST_LOCATION}/Stub.cpp ${SOURCE_FILE}
        DEPENDS ${TARGET_NAME}
    )
endfunction()

create_clang_tidy_test(SoldatIfdefReplacer soldat-ifdef-replacer ${CMAKE_CURRENT_LIST_DIR}/Stub.cpp)