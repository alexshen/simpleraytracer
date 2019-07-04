#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Renderer;

class Application
{
public:
    Application(Renderer& renderer);
    ~Application();

    bool init();
    void run();

private:
    static void onKeyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void onKeyPressed(int key, int action);

    Renderer& m_renderer;
    GLFWwindow* m_window;
};

#endif // APPLICATION_H 