#ifndef FS_RENDERER_H
#define FS_RENDERER_H

#pragma once

#include "renderer.h"
#include "glslprogram.h"
#include "fullscreenquad.h"

#include <glm/glm.hpp>
#include <memory>

class RenderInput;

class FragmentShaderRenderer : public Renderer
{
public:
    FragmentShaderRenderer();
    ~FragmentShaderRenderer();

    void init(int width, int height);
    void render();
private:
    void checkQueryEnd();

    std::unique_ptr<GLSLProgram> m_prog;
    std::unique_ptr<FullScreenQuad> m_quad;
    std::unique_ptr<RenderInput> m_renderInput;

    glm::vec2 m_windowOrigin;
    glm::vec2 m_windowSize;

    int m_width;
    int m_height;

    GLuint m_fbo;
    GLuint m_colorTex;

    int m_curIter;
    int m_iterNum;
    int m_numSamples;

    GLuint m_timeQuery;
    bool m_queryEnded;
};

#endif // FS_RENDERER_H