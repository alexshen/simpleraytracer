#include "application.h"
#include "fsrenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>

int main(int argc, char* argv[])
{
    try {
        FragmentShaderRenderer render;
        Application app(render);
        if (app.init()) {
            app.run();
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    } catch (...) {
        std::cerr << "unknown exception\n";
    }
    return 1;
}