#include "application.h"
#include "renderer.h"
#include "glutils.h"

#include <glad/glad.h>
#include <iostream>
#include <cassert>

namespace 
{
    const char* const AppName = "raytracer";
}

Application::Application(Renderer& render)
    : m_renderer(render)
    , m_window(nullptr)
{
}

Application::~Application()
{
    glfwTerminate();
}

bool Application::init()
{
    if (!glfwInit()) {
        std::cerr << "failed to initialize glfw\n";
        return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifndef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

#ifndef __APPLE__
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(800, 600, AppName, nullptr, nullptr);
    if (!m_window) {
        const char* error;
        glfwGetError(&error);
        std::cerr << "failed to create the app window: " << error << "\n";
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSetKeyCallback(m_window, onKeyPressedCallback);
    glfwSetWindowUserPointer(m_window, this);
    
    if (!ogl_LoadFunctions()) {
        std::cerr << "failed to load ogl\n";
        return false;
    }

#ifndef __APPLE__
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLUtils::debugCallback, nullptr);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
        GL_DEBUG_SEVERITY_NOTIFICATION, -1 , "Start debugging");
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    m_renderer.init(width, height);
    return true;
}

void Application::run()
{
    assert(m_window);

    int frames = 0;
    double accum = 0;
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window)) {
        double curTime = glfwGetTime();
        accum += curTime - lastTime;
        lastTime = curTime;

        if (++frames == 30) {
            char title[256];
            std::snprintf(title, sizeof(title), "%s - %.2f", AppName, frames / accum);
            frames = 0;
            accum = 0;
            glfwSetWindowTitle(m_window, title);
        }

        glfwPollEvents();

        m_renderer.render();
        glFinish();
        glfwSwapBuffers(m_window);

        auto error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << error << "\n";
        }
    }
}

void Application::onKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->onKeyPressed(key, action);
}

void Application::onKeyPressed(int key, int action)
{
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}