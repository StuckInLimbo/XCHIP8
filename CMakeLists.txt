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

# GLFW Lib
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/glfw/")

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
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_glfw.h
    src/imgui/opensans.h
)

# Add source to this project's executable.
if (WIN32)
    message(STATUS ">>> WIN32")
    add_executable (XCHIP8 ${sources})
    # add_executable (XCHIP8 WIN32 ${sources})
else()
    message(STATUS ">>> NOT WIN32")
    add_executable (XCHIP8 ${sources})
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET XCHIP8 PROPERTY CXX_STANDARD 20)
endif()

if (WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_gl_LIBRARY} "glad" glfw)
elseif(LINUX)
    target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_gl_LIBRARY} "glad" glfw)
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