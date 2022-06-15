cmake_minimum_required(VERSION 3.21)
project(DAT205_project)
set(CMAKE_CXX_STANDARD 17)

# -------------------- external libraries -------------------------
# CREDIT: importing opengl, glfw, glew: https://github.com/morizotter/imgui-opengl-glfw-glew-cmake-demo/blob/master/CMakeLists.txt
# --- opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
# glfw
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARY_DIRS})
# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
#---assimp
find_package(assimp REQUIRED)
link_directories(/usr/local/lib)

# link required libs for Mac
if (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    SET(EXTRA_LIBS ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
endif (APPLE)

add_executable(project main.cpp
        includes/utils/my_gl_init.cpp includes/utils/my_gl_init.h
        includes/utils/misc.cpp includes/utils/misc.h

        includes/external/imgui/imgui.cpp includes/external/imgui/imgui_demo.cpp includes/external/imgui/imgui_draw.cpp
        includes/external/imgui/imgui_tables.cpp includes/external/imgui/imgui_widgets.cpp
        includes/external/imgui/imgui_impl_glfw.cpp includes/external/imgui/imgui_impl_opengl3.cpp

        includes/utils/free_camera.h
        includes/utils/xyz_gizmo.h
        includes/utils/shader.h
        includes/systems/ParticleSystem.h
        includes/utils/Model.h
        includes/utils/Plane.h)

add_executable(water water.cpp
        includes/utils/my_gl_init.cpp includes/utils/my_gl_init.h
        includes/utils/misc.cpp includes/utils/misc.h

        includes/external/imgui/imgui.cpp includes/external/imgui/imgui_demo.cpp includes/external/imgui/imgui_draw.cpp
        includes/external/imgui/imgui_tables.cpp includes/external/imgui/imgui_widgets.cpp
        includes/external/imgui/imgui_impl_glfw.cpp includes/external/imgui/imgui_impl_opengl3.cpp

        includes/utils/free_camera.h
        includes/utils/xyz_gizmo.h
        includes/utils/shader.h
        includes/systems/ParticleSystem.h
        includes/utils/Model.h
        includes/utils/Plane.h includes/utils/Cube.h includes/utils/cube_with_tex.h)

add_executable(terrain terrain.cpp
        includes/utils/my_gl_init.cpp includes/utils/my_gl_init.h
        includes/utils/misc.cpp includes/utils/misc.h

        includes/external/imgui/imgui.cpp includes/external/imgui/imgui_demo.cpp includes/external/imgui/imgui_draw.cpp
        includes/external/imgui/imgui_tables.cpp includes/external/imgui/imgui_widgets.cpp
        includes/external/imgui/imgui_impl_glfw.cpp includes/external/imgui/imgui_impl_opengl3.cpp

        includes/utils/free_camera.h
        includes/utils/xyz_gizmo.h
        includes/utils/shader.h
        includes/systems/ParticleSystem.h
        includes/utils/Model.h
        includes/utils/Plane.h includes/utils/Cube.h includes/utils/cube_with_tex.h)

target_link_libraries(project glfw ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES})
target_link_libraries(water glfw ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES})
target_link_libraries(terrain glfw ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES})
include_directories(${PROJECT_SOURCE_DIR}/includes) # add /includes as search path for #include "whatever.h"

# ------------------------------------------------------------------------
# --- COPY /shaders to /out/resources!
# --- note: rebuild project to copy changes
add_custom_command(TARGET project PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:project>)

add_custom_command(TARGET water PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:water>)

add_custom_command(TARGET terrain PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:terrain>)