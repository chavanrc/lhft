cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake ${CMAKE_CURRENT_SOURCE_DIR}/../external/cmake)

find_package(MatchingEngine REQUIRED CONFIG QUIET)

find_package(Catch2 CONFIG REQUIRED QUIET)

#find_package(benchmark REQUIRED QUIET)

find_package(Threads REQUIRED)

set(SOURCE_FILES
        me_tests.cpp
        lhft_tests.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        #benchmark::benchmark
        Catch2::Catch2 Threads::Threads MatchingEngine::MatchingEngine)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
