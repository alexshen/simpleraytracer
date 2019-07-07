#ifndef SSBO_RENDER_INPUT_H
#define SSBO_RENDER_INPUT_H

#pragma once

#include "renderinput.h"
#include <glad/glad.h>

struct Scene;
class GLSLProgram;

class SsboRenderInput : public RenderInput
{
public:
    SsboRenderInput(const Scene& scene);
    ~SsboRenderInput();

    void setInput(GLSLProgram& prog) const override;
private:
    GLuint m_ssbo;

    int m_nodeBufferSize;
    int m_nodeBufferAlignedSize;
    int m_objectBufferSize;
    int m_objectBufferAlignedSize;
    int m_materialBufferSize;
};

#endif // SSBO_RENDER_INPUT_H 