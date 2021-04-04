cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(lhft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")

set(BUILD_WITH_MARCH_NATIVE ON CACHE BOOL "Enable architecture-aware optimization (-march=native)")
if (BUILD_WITH_MARCH_NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=native -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=native -march=native")
    message(STATUS "Architecture-aware optimization (-march=native): ENABLED")
else ()
    message(STATUS "Architecture-aware optimization (-march=native): DISABLED")
endif ()

set(BENCHMARK_ENABLED OFF CACHE BOOL "Enable Benchmark")

set(CMAKE_NOOP ${CMAKE_COMMAND} -E echo)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake)
include(Versions)
include(StandardProjectSettings)
include(ExternalProject)

set_property(DIRECTORY PROPERTY EP_BASE ${CMAKE_BINARY_DIR}/subprojects)
set(STAGED_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/stage)
message(STATUS "${PROJECT_NAME} staged install: ${STAGED_INSTALL_PREFIX}")

#add_subdirectory(external/benchmark)

add_subdirectory(external/catch)

include(GNUInstallDirs)

ExternalProject_Add(problem
        DEPENDS
        PREFIX
            ${STAGED_INSTALL_PREFIX}/problem
        SOURCE_DIR
            ${CMAKE_CURRENT_LIST_DIR}/problem
        CMAKE_ARGS
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
            -DCMAKE_INSTALL_PREFIX=${STAGED_INSTALL_PREFIX}/problem
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
            -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
            -DBENCHMARK_ENABLED=${BENCHMARK_ENABLED}
        CMAKE_CACHE_ARGS
            -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
            -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};
            -DCMAKE_LIBRARY_PATH:PATH=${CMAKE_LIBRARY_PATH};
        BUILD_ALWAYS
            1)

ExternalProject_Add(tests
        DEPENDS
            #benchmark_external
            catch_external
            problem
        PREFIX
            ${STAGED_INSTALL_PREFIX}/tests
        SOURCE_DIR
            ${CMAKE_CURRENT_LIST_DIR}/tests
        CMAKE_ARGS
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
            -DCMAKE_INSTALL_PREFIX=${STAGED_INSTALL_PREFIX}/tests
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
            -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
            -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
        CMAKE_CACHE_ARGS
            -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
            -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH};
                                     ${CATCH_CONFIG_DIR};
                                     #${BENCHMARK_CONFIG_DIR};
                                     ${STAGED_INSTALL_PREFIX}/problem/${CMAKE_INSTALL_LIBDIR}/cmake/MatchingEngine;
            -DCMAKE_INCLUDE_PATH:PATH=${CATCH_INCLUDE_DIR};
                                      #${BENCHMARK_INCLUDE_DIR};
                                      ${STAGED_INSTALL_PREFIX}/problem/include
            #-DCMAKE_LIBRARY_PATH:PATH=${BENCHMARK_LIB_PATH};
        BUILD_ALWAYS
            1)