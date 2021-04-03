cmake_minimum_required(VERSION 3.17)

project(matching_engine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB HEADER_FILES include/*.hpp)

file(GLOB SOURCE_FILES src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

#add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE include)