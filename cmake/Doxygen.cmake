option(ENABLE_DOXYGEN "Enable doxygen doc builds of source")

if (ENABLE_DOXYGEN)
    find_package(Doxygen REQUIRED dot)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_CALLER_GRAPH ON)
        set(DOXYGEN_CALL_GRAPH ON)
        set(DOXYGEN_EXTRACT_ALL ON)
        doxygen_add_docs(doxygen-docs ${PROJECT_SOURCE_DIR}
                COMMENT "Generating doxygen documentation for ${PROJECT_NAME}")
    else ()
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif ()
endif ()
