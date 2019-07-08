#include "fullscreenquad.h"
#include <cstring>

FullScreenQuad::FullScreenQuad()
{
    GLfloat vertices[] = {
        -1, -1, 1, -1, -1, 1,
        1, -1, 1, 1, -1, 1
    };

    GLfloat texcoords[] = {
        0, 0, 1, 0, 0, 1,
        1, 0, 1, 1, 0, 1
    };

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &m_vboTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboTexCoord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

FullScreenQuad::~FullScreenQuad()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vboVertices);
    glDeleteBuffers(1, &m_vboTexCoord);
}

void FullScreenQuad::render()
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}