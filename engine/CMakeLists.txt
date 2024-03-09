cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE SRCS src/*.cpp src/*.c)
file(GLOB_RECURSE INCLUDES src/*.hpp src/*.h src/*.inl)

add_compile_options(-D_DEBUG -DNS_EXPORT -fPIC)

add_library(NSEngine SHARED ${SRCS} ${INCLUDES})

target_link_directories(NSEngine PRIVATE
                        ${VULKAN_SDK}/lib
                        /usr/X11R6/lib)

target_link_libraries(NSEngine PRIVATE
                      -lX11
                      -lX11-xcb
                      -lvulkan
                      -lxcb
                      -lxkbcommon)

target_include_directories(NSEngine INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor")
