#include "texrenderinput.h"
#include "scene.h"
#include "glslprogram.h"
#include "bvh_node.h"

#include <glm/glm.hpp>

#include <iterator>
#include <cassert>
#include <vector>
#include <stdexcept>

TextureRenderInput::TextureRenderInput(const Scene& scene)
{
    SceneBuffer buffer(scene);

    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if ((int)buffer.nodes.size() * 2 > maxSize) {
        throw std::runtime_error("too many nodes");
    }
    if ((int)buffer.materials.size() * 2 > maxSize) {
        throw std::runtime_error("too many objects");
    }

    std::vector<glm::vec4> aabbData(buffer.nodes.size() * 2);
    std::vector<glm::ivec4> nodeData(buffer.nodes.size());
    for (auto i = 0u; i < buffer.nodes.size(); ++i) {
        const auto& node = buffer.nodes[i];
        aabbData[i * 2] = { node.min, 0 };
        aabbData[i * 2 + 1] = { node.max, 0 };
        nodeData[i] = { node.left, node.right, node.firstObjIndex, node.numObj };
    }

    glGenTextures(1, &m_nodeAABBTex);
    glBindTexture(GL_TEXTURE_1D, m_nodeAABBTex);
    // aabb min, max, vec4
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, aabbData.size());
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, aabbData.size(), GL_RGBA, GL_FLOAT, aabbData.data());
    
    glGenTextures(1, &m_nodeDataTex);
    glBindTexture(GL_TEXTURE_1D, m_nodeDataTex);
    // left, right, first, num
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32I, nodeData.size());
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, nodeData.size(), GL_RGBA_INTEGER, GL_INT, nodeData.data());

    glGenTextures(1, &m_objectTex);
    glBindTexture(GL_TEXTURE_1D, m_objectTex);
    // center and radius
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, buffer.objects.size());
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, buffer.objects.size(), GL_RGBA, GL_FLOAT, buffer.objects.data());

    glGenTextures(1, &m_materialTex);
    glBindTexture(GL_TEXTURE_1D, m_materialTex);
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA32F, buffer.materials.size() * 2);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, buffer.materials.size() * 2, GL_RGBA, GL_FLOAT, buffer.materials.data());
}

TextureRenderInput::~TextureRenderInput()
{
    GLuint texs[4] = {
        m_nodeAABBTex,
        m_nodeDataTex,
        m_objectTex,
        m_materialTex
    };
    glDeleteTextures(std::size(texs), texs);
}

void TextureRenderInput::setInput(GLSLProgram& prog) const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, m_nodeAABBTex);
    prog.setUniform("NodeAABBTex", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, m_nodeDataTex);
    prog.setUniform("NodeDataTex", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, m_objectTex);
    prog.setUniform("ObjectTex", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_1D, m_materialTex);
    prog.setUniform("MaterialTex", 3);
}