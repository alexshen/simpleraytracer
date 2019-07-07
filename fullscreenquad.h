#ifndef FULL_SCREEN_QUAD_H
#define FULL_SCREEN_QUAD_H

#pragma once

#include <glad/glad.h>

class GLSLProgram;

class FullScreenQuad
{
public:
    FullScreenQuad();
    ~FullScreenQuad();

    FullScreenQuad(FullScreenQuad&) = delete;

    void render(GLSLProgram& prog);
private:
    GLuint m_vao;
    GLuint m_vboVertices;
    GLuint m_vboTexCoord;
};

#endif // FULL_SCREEN_QUAD_H