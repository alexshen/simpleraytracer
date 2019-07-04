#include "application.h"
#include "fsrenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>

int main(int argc, char* argv[])
{
    FragmentShaderRenderer render;
    try {
        Application app(render);
        if (app.init()) {
            app.run();
            return 0;
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return 1;
}