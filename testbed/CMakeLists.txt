cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE INCLUDES src/*.hpp src/*.h src/*.inl)

add_compile_options(-D_DEBUG -DNS_IMPORT)

add_executable(testbed ${SRCS} ${INCLUDES})

target_link_libraries(testbed PRIVATE NSEngine)
