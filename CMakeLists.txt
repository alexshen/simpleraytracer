cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED 1)

project (raytracer)

find_package( glm CONFIG REQUIRED )
find_package( glfw3 CONFIG REQUIRED )
find_package( OpenGL REQUIRED )

set(SOURCES 
    main.cpp
    application.cpp
    application.h
    renderer.h
    fsrenderer.cpp
    fsrenderer.h
    fullscreenquad.cpp
    fullscreenquad.h
    glslprogram.cpp
    glslprogram.h
    glutils.cpp
    glutils.h
    scene.cpp
    scene.h
    utils.cpp
    utils.h

    glad/gl_core_4_3.c
    glad/gl_core_4_3.h
    )

add_executable(raytracer
    ${SOURCES}
)

target_include_directories(raytracer PRIVATE .)
target_link_libraries(raytracer PRIVATE glm glfw ${OPENGL_gl_LIBRARY})