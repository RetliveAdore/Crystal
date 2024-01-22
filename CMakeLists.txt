cmake_minimum_required(VERSION 3.20)

project("Crystal" C CXX)

include_directories("./ThirdParty/freetype/include")
include_directories("./include")
include_directories("./src/headers")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin$<$<CONFIG:Debug>:_debug>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin$<$<CONFIG:Debug>:_debug>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin$<$<CONFIG:Debug>:_debug>")


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_compile_options(-fPIC)
  add_link_options(-no-pie)
  link_libraries(m dl pthread X11 GL GLU asound)
  link_directories("./ThirdParty/freetype//Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  link_directories("./ThirdParty/freetype//Windows")
endif()

file(GLOB_RECURSE Crystal_Files
"./include/*.*"
"./src/core/*.*"
)

file(GLOB_RECURSE Dynextra_Files
"./include/*.*"
"./src/parts/dynextra/*.*"
)

file(GLOB_RECURSE Treextra_Files
"./include/*.*"
"./src/parts/treextra/*.*"
)

file(GLOB_RECURSE Basic_Files
"./include/*.*"
"./src/parts/basic/*.*"
)

file(GLOB_RECURSE Binary_Files
"./include/*.*"
"./src/parts/binary/*.*"
)

file(GLOB_RECURSE CrUI_Files
"./include/*.*"
"./src/parts/UI/*.*"
)

file(GLOB_RECURSE CrAudio_Files
"./include/*.*"
"./src/parts/Audio/*.*"
)

file(GLOB_RECURSE Demo_Files
"./Demo/*.*"
)

file(GLOB_RECURSE Player_Files
"./player/*.*"
)

#拷贝一些测试用资源
add_custom_target("CopyTask"
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
  #
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/resource
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)

add_library("Crystal" SHARED ${Crystal_Files})
add_library("Crdynextra" SHARED ${Dynextra_Files})
add_library("Crtreextra" SHARED ${Treextra_Files})
add_library("Crbasic" SHARED ${Basic_Files})
add_library("Crbinary" SHARED ${Binary_Files})
add_library("CrUI" SHARED ${CrUI_Files})
add_library("CrAudio" SHARED ${CrAudio_Files})
add_compile_definitions(CR_SHARED_LIB CR_BUILD_DLL)

#这里面会有很多用于验证的演示
add_executable("Demo" ${Demo_Files})
add_executable("Player" ${Player_Files})

#添加库依赖
target_link_libraries("Demo" Crystal Crdynextra Crtreextra Crbasic Crbinary CrUI CrAudio)
target_link_libraries("Player" Crystal Crdynextra Crtreextra Crbasic Crbinary CrUI CrAudio)
target_link_libraries("Crdynextra" Crystal)
target_link_libraries("Crtreextra" Crystal)
target_link_libraries("Crbasic" Crystal)
target_link_libraries("Crbinary" Crystal Crdynextra)
target_link_libraries("CrUI" Crystal Crbasic freetype Crtreextra)
target_link_libraries("CrAudio" Crystal Crbasic)

add_dependencies("Demo"
  "CopyTask" "Crystal" "Crbasic"
  "Crbinary" "CrUI" "CrAudio"
)
add_dependencies("Player"
  "CopyTask" "Crystal" "Crbasic"
  "Crbinary" "CrUI" "CrAudio"
)
add_dependencies("Crdynextra" "Crystal")
add_dependencies("Crtreextra" "Crystal")
add_dependencies("Crbasic" "Crystal")
add_dependencies("Crbinary" "Crystal" "Crdynextra")
add_dependencies("CrUI" "Crystal" "Crbasic" "Crtreextra")
add_dependencies("CrAudio" "Crystal" "Crbasic")

if(MSVC)
  set_property(TARGET "Demo" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Player" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Crystal" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Crdynextra" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Crtreextra" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Crbasic" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "Crbinary" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "CrUI" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET "CrAudio" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
