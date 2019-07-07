cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED 1)

project (raytracer)

find_package( glm CONFIG REQUIRED )
find_package( glfw3 CONFIG REQUIRED )
find_package( OpenGL REQUIRED )

if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
endif()

find_package( Boost COMPONENTS program_options REQUIRED )

set(SOURCES 
    main.cpp
    application.cpp
    application.h
    renderer.cpp
    renderer.h
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
    bvh_node.cpp
    bvh_node.h
    aabb.h
    object.cpp
    object.h
    sphere_object.h
    ssborenderinput.cpp
    ssborenderinput.h
    uborenderinput.cpp
    uborenderinput.h
    texrenderinput.cpp
    texrenderinput.h
    renderinput.h

    glad/glad.h
    glad/gl_core_4_3.c
    glad/gl_core_4_3.h
    )

add_executable(raytracer
    ${SOURCES}
)

target_include_directories(raytracer PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(raytracer PRIVATE .)
target_link_libraries(raytracer PRIVATE glm glfw ${Boost_LIBRARIES} ${OPENGL_gl_LIBRARY})