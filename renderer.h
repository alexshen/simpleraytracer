#ifndef FS_RENDERER_H
#define FS_RENDERER_H

#pragma once

#include "renderer.h"
#include "glslprogram.h"
#include "fullscreenquad.h"

#include <glm/glm.hpp>
#include <memory>

class RenderInput;

enum RenderMode
{
    Blocked,
    FullScreenIncremental
};

enum ShaderInput
{
    UniformBuffer,
    Texture,
    ShaderStorageBuffer
};

enum HitTest
{
    BruteForce,
    BVH
};

enum ShaderType
{
    FragmentShader,
    ComputeShader
};

struct RenderConfig
{
    int width;
    int height;
    ShaderType shaderType;
    RenderMode renderMode;
    ShaderInput shaderInput;
    HitTest hitTest;
    bool debugEnabled;
};

class Renderer
{
public:
    Renderer(const RenderConfig& config);
    ~Renderer();

    void render();
private:
    void init(const RenderConfig& config);
    void checkQueryEnd();

    std::unique_ptr<GLSLProgram> m_prog;
    std::unique_ptr<GLSLProgram> m_renderTexProg;
    std::unique_ptr<FullScreenQuad> m_quad;
    std::unique_ptr<RenderInput> m_renderInput;

    glm::vec2 m_windowOrigin;
    glm::vec2 m_windowSize;

    int m_width;
    int m_height;
    RenderMode m_renderMode;
    ShaderType m_shaderType;

    GLuint m_fbo;
    GLuint m_colorTex;

    int m_curIter;
    int m_iterNum;
    int m_numSamples;

    GLuint m_timeQuery;
    bool m_queryEnded;
};

#endif // FS_RENDERER_H