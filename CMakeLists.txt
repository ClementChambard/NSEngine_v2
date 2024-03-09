cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")

add_compile_options(-Wall
                    -Wextra
                    -Werror)
                    #-O4)

project(NSEngine VERSION 2.0)

add_subdirectory(engine)
add_subdirectory(testbed)
add_subdirectory(tests)
