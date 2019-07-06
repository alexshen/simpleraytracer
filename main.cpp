#include "application.h"
#include "renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <exception>
#include <iostream>

std::istream& operator >>(std::istream& in, RenderMode& mode)
{
    std::string input;
    in >> input;
    if (input == "fullscreen") {
        mode = RenderMode::FullScreenIncremental;
    } else if (input == "blocked") {
        mode = RenderMode::Blocked;
    } else {
        throw po::invalid_option_value("render mode");
    }
    return in;
}

std::ostream& operator <<(std::ostream& out, RenderMode mode)
{
    if (mode == RenderMode::FullScreenIncremental) {
        out << "fullscreen";
    } else if (mode == RenderMode::Blocked) {
        out << "blocked";
    }
    return out;
}

std::istream& operator >>(std::istream& in, ShaderInput& mode)
{
    std::string input;
    in >> input;
    if (input == "ubo") {
        mode = ShaderInput::UniformBuffer;
    } else if (input == "texture") {
        mode = ShaderInput::Texture;
    } else {
        throw po::invalid_option_value("shader input");
    }
    return in;
}

std::ostream& operator <<(std::ostream& out, ShaderInput mode)
{
    if (mode == ShaderInput::UniformBuffer) {
        out << "ubo";
    } else if (mode == ShaderInput::Texture) {
        out << "texture";
    }
    return out;
}

std::istream& operator >>(std::istream& in, HitTest& mode)
{
    std::string input;
    in >> input;
    if (input == "bvh") {
        mode = HitTest::BVH;
    } else if (input == "brute-force") {
        mode = HitTest::BruteForce;
    } else {
        throw po::invalid_option_value("brute-force");
    }
    return in;
}

std::ostream& operator <<(std::ostream& out, HitTest mode)
{
    if (mode == HitTest::BruteForce) {
        out << "brute-force";
    } else if (mode == HitTest::BVH) {
        out << "bvh";
    }
    return out;
}

bool parseRenderConfig(int argc, char* argv[], RenderConfig& config)
{
    po::options_description desc;
    desc.add_options()
        ("help,h", "help message")
        ("render", po::value<RenderMode>(&config.renderMode)->default_value(RenderMode::FullScreenIncremental), "render mode")
        ("input", po::value<ShaderInput>(&config.shaderInput)->default_value(ShaderInput::UniformBuffer), "shader input source")
        ("hit-test", po::value<HitTest>(&config.hitTest)->default_value(HitTest::BVH), "hit test method")
        ("debug,d", po::bool_switch(&config.debugEnabled)->default_value(false), "debug bvh hit test")
        ("width", po::value<int>(&config.width)->default_value(800), "window width")
        ("height", po::value<int>(&config.height)->default_value(600), "window height")
    ;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error& e) {
        std::cerr << e.what() << "\n";
        return false;
    }
    
    if (vm.count("help") || vm.count("h")) {
        std::cerr << desc << "\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    RenderConfig config;
    if (!parseRenderConfig(argc, argv, config)) {
        return 1;
    }

    try {
        Application app;
        if (app.init(config)) {
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