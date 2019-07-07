#include "uborenderinput.h"

#include "glslprogram.h"
#include "bvh_node.h"
#include "scene.h"
#include "sphere_object.h"

#include <glm/glm.hpp>
#include <cassert>
#include <stdexcept>

namespace
{

constexpr int MaxNodes = 640;
constexpr int MaxObjects = 512;

} // anonymouse namespace

UboRenderInput::UboRenderInput(const Scene& scene)
{
    SceneBuffer buffer(scene);

    if (buffer.nodes.size() > MaxNodes) {
        throw std::runtime_error("too many nodes");
    }
    if (buffer.objects.size() > MaxObjects) {
        throw std::runtime_error("too many objects");
    }

    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);

    constexpr int TotalSize = sizeof(Node) * MaxNodes + (sizeof(Material) + sizeof(Sphere)) * MaxObjects;
    glBufferData(GL_UNIFORM_BUFFER, TotalSize, nullptr, GL_STATIC_DRAW);
    auto p = static_cast<char*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));

    std::memcpy(p, buffer.nodes.data(), buffer.nodes.size() * sizeof(Node));
    p += sizeof(Node) * MaxNodes;
    std::memcpy(p, buffer.objects.data(), buffer.objects.size() * sizeof(Sphere));
    p += sizeof(Sphere) * MaxObjects;
    std::memcpy(p, buffer.materials.data(), buffer.materials.size() * sizeof(Material));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

UboRenderInput::~UboRenderInput()
{
    glDeleteBuffers(1, &m_ubo);
}

void UboRenderInput::setInput(GLSLProgram& prog) const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);

    auto index = glGetUniformBlockIndex(prog.getHandle(), "Scene");
    glUniformBlockBinding(prog.getHandle(), index, 0);
}