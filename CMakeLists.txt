cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(soldat-clang-tidy LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

option(BUILD_WARNINGS "Build with project's warnings setting" ON)
option(BUILD_WARNING_AS_ERROR "Build with warning as error" ON)
if(BUILD_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-Wall)
    endif()
endif()
if(BUILD_WARNING_AS_ERROR)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-Werror)
    endif()
endif()

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

enable_testing()
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)


add_subdirectory(src)

