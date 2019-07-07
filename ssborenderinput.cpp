#include "ssborenderinput.h"
#include "scene.h"
#include <cstring>
#include <memory>

namespace
{
    inline int roundUp(int size, int alignment)
    {
        return (size + alignment - 1) / alignment * alignment;
    }
}

SsboRenderInput::SsboRenderInput(const Scene& scene)
{
    SceneBuffer buffer(scene);

    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

    GLint align;
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &align);

    m_nodeBufferSize = sizeof(Node) * buffer.nodes.size();
    m_nodeBufferAlignedSize = roundUp(m_nodeBufferSize, align);

    m_objectBufferSize = sizeof(Sphere) * buffer.objects.size();
    m_objectBufferAlignedSize = roundUp(m_objectBufferSize, align);

    m_materialBufferSize = sizeof(Material) * buffer.objects.size();
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 m_nodeBufferAlignedSize + m_objectBufferAlignedSize + m_materialBufferSize, nullptr,
                 GL_STATIC_DRAW);

    auto p = static_cast<char*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY));
    std::memcpy(p, buffer.nodes.data(), m_nodeBufferSize);
    p += m_nodeBufferAlignedSize;
    std::memcpy(p, buffer.objects.data(), m_objectBufferSize);
    p += m_objectBufferAlignedSize;
    std::memcpy(p, buffer.materials.data(), m_materialBufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

SsboRenderInput::~SsboRenderInput()
{
    glDeleteBuffers(1, &m_ssbo);
}

void SsboRenderInput::setInput(GLSLProgram& prog) const
{
    int offset = 0;
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo, offset, m_nodeBufferSize);
    offset += m_nodeBufferAlignedSize;

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, m_ssbo, offset, m_objectBufferSize);
    offset += m_objectBufferAlignedSize;

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, m_ssbo, offset, m_materialBufferSize);
}