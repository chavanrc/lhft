set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
include(Versions)
find_package(catch REQUIRED QUIET)

if (catch_FOUND)
    message("-- Found catch2 library.")
    add_library(catch_external INTERFACE)
else ()
    message(STATUS "Catch2 TAG: ${CATCH_TAG} could not be located.")
    include(ExternalProject)
    set(EXTERNAL_INSTALL_LOCATION ${STAGED_INSTALL_PREFIX}/catch)
    ExternalProject_Add(catch_external
            PREFIX
                ${EXTERNAL_INSTALL_LOCATION}
            GIT_REPOSITORY
                ${CATCH_URL}
            GIT_TAG
                ${CATCH_TAG}
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
                -DCATCH_BUILD_TESTING=OFF
            CMAKE_CACHE_ARGS
                -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS})
    ExternalProject_Get_Property(catch_external source_dir)
    set(CATCH_ROOT ${EXTERNAL_INSTALL_LOCATION}
            CACHE PATH "Path to internally built catch installation root"
            FORCE)
    set(CATCH_INCLUDE_DIR ${CATCH_ROOT}/include
            CACHE PATH "Path to internally built catch include directories"
            FORCE)
    set(CATCH_LIB_PATH ${CATCH_ROOT}/lib
            CACHE PATH "Path to internally built catch library directories"
            FORCE)
    set(CATCH_CONFIG_DIR ${CATCH_LIB_PATH}/cmake/Catch2
            CACHE PATH "Path to internally built catch library config directories"
            FORCE)
    # Unset internal variables
    unset(EXTERNAL_INSTALL_LOCATION)
endif ()