#ifndef UBO_RENDER_INPUT_H
#define UBO_RENDER_INPUT_H

#pragma once

#include "renderinput.h"
#include <glad/glad.h>

struct Scene;

class UboRenderInput : public RenderInput
{
public:
    UboRenderInput(const Scene& scene);
    ~UboRenderInput();

    void setInput(GLSLProgram& prog) const override;
private:
    GLuint m_ubo;
};

#endif // UBO_RENDER_INPUT_H