cmake_minimum_required(VERSION 3.17)

project(tests)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCE_FILES *.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../problem/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../external/Catch2/single_include)

target_link_libraries(${PROJECT_NAME} PRIVATE matching_engine)