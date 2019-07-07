#include "renderer.h"
#include "glutils.h"
#include "scene.h" 
#include "uborenderinput.h"
#include "texrenderinput.h"
#include "ssborenderinput.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

//#define BLOCK_REFINE
#define UBO_INPUT 0
#define TEX_INPUT 1
#define RENDER_INPUT 1

Renderer::Renderer(const RenderConfig& config)
    : m_prog()
    , m_quad()
    , m_renderInput()
    , m_windowOrigin(0)
    , m_windowSize(64, 64)
    , m_curIter(0)
    , m_iterNum(1)
    , m_numSamples(100)
    , m_timeQuery(0)
    , m_queryEnded(false)
{
    init(config);
}

Renderer::~Renderer()
{
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_colorTex);
    glDeleteQueries(1, &m_timeQuery);
}

void Renderer::init(const RenderConfig& config)
{
    m_width = config.width;
    m_height = config.height;

    m_prog = std::make_unique<GLSLProgram>();
    m_quad = std::make_unique<FullScreenQuad>();

    m_renderMode = config.renderMode;
    if (config.renderMode == RenderMode::Blocked) {
        m_prog->define("BLOCK_REFINE");
    }
    if (config.debugEnabled) {
        m_prog->define("DEBUG_BVH_HITS");
    }
    if (config.hitTest == HitTest::BruteForce) {
        m_prog->define("BRUTE_FORCE_HIT_TEST");
    }
    if (config.shaderInput == ShaderInput::UniformBuffer) {
        m_prog->define("UBO_INPUT");
    } else if (config.shaderInput == ShaderInput::ShaderStorageBuffer) {
        m_prog->overrideVersion(430);
        m_prog->define("SSBO_INPUT");
    } else {
        m_prog->define("TEXTURE_INPUT");
    }

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
    m_prog->setUniform("ScreenSize", glm::vec2(config.width, config.height));
    m_prog->setUniform("NumSamples", m_numSamples);
    GL_CHECK_ERROR;

    auto scene = createScene();
    if (config.shaderInput == ShaderInput::UniformBuffer) {
        m_renderInput = std::make_unique<UboRenderInput>(scene);
    } else if (config.shaderInput == ShaderInput::Texture) {
        m_renderInput = std::make_unique<TextureRenderInput>(scene);
    } else {
        m_renderInput = std::make_unique<SsboRenderInput>(scene);
    }
    m_renderInput->setInput(*m_prog);
    m_prog->setUniform("NumSpheres", (int)scene.objects.size());

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    if (config.renderMode == RenderMode::Blocked) {
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, m_width, m_height);
    } else {
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16, m_width, m_height);
    }
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTex, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    GL_CHECK_FRAMEBUFFER_STATUS;
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (config.renderMode == RenderMode::FullScreenIncremental) {
        m_prog->setUniform("IterNum", m_iterNum);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
    }

    glGenQueries(1, &m_timeQuery);
    glBeginQuery(GL_TIME_ELAPSED, m_timeQuery);
}

void Renderer::render()
{
    if (m_renderMode == RenderMode::Blocked) {
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

            if (m_windowOrigin.y >= m_height) {
                glEndQuery(GL_TIME_ELAPSED);
                m_queryEnded = true;
            }
        }
    } else {
        if (m_curIter < m_numSamples) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            m_prog->setUniform("IterStart", m_curIter);
            m_curIter += m_iterNum;
            m_quad->render(*m_prog);

            if (m_curIter >= m_numSamples) {
                glEndQuery(GL_TIME_ELAPSED);
                m_queryEnded = true;
            }
        }
    }
    checkQueryEnd();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Renderer::checkQueryEnd()
{
    if (m_queryEnded) {
        GLint ready;
        glGetQueryObjectiv(m_timeQuery, GL_QUERY_RESULT_AVAILABLE, &ready);
        if (ready) {
            GLint64 time;
            glGetQueryObjecti64v(m_timeQuery, GL_QUERY_RESULT, &time);
            std::cout << "Render Time: " << time / 1e9f << "s\n";
            glDeleteQueries(1, &m_timeQuery);
            m_timeQuery = 0;
            m_queryEnded = false;
        }
    }
}