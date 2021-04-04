find_program(CLANG_FORMAT
        NAMES "clang-format"
        DOC "Path to clang-format executable")

if (NOT CLANG_FORMAT)
    message(STATUS "clang-format not found.")
else ()
    message(STATUS "clang-format found: ${CLANG_FORMAT}")
    set(DO_CLANG_FORMAT "${CLANG_FORMAT}" "-i -style=file")
endif ()

if (CLANG_FORMAT)
    # get all project files
    file(GLOB_RECURSE ALL_SOURCE_FILES
            RELATIVE ${CMAKE_CURRENT_BINARY_DIR}
            ${CMAKE_SOURCE_DIR}/problem/*.cpp ${CMAKE_SOURCE_DIR}/problem/*.hpp ${CMAKE_SOURCE_DIR}/problem/*.inl)

    add_custom_target(
            clang-format
            COMMAND ${CLANG_FORMAT} -style=file -i ${ALL_SOURCE_FILES})

    add_custom_target(
            clang-format-diff
            COMMAND ${CLANG_FORMAT} -style=file -i ${ALL_SOURCE_FILES}
            COMMAND git diff ${ALL_SOURCE_FILES}
            COMMENT "Formatting with clang-format (using ${CLANG_FORMAT}) and showing differences with latest commit"
    )
endif ()