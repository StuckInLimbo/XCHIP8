# CMakeList.txt : CMake project for XCHIP8, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("XCHIP8")

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

find_package(OpenGL REQUIRED)

# Glad Lib
add_library("glad" "${CMAKE_SOURCE_DIR}/src/glad/src/glad.c")

include_directories("${CMAKE_SOURCE_DIR}/src/glad/include")

if(WIN32) 
    message(STATUS ">>> Windows")
    # SDL Library
    find_library(SDL SDL2 PATHS "${CMAKE_SOURCE_DIR}/src/SDL/lib/x64")
    # SDL Main for windows
    find_library(SDLmain SDL2main PATHS "${CMAKE_SOURCE_DIR}/src/SDL/lib/x64")
    # headers
    include_directories("${CMAKE_SOURCE_DIR}/src/SDL/include")
elseif(LINUX)
    message(STATUS ">>> Linux")
    # SDL Library
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

set(sources
    src/main.cpp
    src/chip8.cpp
    src/chip8.h
    src/shader.h
    src/imgui/imconfig.h
    src/imgui/imgui.cpp
    src/imgui/imgui.h
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_internal.h
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imstb_rectpack.h
    src/imgui/imstb_textedit.h
    src/imgui/imstb_truetype.h
    src/imgui/imgui_impl_opengl3.cpp
    src/imgui/imgui_impl_opengl3.h
    src/imgui/imgui_impl_sdl.cpp
    src/imgui/imgui_impl_sdl.h
    src/imgui/opensans.h
)

# Add source to this project's executable.
if (WIN32)
    message(STATUS ">>> WIN32")
    add_executable (XCHIP8 WIN32 ${sources})
else()
    message(STATUS ">>> NOT WIN32")
    add_executable (XCHIP8 ${sources})
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XCHIP8 PROPERTY CXX_STANDARD 20)
endif()

if (WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_gl_LIBRARY} "glad" ${SDL} ${SDLmain})
elseif(LINUX)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_gl_LIBRARY} "glad" ${SDL2_LIBRARIES})
endif()

# Post build cmds
add_custom_command(TARGET XCHIP8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_SOURCE_DIR}/roms/ 
    $<TARGET_FILE_DIR:XCHIP8>/roms/
)

add_custom_command(TARGET XCHIP8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_SOURCE_DIR}/src/shaders/ 
    $<TARGET_FILE_DIR:XCHIP8>/shaders/
)