set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(Versions)
find_package(benchmark REQUIRED QUIET)

if (benchmark_FOUND)
    message("-- Found Google benchmark library.")
    add_library(benchmark_external INTERFACE)
else ()
    message(STATUS "Google benchmark TAG: ${BENCHMARK_TAG} could not be located.")
    include(ExternalProject)
    set(EXTERNAL_INSTALL_LOCATION ${STAGED_INSTALL_PREFIX}/benchmark)
    ExternalProject_Add(benchmark_external
            PREFIX
                ${EXTERNAL_INSTALL_LOCATION}
            GIT_REPOSITORY
                ${BENCHMARK_URL}
            GIT_TAG
                ${BENCHMARK_TAG}
            GIT_SUBMODULES_RECURSE
                1
            GIT_SHALLOW
                1
            GIT_PROGRESS
                1
            LOG_CONFIGURE
                1
            LOG_BUILD
                1
            LOG_INSTALL
                1
            LOG_DOWNLOAD
                1
            UPDATE_COMMAND
                ${CMAKE_NOOP}
            CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DBENCHMARK_ENABLE_TESTING=OFF
                -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
                -DBENCHMARK_ENABLE_LTO=OFF
                -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON
            CMAKE_CACHE_ARGS
                -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS})
    set(BENCHMARK_ROOT ${EXTERNAL_INSTALL_LOCATION}
            CACHE PATH "Path to internally built benchmark installation root"
            FORCE)
    set(BENCHMARK_INCLUDE_DIR ${BENCHMARK_ROOT}/include
            CACHE PATH "Path to internally built benchmark include directories"
            FORCE)
    set(BENCHMARK_LIB_PATH ${BENCHMARK_ROOT}/lib
            CACHE PATH "Path to internally built benchmark library directories"
            FORCE)
    set(BENCHMARK_CONFIG_DIR ${BENCHMARK_LIB_PATH}/cmake/benchmark
            CACHE PATH "Path to internally built benchmark library config directories"
            FORCE)
    # Unset internal variables
    unset(EXTERNAL_INSTALL_LOCATION)
endif ()