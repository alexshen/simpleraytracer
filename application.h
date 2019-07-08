#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include "renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

class Renderer;
struct RenderConfig;

class Application
{
public:
    Application(const RenderConfig& config);
    ~Application();

    void run();
private:
    void init(const RenderConfig& config);
    void onRenderComplete();

    static void onKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void onKeyPressed(int key, int action);

    std::unique_ptr<Renderer> m_renderer;
    GLFWwindow* m_window;
};

#endif // APPLICATION_H 