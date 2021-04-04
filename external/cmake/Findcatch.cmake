find_path(catch_INCLUDE_DIR
        NAMES catch2/catch.hpp
        PATHS ${CATCH_INCLUDE_DIR}
        NO_CMAKE_SYSTEM_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        DOC "The directory where catch includes reside")

if (catch_INCLUDE_DIR)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(catch DEFAULT_MSG
            catch_INCLUDE_DIR)
endif ()