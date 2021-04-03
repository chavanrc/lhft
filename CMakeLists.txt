cmake_minimum_required(VERSION 3.17)

project(lhft)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/Catch2)

add_subdirectory(problem)

add_subdirectory(tests)