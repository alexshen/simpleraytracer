#include "fullscreenquad.h"
#include "glslprogram.h"
#include <cstring>

FullScreenQuad::FullScreenQuad()
{
    GLfloat vertices[] = {
        -1, -1, 1, -1, -1, 1,
        1, -1, 1, 1, -1, 1
    };

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    auto buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    std::memcpy(buffer, vertices, sizeof(vertices));
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

FullScreenQuad::~FullScreenQuad()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void FullScreenQuad::render(GLSLProgram& prog)
{
    prog.setUniform("MVP", glm::mat4(1.0f));

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}