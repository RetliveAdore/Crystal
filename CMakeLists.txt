cmake_minimum_required(VERSION 3.20)

project("Crystal" C CXX)

include_directories("./include")
include_directories("./src/headers")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin$<$<CONFIG:Debug>:_debug>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin$<$<CONFIG:Debug>:_debug>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin$<$<CONFIG:Debug>:_debug>")

add_compile_options(-fPIC)
add_link_options(-no-pie)

file(GLOB_RECURSE Crystal_Files
"./include/*.h"
"./src/core/*.h"
"./src/core/*.hpp"
"./src/core/*.c"
"./src/core/*.cpp"
)

file(GLOB_RECURSE Demo_Files
"./Demo/*.h"
"./Demo/*.hpp"
"./Demo/*.c"
"./Demo/*.cpp"
)

add_library("Crystal" SHARED ${Crystal_Files})
add_compile_definitions("Crystal" CR_SHARED_LIB CR_BUILD_DLL)

#这里面会有很多用于验证的演示
add_executable("Demo" ${Demo_Files})
target_link_libraries("Demo" Crystal)

if(MSVC)
  set_property(TARGET "Crystal" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Demo" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_dependencies("Demo" "Crystal")