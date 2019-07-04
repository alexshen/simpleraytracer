#include "fsrenderer.h"
#include "glutils.h"
#include "scene.h" 
#include <cstring>
#include <cstdlib>

//#define BLOCK_REFINE

FragmentShaderRenderer::FragmentShaderRenderer()
    : m_ubo(0)
    , m_fbo(0)
    , m_colorTex(0)
    , m_curIter(0)
    , m_iterNum(1)
    , m_totalIterNum(100)
    , m_windowOrigin(0)
    , m_windowSize(64, 64)
{
}

FragmentShaderRenderer::~FragmentShaderRenderer()
{
    glDeleteBuffers(1, &m_ubo);
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_colorTex);
}

void FragmentShaderRenderer::init(int width, int height)
{
    m_width = width;
    m_height = height;

    m_prog = std::make_unique<GLSLProgram>();
    m_quad = std::make_unique<FullScreenQuad>();

#ifdef BLOCK_REFINE
    m_prog->define("BLOCK_REFINE");
#endif
    m_prog->compileShader("shader/passthru.vs");
    m_prog->compileShader("shader/raytracing.fs");
    m_prog->link();
    m_prog->use();
    GL_CHECK_ERROR;

    m_prog->setUniform("BackgroundColor", glm::vec3(0.5f, 0.7f, 1.0f));
    m_prog->setUniform("CameraPos", glm::vec3(13, 2, 3));
    m_prog->setUniform("CameraLookAt", glm::vec3(0));
    m_prog->setUniform("FocalLength", 1.0f);
    m_prog->setUniform("FovY", glm::radians(60.0f));
    m_prog->setUniform("ScreenSize", glm::vec2(width, height));
    m_prog->setUniform("NumSamples", 100);
    GL_CHECK_ERROR;

    auto sceneUbo = createSceneUniformBuffer();
    m_ubo = sceneUbo.handle;

    auto index = glGetUniformBlockIndex(m_prog->getHandle(), "Scene");
    glUniformBlockBinding(m_prog->getHandle(), index, 0);
    m_prog->setUniform("NumSpheres", sceneUbo.numSpheres);  

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
#ifdef BLOCK_REFINE
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_width, m_height);
#else
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16, m_width, m_height);
#endif
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GL_CHECK_FRAMEBUFFER_STATUS;
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifndef BLOCK_REFINE
    m_prog->setUniform("IterTotal", m_totalIterNum);
    m_prog->setUniform("IterNum", m_iterNum);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
#endif
}

void FragmentShaderRenderer::render()
{
#ifdef BLOCK_REFINE
    if (m_windowOrigin.y < m_height) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        auto size = glm::vec2(m_width, m_height);
        auto rightTop = glm::min(m_windowOrigin + m_windowSize, size);
        m_prog->setUniform("Window", glm::vec4(m_windowOrigin, rightTop));
        m_quad->render(*m_prog);

        m_windowOrigin.x += m_windowSize.x;
        if (m_windowOrigin.x >= m_width) {
            m_windowOrigin.x = 0;
            m_windowOrigin.y += m_windowSize.y;
        }
    }
#else
    if (m_curIter < m_totalIterNum) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        m_prog->setUniform("IterStart", m_curIter);
        m_curIter += m_iterNum;
        m_quad->render(*m_prog);
    }
#endif
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}