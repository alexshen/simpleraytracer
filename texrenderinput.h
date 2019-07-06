#ifndef TEX_RENDER_INPUT_H
#define TEX_RENDER_INPUT_H

#pragma once

#include "renderinput.h"
#include <glad/glad.h>

struct Scene;
class bvh_node;

class TextureRenderInput : public RenderInput
{
public:
    TextureRenderInput(const Scene& scene);
    ~TextureRenderInput();

    void setInput(GLSLProgram& prog) const override;
private:
    GLuint m_nodeAABBTex;
    GLuint m_nodeDataTex;
    GLuint m_objectTex;
    GLuint m_materialTex;
};

#endif // TEX_RENDER_INPUT_H