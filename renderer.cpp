#include "renderer.h"
#include "glutils.h"
#include "scene.h" 
#include "uborenderinput.h"
#include "texrenderinput.h"
#include "ssborenderinput.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>

//#define BLOCK_REFINE
#define UBO_INPUT 0
#define TEX_INPUT 1
#define RENDER_INPUT 1

namespace 
{

constexpr int KernelSize = 16;

}

Renderer::Renderer(const RenderConfig& config)
    : m_prog()
    , m_quad()
    , m_fbo(0)
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
    m_shaderType = config.shaderType;

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
    if (config.shaderType == ShaderType::FragmentShader) {
        m_prog->define("FRAGMENT_SHADER");
        m_prog->compileShader("shader/passthru.vs");
        m_prog->compileShader("shader/raytracing.fs");
    } else {
        m_prog->overrideVersion(430);
        m_prog->define("COMPUTE_SHADER");
        m_prog->define("KERNEL_SIZE " + std::to_string(KernelSize));
        m_prog->compileShader("shader/raytracing.fs", GLSLShader::COMPUTE);

        m_renderTexProg = std::make_unique<GLSLProgram>();
        m_renderTexProg->compileShader("shader/passthru.vs");
        m_renderTexProg->compileShader("shader/texcolor.fs");
        m_renderTexProg->link();
        m_renderTexProg->use();
        m_renderTexProg->setUniform("MainTex", 0);
    }

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

    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    GLenum texFormat;
    int size;
    if (config.renderMode == RenderMode::Blocked) {
        texFormat = GL_RGBA8;
        size = 4;
    } else {
        texFormat = GL_RGBA16;
        size = 8;
    }
    size *= m_width * m_height;
    std::vector<char> buf(size);
    glTexStorage2D(GL_TEXTURE_2D, 1, texFormat, m_width, m_height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

    if (config.shaderType == ShaderType::FragmentShader) {
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTex, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        GL_CHECK_FRAMEBUFFER_STATUS;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindImageTexture(0, m_colorTex, 0, GL_FALSE, 0, GL_READ_WRITE, texFormat);
    }

    m_prog->setUniform("IterNum", m_iterNum);
    if (config.renderMode == RenderMode::FullScreenIncremental &&
        config.shaderType == ShaderType::FragmentShader) {
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
            if (m_fbo) {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            }
            auto size = glm::vec2(m_width, m_height);
            auto rightTop = glm::min(m_windowOrigin + m_windowSize, size);
            m_prog->use();
            m_prog->setUniform("Window", glm::vec4(m_windowOrigin, rightTop));
            if (m_shaderType == ShaderType::FragmentShader) {
                auto model = glm::translate(glm::mat4(1.0f), glm::vec3((m_windowOrigin + rightTop) * 0.5f, 0));
                auto size = rightTop - m_windowOrigin;
                model = glm::scale(model, glm::vec3(size / 2.0f, 1));
                auto view = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height);

                m_prog->setUniform("MVP", view * model);
                m_quad->render();
            } else {
                glDispatchCompute((int)m_windowSize.x / KernelSize, (int)m_windowSize.y / KernelSize, 1);
            }

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
            if (m_fbo) { 
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
            }
            m_prog->use();
            m_prog->setUniform("IterStart", m_curIter);
            m_curIter += m_iterNum;
            if (m_shaderType == ShaderType::FragmentShader) {
                m_prog->setUniform("MVP", glm::mat4(1.0f));
                m_quad->render();
            } else {
                glDispatchCompute((m_width + KernelSize - 1) / KernelSize, 
                                  (m_height + KernelSize - 1) / KernelSize, 
                                  1);
            }

            if (m_curIter >= m_numSamples) {
                glEndQuery(GL_TIME_ELAPSED);
                m_queryEnded = true;
            }
        }
    }
    checkQueryEnd();

    if (m_shaderType == ShaderType::FragmentShader) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    } else {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
        m_renderTexProg->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorTex);
        m_renderTexProg->setUniform("MVP", glm::mat4(1.0f));
        m_quad->render();
    }
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

            if (m_onRenderComplete) {
                m_onRenderComplete();
            }
        }
    }
}

void Renderer::onRenderComplete(const renderComplete_t& cb)
{
    m_onRenderComplete = cb;
}