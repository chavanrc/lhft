find_path(benchmark_INCLUDE_DIR
        NAMES benchmark/benchmark.h
        PATHS ${BENCHMARK_INCLUDE_DIR}
        NO_CMAKE_SYSTEM_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        DOC "The directory where benchmark includes reside")

find_library(benchmark_LIBRARY
        NAMES benchmark
        PATHS ${BENCHMARK_LIB_PATH}
        NO_CMAKE_SYSTEM_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        DOC "The benchmark library")

if (benchmark_INCLUDE_DIR AND benchmark_LIBRARY)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(benchmark DEFAULT_MSG
            benchmark_INCLUDE_DIR benchmark_LIBRARY)
    if (benchmark_FOUND)
        add_library(benchmark::benchmark STATIC IMPORTED)
        set_target_properties(benchmark::benchmark PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${benchmark_INCLUDE_DIR}"
                IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
                IMPORTED_LOCATION "${benchmark_LIBRARY}")
    endif ()
endif ()