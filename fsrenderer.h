#ifndef FS_RENDERER_H
#define FS_RENDERER_H

#pragma once

#include "renderer.h"
#include "glslprogram.h"
#include "fullscreenquad.h"
#include <glm/glm.hpp>
#include <memory>

class FragmentShaderRenderer : public Renderer
{
public:
    FragmentShaderRenderer();
    ~FragmentShaderRenderer();

    void init(int width, int height);
    void render();
private:
    std::unique_ptr<GLSLProgram> m_prog;
    std::unique_ptr<FullScreenQuad> m_quad;
    glm::vec2 m_windowOrigin;
    glm::vec2 m_windowSize;
    int m_width;
    int m_height;
    GLuint m_ubo;
    GLuint m_fbo;
    GLuint m_colorTex;
    int m_curIter;
    int m_iterNum;
    int m_totalIterNum;
};

#endif // FS_RENDERER_H