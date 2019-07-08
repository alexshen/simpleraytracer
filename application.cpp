#include "application.h"
#include "glutils.h"

#include <glad/glad.h>
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <string>

namespace 
{
    const char* const AppName = "raytracer";
}

Application::Application(const RenderConfig& config)
{
    init(config);
}

Application::~Application()
{
    glfwTerminate();
}

void Application::init(const RenderConfig& config)
{
    if (!glfwInit()) {
        throw std::runtime_error("failed to initialize glfw");
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

    m_window = glfwCreateWindow(config.width, config.height, AppName, nullptr, nullptr);
    if (!m_window) {
        const char* error;
        glfwGetError(&error);
        throw std::runtime_error(std::string("failed to create the app window: ") + error);
    }
    glfwMakeContextCurrent(m_window);
    glfwSetKeyCallback(m_window, onKeyPressedCallback);
    glfwSetWindowUserPointer(m_window, this);
    
    if (!ogl_LoadFunctions()) {
        throw std::runtime_error("failed to load ogl");
    }

    if (config.shaderInput == ShaderInput::ShaderStorageBuffer && 
        !glfwExtensionSupported("GL_ARB_shader_storage_buffer_object")) {
        throw std::runtime_error("ssbo input is not supported");
    }

    if (config.shaderType == ShaderType::ComputeShader && 
        !glfwExtensionSupported("GL_ARB_compute_shader")) {
        throw std::runtime_error("compute shader is not supported");
    }

#ifndef __APPLE__
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLUtils::debugCallback, nullptr);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
        GL_DEBUG_SEVERITY_NOTIFICATION, -1 , "Start debugging");
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

    RenderConfig tmpConfig = config;
    glfwGetFramebufferSize(m_window, &tmpConfig.width, &tmpConfig.height);
    glViewport(0, 0, tmpConfig.width, tmpConfig.height);
    m_renderer = std::make_unique<Renderer>(tmpConfig);
    m_renderer->onRenderComplete([&] { onRenderComplete(); });
    glfwSwapInterval(0);
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

        m_renderer->render();
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

void Application::onRenderComplete()
{
    glfwSwapInterval(1);
}