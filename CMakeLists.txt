# CMakeList.txt : CMake project for XCHIP8, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("XCHIP8")

find_package(OpenGL REQUIRED)

add_library("glad" "${CMAKE_SOURCE_DIR}/glad/src/glad.c")

include_directories("${CMAKE_SOURCE_DIR}/glad/include")

# SDL Library
find_library(SDL SDL2 PATHS "${CMAKE_SOURCE_DIR}/SDL/lib/x64")
# SDL Main for windows
find_library(SDLmain SDL2main PATHS "${CMAKE_SOURCE_DIR}/SDL/lib/x64")
# headers
include_directories("${CMAKE_SOURCE_DIR}/SDL/include")

set(sources
    main.cpp
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_impl_opengl3.h
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_impl_sdl.h
)

# Add source to this project's executable.
add_executable (XCHIP8 ${sources})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XCHIP8 PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_gl_LIBRARY} "glad" ${SDL} ${SDLmain})

# TODO: Add tests and install targets if needed.
